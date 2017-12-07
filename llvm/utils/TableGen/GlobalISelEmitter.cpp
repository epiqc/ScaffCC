//===- GlobalISelEmitter.cpp - Generate an instruction selector -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This tablegen backend emits code for use by the GlobalISel instruction
/// selector. See include/llvm/CodeGen/TargetGlobalISel.td.
///
/// This file analyzes the patterns recognized by the SelectionDAGISel tablegen
/// backend, filters out the ones that are unsupported, maps
/// SelectionDAG-specific constructs to their GlobalISel counterpart
/// (when applicable: MVT to LLT;  SDNode to generic Instruction).
///
/// Not all patterns are supported: pass the tablegen invocation
/// "-warn-on-skipped-patterns" to emit a warning when a pattern is skipped,
/// as well as why.
///
/// The generated file defines a single method:
///     bool <Target>InstructionSelector::selectImpl(MachineInstr &I) const;
/// intended to be used in InstructionSelector::select as the first-step
/// selector for the patterns that don't require complex C++.
///
/// FIXME: We'll probably want to eventually define a base
/// "TargetGenInstructionSelector" class.
///
//===----------------------------------------------------------------------===//

#include "CodeGenDAGPatterns.h"
#include "SubtargetFeatureInfo.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineValueType.h"
#include "llvm/Support/CodeGenCoverage.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/LowLevelTypeImpl.h"
#include "llvm/Support/ScopedPrinter.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/TableGenBackend.h"
#include <numeric>
#include <string>
using namespace llvm;

#define DEBUG_TYPE "gisel-emitter"

STATISTIC(NumPatternTotal, "Total number of patterns");
STATISTIC(NumPatternImported, "Number of patterns imported from SelectionDAG");
STATISTIC(NumPatternImportsSkipped, "Number of SelectionDAG imports skipped");
STATISTIC(NumPatternsTested, "Number of patterns executed according to coverage information");
STATISTIC(NumPatternEmitted, "Number of patterns emitted");

cl::OptionCategory GlobalISelEmitterCat("Options for -gen-global-isel");

static cl::opt<bool> WarnOnSkippedPatterns(
    "warn-on-skipped-patterns",
    cl::desc("Explain why a pattern was skipped for inclusion "
             "in the GlobalISel selector"),
    cl::init(false), cl::cat(GlobalISelEmitterCat));

static cl::opt<bool> GenerateCoverage(
    "instrument-gisel-coverage",
    cl::desc("Generate coverage instrumentation for GlobalISel"),
    cl::init(false), cl::cat(GlobalISelEmitterCat));

static cl::opt<std::string> UseCoverageFile(
    "gisel-coverage-file", cl::init(""),
    cl::desc("Specify file to retrieve coverage information from"),
    cl::cat(GlobalISelEmitterCat));

namespace {
//===- Helper functions ---------------------------------------------------===//


/// Get the name of the enum value used to number the predicate function.
std::string getEnumNameForPredicate(const TreePredicateFn &Predicate) {
  return "GIPFP_" + Predicate.getImmTypeIdentifier().str() + "_" +
         Predicate.getFnName();
}

/// Get the opcode used to check this predicate.
std::string getMatchOpcodeForPredicate(const TreePredicateFn &Predicate) {
  return "GIM_Check" + Predicate.getImmTypeIdentifier().str() + "ImmPredicate";
}

/// This class stands in for LLT wherever we want to tablegen-erate an
/// equivalent at compiler run-time.
class LLTCodeGen {
private:
  LLT Ty;

public:
  LLTCodeGen(const LLT &Ty) : Ty(Ty) {}

  std::string getCxxEnumValue() const {
    std::string Str;
    raw_string_ostream OS(Str);

    emitCxxEnumValue(OS);
    return OS.str();
  }

  void emitCxxEnumValue(raw_ostream &OS) const {
    if (Ty.isScalar()) {
      OS << "GILLT_s" << Ty.getSizeInBits();
      return;
    }
    if (Ty.isVector()) {
      OS << "GILLT_v" << Ty.getNumElements() << "s" << Ty.getScalarSizeInBits();
      return;
    }
    if (Ty.isPointer()) {
      OS << "GILLT_p" << Ty.getAddressSpace();
      if (Ty.getSizeInBits() > 0)
        OS << "s" << Ty.getSizeInBits();
      return;
    }
    llvm_unreachable("Unhandled LLT");
  }

  void emitCxxConstructorCall(raw_ostream &OS) const {
    if (Ty.isScalar()) {
      OS << "LLT::scalar(" << Ty.getSizeInBits() << ")";
      return;
    }
    if (Ty.isVector()) {
      OS << "LLT::vector(" << Ty.getNumElements() << ", "
         << Ty.getScalarSizeInBits() << ")";
      return;
    }
    if (Ty.isPointer() && Ty.getSizeInBits() > 0) {
      OS << "LLT::pointer(" << Ty.getAddressSpace() << ", "
         << Ty.getSizeInBits() << ")";
      return;
    }
    llvm_unreachable("Unhandled LLT");
  }

  const LLT &get() const { return Ty; }

  /// This ordering is used for std::unique() and std::sort(). There's no
  /// particular logic behind the order but either A < B or B < A must be
  /// true if A != B.
  bool operator<(const LLTCodeGen &Other) const {
    if (Ty.isValid() != Other.Ty.isValid())
      return Ty.isValid() < Other.Ty.isValid();
    if (!Ty.isValid())
      return false;

    if (Ty.isVector() != Other.Ty.isVector())
      return Ty.isVector() < Other.Ty.isVector();
    if (Ty.isScalar() != Other.Ty.isScalar())
      return Ty.isScalar() < Other.Ty.isScalar();
    if (Ty.isPointer() != Other.Ty.isPointer())
      return Ty.isPointer() < Other.Ty.isPointer();

    if (Ty.isPointer() && Ty.getAddressSpace() != Other.Ty.getAddressSpace())
      return Ty.getAddressSpace() < Other.Ty.getAddressSpace();

    if (Ty.isVector() && Ty.getNumElements() != Other.Ty.getNumElements())
      return Ty.getNumElements() < Other.Ty.getNumElements();

    return Ty.getSizeInBits() < Other.Ty.getSizeInBits();
  }
};

class InstructionMatcher;
/// Convert an MVT to an equivalent LLT if possible, or the invalid LLT() for
/// MVTs that don't map cleanly to an LLT (e.g., iPTR, *any, ...).
static Optional<LLTCodeGen> MVTToLLT(MVT::SimpleValueType SVT) {
  MVT VT(SVT);

  if (VT.isVector() && VT.getVectorNumElements() != 1)
    return LLTCodeGen(
        LLT::vector(VT.getVectorNumElements(), VT.getScalarSizeInBits()));

  if (VT.isInteger() || VT.isFloatingPoint())
    return LLTCodeGen(LLT::scalar(VT.getSizeInBits()));
  return None;
}

static std::string explainPredicates(const TreePatternNode *N) {
  std::string Explanation = "";
  StringRef Separator = "";
  for (const auto &P : N->getPredicateFns()) {
    Explanation +=
        (Separator + P.getOrigPatFragRecord()->getRecord()->getName()).str();
    Separator = ", ";

    if (P.isAlwaysTrue())
      Explanation += " always-true";
    if (P.isImmediatePattern())
      Explanation += " immediate";

    if (P.isUnindexed())
      Explanation += " unindexed";

    if (P.isNonExtLoad())
      Explanation += " non-extload";
    if (P.isAnyExtLoad())
      Explanation += " extload";
    if (P.isSignExtLoad())
      Explanation += " sextload";
    if (P.isZeroExtLoad())
      Explanation += " zextload";

    if (P.isNonTruncStore())
      Explanation += " non-truncstore";
    if (P.isTruncStore())
      Explanation += " truncstore";

    if (Record *VT = P.getMemoryVT())
      Explanation += (" MemVT=" + VT->getName()).str();
    if (Record *VT = P.getScalarMemoryVT())
      Explanation += (" ScalarVT(MemVT)=" + VT->getName()).str();

    if (P.isAtomicOrderingMonotonic())
      Explanation += " monotonic";
    if (P.isAtomicOrderingAcquire())
      Explanation += " acquire";
    if (P.isAtomicOrderingRelease())
      Explanation += " release";
    if (P.isAtomicOrderingAcquireRelease())
      Explanation += " acq_rel";
    if (P.isAtomicOrderingSequentiallyConsistent())
      Explanation += " seq_cst";
    if (P.isAtomicOrderingAcquireOrStronger())
      Explanation += " >=acquire";
    if (P.isAtomicOrderingWeakerThanAcquire())
      Explanation += " <acquire";
    if (P.isAtomicOrderingReleaseOrStronger())
      Explanation += " >=release";
    if (P.isAtomicOrderingWeakerThanRelease())
      Explanation += " <release";
  }
  return Explanation;
}

std::string explainOperator(Record *Operator) {
  if (Operator->isSubClassOf("SDNode"))
    return (" (" + Operator->getValueAsString("Opcode") + ")").str();

  if (Operator->isSubClassOf("Intrinsic"))
    return (" (Operator is an Intrinsic, " + Operator->getName() + ")").str();

  if (Operator->isSubClassOf("ComplexPattern"))
    return (" (Operator is an unmapped ComplexPattern, " + Operator->getName() +
            ")")
        .str();

  return (" (Operator " + Operator->getName() + " not understood)").str();
}

/// Helper function to let the emitter report skip reason error messages.
static Error failedImport(const Twine &Reason) {
  return make_error<StringError>(Reason, inconvertibleErrorCode());
}

static Error isTrivialOperatorNode(const TreePatternNode *N) {
  std::string Explanation = "";
  std::string Separator = "";

  bool HasUnsupportedPredicate = false;
  for (const auto &Predicate : N->getPredicateFns()) {
    if (Predicate.isAlwaysTrue())
      continue;

    if (Predicate.isImmediatePattern())
      continue;

    if (Predicate.isNonExtLoad())
      continue;

    if (Predicate.isNonTruncStore())
      continue;

    if (Predicate.isLoad() || Predicate.isStore()) {
      if (Predicate.isUnindexed())
        continue;
    }

    if (Predicate.isAtomic() && Predicate.getMemoryVT())
      continue;

    if (Predicate.isAtomic() &&
        (Predicate.isAtomicOrderingMonotonic() ||
         Predicate.isAtomicOrderingAcquire() ||
         Predicate.isAtomicOrderingRelease() ||
         Predicate.isAtomicOrderingAcquireRelease() ||
         Predicate.isAtomicOrderingSequentiallyConsistent() ||
         Predicate.isAtomicOrderingAcquireOrStronger() ||
         Predicate.isAtomicOrderingWeakerThanAcquire() ||
         Predicate.isAtomicOrderingReleaseOrStronger() ||
         Predicate.isAtomicOrderingWeakerThanRelease()))
      continue;

    HasUnsupportedPredicate = true;
    Explanation = Separator + "Has a predicate (" + explainPredicates(N) + ")";
    Separator = ", ";
    Explanation += (Separator + "first-failing:" +
                    Predicate.getOrigPatFragRecord()->getRecord()->getName())
                       .str();
    break;
  }

  if (N->getTransformFn()) {
    Explanation += Separator + "Has a transform function";
    Separator = ", ";
  }

  if (!HasUnsupportedPredicate && !N->getTransformFn())
    return Error::success();

  return failedImport(Explanation);
}

static Record *getInitValueAsRegClass(Init *V) {
  if (DefInit *VDefInit = dyn_cast<DefInit>(V)) {
    if (VDefInit->getDef()->isSubClassOf("RegisterOperand"))
      return VDefInit->getDef()->getValueAsDef("RegClass");
    if (VDefInit->getDef()->isSubClassOf("RegisterClass"))
      return VDefInit->getDef();
  }
  return nullptr;
}

std::string
getNameForFeatureBitset(const std::vector<Record *> &FeatureBitset) {
  std::string Name = "GIFBS";
  for (const auto &Feature : FeatureBitset)
    Name += ("_" + Feature->getName()).str();
  return Name;
}

//===- MatchTable Helpers -------------------------------------------------===//

class MatchTable;

/// A record to be stored in a MatchTable.
///
/// This class represents any and all output that may be required to emit the
/// MatchTable. Instances  are most often configured to represent an opcode or
/// value that will be emitted to the table with some formatting but it can also
/// represent commas, comments, and other formatting instructions.
struct MatchTableRecord {
  enum RecordFlagsBits {
    MTRF_None = 0x0,
    /// Causes EmitStr to be formatted as comment when emitted.
    MTRF_Comment = 0x1,
    /// Causes the record value to be followed by a comma when emitted.
    MTRF_CommaFollows = 0x2,
    /// Causes the record value to be followed by a line break when emitted.
    MTRF_LineBreakFollows = 0x4,
    /// Indicates that the record defines a label and causes an additional
    /// comment to be emitted containing the index of the label.
    MTRF_Label = 0x8,
    /// Causes the record to be emitted as the index of the label specified by
    /// LabelID along with a comment indicating where that label is.
    MTRF_JumpTarget = 0x10,
    /// Causes the formatter to add a level of indentation before emitting the
    /// record.
    MTRF_Indent = 0x20,
    /// Causes the formatter to remove a level of indentation after emitting the
    /// record.
    MTRF_Outdent = 0x40,
  };

  /// When MTRF_Label or MTRF_JumpTarget is used, indicates a label id to
  /// reference or define.
  unsigned LabelID;
  /// The string to emit. Depending on the MTRF_* flags it may be a comment, a
  /// value, a label name.
  std::string EmitStr;

private:
  /// The number of MatchTable elements described by this record. Comments are 0
  /// while values are typically 1. Values >1 may occur when we need to emit
  /// values that exceed the size of a MatchTable element.
  unsigned NumElements;

public:
  /// A bitfield of RecordFlagsBits flags.
  unsigned Flags;

  MatchTableRecord(Optional<unsigned> LabelID_, StringRef EmitStr,
                   unsigned NumElements, unsigned Flags)
      : LabelID(LabelID_.hasValue() ? LabelID_.getValue() : ~0u),
        EmitStr(EmitStr), NumElements(NumElements), Flags(Flags) {
    assert((!LabelID_.hasValue() || LabelID != ~0u) &&
           "This value is reserved for non-labels");
  }

  void emit(raw_ostream &OS, bool LineBreakNextAfterThis,
            const MatchTable &Table) const;
  unsigned size() const { return NumElements; }
};

/// Holds the contents of a generated MatchTable to enable formatting and the
/// necessary index tracking needed to support GIM_Try.
class MatchTable {
  /// An unique identifier for the table. The generated table will be named
  /// MatchTable${ID}.
  unsigned ID;
  /// The records that make up the table. Also includes comments describing the
  /// values being emitted and line breaks to format it.
  std::vector<MatchTableRecord> Contents;
  /// The currently defined labels.
  DenseMap<unsigned, unsigned> LabelMap;
  /// Tracks the sum of MatchTableRecord::NumElements as the table is built.
  unsigned CurrentSize;

  /// A unique identifier for a MatchTable label.
  static unsigned CurrentLabelID;

public:
  static MatchTableRecord LineBreak;
  static MatchTableRecord Comment(StringRef Comment) {
    return MatchTableRecord(None, Comment, 0, MatchTableRecord::MTRF_Comment);
  }
  static MatchTableRecord Opcode(StringRef Opcode, int IndentAdjust = 0) {
    unsigned ExtraFlags = 0;
    if (IndentAdjust > 0)
      ExtraFlags |= MatchTableRecord::MTRF_Indent;
    if (IndentAdjust < 0)
      ExtraFlags |= MatchTableRecord::MTRF_Outdent;

    return MatchTableRecord(None, Opcode, 1,
                            MatchTableRecord::MTRF_CommaFollows | ExtraFlags);
  }
  static MatchTableRecord NamedValue(StringRef NamedValue) {
    return MatchTableRecord(None, NamedValue, 1,
                            MatchTableRecord::MTRF_CommaFollows);
  }
  static MatchTableRecord NamedValue(StringRef Namespace,
                                     StringRef NamedValue) {
    return MatchTableRecord(None, (Namespace + "::" + NamedValue).str(), 1,
                            MatchTableRecord::MTRF_CommaFollows);
  }
  static MatchTableRecord IntValue(int64_t IntValue) {
    return MatchTableRecord(None, llvm::to_string(IntValue), 1,
                            MatchTableRecord::MTRF_CommaFollows);
  }
  static MatchTableRecord Label(unsigned LabelID) {
    return MatchTableRecord(LabelID, "Label " + llvm::to_string(LabelID), 0,
                            MatchTableRecord::MTRF_Label |
                                MatchTableRecord::MTRF_Comment |
                                MatchTableRecord::MTRF_LineBreakFollows);
  }
  static MatchTableRecord JumpTarget(unsigned LabelID) {
    return MatchTableRecord(LabelID, "Label " + llvm::to_string(LabelID), 1,
                            MatchTableRecord::MTRF_JumpTarget |
                                MatchTableRecord::MTRF_Comment |
                                MatchTableRecord::MTRF_CommaFollows);
  }

  MatchTable(unsigned ID) : ID(ID), CurrentSize(0) {}

  void push_back(const MatchTableRecord &Value) {
    if (Value.Flags & MatchTableRecord::MTRF_Label)
      defineLabel(Value.LabelID);
    Contents.push_back(Value);
    CurrentSize += Value.size();
  }

  unsigned allocateLabelID() const { return CurrentLabelID++; }

  void defineLabel(unsigned LabelID) {
    LabelMap.insert(std::make_pair(LabelID, CurrentSize));
  }

  unsigned getLabelIndex(unsigned LabelID) const {
    const auto I = LabelMap.find(LabelID);
    assert(I != LabelMap.end() && "Use of undeclared label");
    return I->second;
  }

  void emitUse(raw_ostream &OS) const { OS << "MatchTable" << ID; }

  void emitDeclaration(raw_ostream &OS) const {
    unsigned Indentation = 4;
    OS << "  constexpr static int64_t MatchTable" << ID << "[] = {";
    LineBreak.emit(OS, true, *this);
    OS << std::string(Indentation, ' ');

    for (auto I = Contents.begin(), E = Contents.end(); I != E;
         ++I) {
      bool LineBreakIsNext = false;
      const auto &NextI = std::next(I);

      if (NextI != E) {
        if (NextI->EmitStr == "" &&
            NextI->Flags == MatchTableRecord::MTRF_LineBreakFollows)
          LineBreakIsNext = true;
      }

      if (I->Flags & MatchTableRecord::MTRF_Indent)
        Indentation += 2;

      I->emit(OS, LineBreakIsNext, *this);
      if (I->Flags & MatchTableRecord::MTRF_LineBreakFollows)
        OS << std::string(Indentation, ' ');

      if (I->Flags & MatchTableRecord::MTRF_Outdent)
        Indentation -= 2;
    }
    OS << "};\n";
  }
};

unsigned MatchTable::CurrentLabelID = 0;

MatchTableRecord MatchTable::LineBreak = {
    None, "" /* Emit String */, 0 /* Elements */,
    MatchTableRecord::MTRF_LineBreakFollows};

void MatchTableRecord::emit(raw_ostream &OS, bool LineBreakIsNextAfterThis,
                            const MatchTable &Table) const {
  bool UseLineComment =
      LineBreakIsNextAfterThis | (Flags & MTRF_LineBreakFollows);
  if (Flags & (MTRF_JumpTarget | MTRF_CommaFollows))
    UseLineComment = false;

  if (Flags & MTRF_Comment)
    OS << (UseLineComment ? "// " : "/*");

  OS << EmitStr;
  if (Flags & MTRF_Label)
    OS << ": @" << Table.getLabelIndex(LabelID);

  if (Flags & MTRF_Comment && !UseLineComment)
    OS << "*/";

  if (Flags & MTRF_JumpTarget) {
    if (Flags & MTRF_Comment)
      OS << " ";
    OS << Table.getLabelIndex(LabelID);
  }

  if (Flags & MTRF_CommaFollows) {
    OS << ",";
    if (!LineBreakIsNextAfterThis && !(Flags & MTRF_LineBreakFollows))
      OS << " ";
  }

  if (Flags & MTRF_LineBreakFollows)
    OS << "\n";
}

MatchTable &operator<<(MatchTable &Table, const MatchTableRecord &Value) {
  Table.push_back(Value);
  return Table;
}

//===- Matchers -----------------------------------------------------------===//

class OperandMatcher;
class MatchAction;

/// Generates code to check that a match rule matches.
class RuleMatcher {
public:
  using ActionVec = std::vector<std::unique_ptr<MatchAction>>;
  using action_iterator = ActionVec::iterator;

protected:
  /// A list of matchers that all need to succeed for the current rule to match.
  /// FIXME: This currently supports a single match position but could be
  /// extended to support multiple positions to support div/rem fusion or
  /// load-multiple instructions.
  std::vector<std::unique_ptr<InstructionMatcher>> Matchers;

  /// A list of actions that need to be taken when all predicates in this rule
  /// have succeeded.
  ActionVec Actions;

  using DefinedInsnVariablesMap =
      std::map<const InstructionMatcher *, unsigned>;

  /// A map of instruction matchers to the local variables created by
  /// emitCaptureOpcodes().
  DefinedInsnVariablesMap InsnVariableIDs;

  using MutatableInsnSet = SmallPtrSet<const InstructionMatcher *, 4>;

  // The set of instruction matchers that have not yet been claimed for mutation
  // by a BuildMI.
  MutatableInsnSet MutatableInsns;

  /// A map of named operands defined by the matchers that may be referenced by
  /// the renderers.
  StringMap<OperandMatcher *> DefinedOperands;

  /// ID for the next instruction variable defined with defineInsnVar()
  unsigned NextInsnVarID;

  /// ID for the next output instruction allocated with allocateOutputInsnID()
  unsigned NextOutputInsnID;

  /// ID for the next temporary register ID allocated with allocateTempRegID()
  unsigned NextTempRegID;

  std::vector<Record *> RequiredFeatures;

  ArrayRef<SMLoc> SrcLoc;

  typedef std::tuple<Record *, unsigned, unsigned>
      DefinedComplexPatternSubOperand;
  typedef StringMap<DefinedComplexPatternSubOperand>
      DefinedComplexPatternSubOperandMap;
  /// A map of Symbolic Names to ComplexPattern sub-operands.
  DefinedComplexPatternSubOperandMap ComplexSubOperands;

  uint64_t RuleID;
  static uint64_t NextRuleID;

public:
  RuleMatcher(ArrayRef<SMLoc> SrcLoc)
      : Matchers(), Actions(), InsnVariableIDs(), MutatableInsns(),
        DefinedOperands(), NextInsnVarID(0), NextOutputInsnID(0),
        NextTempRegID(0), SrcLoc(SrcLoc), ComplexSubOperands(),
        RuleID(NextRuleID++) {}
  RuleMatcher(RuleMatcher &&Other) = default;
  RuleMatcher &operator=(RuleMatcher &&Other) = default;

  uint64_t getRuleID() const { return RuleID; }

  InstructionMatcher &addInstructionMatcher(StringRef SymbolicName);
  void addRequiredFeature(Record *Feature);
  const std::vector<Record *> &getRequiredFeatures() const;

  template <class Kind, class... Args> Kind &addAction(Args &&... args);
  template <class Kind, class... Args>
  action_iterator insertAction(action_iterator InsertPt, Args &&... args);

  /// Define an instruction without emitting any code to do so.
  /// This is used for the root of the match.
  unsigned implicitlyDefineInsnVar(const InstructionMatcher &Matcher);
  /// Define an instruction and emit corresponding state-machine opcodes.
  unsigned defineInsnVar(MatchTable &Table, const InstructionMatcher &Matcher,
                         unsigned InsnVarID, unsigned OpIdx);
  unsigned getInsnVarID(const InstructionMatcher &InsnMatcher) const;
  DefinedInsnVariablesMap::const_iterator defined_insn_vars_begin() const {
    return InsnVariableIDs.begin();
  }
  DefinedInsnVariablesMap::const_iterator defined_insn_vars_end() const {
    return InsnVariableIDs.end();
  }
  iterator_range<typename DefinedInsnVariablesMap::const_iterator>
  defined_insn_vars() const {
    return make_range(defined_insn_vars_begin(), defined_insn_vars_end());
  }

  MutatableInsnSet::const_iterator mutatable_insns_begin() const {
    return MutatableInsns.begin();
  }
  MutatableInsnSet::const_iterator mutatable_insns_end() const {
    return MutatableInsns.end();
  }
  iterator_range<typename MutatableInsnSet::const_iterator>
  mutatable_insns() const {
    return make_range(mutatable_insns_begin(), mutatable_insns_end());
  }
  void reserveInsnMatcherForMutation(const InstructionMatcher *InsnMatcher) {
    bool R = MutatableInsns.erase(InsnMatcher);
    assert(R && "Reserving a mutatable insn that isn't available");
    (void)R;
  }

  action_iterator actions_begin() { return Actions.begin(); }
  action_iterator actions_end() { return Actions.end(); }
  iterator_range<action_iterator> actions() {
    return make_range(actions_begin(), actions_end());
  }

  void defineOperand(StringRef SymbolicName, OperandMatcher &OM);

  void defineComplexSubOperand(StringRef SymbolicName, Record *ComplexPattern,
                               unsigned RendererID, unsigned SubOperandID) {
    assert(ComplexSubOperands.count(SymbolicName) == 0 && "Already defined");
    ComplexSubOperands[SymbolicName] =
        std::make_tuple(ComplexPattern, RendererID, SubOperandID);
  }
  Optional<DefinedComplexPatternSubOperand>
  getComplexSubOperand(StringRef SymbolicName) const {
    const auto &I = ComplexSubOperands.find(SymbolicName);
    if (I == ComplexSubOperands.end())
      return None;
    return I->second;
  }

  const InstructionMatcher &getInstructionMatcher(StringRef SymbolicName) const;
  const OperandMatcher &getOperandMatcher(StringRef Name) const;

  void emitCaptureOpcodes(MatchTable &Table);

  void emit(MatchTable &Table);

  /// Compare the priority of this object and B.
  ///
  /// Returns true if this object is more important than B.
  bool isHigherPriorityThan(const RuleMatcher &B) const;

  /// Report the maximum number of temporary operands needed by the rule
  /// matcher.
  unsigned countRendererFns() const;

  // FIXME: Remove this as soon as possible
  InstructionMatcher &insnmatcher_front() const { return *Matchers.front(); }

  unsigned allocateOutputInsnID() { return NextOutputInsnID++; }
  unsigned allocateTempRegID() { return NextTempRegID++; }
};

uint64_t RuleMatcher::NextRuleID = 0;

using action_iterator = RuleMatcher::action_iterator;

template <class PredicateTy> class PredicateListMatcher {
private:
  typedef std::vector<std::unique_ptr<PredicateTy>> PredicateVec;
  PredicateVec Predicates;

  /// Template instantiations should specialize this to return a string to use
  /// for the comment emitted when there are no predicates.
  std::string getNoPredicateComment() const;

public:
  /// Construct a new operand predicate and add it to the matcher.
  template <class Kind, class... Args>
  Optional<Kind *> addPredicate(Args&&... args) {
    Predicates.emplace_back(
        llvm::make_unique<Kind>(std::forward<Args>(args)...));
    return static_cast<Kind *>(Predicates.back().get());
  }

  typename PredicateVec::const_iterator predicates_begin() const {
    return Predicates.begin();
  }
  typename PredicateVec::const_iterator predicates_end() const {
    return Predicates.end();
  }
  iterator_range<typename PredicateVec::const_iterator> predicates() const {
    return make_range(predicates_begin(), predicates_end());
  }
  typename PredicateVec::size_type predicates_size() const {
    return Predicates.size();
  }

  /// Emit MatchTable opcodes that tests whether all the predicates are met.
  template <class... Args>
  void emitPredicateListOpcodes(MatchTable &Table, Args &&... args) const {
    if (Predicates.empty()) {
      Table << MatchTable::Comment(getNoPredicateComment())
            << MatchTable::LineBreak;
      return;
    }

    for (const auto &Predicate : predicates())
      Predicate->emitPredicateOpcodes(Table, std::forward<Args>(args)...);
  }
};

/// Generates code to check a predicate of an operand.
///
/// Typical predicates include:
/// * Operand is a particular register.
/// * Operand is assigned a particular register bank.
/// * Operand is an MBB.
class OperandPredicateMatcher {
public:
  /// This enum is used for RTTI and also defines the priority that is given to
  /// the predicate when generating the matcher code. Kinds with higher priority
  /// must be tested first.
  ///
  /// The relative priority of OPM_LLT, OPM_RegBank, and OPM_MBB do not matter
  /// but OPM_Int must have priority over OPM_RegBank since constant integers
  /// are represented by a virtual register defined by a G_CONSTANT instruction.
  enum PredicateKind {
    OPM_SameOperand,
    OPM_ComplexPattern,
    OPM_IntrinsicID,
    OPM_Instruction,
    OPM_Int,
    OPM_LiteralInt,
    OPM_LLT,
    OPM_PointerToAny,
    OPM_RegBank,
    OPM_MBB,
  };

protected:
  PredicateKind Kind;

public:
  OperandPredicateMatcher(PredicateKind Kind) : Kind(Kind) {}
  virtual ~OperandPredicateMatcher() {}

  PredicateKind getKind() const { return Kind; }

  /// Emit MatchTable opcodes to capture instructions into the MIs table.
  ///
  /// Only InstructionOperandMatcher needs to do anything for this method the
  /// rest just walk the tree.
  virtual void emitCaptureOpcodes(MatchTable &Table, RuleMatcher &Rule,
                                  unsigned InsnVarID, unsigned OpIdx) const {}

  /// Emit MatchTable opcodes that check the predicate for the given operand.
  virtual void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                                    unsigned InsnVarID,
                                    unsigned OpIdx) const = 0;

  /// Compare the priority of this object and B.
  ///
  /// Returns true if this object is more important than B.
  virtual bool isHigherPriorityThan(const OperandPredicateMatcher &B) const;

  /// Report the maximum number of temporary operands needed by the predicate
  /// matcher.
  virtual unsigned countRendererFns() const { return 0; }
};

template <>
std::string
PredicateListMatcher<OperandPredicateMatcher>::getNoPredicateComment() const {
  return "No operand predicates";
}

/// Generates code to check that a register operand is defined by the same exact
/// one as another.
class SameOperandMatcher : public OperandPredicateMatcher {
  std::string MatchingName;

public:
  SameOperandMatcher(StringRef MatchingName)
      : OperandPredicateMatcher(OPM_SameOperand), MatchingName(MatchingName) {}

  static bool classof(const OperandPredicateMatcher *P) {
    return P->getKind() == OPM_SameOperand;
  }

  void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                            unsigned InsnVarID, unsigned OpIdx) const override;
};

/// Generates code to check that an operand is a particular LLT.
class LLTOperandMatcher : public OperandPredicateMatcher {
protected:
  LLTCodeGen Ty;

public:
  static std::set<LLTCodeGen> KnownTypes;

  LLTOperandMatcher(const LLTCodeGen &Ty)
      : OperandPredicateMatcher(OPM_LLT), Ty(Ty) {
    KnownTypes.insert(Ty);
  }

  static bool classof(const OperandPredicateMatcher *P) {
    return P->getKind() == OPM_LLT;
  }

  void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                            unsigned InsnVarID, unsigned OpIdx) const override {
    Table << MatchTable::Opcode("GIM_CheckType") << MatchTable::Comment("MI")
          << MatchTable::IntValue(InsnVarID) << MatchTable::Comment("Op")
          << MatchTable::IntValue(OpIdx) << MatchTable::Comment("Type")
          << MatchTable::NamedValue(Ty.getCxxEnumValue())
          << MatchTable::LineBreak;
  }
};

std::set<LLTCodeGen> LLTOperandMatcher::KnownTypes;

/// Generates code to check that an operand is a pointer to any address space.
///
/// In SelectionDAG, the types did not describe pointers or address spaces. As a
/// result, iN is used to describe a pointer of N bits to any address space and
/// PatFrag predicates are typically used to constrain the address space. There's
/// no reliable means to derive the missing type information from the pattern so
/// imported rules must test the components of a pointer separately.
///
/// If SizeInBits is zero, then the pointer size will be obtained from the
/// subtarget.
class PointerToAnyOperandMatcher : public OperandPredicateMatcher {
protected:
  unsigned SizeInBits;

public:
  PointerToAnyOperandMatcher(unsigned SizeInBits)
      : OperandPredicateMatcher(OPM_PointerToAny), SizeInBits(SizeInBits) {}

  static bool classof(const OperandPredicateMatcher *P) {
    return P->getKind() == OPM_PointerToAny;
  }

  void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                            unsigned InsnVarID, unsigned OpIdx) const override {
    Table << MatchTable::Opcode("GIM_CheckPointerToAny") << MatchTable::Comment("MI")
          << MatchTable::IntValue(InsnVarID) << MatchTable::Comment("Op")
          << MatchTable::IntValue(OpIdx) << MatchTable::Comment("SizeInBits")
          << MatchTable::IntValue(SizeInBits) << MatchTable::LineBreak;
  }
};

/// Generates code to check that an operand is a particular target constant.
class ComplexPatternOperandMatcher : public OperandPredicateMatcher {
protected:
  const OperandMatcher &Operand;
  const Record &TheDef;

  unsigned getAllocatedTemporariesBaseID() const;

public:
  ComplexPatternOperandMatcher(const OperandMatcher &Operand,
                               const Record &TheDef)
      : OperandPredicateMatcher(OPM_ComplexPattern), Operand(Operand),
        TheDef(TheDef) {}

  static bool classof(const OperandPredicateMatcher *P) {
    return P->getKind() == OPM_ComplexPattern;
  }

  void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                            unsigned InsnVarID, unsigned OpIdx) const override {
    unsigned ID = getAllocatedTemporariesBaseID();
    Table << MatchTable::Opcode("GIM_CheckComplexPattern")
          << MatchTable::Comment("MI") << MatchTable::IntValue(InsnVarID)
          << MatchTable::Comment("Op") << MatchTable::IntValue(OpIdx)
          << MatchTable::Comment("Renderer") << MatchTable::IntValue(ID)
          << MatchTable::NamedValue(("GICP_" + TheDef.getName()).str())
          << MatchTable::LineBreak;
  }

  unsigned countRendererFns() const override {
    return 1;
  }
};

/// Generates code to check that an operand is in a particular register bank.
class RegisterBankOperandMatcher : public OperandPredicateMatcher {
protected:
  const CodeGenRegisterClass &RC;

public:
  RegisterBankOperandMatcher(const CodeGenRegisterClass &RC)
      : OperandPredicateMatcher(OPM_RegBank), RC(RC) {}

  static bool classof(const OperandPredicateMatcher *P) {
    return P->getKind() == OPM_RegBank;
  }

  void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                            unsigned InsnVarID, unsigned OpIdx) const override {
    Table << MatchTable::Opcode("GIM_CheckRegBankForClass")
          << MatchTable::Comment("MI") << MatchTable::IntValue(InsnVarID)
          << MatchTable::Comment("Op") << MatchTable::IntValue(OpIdx)
          << MatchTable::Comment("RC")
          << MatchTable::NamedValue(RC.getQualifiedName() + "RegClassID")
          << MatchTable::LineBreak;
  }
};

/// Generates code to check that an operand is a basic block.
class MBBOperandMatcher : public OperandPredicateMatcher {
public:
  MBBOperandMatcher() : OperandPredicateMatcher(OPM_MBB) {}

  static bool classof(const OperandPredicateMatcher *P) {
    return P->getKind() == OPM_MBB;
  }

  void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                            unsigned InsnVarID, unsigned OpIdx) const override {
    Table << MatchTable::Opcode("GIM_CheckIsMBB") << MatchTable::Comment("MI")
          << MatchTable::IntValue(InsnVarID) << MatchTable::Comment("Op")
          << MatchTable::IntValue(OpIdx) << MatchTable::LineBreak;
  }
};

/// Generates code to check that an operand is a G_CONSTANT with a particular
/// int.
class ConstantIntOperandMatcher : public OperandPredicateMatcher {
protected:
  int64_t Value;

public:
  ConstantIntOperandMatcher(int64_t Value)
      : OperandPredicateMatcher(OPM_Int), Value(Value) {}

  static bool classof(const OperandPredicateMatcher *P) {
    return P->getKind() == OPM_Int;
  }

  void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                            unsigned InsnVarID, unsigned OpIdx) const override {
    Table << MatchTable::Opcode("GIM_CheckConstantInt")
          << MatchTable::Comment("MI") << MatchTable::IntValue(InsnVarID)
          << MatchTable::Comment("Op") << MatchTable::IntValue(OpIdx)
          << MatchTable::IntValue(Value) << MatchTable::LineBreak;
  }
};

/// Generates code to check that an operand is a raw int (where MO.isImm() or
/// MO.isCImm() is true).
class LiteralIntOperandMatcher : public OperandPredicateMatcher {
protected:
  int64_t Value;

public:
  LiteralIntOperandMatcher(int64_t Value)
      : OperandPredicateMatcher(OPM_LiteralInt), Value(Value) {}

  static bool classof(const OperandPredicateMatcher *P) {
    return P->getKind() == OPM_LiteralInt;
  }

  void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                            unsigned InsnVarID, unsigned OpIdx) const override {
    Table << MatchTable::Opcode("GIM_CheckLiteralInt")
          << MatchTable::Comment("MI") << MatchTable::IntValue(InsnVarID)
          << MatchTable::Comment("Op") << MatchTable::IntValue(OpIdx)
          << MatchTable::IntValue(Value) << MatchTable::LineBreak;
  }
};

/// Generates code to check that an operand is an intrinsic ID.
class IntrinsicIDOperandMatcher : public OperandPredicateMatcher {
protected:
  const CodeGenIntrinsic *II;

public:
  IntrinsicIDOperandMatcher(const CodeGenIntrinsic *II)
      : OperandPredicateMatcher(OPM_IntrinsicID), II(II) {}

  static bool classof(const OperandPredicateMatcher *P) {
    return P->getKind() == OPM_IntrinsicID;
  }

  void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                            unsigned InsnVarID, unsigned OpIdx) const override {
    Table << MatchTable::Opcode("GIM_CheckIntrinsicID")
          << MatchTable::Comment("MI") << MatchTable::IntValue(InsnVarID)
          << MatchTable::Comment("Op") << MatchTable::IntValue(OpIdx)
          << MatchTable::NamedValue("Intrinsic::" + II->EnumName)
          << MatchTable::LineBreak;
  }
};

/// Generates code to check that a set of predicates match for a particular
/// operand.
class OperandMatcher : public PredicateListMatcher<OperandPredicateMatcher> {
protected:
  InstructionMatcher &Insn;
  unsigned OpIdx;
  std::string SymbolicName;

  /// The index of the first temporary variable allocated to this operand. The
  /// number of allocated temporaries can be found with
  /// countRendererFns().
  unsigned AllocatedTemporariesBaseID;

public:
  OperandMatcher(InstructionMatcher &Insn, unsigned OpIdx,
                 const std::string &SymbolicName,
                 unsigned AllocatedTemporariesBaseID)
      : Insn(Insn), OpIdx(OpIdx), SymbolicName(SymbolicName),
        AllocatedTemporariesBaseID(AllocatedTemporariesBaseID) {}

  bool hasSymbolicName() const { return !SymbolicName.empty(); }
  const StringRef getSymbolicName() const { return SymbolicName; }
  void setSymbolicName(StringRef Name) {
    assert(SymbolicName.empty() && "Operand already has a symbolic name");
    SymbolicName = Name;
  }
  unsigned getOperandIndex() const { return OpIdx; }

  std::string getOperandExpr(unsigned InsnVarID) const {
    return "State.MIs[" + llvm::to_string(InsnVarID) + "]->getOperand(" +
           llvm::to_string(OpIdx) + ")";
  }

  InstructionMatcher &getInstructionMatcher() const { return Insn; }

  Error addTypeCheckPredicate(const TypeSetByHwMode &VTy,
                              bool OperandIsAPointer);

  /// Emit MatchTable opcodes to capture instructions into the MIs table.
  void emitCaptureOpcodes(MatchTable &Table, RuleMatcher &Rule,
                          unsigned InsnVarID) const {
    for (const auto &Predicate : predicates())
      Predicate->emitCaptureOpcodes(Table, Rule, InsnVarID, OpIdx);
  }

  /// Emit MatchTable opcodes that test whether the instruction named in
  /// InsnVarID matches all the predicates and all the operands.
  void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                            unsigned InsnVarID) const {
    std::string Comment;
    raw_string_ostream CommentOS(Comment);
    CommentOS << "MIs[" << InsnVarID << "] ";
    if (SymbolicName.empty())
      CommentOS << "Operand " << OpIdx;
    else
      CommentOS << SymbolicName;
    Table << MatchTable::Comment(CommentOS.str()) << MatchTable::LineBreak;

    emitPredicateListOpcodes(Table, Rule, InsnVarID, OpIdx);
  }

  /// Compare the priority of this object and B.
  ///
  /// Returns true if this object is more important than B.
  bool isHigherPriorityThan(const OperandMatcher &B) const {
    // Operand matchers involving more predicates have higher priority.
    if (predicates_size() > B.predicates_size())
      return true;
    if (predicates_size() < B.predicates_size())
      return false;

    // This assumes that predicates are added in a consistent order.
    for (const auto &Predicate : zip(predicates(), B.predicates())) {
      if (std::get<0>(Predicate)->isHigherPriorityThan(*std::get<1>(Predicate)))
        return true;
      if (std::get<1>(Predicate)->isHigherPriorityThan(*std::get<0>(Predicate)))
        return false;
    }

    return false;
  };

  /// Report the maximum number of temporary operands needed by the operand
  /// matcher.
  unsigned countRendererFns() const {
    return std::accumulate(
        predicates().begin(), predicates().end(), 0,
        [](unsigned A,
           const std::unique_ptr<OperandPredicateMatcher> &Predicate) {
          return A + Predicate->countRendererFns();
        });
  }

  unsigned getAllocatedTemporariesBaseID() const {
    return AllocatedTemporariesBaseID;
  }

  bool isSameAsAnotherOperand() const {
    for (const auto &Predicate : predicates())
      if (isa<SameOperandMatcher>(Predicate))
        return true;
    return false;
  }
};

// Specialize OperandMatcher::addPredicate() to refrain from adding redundant
// predicates.
template <>
template <class Kind, class... Args>
Optional<Kind *>
PredicateListMatcher<OperandPredicateMatcher>::addPredicate(Args &&... args) {
  if (static_cast<OperandMatcher *>(this)->isSameAsAnotherOperand())
    return None;
  Predicates.emplace_back(llvm::make_unique<Kind>(std::forward<Args>(args)...));
  return static_cast<Kind *>(Predicates.back().get());
}

Error OperandMatcher::addTypeCheckPredicate(const TypeSetByHwMode &VTy,
                                                     bool OperandIsAPointer) {
  if (!VTy.isMachineValueType())
    return failedImport("unsupported typeset");

  if (VTy.getMachineValueType() == MVT::iPTR && OperandIsAPointer) {
    addPredicate<PointerToAnyOperandMatcher>(0);
    return Error::success();
  }

  auto OpTyOrNone = MVTToLLT(VTy.getMachineValueType().SimpleTy);
  if (!OpTyOrNone)
    return failedImport("unsupported type");

  if (OperandIsAPointer)
    addPredicate<PointerToAnyOperandMatcher>(OpTyOrNone->get().getSizeInBits());
  else
    addPredicate<LLTOperandMatcher>(*OpTyOrNone);
  return Error::success();
}

unsigned ComplexPatternOperandMatcher::getAllocatedTemporariesBaseID() const {
  return Operand.getAllocatedTemporariesBaseID();
}

/// Generates code to check a predicate on an instruction.
///
/// Typical predicates include:
/// * The opcode of the instruction is a particular value.
/// * The nsw/nuw flag is/isn't set.
class InstructionPredicateMatcher {
protected:
  /// This enum is used for RTTI and also defines the priority that is given to
  /// the predicate when generating the matcher code. Kinds with higher priority
  /// must be tested first.
  enum PredicateKind {
    IPM_Opcode,
    IPM_ImmPredicate,
    IPM_AtomicOrderingMMO,
  };

  PredicateKind Kind;

public:
  InstructionPredicateMatcher(PredicateKind Kind) : Kind(Kind) {}
  virtual ~InstructionPredicateMatcher() {}

  PredicateKind getKind() const { return Kind; }

  /// Emit MatchTable opcodes that test whether the instruction named in
  /// InsnVarID matches the predicate.
  virtual void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                                    unsigned InsnVarID) const = 0;

  /// Compare the priority of this object and B.
  ///
  /// Returns true if this object is more important than B.
  virtual bool
  isHigherPriorityThan(const InstructionPredicateMatcher &B) const {
    return Kind < B.Kind;
  };

  /// Report the maximum number of temporary operands needed by the predicate
  /// matcher.
  virtual unsigned countRendererFns() const { return 0; }
};

template <>
std::string
PredicateListMatcher<InstructionPredicateMatcher>::getNoPredicateComment() const {
  return "No instruction predicates";
}

/// Generates code to check the opcode of an instruction.
class InstructionOpcodeMatcher : public InstructionPredicateMatcher {
protected:
  const CodeGenInstruction *I;

public:
  InstructionOpcodeMatcher(const CodeGenInstruction *I)
      : InstructionPredicateMatcher(IPM_Opcode), I(I) {}

  static bool classof(const InstructionPredicateMatcher *P) {
    return P->getKind() == IPM_Opcode;
  }

  void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                            unsigned InsnVarID) const override {
    Table << MatchTable::Opcode("GIM_CheckOpcode") << MatchTable::Comment("MI")
          << MatchTable::IntValue(InsnVarID)
          << MatchTable::NamedValue(I->Namespace, I->TheDef->getName())
          << MatchTable::LineBreak;
  }

  /// Compare the priority of this object and B.
  ///
  /// Returns true if this object is more important than B.
  bool
  isHigherPriorityThan(const InstructionPredicateMatcher &B) const override {
    if (InstructionPredicateMatcher::isHigherPriorityThan(B))
      return true;
    if (B.InstructionPredicateMatcher::isHigherPriorityThan(*this))
      return false;

    // Prioritize opcodes for cosmetic reasons in the generated source. Although
    // this is cosmetic at the moment, we may want to drive a similar ordering
    // using instruction frequency information to improve compile time.
    if (const InstructionOpcodeMatcher *BO =
            dyn_cast<InstructionOpcodeMatcher>(&B))
      return I->TheDef->getName() < BO->I->TheDef->getName();

    return false;
  };

  bool isConstantInstruction() const {
    return I->TheDef->getName() == "G_CONSTANT";
  }
};

/// Generates code to check that this instruction is a constant whose value
/// meets an immediate predicate.
///
/// Immediates are slightly odd since they are typically used like an operand
/// but are represented as an operator internally. We typically write simm8:$src
/// in a tablegen pattern, but this is just syntactic sugar for
/// (imm:i32)<<P:Predicate_simm8>>:$imm which more directly describes the nodes
/// that will be matched and the predicate (which is attached to the imm
/// operator) that will be tested. In SelectionDAG this describes a
/// ConstantSDNode whose internal value will be tested using the simm8 predicate.
///
/// The corresponding GlobalISel representation is %1 = G_CONSTANT iN Value. In
/// this representation, the immediate could be tested with an
/// InstructionMatcher, InstructionOpcodeMatcher, OperandMatcher, and a
/// OperandPredicateMatcher-subclass to check the Value meets the predicate but
/// there are two implementation issues with producing that matcher
/// configuration from the SelectionDAG pattern:
/// * ImmLeaf is a PatFrag whose root is an InstructionMatcher. This means that
///   were we to sink the immediate predicate to the operand we would have to
///   have two partial implementations of PatFrag support, one for immediates
///   and one for non-immediates.
/// * At the point we handle the predicate, the OperandMatcher hasn't been
///   created yet. If we were to sink the predicate to the OperandMatcher we
///   would also have to complicate (or duplicate) the code that descends and
///   creates matchers for the subtree.
/// Overall, it's simpler to handle it in the place it was found.
class InstructionImmPredicateMatcher : public InstructionPredicateMatcher {
protected:
  TreePredicateFn Predicate;

public:
  InstructionImmPredicateMatcher(const TreePredicateFn &Predicate)
      : InstructionPredicateMatcher(IPM_ImmPredicate), Predicate(Predicate) {}

  static bool classof(const InstructionPredicateMatcher *P) {
    return P->getKind() == IPM_ImmPredicate;
  }

  void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                            unsigned InsnVarID) const override {
    Table << MatchTable::Opcode(getMatchOpcodeForPredicate(Predicate))
          << MatchTable::Comment("MI") << MatchTable::IntValue(InsnVarID)
          << MatchTable::Comment("Predicate")
          << MatchTable::NamedValue(getEnumNameForPredicate(Predicate))
          << MatchTable::LineBreak;
  }
};

/// Generates code to check that a memory instruction has a atomic ordering
/// MachineMemoryOperand.
class AtomicOrderingMMOPredicateMatcher : public InstructionPredicateMatcher {
public:
  enum AOComparator {
    AO_Exactly,
    AO_OrStronger,
    AO_WeakerThan,
  };

protected:
  StringRef Order;
  AOComparator Comparator;

public:
  AtomicOrderingMMOPredicateMatcher(StringRef Order,
                                    AOComparator Comparator = AO_Exactly)
      : InstructionPredicateMatcher(IPM_AtomicOrderingMMO), Order(Order),
        Comparator(Comparator) {}

  static bool classof(const InstructionPredicateMatcher *P) {
    return P->getKind() == IPM_AtomicOrderingMMO;
  }

  void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                            unsigned InsnVarID) const override {
    StringRef Opcode = "GIM_CheckAtomicOrdering";

    if (Comparator == AO_OrStronger)
      Opcode = "GIM_CheckAtomicOrderingOrStrongerThan";
    if (Comparator == AO_WeakerThan)
      Opcode = "GIM_CheckAtomicOrderingWeakerThan";

    Table << MatchTable::Opcode(Opcode) << MatchTable::Comment("MI")
          << MatchTable::IntValue(InsnVarID) << MatchTable::Comment("Order")
          << MatchTable::NamedValue(("(int64_t)AtomicOrdering::" + Order).str())
          << MatchTable::LineBreak;
  }
};

/// Generates code to check that a set of predicates and operands match for a
/// particular instruction.
///
/// Typical predicates include:
/// * Has a specific opcode.
/// * Has an nsw/nuw flag or doesn't.
class InstructionMatcher
    : public PredicateListMatcher<InstructionPredicateMatcher> {
protected:
  typedef std::vector<std::unique_ptr<OperandMatcher>> OperandVec;

  RuleMatcher &Rule;

  /// The operands to match. All rendered operands must be present even if the
  /// condition is always true.
  OperandVec Operands;

  std::string SymbolicName;

public:
  InstructionMatcher(RuleMatcher &Rule, StringRef SymbolicName)
      : Rule(Rule), SymbolicName(SymbolicName) {}

  RuleMatcher &getRuleMatcher() const { return Rule; }

  /// Add an operand to the matcher.
  OperandMatcher &addOperand(unsigned OpIdx, const std::string &SymbolicName,
                             unsigned AllocatedTemporariesBaseID) {
    Operands.emplace_back(new OperandMatcher(*this, OpIdx, SymbolicName,
                                             AllocatedTemporariesBaseID));
    if (!SymbolicName.empty())
      Rule.defineOperand(SymbolicName, *Operands.back());

    return *Operands.back();
  }

  OperandMatcher &getOperand(unsigned OpIdx) {
    auto I = std::find_if(Operands.begin(), Operands.end(),
                          [&OpIdx](const std::unique_ptr<OperandMatcher> &X) {
                            return X->getOperandIndex() == OpIdx;
                          });
    if (I != Operands.end())
      return **I;
    llvm_unreachable("Failed to lookup operand");
  }

  StringRef getSymbolicName() const { return SymbolicName; }
  unsigned getNumOperands() const { return Operands.size(); }
  OperandVec::iterator operands_begin() { return Operands.begin(); }
  OperandVec::iterator operands_end() { return Operands.end(); }
  iterator_range<OperandVec::iterator> operands() {
    return make_range(operands_begin(), operands_end());
  }
  OperandVec::const_iterator operands_begin() const { return Operands.begin(); }
  OperandVec::const_iterator operands_end() const { return Operands.end(); }
  iterator_range<OperandVec::const_iterator> operands() const {
    return make_range(operands_begin(), operands_end());
  }

  /// Emit MatchTable opcodes to check the shape of the match and capture
  /// instructions into the MIs table.
  void emitCaptureOpcodes(MatchTable &Table, RuleMatcher &Rule,
                          unsigned InsnID) {
    Table << MatchTable::Opcode("GIM_CheckNumOperands")
          << MatchTable::Comment("MI") << MatchTable::IntValue(InsnID)
          << MatchTable::Comment("Expected")
          << MatchTable::IntValue(getNumOperands()) << MatchTable::LineBreak;
    for (const auto &Operand : Operands)
      Operand->emitCaptureOpcodes(Table, Rule, InsnID);
  }

  /// Emit MatchTable opcodes that test whether the instruction named in
  /// InsnVarName matches all the predicates and all the operands.
  void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                            unsigned InsnVarID) const {
    emitPredicateListOpcodes(Table, Rule, InsnVarID);
    for (const auto &Operand : Operands)
      Operand->emitPredicateOpcodes(Table, Rule, InsnVarID);
  }

  /// Compare the priority of this object and B.
  ///
  /// Returns true if this object is more important than B.
  bool isHigherPriorityThan(const InstructionMatcher &B) const {
    // Instruction matchers involving more operands have higher priority.
    if (Operands.size() > B.Operands.size())
      return true;
    if (Operands.size() < B.Operands.size())
      return false;

    for (const auto &Predicate : zip(predicates(), B.predicates())) {
      if (std::get<0>(Predicate)->isHigherPriorityThan(*std::get<1>(Predicate)))
        return true;
      if (std::get<1>(Predicate)->isHigherPriorityThan(*std::get<0>(Predicate)))
        return false;
    }

    for (const auto &Operand : zip(Operands, B.Operands)) {
      if (std::get<0>(Operand)->isHigherPriorityThan(*std::get<1>(Operand)))
        return true;
      if (std::get<1>(Operand)->isHigherPriorityThan(*std::get<0>(Operand)))
        return false;
    }

    return false;
  };

  /// Report the maximum number of temporary operands needed by the instruction
  /// matcher.
  unsigned countRendererFns() const {
    return std::accumulate(predicates().begin(), predicates().end(), 0,
                           [](unsigned A,
                              const std::unique_ptr<InstructionPredicateMatcher>
                                  &Predicate) {
                             return A + Predicate->countRendererFns();
                           }) +
           std::accumulate(
               Operands.begin(), Operands.end(), 0,
               [](unsigned A, const std::unique_ptr<OperandMatcher> &Operand) {
                 return A + Operand->countRendererFns();
               });
  }

  bool isConstantInstruction() const {
    for (const auto &P : predicates())
      if (const InstructionOpcodeMatcher *Opcode =
              dyn_cast<InstructionOpcodeMatcher>(P.get()))
        return Opcode->isConstantInstruction();
    return false;
  }
};

/// Generates code to check that the operand is a register defined by an
/// instruction that matches the given instruction matcher.
///
/// For example, the pattern:
///   (set $dst, (G_MUL (G_ADD $src1, $src2), $src3))
/// would use an InstructionOperandMatcher for operand 1 of the G_MUL to match
/// the:
///   (G_ADD $src1, $src2)
/// subpattern.
class InstructionOperandMatcher : public OperandPredicateMatcher {
protected:
  std::unique_ptr<InstructionMatcher> InsnMatcher;

public:
  InstructionOperandMatcher(RuleMatcher &Rule, StringRef SymbolicName)
      : OperandPredicateMatcher(OPM_Instruction),
        InsnMatcher(new InstructionMatcher(Rule, SymbolicName)) {}

  static bool classof(const OperandPredicateMatcher *P) {
    return P->getKind() == OPM_Instruction;
  }

  InstructionMatcher &getInsnMatcher() const { return *InsnMatcher; }

  void emitCaptureOpcodes(MatchTable &Table, RuleMatcher &Rule,
                          unsigned InsnID, unsigned OpIdx) const override {
    unsigned InsnVarID = Rule.defineInsnVar(Table, *InsnMatcher, InsnID, OpIdx);
    InsnMatcher->emitCaptureOpcodes(Table, Rule, InsnVarID);
  }

  void emitPredicateOpcodes(MatchTable &Table, RuleMatcher &Rule,
                            unsigned InsnVarID_,
                            unsigned OpIdx_) const override {
    unsigned InsnVarID = Rule.getInsnVarID(*InsnMatcher);
    InsnMatcher->emitPredicateOpcodes(Table, Rule, InsnVarID);
  }
};

//===- Actions ------------------------------------------------------------===//
class OperandRenderer {
public:
  enum RendererKind {
    OR_Copy,
    OR_CopyOrAddZeroReg,
    OR_CopySubReg,
    OR_CopyConstantAsImm,
    OR_CopyFConstantAsFPImm,
    OR_Imm,
    OR_Register,
    OR_TempRegister,
    OR_ComplexPattern
  };

protected:
  RendererKind Kind;

public:
  OperandRenderer(RendererKind Kind) : Kind(Kind) {}
  virtual ~OperandRenderer() {}

  RendererKind getKind() const { return Kind; }

  virtual void emitRenderOpcodes(MatchTable &Table,
                                 RuleMatcher &Rule) const = 0;
};

/// A CopyRenderer emits code to copy a single operand from an existing
/// instruction to the one being built.
class CopyRenderer : public OperandRenderer {
protected:
  unsigned NewInsnID;
  /// The name of the operand.
  const StringRef SymbolicName;

public:
  CopyRenderer(unsigned NewInsnID, StringRef SymbolicName)
      : OperandRenderer(OR_Copy), NewInsnID(NewInsnID),
        SymbolicName(SymbolicName) {
    assert(!SymbolicName.empty() && "Cannot copy from an unspecified source");
  }

  static bool classof(const OperandRenderer *R) {
    return R->getKind() == OR_Copy;
  }

  const StringRef getSymbolicName() const { return SymbolicName; }

  void emitRenderOpcodes(MatchTable &Table, RuleMatcher &Rule) const override {
    const OperandMatcher &Operand = Rule.getOperandMatcher(SymbolicName);
    unsigned OldInsnVarID = Rule.getInsnVarID(Operand.getInstructionMatcher());
    Table << MatchTable::Opcode("GIR_Copy") << MatchTable::Comment("NewInsnID")
          << MatchTable::IntValue(NewInsnID) << MatchTable::Comment("OldInsnID")
          << MatchTable::IntValue(OldInsnVarID) << MatchTable::Comment("OpIdx")
          << MatchTable::IntValue(Operand.getOperandIndex())
          << MatchTable::Comment(SymbolicName) << MatchTable::LineBreak;
  }
};

/// A CopyOrAddZeroRegRenderer emits code to copy a single operand from an
/// existing instruction to the one being built. If the operand turns out to be
/// a 'G_CONSTANT 0' then it replaces the operand with a zero register.
class CopyOrAddZeroRegRenderer : public OperandRenderer {
protected:
  unsigned NewInsnID;
  /// The name of the operand.
  const StringRef SymbolicName;
  const Record *ZeroRegisterDef;

public:
  CopyOrAddZeroRegRenderer(unsigned NewInsnID,
                           StringRef SymbolicName, Record *ZeroRegisterDef)
      : OperandRenderer(OR_CopyOrAddZeroReg), NewInsnID(NewInsnID),
        SymbolicName(SymbolicName), ZeroRegisterDef(ZeroRegisterDef) {
    assert(!SymbolicName.empty() && "Cannot copy from an unspecified source");
  }

  static bool classof(const OperandRenderer *R) {
    return R->getKind() == OR_CopyOrAddZeroReg;
  }

  const StringRef getSymbolicName() const { return SymbolicName; }

  void emitRenderOpcodes(MatchTable &Table, RuleMatcher &Rule) const override {
    const OperandMatcher &Operand = Rule.getOperandMatcher(SymbolicName);
    unsigned OldInsnVarID = Rule.getInsnVarID(Operand.getInstructionMatcher());
    Table << MatchTable::Opcode("GIR_CopyOrAddZeroReg")
          << MatchTable::Comment("NewInsnID") << MatchTable::IntValue(NewInsnID)
          << MatchTable::Comment("OldInsnID")
          << MatchTable::IntValue(OldInsnVarID) << MatchTable::Comment("OpIdx")
          << MatchTable::IntValue(Operand.getOperandIndex())
          << MatchTable::NamedValue(
                 (ZeroRegisterDef->getValue("Namespace")
                      ? ZeroRegisterDef->getValueAsString("Namespace")
                      : ""),
                 ZeroRegisterDef->getName())
          << MatchTable::Comment(SymbolicName) << MatchTable::LineBreak;
  }
};

/// A CopyConstantAsImmRenderer emits code to render a G_CONSTANT instruction to
/// an extended immediate operand.
class CopyConstantAsImmRenderer : public OperandRenderer {
protected:
  unsigned NewInsnID;
  /// The name of the operand.
  const std::string SymbolicName;
  bool Signed;

public:
  CopyConstantAsImmRenderer(unsigned NewInsnID, StringRef SymbolicName)
      : OperandRenderer(OR_CopyConstantAsImm), NewInsnID(NewInsnID),
        SymbolicName(SymbolicName), Signed(true) {}

  static bool classof(const OperandRenderer *R) {
    return R->getKind() == OR_CopyConstantAsImm;
  }

  const StringRef getSymbolicName() const { return SymbolicName; }

  void emitRenderOpcodes(MatchTable &Table, RuleMatcher &Rule) const override {
    const InstructionMatcher &InsnMatcher = Rule.getInstructionMatcher(SymbolicName);
    unsigned OldInsnVarID = Rule.getInsnVarID(InsnMatcher);
    Table << MatchTable::Opcode(Signed ? "GIR_CopyConstantAsSImm"
                                       : "GIR_CopyConstantAsUImm")
          << MatchTable::Comment("NewInsnID") << MatchTable::IntValue(NewInsnID)
          << MatchTable::Comment("OldInsnID")
          << MatchTable::IntValue(OldInsnVarID)
          << MatchTable::Comment(SymbolicName) << MatchTable::LineBreak;
  }
};

/// A CopyFConstantAsFPImmRenderer emits code to render a G_FCONSTANT
/// instruction to an extended immediate operand.
class CopyFConstantAsFPImmRenderer : public OperandRenderer {
protected:
  unsigned NewInsnID;
  /// The name of the operand.
  const std::string SymbolicName;

public:
  CopyFConstantAsFPImmRenderer(unsigned NewInsnID, StringRef SymbolicName)
      : OperandRenderer(OR_CopyFConstantAsFPImm), NewInsnID(NewInsnID),
        SymbolicName(SymbolicName) {}

  static bool classof(const OperandRenderer *R) {
    return R->getKind() == OR_CopyFConstantAsFPImm;
  }

  const StringRef getSymbolicName() const { return SymbolicName; }

  void emitRenderOpcodes(MatchTable &Table, RuleMatcher &Rule) const override {
    const InstructionMatcher &InsnMatcher = Rule.getInstructionMatcher(SymbolicName);
    unsigned OldInsnVarID = Rule.getInsnVarID(InsnMatcher);
    Table << MatchTable::Opcode("GIR_CopyFConstantAsFPImm")
          << MatchTable::Comment("NewInsnID") << MatchTable::IntValue(NewInsnID)
          << MatchTable::Comment("OldInsnID")
          << MatchTable::IntValue(OldInsnVarID)
          << MatchTable::Comment(SymbolicName) << MatchTable::LineBreak;
  }
};

/// A CopySubRegRenderer emits code to copy a single register operand from an
/// existing instruction to the one being built and indicate that only a
/// subregister should be copied.
class CopySubRegRenderer : public OperandRenderer {
protected:
  unsigned NewInsnID;
  /// The name of the operand.
  const StringRef SymbolicName;
  /// The subregister to extract.
  const CodeGenSubRegIndex *SubReg;

public:
  CopySubRegRenderer(unsigned NewInsnID, StringRef SymbolicName,
                     const CodeGenSubRegIndex *SubReg)
      : OperandRenderer(OR_CopySubReg), NewInsnID(NewInsnID),
        SymbolicName(SymbolicName), SubReg(SubReg) {}

  static bool classof(const OperandRenderer *R) {
    return R->getKind() == OR_CopySubReg;
  }

  const StringRef getSymbolicName() const { return SymbolicName; }

  void emitRenderOpcodes(MatchTable &Table, RuleMatcher &Rule) const override {
    const OperandMatcher &Operand = Rule.getOperandMatcher(SymbolicName);
    unsigned OldInsnVarID = Rule.getInsnVarID(Operand.getInstructionMatcher());
    Table << MatchTable::Opcode("GIR_CopySubReg")
          << MatchTable::Comment("NewInsnID") << MatchTable::IntValue(NewInsnID)
          << MatchTable::Comment("OldInsnID")
          << MatchTable::IntValue(OldInsnVarID) << MatchTable::Comment("OpIdx")
          << MatchTable::IntValue(Operand.getOperandIndex())
          << MatchTable::Comment("SubRegIdx")
          << MatchTable::IntValue(SubReg->EnumValue)
          << MatchTable::Comment(SymbolicName) << MatchTable::LineBreak;
  }
};

/// Adds a specific physical register to the instruction being built.
/// This is typically useful for WZR/XZR on AArch64.
class AddRegisterRenderer : public OperandRenderer {
protected:
  unsigned InsnID;
  const Record *RegisterDef;

public:
  AddRegisterRenderer(unsigned InsnID, const Record *RegisterDef)
      : OperandRenderer(OR_Register), InsnID(InsnID), RegisterDef(RegisterDef) {
  }

  static bool classof(const OperandRenderer *R) {
    return R->getKind() == OR_Register;
  }

  void emitRenderOpcodes(MatchTable &Table, RuleMatcher &Rule) const override {
    Table << MatchTable::Opcode("GIR_AddRegister")
          << MatchTable::Comment("InsnID") << MatchTable::IntValue(InsnID)
          << MatchTable::NamedValue(
                 (RegisterDef->getValue("Namespace")
                      ? RegisterDef->getValueAsString("Namespace")
                      : ""),
                 RegisterDef->getName())
          << MatchTable::LineBreak;
  }
};

/// Adds a specific temporary virtual register to the instruction being built.
/// This is used to chain instructions together when emitting multiple
/// instructions.
class TempRegRenderer : public OperandRenderer {
protected:
  unsigned InsnID;
  unsigned TempRegID;
  bool IsDef;

public:
  TempRegRenderer(unsigned InsnID, unsigned TempRegID, bool IsDef = false)
      : OperandRenderer(OR_Register), InsnID(InsnID), TempRegID(TempRegID),
        IsDef(IsDef) {}

  static bool classof(const OperandRenderer *R) {
    return R->getKind() == OR_TempRegister;
  }

  void emitRenderOpcodes(MatchTable &Table, RuleMatcher &Rule) const override {
    Table << MatchTable::Opcode("GIR_AddTempRegister")
          << MatchTable::Comment("InsnID") << MatchTable::IntValue(InsnID)
          << MatchTable::Comment("TempRegID") << MatchTable::IntValue(TempRegID)
          << MatchTable::Comment("TempRegFlags");
    if (IsDef)
      Table << MatchTable::NamedValue("RegState::Define");
    else
      Table << MatchTable::IntValue(0);
    Table << MatchTable::LineBreak;
  }
};

/// Adds a specific immediate to the instruction being built.
class ImmRenderer : public OperandRenderer {
protected:
  unsigned InsnID;
  int64_t Imm;

public:
  ImmRenderer(unsigned InsnID, int64_t Imm)
      : OperandRenderer(OR_Imm), InsnID(InsnID), Imm(Imm) {}

  static bool classof(const OperandRenderer *R) {
    return R->getKind() == OR_Imm;
  }

  void emitRenderOpcodes(MatchTable &Table, RuleMatcher &Rule) const override {
    Table << MatchTable::Opcode("GIR_AddImm") << MatchTable::Comment("InsnID")
          << MatchTable::IntValue(InsnID) << MatchTable::Comment("Imm")
          << MatchTable::IntValue(Imm) << MatchTable::LineBreak;
  }
};

/// Adds operands by calling a renderer function supplied by the ComplexPattern
/// matcher function.
class RenderComplexPatternOperand : public OperandRenderer {
private:
  unsigned InsnID;
  const Record &TheDef;
  /// The name of the operand.
  const StringRef SymbolicName;
  /// The renderer number. This must be unique within a rule since it's used to
  /// identify a temporary variable to hold the renderer function.
  unsigned RendererID;
  /// When provided, this is the suboperand of the ComplexPattern operand to
  /// render. Otherwise all the suboperands will be rendered.
  Optional<unsigned> SubOperand;

  unsigned getNumOperands() const {
    return TheDef.getValueAsDag("Operands")->getNumArgs();
  }

public:
  RenderComplexPatternOperand(unsigned InsnID, const Record &TheDef,
                              StringRef SymbolicName, unsigned RendererID,
                              Optional<unsigned> SubOperand = None)
      : OperandRenderer(OR_ComplexPattern), InsnID(InsnID), TheDef(TheDef),
        SymbolicName(SymbolicName), RendererID(RendererID),
        SubOperand(SubOperand) {}

  static bool classof(const OperandRenderer *R) {
    return R->getKind() == OR_ComplexPattern;
  }

  void emitRenderOpcodes(MatchTable &Table, RuleMatcher &Rule) const override {
    Table << MatchTable::Opcode(SubOperand.hasValue() ? "GIR_ComplexSubOperandRenderer"
                                                      : "GIR_ComplexRenderer")
          << MatchTable::Comment("InsnID") << MatchTable::IntValue(InsnID)
          << MatchTable::Comment("RendererID")
          << MatchTable::IntValue(RendererID);
    if (SubOperand.hasValue())
      Table << MatchTable::Comment("SubOperand")
            << MatchTable::IntValue(SubOperand.getValue());
    Table << MatchTable::Comment(SymbolicName) << MatchTable::LineBreak;
  }
};

/// An action taken when all Matcher predicates succeeded for a parent rule.
///
/// Typical actions include:
/// * Changing the opcode of an instruction.
/// * Adding an operand to an instruction.
class MatchAction {
public:
  virtual ~MatchAction() {}

  /// Emit the MatchTable opcodes to implement the action.
  virtual void emitActionOpcodes(MatchTable &Table,
                                 RuleMatcher &Rule) const = 0;
};

/// Generates a comment describing the matched rule being acted upon.
class DebugCommentAction : public MatchAction {
private:
  std::string S;

public:
  DebugCommentAction(StringRef S) : S(S) {}

  void emitActionOpcodes(MatchTable &Table, RuleMatcher &Rule) const override {
    Table << MatchTable::Comment(S) << MatchTable::LineBreak;
  }
};

/// Generates code to build an instruction or mutate an existing instruction
/// into the desired instruction when this is possible.
class BuildMIAction : public MatchAction {
private:
  unsigned InsnID;
  const CodeGenInstruction *I;
  const InstructionMatcher *Matched;
  std::vector<std::unique_ptr<OperandRenderer>> OperandRenderers;

  /// True if the instruction can be built solely by mutating the opcode.
  bool canMutate(RuleMatcher &Rule, const InstructionMatcher *Insn) const {
    if (!Insn)
      return false;

    if (OperandRenderers.size() != Insn->getNumOperands())
      return false;

    for (const auto &Renderer : enumerate(OperandRenderers)) {
      if (const auto *Copy = dyn_cast<CopyRenderer>(&*Renderer.value())) {
        const OperandMatcher &OM = Rule.getOperandMatcher(Copy->getSymbolicName());
        if (Insn != &OM.getInstructionMatcher() ||
            OM.getOperandIndex() != Renderer.index())
          return false;
      } else
        return false;
    }

    return true;
  }

public:
  BuildMIAction(unsigned InsnID, const CodeGenInstruction *I)
      : InsnID(InsnID), I(I), Matched(nullptr) {}

  const CodeGenInstruction *getCGI() const { return I; }

  void chooseInsnToMutate(RuleMatcher &Rule) {
    for (const auto *MutateCandidate : Rule.mutatable_insns()) {
      if (canMutate(Rule, MutateCandidate)) {
        // Take the first one we're offered that we're able to mutate.
        Rule.reserveInsnMatcherForMutation(MutateCandidate);
        Matched = MutateCandidate;
        return;
      }
    }
  }

  template <class Kind, class... Args>
  Kind &addRenderer(Args&&... args) {
    OperandRenderers.emplace_back(
        llvm::make_unique<Kind>(InsnID, std::forward<Args>(args)...));
    return *static_cast<Kind *>(OperandRenderers.back().get());
  }

  void emitActionOpcodes(MatchTable &Table, RuleMatcher &Rule) const override {
    if (Matched) {
      assert(canMutate(Rule, Matched) &&
             "Arranged to mutate an insn that isn't mutatable");

      unsigned RecycleInsnID = Rule.getInsnVarID(*Matched);
      Table << MatchTable::Opcode("GIR_MutateOpcode")
            << MatchTable::Comment("InsnID") << MatchTable::IntValue(InsnID)
            << MatchTable::Comment("RecycleInsnID")
            << MatchTable::IntValue(RecycleInsnID)
            << MatchTable::Comment("Opcode")
            << MatchTable::NamedValue(I->Namespace, I->TheDef->getName())
            << MatchTable::LineBreak;

      if (!I->ImplicitDefs.empty() || !I->ImplicitUses.empty()) {
        for (auto Def : I->ImplicitDefs) {
          auto Namespace = Def->getValue("Namespace")
                               ? Def->getValueAsString("Namespace")
                               : "";
          Table << MatchTable::Opcode("GIR_AddImplicitDef")
                << MatchTable::Comment("InsnID") << MatchTable::IntValue(InsnID)
                << MatchTable::NamedValue(Namespace, Def->getName())
                << MatchTable::LineBreak;
        }
        for (auto Use : I->ImplicitUses) {
          auto Namespace = Use->getValue("Namespace")
                               ? Use->getValueAsString("Namespace")
                               : "";
          Table << MatchTable::Opcode("GIR_AddImplicitUse")
                << MatchTable::Comment("InsnID") << MatchTable::IntValue(InsnID)
                << MatchTable::NamedValue(Namespace, Use->getName())
                << MatchTable::LineBreak;
        }
      }
      return;
    }

    // TODO: Simple permutation looks like it could be almost as common as
    //       mutation due to commutative operations.

    Table << MatchTable::Opcode("GIR_BuildMI") << MatchTable::Comment("InsnID")
          << MatchTable::IntValue(InsnID) << MatchTable::Comment("Opcode")
          << MatchTable::NamedValue(I->Namespace, I->TheDef->getName())
          << MatchTable::LineBreak;
    for (const auto &Renderer : OperandRenderers)
      Renderer->emitRenderOpcodes(Table, Rule);

    if (I->mayLoad || I->mayStore) {
      Table << MatchTable::Opcode("GIR_MergeMemOperands")
            << MatchTable::Comment("InsnID") << MatchTable::IntValue(InsnID)
            << MatchTable::Comment("MergeInsnID's");
      // Emit the ID's for all the instructions that are matched by this rule.
      // TODO: Limit this to matched instructions that mayLoad/mayStore or have
      //       some other means of having a memoperand. Also limit this to
      //       emitted instructions that expect to have a memoperand too. For
      //       example, (G_SEXT (G_LOAD x)) that results in separate load and
      //       sign-extend instructions shouldn't put the memoperand on the
      //       sign-extend since it has no effect there.
      std::vector<unsigned> MergeInsnIDs;
      for (const auto &IDMatcherPair : Rule.defined_insn_vars())
        MergeInsnIDs.push_back(IDMatcherPair.second);
      std::sort(MergeInsnIDs.begin(), MergeInsnIDs.end());
      for (const auto &MergeInsnID : MergeInsnIDs)
        Table << MatchTable::IntValue(MergeInsnID);
      Table << MatchTable::NamedValue("GIU_MergeMemOperands_EndOfList")
            << MatchTable::LineBreak;
    }

    // FIXME: This is a hack but it's sufficient for ISel. We'll need to do
    //        better for combines. Particularly when there are multiple match
    //        roots.
    if (InsnID == 0)
      Table << MatchTable::Opcode("GIR_EraseFromParent")
            << MatchTable::Comment("InsnID") << MatchTable::IntValue(InsnID)
            << MatchTable::LineBreak;
  }
};

/// Generates code to constrain the operands of an output instruction to the
/// register classes specified by the definition of that instruction.
class ConstrainOperandsToDefinitionAction : public MatchAction {
  unsigned InsnID;

public:
  ConstrainOperandsToDefinitionAction(unsigned InsnID) : InsnID(InsnID) {}

  void emitActionOpcodes(MatchTable &Table, RuleMatcher &Rule) const override {
    Table << MatchTable::Opcode("GIR_ConstrainSelectedInstOperands")
          << MatchTable::Comment("InsnID") << MatchTable::IntValue(InsnID)
          << MatchTable::LineBreak;
  }
};

/// Generates code to constrain the specified operand of an output instruction
/// to the specified register class.
class ConstrainOperandToRegClassAction : public MatchAction {
  unsigned InsnID;
  unsigned OpIdx;
  const CodeGenRegisterClass &RC;

public:
  ConstrainOperandToRegClassAction(unsigned InsnID, unsigned OpIdx,
                                   const CodeGenRegisterClass &RC)
      : InsnID(InsnID), OpIdx(OpIdx), RC(RC) {}

  void emitActionOpcodes(MatchTable &Table, RuleMatcher &Rule) const override {
    Table << MatchTable::Opcode("GIR_ConstrainOperandRC")
          << MatchTable::Comment("InsnID") << MatchTable::IntValue(InsnID)
          << MatchTable::Comment("Op") << MatchTable::IntValue(OpIdx)
          << MatchTable::Comment("RC " + RC.getName())
          << MatchTable::IntValue(RC.EnumValue) << MatchTable::LineBreak;
  }
};

/// Generates code to create a temporary register which can be used to chain
/// instructions together.
class MakeTempRegisterAction : public MatchAction {
private:
  LLTCodeGen Ty;
  unsigned TempRegID;

public:
  MakeTempRegisterAction(const LLTCodeGen &Ty, unsigned TempRegID)
      : Ty(Ty), TempRegID(TempRegID) {}

  void emitActionOpcodes(MatchTable &Table, RuleMatcher &Rule) const override {
    Table << MatchTable::Opcode("GIR_MakeTempReg")
          << MatchTable::Comment("TempRegID") << MatchTable::IntValue(TempRegID)
          << MatchTable::Comment("TypeID")
          << MatchTable::NamedValue(Ty.getCxxEnumValue())
          << MatchTable::LineBreak;
  }
};

InstructionMatcher &RuleMatcher::addInstructionMatcher(StringRef SymbolicName) {
  Matchers.emplace_back(new InstructionMatcher(*this, SymbolicName));
  MutatableInsns.insert(Matchers.back().get());
  return *Matchers.back();
}

void RuleMatcher::addRequiredFeature(Record *Feature) {
  RequiredFeatures.push_back(Feature);
}

const std::vector<Record *> &RuleMatcher::getRequiredFeatures() const {
  return RequiredFeatures;
}

// Emplaces an action of the specified Kind at the end of the action list.
//
// Returns a reference to the newly created action.
//
// Like std::vector::emplace_back(), may invalidate all iterators if the new
// size exceeds the capacity. Otherwise, only invalidates the past-the-end
// iterator.
template <class Kind, class... Args>
Kind &RuleMatcher::addAction(Args &&... args) {
  Actions.emplace_back(llvm::make_unique<Kind>(std::forward<Args>(args)...));
  return *static_cast<Kind *>(Actions.back().get());
}

// Emplaces an action of the specified Kind before the given insertion point.
//
// Returns an iterator pointing at the newly created instruction.
//
// Like std::vector::insert(), may invalidate all iterators if the new size
// exceeds the capacity. Otherwise, only invalidates the iterators from the
// insertion point onwards.
template <class Kind, class... Args>
action_iterator RuleMatcher::insertAction(action_iterator InsertPt,
                                          Args &&... args) {
  return Actions.emplace(InsertPt,
                         llvm::make_unique<Kind>(std::forward<Args>(args)...));
}

unsigned
RuleMatcher::implicitlyDefineInsnVar(const InstructionMatcher &Matcher) {
  unsigned NewInsnVarID = NextInsnVarID++;
  InsnVariableIDs[&Matcher] = NewInsnVarID;
  return NewInsnVarID;
}

unsigned RuleMatcher::defineInsnVar(MatchTable &Table,
                                    const InstructionMatcher &Matcher,
                                    unsigned InsnID, unsigned OpIdx) {
  unsigned NewInsnVarID = implicitlyDefineInsnVar(Matcher);
  Table << MatchTable::Opcode("GIM_RecordInsn")
        << MatchTable::Comment("DefineMI") << MatchTable::IntValue(NewInsnVarID)
        << MatchTable::Comment("MI") << MatchTable::IntValue(InsnID)
        << MatchTable::Comment("OpIdx") << MatchTable::IntValue(OpIdx)
        << MatchTable::Comment("MIs[" + llvm::to_string(NewInsnVarID) + "]")
        << MatchTable::LineBreak;
  return NewInsnVarID;
}

unsigned RuleMatcher::getInsnVarID(const InstructionMatcher &InsnMatcher) const {
  const auto &I = InsnVariableIDs.find(&InsnMatcher);
  if (I != InsnVariableIDs.end())
    return I->second;
  llvm_unreachable("Matched Insn was not captured in a local variable");
}

void RuleMatcher::defineOperand(StringRef SymbolicName, OperandMatcher &OM) {
  if (DefinedOperands.find(SymbolicName) == DefinedOperands.end()) {
    DefinedOperands[SymbolicName] = &OM;
    return;
  }

  // If the operand is already defined, then we must ensure both references in
  // the matcher have the exact same node.
  OM.addPredicate<SameOperandMatcher>(OM.getSymbolicName());
}

const InstructionMatcher &
RuleMatcher::getInstructionMatcher(StringRef SymbolicName) const {
  for (const auto &I : InsnVariableIDs)
    if (I.first->getSymbolicName() == SymbolicName)
      return *I.first;
  llvm_unreachable(
      ("Failed to lookup instruction " + SymbolicName).str().c_str());
}

const OperandMatcher &
RuleMatcher::getOperandMatcher(StringRef Name) const {
  const auto &I = DefinedOperands.find(Name);

  if (I == DefinedOperands.end())
    PrintFatalError(SrcLoc, "Operand " + Name + " was not declared in matcher");

  return *I->second;
}

/// Emit MatchTable opcodes to check the shape of the match and capture
/// instructions into local variables.
void RuleMatcher::emitCaptureOpcodes(MatchTable &Table) {
  assert(Matchers.size() == 1 && "Cannot handle multi-root matchers yet");
  unsigned InsnVarID = implicitlyDefineInsnVar(*Matchers.front());
  Matchers.front()->emitCaptureOpcodes(Table, *this, InsnVarID);
}

void RuleMatcher::emit(MatchTable &Table) {
  if (Matchers.empty())
    llvm_unreachable("Unexpected empty matcher!");

  // The representation supports rules that require multiple roots such as:
  //    %ptr(p0) = ...
  //    %elt0(s32) = G_LOAD %ptr
  //    %1(p0) = G_ADD %ptr, 4
  //    %elt1(s32) = G_LOAD p0 %1
  // which could be usefully folded into:
  //    %ptr(p0) = ...
  //    %elt0(s32), %elt1(s32) = TGT_LOAD_PAIR %ptr
  // on some targets but we don't need to make use of that yet.
  assert(Matchers.size() == 1 && "Cannot handle multi-root matchers yet");

  unsigned LabelID = Table.allocateLabelID();
  Table << MatchTable::Opcode("GIM_Try", +1)
        << MatchTable::Comment("On fail goto") << MatchTable::JumpTarget(LabelID)
        << MatchTable::LineBreak;

  if (!RequiredFeatures.empty()) {
    Table << MatchTable::Opcode("GIM_CheckFeatures")
          << MatchTable::NamedValue(getNameForFeatureBitset(RequiredFeatures))
          << MatchTable::LineBreak;
  }

  emitCaptureOpcodes(Table);

  Matchers.front()->emitPredicateOpcodes(Table, *this,
                                         getInsnVarID(*Matchers.front()));

  // We must also check if it's safe to fold the matched instructions.
  if (InsnVariableIDs.size() >= 2) {
    // Invert the map to create stable ordering (by var names)
    SmallVector<unsigned, 2> InsnIDs;
    for (const auto &Pair : InsnVariableIDs) {
      // Skip the root node since it isn't moving anywhere. Everything else is
      // sinking to meet it.
      if (Pair.first == Matchers.front().get())
        continue;

      InsnIDs.push_back(Pair.second);
    }
    std::sort(InsnIDs.begin(), InsnIDs.end());

    for (const auto &InsnID : InsnIDs) {
      // Reject the difficult cases until we have a more accurate check.
      Table << MatchTable::Opcode("GIM_CheckIsSafeToFold")
            << MatchTable::Comment("InsnID") << MatchTable::IntValue(InsnID)
            << MatchTable::LineBreak;

      // FIXME: Emit checks to determine it's _actually_ safe to fold and/or
      //        account for unsafe cases.
      //
      //        Example:
      //          MI1--> %0 = ...
      //                 %1 = ... %0
      //          MI0--> %2 = ... %0
      //          It's not safe to erase MI1. We currently handle this by not
      //          erasing %0 (even when it's dead).
      //
      //        Example:
      //          MI1--> %0 = load volatile @a
      //                 %1 = load volatile @a
      //          MI0--> %2 = ... %0
      //          It's not safe to sink %0's def past %1. We currently handle
      //          this by rejecting all loads.
      //
      //        Example:
      //          MI1--> %0 = load @a
      //                 %1 = store @a
      //          MI0--> %2 = ... %0
      //          It's not safe to sink %0's def past %1. We currently handle
      //          this by rejecting all loads.
      //
      //        Example:
      //                   G_CONDBR %cond, @BB1
      //                 BB0:
      //          MI1-->   %0 = load @a
      //                   G_BR @BB1
      //                 BB1:
      //          MI0-->   %2 = ... %0
      //          It's not always safe to sink %0 across control flow. In this
      //          case it may introduce a memory fault. We currentl handle this
      //          by rejecting all loads.
    }
  }

  for (const auto &MA : Actions)
    MA->emitActionOpcodes(Table, *this);

  if (GenerateCoverage)
    Table << MatchTable::Opcode("GIR_Coverage") << MatchTable::IntValue(RuleID)
          << MatchTable::LineBreak;

  Table << MatchTable::Opcode("GIR_Done", -1) << MatchTable::LineBreak
        << MatchTable::Label(LabelID);
}

bool RuleMatcher::isHigherPriorityThan(const RuleMatcher &B) const {
  // Rules involving more match roots have higher priority.
  if (Matchers.size() > B.Matchers.size())
    return true;
  if (Matchers.size() < B.Matchers.size())
    return false;

  for (const auto &Matcher : zip(Matchers, B.Matchers)) {
    if (std::get<0>(Matcher)->isHigherPriorityThan(*std::get<1>(Matcher)))
      return true;
    if (std::get<1>(Matcher)->isHigherPriorityThan(*std::get<0>(Matcher)))
      return false;
  }

  return false;
}

unsigned RuleMatcher::countRendererFns() const {
  return std::accumulate(
      Matchers.begin(), Matchers.end(), 0,
      [](unsigned A, const std::unique_ptr<InstructionMatcher> &Matcher) {
        return A + Matcher->countRendererFns();
      });
}

bool OperandPredicateMatcher::isHigherPriorityThan(
    const OperandPredicateMatcher &B) const {
  // Generally speaking, an instruction is more important than an Int or a
  // LiteralInt because it can cover more nodes but theres an exception to
  // this. G_CONSTANT's are less important than either of those two because they
  // are more permissive.

  const InstructionOperandMatcher *AOM =
      dyn_cast<InstructionOperandMatcher>(this);
  const InstructionOperandMatcher *BOM =
      dyn_cast<InstructionOperandMatcher>(&B);
  bool AIsConstantInsn = AOM && AOM->getInsnMatcher().isConstantInstruction();
  bool BIsConstantInsn = BOM && BOM->getInsnMatcher().isConstantInstruction();

  if (AOM && BOM) {
    // The relative priorities between a G_CONSTANT and any other instruction
    // don't actually matter but this code is needed to ensure a strict weak
    // ordering. This is particularly important on Windows where the rules will
    // be incorrectly sorted without it.
    if (AIsConstantInsn != BIsConstantInsn)
      return AIsConstantInsn < BIsConstantInsn;
    return false;
  }

  if (AOM && AIsConstantInsn && (B.Kind == OPM_Int || B.Kind == OPM_LiteralInt))
    return false;
  if (BOM && BIsConstantInsn && (Kind == OPM_Int || Kind == OPM_LiteralInt))
    return true;

  return Kind < B.Kind;
}

void SameOperandMatcher::emitPredicateOpcodes(MatchTable &Table,
                                              RuleMatcher &Rule,
                                              unsigned InsnVarID,
                                              unsigned OpIdx) const {
  const OperandMatcher &OtherOM = Rule.getOperandMatcher(MatchingName);
  unsigned OtherInsnVarID = Rule.getInsnVarID(OtherOM.getInstructionMatcher());

  Table << MatchTable::Opcode("GIM_CheckIsSameOperand")
        << MatchTable::Comment("MI") << MatchTable::IntValue(InsnVarID)
        << MatchTable::Comment("OpIdx") << MatchTable::IntValue(OpIdx)
        << MatchTable::Comment("OtherMI")
        << MatchTable::IntValue(OtherInsnVarID)
        << MatchTable::Comment("OtherOpIdx")
        << MatchTable::IntValue(OtherOM.getOperandIndex())
        << MatchTable::LineBreak;
}

//===- GlobalISelEmitter class --------------------------------------------===//

class GlobalISelEmitter {
public:
  explicit GlobalISelEmitter(RecordKeeper &RK);
  void run(raw_ostream &OS);

private:
  const RecordKeeper &RK;
  const CodeGenDAGPatterns CGP;
  const CodeGenTarget &Target;
  CodeGenRegBank CGRegs;

  /// Keep track of the equivalence between SDNodes and Instruction by mapping
  /// SDNodes to the GINodeEquiv mapping. We need to map to the GINodeEquiv to
  /// check for attributes on the relation such as CheckMMOIsNonAtomic.
  /// This is defined using 'GINodeEquiv' in the target description.
  DenseMap<Record *, Record *> NodeEquivs;

  /// Keep track of the equivalence between ComplexPattern's and
  /// GIComplexOperandMatcher. Map entries are specified by subclassing
  /// GIComplexPatternEquiv.
  DenseMap<const Record *, const Record *> ComplexPatternEquivs;

  // Map of predicates to their subtarget features.
  SubtargetFeatureInfoMap SubtargetFeatures;

  // Rule coverage information.
  Optional<CodeGenCoverage> RuleCoverage;

  void gatherNodeEquivs();
  Record *findNodeEquiv(Record *N) const;

  Error importRulePredicates(RuleMatcher &M, ArrayRef<Predicate> Predicates);
  Expected<InstructionMatcher &> createAndImportSelDAGMatcher(
      RuleMatcher &Rule, InstructionMatcher &InsnMatcher,
      const TreePatternNode *Src, unsigned &TempOpIdx) const;
  Error importComplexPatternOperandMatcher(OperandMatcher &OM, Record *R,
                                           unsigned &TempOpIdx) const;
  Error importChildMatcher(RuleMatcher &Rule, InstructionMatcher &InsnMatcher,
                           const TreePatternNode *SrcChild,
                           bool OperandIsAPointer, unsigned OpIdx,
                           unsigned &TempOpIdx) const;

  Expected<BuildMIAction &>
  createAndImportInstructionRenderer(RuleMatcher &M,
                                     const TreePatternNode *Dst);
  Expected<action_iterator> createAndImportSubInstructionRenderer(
      action_iterator InsertPt, RuleMatcher &M, const TreePatternNode *Dst,
      unsigned TempReg);
  Expected<action_iterator>
  createInstructionRenderer(action_iterator InsertPt, RuleMatcher &M,
                            const TreePatternNode *Dst);
  void importExplicitDefRenderers(BuildMIAction &DstMIBuilder);
  Expected<action_iterator>
  importExplicitUseRenderers(action_iterator InsertPt, RuleMatcher &M,
                             BuildMIAction &DstMIBuilder,
                             const llvm::TreePatternNode *Dst);
  Expected<action_iterator>
  importExplicitUseRenderer(action_iterator InsertPt, RuleMatcher &Rule,
                            BuildMIAction &DstMIBuilder,
                            TreePatternNode *DstChild);
  Error importDefaultOperandRenderers(BuildMIAction &DstMIBuilder,
                                      DagInit *DefaultOps) const;
  Error
  importImplicitDefRenderers(BuildMIAction &DstMIBuilder,
                             const std::vector<Record *> &ImplicitDefs) const;

  void emitImmPredicates(raw_ostream &OS, StringRef TypeIdentifier,
                         StringRef Type,
                         std::function<bool(const Record *R)> Filter);

  /// Analyze pattern \p P, returning a matcher for it if possible.
  /// Otherwise, return an Error explaining why we don't support it.
  Expected<RuleMatcher> runOnPattern(const PatternToMatch &P);

  void declareSubtargetFeature(Record *Predicate);

  TreePatternNode *fixupPatternNode(TreePatternNode *N);
  void fixupPatternTrees(TreePattern *P);
};

void GlobalISelEmitter::gatherNodeEquivs() {
  assert(NodeEquivs.empty());
  for (Record *Equiv : RK.getAllDerivedDefinitions("GINodeEquiv"))
    NodeEquivs[Equiv->getValueAsDef("Node")] = Equiv;

  assert(ComplexPatternEquivs.empty());
  for (Record *Equiv : RK.getAllDerivedDefinitions("GIComplexPatternEquiv")) {
    Record *SelDAGEquiv = Equiv->getValueAsDef("SelDAGEquivalent");
    if (!SelDAGEquiv)
      continue;
    ComplexPatternEquivs[SelDAGEquiv] = Equiv;
 }
}

Record *GlobalISelEmitter::findNodeEquiv(Record *N) const {
  return NodeEquivs.lookup(N);
}

GlobalISelEmitter::GlobalISelEmitter(RecordKeeper &RK)
    : RK(RK), CGP(RK, [&](TreePattern *P) { fixupPatternTrees(P); }),
      Target(CGP.getTargetInfo()), CGRegs(RK, Target.getHwModes()) {}

//===- Emitter ------------------------------------------------------------===//

Error
GlobalISelEmitter::importRulePredicates(RuleMatcher &M,
                                        ArrayRef<Predicate> Predicates) {
  for (const Predicate &P : Predicates) {
    if (!P.Def)
      continue;
    declareSubtargetFeature(P.Def);
    M.addRequiredFeature(P.Def);
  }

  return Error::success();
}

Expected<InstructionMatcher &> GlobalISelEmitter::createAndImportSelDAGMatcher(
    RuleMatcher &Rule, InstructionMatcher &InsnMatcher,
    const TreePatternNode *Src, unsigned &TempOpIdx) const {
  Record *SrcGIEquivOrNull = nullptr;
  const CodeGenInstruction *SrcGIOrNull = nullptr;

  // Start with the defined operands (i.e., the results of the root operator).
  if (Src->getExtTypes().size() > 1)
    return failedImport("Src pattern has multiple results");

  if (Src->isLeaf()) {
    Init *SrcInit = Src->getLeafValue();
    if (isa<IntInit>(SrcInit)) {
      InsnMatcher.addPredicate<InstructionOpcodeMatcher>(
          &Target.getInstruction(RK.getDef("G_CONSTANT")));
    } else
      return failedImport(
          "Unable to deduce gMIR opcode to handle Src (which is a leaf)");
  } else {
    SrcGIEquivOrNull = findNodeEquiv(Src->getOperator());
    if (!SrcGIEquivOrNull)
      return failedImport("Pattern operator lacks an equivalent Instruction" +
                          explainOperator(Src->getOperator()));
    SrcGIOrNull = &Target.getInstruction(SrcGIEquivOrNull->getValueAsDef("I"));

    // The operators look good: match the opcode
    InsnMatcher.addPredicate<InstructionOpcodeMatcher>(SrcGIOrNull);
  }

  unsigned OpIdx = 0;
  for (const TypeSetByHwMode &VTy : Src->getExtTypes()) {
    // Results don't have a name unless they are the root node. The caller will
    // set the name if appropriate.
    OperandMatcher &OM = InsnMatcher.addOperand(OpIdx++, "", TempOpIdx);
    if (auto Error = OM.addTypeCheckPredicate(VTy, false /* OperandIsAPointer */))
      return failedImport(toString(std::move(Error)) +
                          " for result of Src pattern operator");
  }

  for (const auto &Predicate : Src->getPredicateFns()) {
    if (Predicate.isAlwaysTrue())
      continue;

    if (Predicate.isImmediatePattern()) {
      InsnMatcher.addPredicate<InstructionImmPredicateMatcher>(Predicate);
      continue;
    }

    // No check required. G_LOAD by itself is a non-extending load.
    if (Predicate.isNonExtLoad())
      continue;

    // No check required. G_STORE by itself is a non-extending store.
    if (Predicate.isNonTruncStore())
      continue;

    if (Predicate.isLoad() || Predicate.isStore() || Predicate.isAtomic()) {
      if (Predicate.getMemoryVT() != nullptr) {
        Optional<LLTCodeGen> MemTyOrNone =
            MVTToLLT(getValueType(Predicate.getMemoryVT()));

        if (!MemTyOrNone)
          return failedImport("MemVT could not be converted to LLT");

        InsnMatcher.getOperand(0).addPredicate<LLTOperandMatcher>(
            MemTyOrNone.getValue());
        continue;
      }
    }

    if (Predicate.isLoad() || Predicate.isStore()) {
      // No check required. A G_LOAD/G_STORE is an unindexed load.
      if (Predicate.isUnindexed())
        continue;
    }

    if (Predicate.isAtomic()) {
      if (Predicate.isAtomicOrderingMonotonic()) {
        InsnMatcher.addPredicate<AtomicOrderingMMOPredicateMatcher>(
            "Monotonic");
        continue;
      }
      if (Predicate.isAtomicOrderingAcquire()) {
        InsnMatcher.addPredicate<AtomicOrderingMMOPredicateMatcher>("Acquire");
        continue;
      }
      if (Predicate.isAtomicOrderingRelease()) {
        InsnMatcher.addPredicate<AtomicOrderingMMOPredicateMatcher>("Release");
        continue;
      }
      if (Predicate.isAtomicOrderingAcquireRelease()) {
        InsnMatcher.addPredicate<AtomicOrderingMMOPredicateMatcher>(
            "AcquireRelease");
        continue;
      }
      if (Predicate.isAtomicOrderingSequentiallyConsistent()) {
        InsnMatcher.addPredicate<AtomicOrderingMMOPredicateMatcher>(
            "SequentiallyConsistent");
        continue;
      }

      if (Predicate.isAtomicOrderingAcquireOrStronger()) {
        InsnMatcher.addPredicate<AtomicOrderingMMOPredicateMatcher>(
            "Acquire", AtomicOrderingMMOPredicateMatcher::AO_OrStronger);
        continue;
      }
      if (Predicate.isAtomicOrderingWeakerThanAcquire()) {
        InsnMatcher.addPredicate<AtomicOrderingMMOPredicateMatcher>(
            "Acquire", AtomicOrderingMMOPredicateMatcher::AO_WeakerThan);
        continue;
      }

      if (Predicate.isAtomicOrderingReleaseOrStronger()) {
        InsnMatcher.addPredicate<AtomicOrderingMMOPredicateMatcher>(
            "Release", AtomicOrderingMMOPredicateMatcher::AO_OrStronger);
        continue;
      }
      if (Predicate.isAtomicOrderingWeakerThanRelease()) {
        InsnMatcher.addPredicate<AtomicOrderingMMOPredicateMatcher>(
            "Release", AtomicOrderingMMOPredicateMatcher::AO_WeakerThan);
        continue;
      }
    }

    return failedImport("Src pattern child has predicate (" +
                        explainPredicates(Src) + ")");
  }
  if (SrcGIEquivOrNull && SrcGIEquivOrNull->getValueAsBit("CheckMMOIsNonAtomic"))
    InsnMatcher.addPredicate<AtomicOrderingMMOPredicateMatcher>("NotAtomic");

  if (Src->isLeaf()) {
    Init *SrcInit = Src->getLeafValue();
    if (IntInit *SrcIntInit = dyn_cast<IntInit>(SrcInit)) {
      OperandMatcher &OM =
          InsnMatcher.addOperand(OpIdx++, Src->getName(), TempOpIdx);
      OM.addPredicate<LiteralIntOperandMatcher>(SrcIntInit->getValue());
    } else
      return failedImport(
          "Unable to deduce gMIR opcode to handle Src (which is a leaf)");
  } else {
    assert(SrcGIOrNull &&
           "Expected to have already found an equivalent Instruction");
    if (SrcGIOrNull->TheDef->getName() == "G_CONSTANT" ||
        SrcGIOrNull->TheDef->getName() == "G_FCONSTANT") {
      // imm/fpimm still have operands but we don't need to do anything with it
      // here since we don't support ImmLeaf predicates yet. However, we still
      // need to note the hidden operand to get GIM_CheckNumOperands correct.
      InsnMatcher.addOperand(OpIdx++, "", TempOpIdx);
      return InsnMatcher;
    }

    // Match the used operands (i.e. the children of the operator).
    for (unsigned i = 0, e = Src->getNumChildren(); i != e; ++i) {
      TreePatternNode *SrcChild = Src->getChild(i);

      // SelectionDAG allows pointers to be represented with iN since it doesn't
      // distinguish between pointers and integers but they are different types in GlobalISel.
      // Coerce integers to pointers to address space 0 if the context indicates a pointer.
      bool OperandIsAPointer = SrcGIOrNull->isOperandAPointer(i);

      // For G_INTRINSIC/G_INTRINSIC_W_SIDE_EFFECTS, the operand immediately
      // following the defs is an intrinsic ID.
      if ((SrcGIOrNull->TheDef->getName() == "G_INTRINSIC" ||
           SrcGIOrNull->TheDef->getName() == "G_INTRINSIC_W_SIDE_EFFECTS") &&
          i == 0) {
        if (const CodeGenIntrinsic *II = Src->getIntrinsicInfo(CGP)) {
          OperandMatcher &OM =
              InsnMatcher.addOperand(OpIdx++, SrcChild->getName(), TempOpIdx);
          OM.addPredicate<IntrinsicIDOperandMatcher>(II);
          continue;
        }

        return failedImport("Expected IntInit containing instrinsic ID)");
      }

      if (auto Error =
              importChildMatcher(Rule, InsnMatcher, SrcChild, OperandIsAPointer,
                                 OpIdx++, TempOpIdx))
        return std::move(Error);
    }
  }

  return InsnMatcher;
}

Error GlobalISelEmitter::importComplexPatternOperandMatcher(
    OperandMatcher &OM, Record *R, unsigned &TempOpIdx) const {
  const auto &ComplexPattern = ComplexPatternEquivs.find(R);
  if (ComplexPattern == ComplexPatternEquivs.end())
    return failedImport("SelectionDAG ComplexPattern (" + R->getName() +
                        ") not mapped to GlobalISel");

  OM.addPredicate<ComplexPatternOperandMatcher>(OM, *ComplexPattern->second);
  TempOpIdx++;
  return Error::success();
}

Error GlobalISelEmitter::importChildMatcher(RuleMatcher &Rule,
                                            InstructionMatcher &InsnMatcher,
                                            const TreePatternNode *SrcChild,
                                            bool OperandIsAPointer,
                                            unsigned OpIdx,
                                            unsigned &TempOpIdx) const {
  OperandMatcher &OM =
      InsnMatcher.addOperand(OpIdx, SrcChild->getName(), TempOpIdx);
  if (OM.isSameAsAnotherOperand())
    return Error::success();

  ArrayRef<TypeSetByHwMode> ChildTypes = SrcChild->getExtTypes();
  if (ChildTypes.size() != 1)
    return failedImport("Src pattern child has multiple results");

  // Check MBB's before the type check since they are not a known type.
  if (!SrcChild->isLeaf()) {
    if (SrcChild->getOperator()->isSubClassOf("SDNode")) {
      auto &ChildSDNI = CGP.getSDNodeInfo(SrcChild->getOperator());
      if (ChildSDNI.getSDClassName() == "BasicBlockSDNode") {
        OM.addPredicate<MBBOperandMatcher>();
        return Error::success();
      }
    }
  }

  if (auto Error =
          OM.addTypeCheckPredicate(ChildTypes.front(), OperandIsAPointer))
    return failedImport(toString(std::move(Error)) + " for Src operand (" +
                        to_string(*SrcChild) + ")");

  // Check for nested instructions.
  if (!SrcChild->isLeaf()) {
    if (SrcChild->getOperator()->isSubClassOf("ComplexPattern")) {
      // When a ComplexPattern is used as an operator, it should do the same
      // thing as when used as a leaf. However, the children of the operator
      // name the sub-operands that make up the complex operand and we must
      // prepare to reference them in the renderer too.
      unsigned RendererID = TempOpIdx;
      if (auto Error = importComplexPatternOperandMatcher(
              OM, SrcChild->getOperator(), TempOpIdx))
        return Error;

      for (unsigned i = 0, e = SrcChild->getNumChildren(); i != e; ++i) {
        auto *SubOperand = SrcChild->getChild(i);
        if (!SubOperand->getName().empty())
          Rule.defineComplexSubOperand(SubOperand->getName(),
                                       SrcChild->getOperator(), RendererID, i);
      }

      return Error::success();
    }

    auto MaybeInsnOperand = OM.addPredicate<InstructionOperandMatcher>(
        InsnMatcher.getRuleMatcher(), SrcChild->getName());
    if (!MaybeInsnOperand.hasValue()) {
      // This isn't strictly true. If the user were to provide exactly the same
      // matchers as the original operand then we could allow it. However, it's
      // simpler to not permit the redundant specification.
      return failedImport("Nested instruction cannot be the same as another operand");
    }

    // Map the node to a gMIR instruction.
    InstructionOperandMatcher &InsnOperand = **MaybeInsnOperand;
    auto InsnMatcherOrError = createAndImportSelDAGMatcher(
        Rule, InsnOperand.getInsnMatcher(), SrcChild, TempOpIdx);
    if (auto Error = InsnMatcherOrError.takeError())
      return Error;

    return Error::success();
  }

  if (SrcChild->hasAnyPredicate())
    return failedImport("Src pattern child has unsupported predicate");

  // Check for constant immediates.
  if (auto *ChildInt = dyn_cast<IntInit>(SrcChild->getLeafValue())) {
    OM.addPredicate<ConstantIntOperandMatcher>(ChildInt->getValue());
    return Error::success();
  }

  // Check for def's like register classes or ComplexPattern's.
  if (auto *ChildDefInit = dyn_cast<DefInit>(SrcChild->getLeafValue())) {
    auto *ChildRec = ChildDefInit->getDef();

    // Check for register classes.
    if (ChildRec->isSubClassOf("RegisterClass") ||
        ChildRec->isSubClassOf("RegisterOperand")) {
      OM.addPredicate<RegisterBankOperandMatcher>(
          Target.getRegisterClass(getInitValueAsRegClass(ChildDefInit)));
      return Error::success();
    }

    // Check for ValueType.
    if (ChildRec->isSubClassOf("ValueType")) {
      // We already added a type check as standard practice so this doesn't need
      // to do anything.
      return Error::success();
    }

    // Check for ComplexPattern's.
    if (ChildRec->isSubClassOf("ComplexPattern"))
      return importComplexPatternOperandMatcher(OM, ChildRec, TempOpIdx);

    if (ChildRec->isSubClassOf("ImmLeaf")) {
      return failedImport(
          "Src pattern child def is an unsupported tablegen class (ImmLeaf)");
    }

    return failedImport(
        "Src pattern child def is an unsupported tablegen class");
  }

  return failedImport("Src pattern child is an unsupported kind");
}

Expected<action_iterator> GlobalISelEmitter::importExplicitUseRenderer(
    action_iterator InsertPt, RuleMatcher &Rule, BuildMIAction &DstMIBuilder,
    TreePatternNode *DstChild) {
  if (DstChild->getTransformFn() != nullptr) {
    return failedImport("Dst pattern child has transform fn " +
                        DstChild->getTransformFn()->getName());
  }

  const auto &SubOperand = Rule.getComplexSubOperand(DstChild->getName());
  if (SubOperand.hasValue()) {
    DstMIBuilder.addRenderer<RenderComplexPatternOperand>(
        *std::get<0>(*SubOperand), DstChild->getName(),
        std::get<1>(*SubOperand), std::get<2>(*SubOperand));
    return InsertPt;
  }

  if (!DstChild->isLeaf()) {
    // We accept 'bb' here. It's an operator because BasicBlockSDNode isn't
    // inline, but in MI it's just another operand.
    if (DstChild->getOperator()->isSubClassOf("SDNode")) {
      auto &ChildSDNI = CGP.getSDNodeInfo(DstChild->getOperator());
      if (ChildSDNI.getSDClassName() == "BasicBlockSDNode") {
        DstMIBuilder.addRenderer<CopyRenderer>(DstChild->getName());
        return InsertPt;
      }
    }

    // Similarly, imm is an operator in TreePatternNode's view but must be
    // rendered as operands.
    // FIXME: The target should be able to choose sign-extended when appropriate
    //        (e.g. on Mips).
    if (DstChild->getOperator()->getName() == "imm") {
      DstMIBuilder.addRenderer<CopyConstantAsImmRenderer>(DstChild->getName());
      return InsertPt;
    } else if (DstChild->getOperator()->getName() == "fpimm") {
      DstMIBuilder.addRenderer<CopyFConstantAsFPImmRenderer>(
          DstChild->getName());
      return InsertPt;
    }

    if (DstChild->getOperator()->isSubClassOf("Instruction")) {
      ArrayRef<TypeSetByHwMode> ChildTypes = DstChild->getExtTypes();
      if (ChildTypes.size() != 1)
        return failedImport("Dst pattern child has multiple results");

      Optional<LLTCodeGen> OpTyOrNone = None;
      if (ChildTypes.front().isMachineValueType())
        OpTyOrNone =
            MVTToLLT(ChildTypes.front().getMachineValueType().SimpleTy);
      if (!OpTyOrNone)
        return failedImport("Dst operand has an unsupported type");

      unsigned TempRegID = Rule.allocateTempRegID();
      InsertPt = Rule.insertAction<MakeTempRegisterAction>(
          InsertPt, OpTyOrNone.getValue(), TempRegID);
      DstMIBuilder.addRenderer<TempRegRenderer>(TempRegID);

      auto InsertPtOrError = createAndImportSubInstructionRenderer(
          ++InsertPt, Rule, DstChild, TempRegID);
      if (auto Error = InsertPtOrError.takeError())
        return std::move(Error);
      return InsertPtOrError.get();
    }

    return failedImport("Dst pattern child isn't a leaf node or an MBB" + llvm::to_string(*DstChild));
  }

  // It could be a specific immediate in which case we should just check for
  // that immediate.
  if (const IntInit *ChildIntInit =
          dyn_cast<IntInit>(DstChild->getLeafValue())) {
    DstMIBuilder.addRenderer<ImmRenderer>(ChildIntInit->getValue());
    return InsertPt;
  }

  // Otherwise, we're looking for a bog-standard RegisterClass operand.
  if (auto *ChildDefInit = dyn_cast<DefInit>(DstChild->getLeafValue())) {
    auto *ChildRec = ChildDefInit->getDef();

    ArrayRef<TypeSetByHwMode> ChildTypes = DstChild->getExtTypes();
    if (ChildTypes.size() != 1)
      return failedImport("Dst pattern child has multiple results");

    Optional<LLTCodeGen> OpTyOrNone = None;
    if (ChildTypes.front().isMachineValueType())
      OpTyOrNone = MVTToLLT(ChildTypes.front().getMachineValueType().SimpleTy);
    if (!OpTyOrNone)
      return failedImport("Dst operand has an unsupported type");

    if (ChildRec->isSubClassOf("Register")) {
      DstMIBuilder.addRenderer<AddRegisterRenderer>(ChildRec);
      return InsertPt;
    }

    if (ChildRec->isSubClassOf("RegisterClass") ||
        ChildRec->isSubClassOf("RegisterOperand") ||
        ChildRec->isSubClassOf("ValueType")) {
      if (ChildRec->isSubClassOf("RegisterOperand") &&
          !ChildRec->isValueUnset("GIZeroRegister")) {
        DstMIBuilder.addRenderer<CopyOrAddZeroRegRenderer>(
            DstChild->getName(), ChildRec->getValueAsDef("GIZeroRegister"));
        return InsertPt;
      }

      DstMIBuilder.addRenderer<CopyRenderer>(DstChild->getName());
      return InsertPt;
    }

    if (ChildRec->isSubClassOf("ComplexPattern")) {
      const auto &ComplexPattern = ComplexPatternEquivs.find(ChildRec);
      if (ComplexPattern == ComplexPatternEquivs.end())
        return failedImport(
            "SelectionDAG ComplexPattern not mapped to GlobalISel");

      const OperandMatcher &OM = Rule.getOperandMatcher(DstChild->getName());
      DstMIBuilder.addRenderer<RenderComplexPatternOperand>(
          *ComplexPattern->second, DstChild->getName(),
          OM.getAllocatedTemporariesBaseID());
      return InsertPt;
    }

    if (ChildRec->isSubClassOf("SDNodeXForm"))
      return failedImport("Dst pattern child def is an unsupported tablegen "
                          "class (SDNodeXForm)");

    return failedImport(
        "Dst pattern child def is an unsupported tablegen class");
  }

  return failedImport("Dst pattern child is an unsupported kind");
}

Expected<BuildMIAction &> GlobalISelEmitter::createAndImportInstructionRenderer(
    RuleMatcher &M, const TreePatternNode *Dst) {
  auto InsertPtOrError = createInstructionRenderer(M.actions_end(), M, Dst);
  if (auto Error = InsertPtOrError.takeError())
    return std::move(Error);

  action_iterator InsertPt = InsertPtOrError.get();
  BuildMIAction &DstMIBuilder = *static_cast<BuildMIAction *>(InsertPt->get());

  importExplicitDefRenderers(DstMIBuilder);

  if (auto Error = importExplicitUseRenderers(InsertPt, M, DstMIBuilder, Dst)
                       .takeError())
    return std::move(Error);

  return DstMIBuilder;
}

Expected<action_iterator>
GlobalISelEmitter::createAndImportSubInstructionRenderer(
    action_iterator InsertPt, RuleMatcher &M, const TreePatternNode *Dst,
    unsigned TempRegID) {
  auto InsertPtOrError = createInstructionRenderer(InsertPt, M, Dst);

  // TODO: Assert there's exactly one result.

  if (auto Error = InsertPtOrError.takeError())
    return std::move(Error);
  InsertPt = InsertPtOrError.get();

  BuildMIAction &DstMIBuilder =
      *static_cast<BuildMIAction *>(InsertPtOrError.get()->get());

  // Assign the result to TempReg.
  DstMIBuilder.addRenderer<TempRegRenderer>(TempRegID, true);

  InsertPtOrError = importExplicitUseRenderers(InsertPt, M, DstMIBuilder, Dst);
  if (auto Error = InsertPtOrError.takeError())
    return std::move(Error);

  return InsertPtOrError.get();
}

Expected<action_iterator> GlobalISelEmitter::createInstructionRenderer(
    action_iterator InsertPt, RuleMatcher &M, const TreePatternNode *Dst) {
  Record *DstOp = Dst->getOperator();
  if (!DstOp->isSubClassOf("Instruction")) {
    if (DstOp->isSubClassOf("ValueType"))
      return failedImport(
          "Pattern operator isn't an instruction (it's a ValueType)");
    return failedImport("Pattern operator isn't an instruction");
  }
  CodeGenInstruction *DstI = &Target.getInstruction(DstOp);

  // COPY_TO_REGCLASS is just a copy with a ConstrainOperandToRegClassAction
  // attached. Similarly for EXTRACT_SUBREG except that's a subregister copy.
  if (DstI->TheDef->getName() == "COPY_TO_REGCLASS")
    DstI = &Target.getInstruction(RK.getDef("COPY"));
  else if (DstI->TheDef->getName() == "EXTRACT_SUBREG")
    DstI = &Target.getInstruction(RK.getDef("COPY"));
  else if (DstI->TheDef->getName() == "REG_SEQUENCE")
    return failedImport("Unable to emit REG_SEQUENCE");

  return M.insertAction<BuildMIAction>(InsertPt, M.allocateOutputInsnID(),
                                       DstI);
}

void GlobalISelEmitter::importExplicitDefRenderers(
    BuildMIAction &DstMIBuilder) {
  const CodeGenInstruction *DstI = DstMIBuilder.getCGI();
  for (unsigned I = 0; I < DstI->Operands.NumDefs; ++I) {
    const CGIOperandList::OperandInfo &DstIOperand = DstI->Operands[I];
    DstMIBuilder.addRenderer<CopyRenderer>(DstIOperand.Name);
  }
}

Expected<action_iterator> GlobalISelEmitter::importExplicitUseRenderers(
    action_iterator InsertPt, RuleMatcher &M, BuildMIAction &DstMIBuilder,
    const llvm::TreePatternNode *Dst) {
  const CodeGenInstruction *DstI = DstMIBuilder.getCGI();
  CodeGenInstruction *OrigDstI = &Target.getInstruction(Dst->getOperator());

  // EXTRACT_SUBREG needs to use a subregister COPY.
  if (OrigDstI->TheDef->getName() == "EXTRACT_SUBREG") {
    if (!Dst->getChild(0)->isLeaf())
      return failedImport("EXTRACT_SUBREG child #1 is not a leaf");

    if (DefInit *SubRegInit =
            dyn_cast<DefInit>(Dst->getChild(1)->getLeafValue())) {
      Record *RCDef = getInitValueAsRegClass(Dst->getChild(0)->getLeafValue());
      if (!RCDef)
        return failedImport("EXTRACT_SUBREG child #0 could not "
                            "be coerced to a register class");

      CodeGenRegisterClass *RC = CGRegs.getRegClass(RCDef);
      CodeGenSubRegIndex *SubIdx = CGRegs.getSubRegIdx(SubRegInit->getDef());

      const auto &SrcRCDstRCPair =
          RC->getMatchingSubClassWithSubRegs(CGRegs, SubIdx);
      if (SrcRCDstRCPair.hasValue()) {
        assert(SrcRCDstRCPair->second && "Couldn't find a matching subclass");
        if (SrcRCDstRCPair->first != RC)
          return failedImport("EXTRACT_SUBREG requires an additional COPY");
      }

      DstMIBuilder.addRenderer<CopySubRegRenderer>(Dst->getChild(0)->getName(),
                                                   SubIdx);
      return InsertPt;
    }

    return failedImport("EXTRACT_SUBREG child #1 is not a subreg index");
  }

  // Render the explicit uses.
  unsigned DstINumUses = OrigDstI->Operands.size() - OrigDstI->Operands.NumDefs;
  unsigned ExpectedDstINumUses = Dst->getNumChildren();
  if (OrigDstI->TheDef->getName() == "COPY_TO_REGCLASS") {
    DstINumUses--; // Ignore the class constraint.
    ExpectedDstINumUses--;
  }

  unsigned Child = 0;
  unsigned NumDefaultOps = 0;
  for (unsigned I = 0; I != DstINumUses; ++I) {
    const CGIOperandList::OperandInfo &DstIOperand =
        DstI->Operands[DstI->Operands.NumDefs + I];

    // If the operand has default values, introduce them now.
    // FIXME: Until we have a decent test case that dictates we should do
    // otherwise, we're going to assume that operands with default values cannot
    // be specified in the patterns. Therefore, adding them will not cause us to
    // end up with too many rendered operands.
    if (DstIOperand.Rec->isSubClassOf("OperandWithDefaultOps")) {
      DagInit *DefaultOps = DstIOperand.Rec->getValueAsDag("DefaultOps");
      if (auto Error = importDefaultOperandRenderers(DstMIBuilder, DefaultOps))
        return std::move(Error);
      ++NumDefaultOps;
      continue;
    }

    auto InsertPtOrError = importExplicitUseRenderer(InsertPt, M, DstMIBuilder,
                                                     Dst->getChild(Child));
    if (auto Error = InsertPtOrError.takeError())
      return std::move(Error);
    InsertPt = InsertPtOrError.get();
    ++Child;
  }

  if (NumDefaultOps + ExpectedDstINumUses != DstINumUses)
    return failedImport("Expected " + llvm::to_string(DstINumUses) +
                        " used operands but found " +
                        llvm::to_string(ExpectedDstINumUses) +
                        " explicit ones and " + llvm::to_string(NumDefaultOps) +
                        " default ones");

  return InsertPt;
}

Error GlobalISelEmitter::importDefaultOperandRenderers(
    BuildMIAction &DstMIBuilder, DagInit *DefaultOps) const {
  for (const auto *DefaultOp : DefaultOps->getArgs()) {
    // Look through ValueType operators.
    if (const DagInit *DefaultDagOp = dyn_cast<DagInit>(DefaultOp)) {
      if (const DefInit *DefaultDagOperator =
              dyn_cast<DefInit>(DefaultDagOp->getOperator())) {
        if (DefaultDagOperator->getDef()->isSubClassOf("ValueType"))
          DefaultOp = DefaultDagOp->getArg(0);
      }
    }

    if (const DefInit *DefaultDefOp = dyn_cast<DefInit>(DefaultOp)) {
      DstMIBuilder.addRenderer<AddRegisterRenderer>(DefaultDefOp->getDef());
      continue;
    }

    if (const IntInit *DefaultIntOp = dyn_cast<IntInit>(DefaultOp)) {
      DstMIBuilder.addRenderer<ImmRenderer>(DefaultIntOp->getValue());
      continue;
    }

    return failedImport("Could not add default op");
  }

  return Error::success();
}

Error GlobalISelEmitter::importImplicitDefRenderers(
    BuildMIAction &DstMIBuilder,
    const std::vector<Record *> &ImplicitDefs) const {
  if (!ImplicitDefs.empty())
    return failedImport("Pattern defines a physical register");
  return Error::success();
}

Expected<RuleMatcher> GlobalISelEmitter::runOnPattern(const PatternToMatch &P) {
  // Keep track of the matchers and actions to emit.
  RuleMatcher M(P.getSrcRecord()->getLoc());
  M.addAction<DebugCommentAction>(llvm::to_string(*P.getSrcPattern()) +
                                  "  =>  " +
                                  llvm::to_string(*P.getDstPattern()));

  if (auto Error = importRulePredicates(M, P.getPredicates()))
    return std::move(Error);

  // Next, analyze the pattern operators.
  TreePatternNode *Src = P.getSrcPattern();
  TreePatternNode *Dst = P.getDstPattern();

  // If the root of either pattern isn't a simple operator, ignore it.
  if (auto Err = isTrivialOperatorNode(Dst))
    return failedImport("Dst pattern root isn't a trivial operator (" +
                        toString(std::move(Err)) + ")");
  if (auto Err = isTrivialOperatorNode(Src))
    return failedImport("Src pattern root isn't a trivial operator (" +
                        toString(std::move(Err)) + ")");

  InstructionMatcher &InsnMatcherTemp = M.addInstructionMatcher(Src->getName());
  unsigned TempOpIdx = 0;
  auto InsnMatcherOrError =
      createAndImportSelDAGMatcher(M, InsnMatcherTemp, Src, TempOpIdx);
  if (auto Error = InsnMatcherOrError.takeError())
    return std::move(Error);
  InstructionMatcher &InsnMatcher = InsnMatcherOrError.get();

  if (Dst->isLeaf()) {
    Record *RCDef = getInitValueAsRegClass(Dst->getLeafValue());

    const CodeGenRegisterClass &RC = Target.getRegisterClass(RCDef);
    if (RCDef) {
      // We need to replace the def and all its uses with the specified
      // operand. However, we must also insert COPY's wherever needed.
      // For now, emit a copy and let the register allocator clean up.
      auto &DstI = Target.getInstruction(RK.getDef("COPY"));
      const auto &DstIOperand = DstI.Operands[0];

      OperandMatcher &OM0 = InsnMatcher.getOperand(0);
      OM0.setSymbolicName(DstIOperand.Name);
      M.defineOperand(OM0.getSymbolicName(), OM0);
      OM0.addPredicate<RegisterBankOperandMatcher>(RC);

      auto &DstMIBuilder =
          M.addAction<BuildMIAction>(M.allocateOutputInsnID(), &DstI);
      DstMIBuilder.addRenderer<CopyRenderer>(DstIOperand.Name);
      DstMIBuilder.addRenderer<CopyRenderer>(Dst->getName());
      M.addAction<ConstrainOperandToRegClassAction>(0, 0, RC);

      // We're done with this pattern!  It's eligible for GISel emission; return
      // it.
      ++NumPatternImported;
      return std::move(M);
    }

    return failedImport("Dst pattern root isn't a known leaf");
  }

  // Start with the defined operands (i.e., the results of the root operator).
  Record *DstOp = Dst->getOperator();
  if (!DstOp->isSubClassOf("Instruction"))
    return failedImport("Pattern operator isn't an instruction");

  auto &DstI = Target.getInstruction(DstOp);
  if (DstI.Operands.NumDefs != Src->getExtTypes().size())
    return failedImport("Src pattern results and dst MI defs are different (" +
                        to_string(Src->getExtTypes().size()) + " def(s) vs " +
                        to_string(DstI.Operands.NumDefs) + " def(s))");

  // The root of the match also has constraints on the register bank so that it
  // matches the result instruction.
  unsigned OpIdx = 0;
  for (const TypeSetByHwMode &VTy : Src->getExtTypes()) {
    (void)VTy;

    const auto &DstIOperand = DstI.Operands[OpIdx];
    Record *DstIOpRec = DstIOperand.Rec;
    if (DstI.TheDef->getName() == "COPY_TO_REGCLASS") {
      DstIOpRec = getInitValueAsRegClass(Dst->getChild(1)->getLeafValue());

      if (DstIOpRec == nullptr)
        return failedImport(
            "COPY_TO_REGCLASS operand #1 isn't a register class");
    } else if (DstI.TheDef->getName() == "EXTRACT_SUBREG") {
      if (!Dst->getChild(0)->isLeaf())
        return failedImport("EXTRACT_SUBREG operand #0 isn't a leaf");

      // We can assume that a subregister is in the same bank as it's super
      // register.
      DstIOpRec = getInitValueAsRegClass(Dst->getChild(0)->getLeafValue());

      if (DstIOpRec == nullptr)
        return failedImport(
            "EXTRACT_SUBREG operand #0 isn't a register class");
    } else if (DstIOpRec->isSubClassOf("RegisterOperand"))
      DstIOpRec = DstIOpRec->getValueAsDef("RegClass");
    else if (!DstIOpRec->isSubClassOf("RegisterClass"))
      return failedImport("Dst MI def isn't a register class" +
                          to_string(*Dst));

    OperandMatcher &OM = InsnMatcher.getOperand(OpIdx);
    OM.setSymbolicName(DstIOperand.Name);
    M.defineOperand(OM.getSymbolicName(), OM);
    OM.addPredicate<RegisterBankOperandMatcher>(
        Target.getRegisterClass(DstIOpRec));
    ++OpIdx;
  }

  auto DstMIBuilderOrError = createAndImportInstructionRenderer(M, Dst);
  if (auto Error = DstMIBuilderOrError.takeError())
    return std::move(Error);
  BuildMIAction &DstMIBuilder = DstMIBuilderOrError.get();

  // Render the implicit defs.
  // These are only added to the root of the result.
  if (auto Error = importImplicitDefRenderers(DstMIBuilder, P.getDstRegs()))
    return std::move(Error);

  DstMIBuilder.chooseInsnToMutate(M);

  // Constrain the registers to classes. This is normally derived from the
  // emitted instruction but a few instructions require special handling.
  if (DstI.TheDef->getName() == "COPY_TO_REGCLASS") {
    // COPY_TO_REGCLASS does not provide operand constraints itself but the
    // result is constrained to the class given by the second child.
    Record *DstIOpRec =
        getInitValueAsRegClass(Dst->getChild(1)->getLeafValue());

    if (DstIOpRec == nullptr)
      return failedImport("COPY_TO_REGCLASS operand #1 isn't a register class");

    M.addAction<ConstrainOperandToRegClassAction>(
        0, 0, Target.getRegisterClass(DstIOpRec));

    // We're done with this pattern!  It's eligible for GISel emission; return
    // it.
    ++NumPatternImported;
    return std::move(M);
  }

  if (DstI.TheDef->getName() == "EXTRACT_SUBREG") {
    // EXTRACT_SUBREG selects into a subregister COPY but unlike most
    // instructions, the result register class is controlled by the
    // subregisters of the operand. As a result, we must constrain the result
    // class rather than check that it's already the right one.
    if (!Dst->getChild(0)->isLeaf())
      return failedImport("EXTRACT_SUBREG child #1 is not a leaf");

    DefInit *SubRegInit = dyn_cast<DefInit>(Dst->getChild(1)->getLeafValue());
    if (!SubRegInit)
      return failedImport("EXTRACT_SUBREG child #1 is not a subreg index");

    // Constrain the result to the same register bank as the operand.
    Record *DstIOpRec =
        getInitValueAsRegClass(Dst->getChild(0)->getLeafValue());

    if (DstIOpRec == nullptr)
      return failedImport("EXTRACT_SUBREG operand #1 isn't a register class");

    CodeGenSubRegIndex *SubIdx = CGRegs.getSubRegIdx(SubRegInit->getDef());
    CodeGenRegisterClass *SrcRC = CGRegs.getRegClass(DstIOpRec);

    // It would be nice to leave this constraint implicit but we're required
    // to pick a register class so constrain the result to a register class
    // that can hold the correct MVT.
    //
    // FIXME: This may introduce an extra copy if the chosen class doesn't
    //        actually contain the subregisters.
    assert(Src->getExtTypes().size() == 1 &&
             "Expected Src of EXTRACT_SUBREG to have one result type");

    const auto &SrcRCDstRCPair =
        SrcRC->getMatchingSubClassWithSubRegs(CGRegs, SubIdx);
    assert(SrcRCDstRCPair->second && "Couldn't find a matching subclass");
    M.addAction<ConstrainOperandToRegClassAction>(0, 0, *SrcRCDstRCPair->second);
    M.addAction<ConstrainOperandToRegClassAction>(0, 1, *SrcRCDstRCPair->first);

    // We're done with this pattern!  It's eligible for GISel emission; return
    // it.
    ++NumPatternImported;
    return std::move(M);
  }

  M.addAction<ConstrainOperandsToDefinitionAction>(0);

  // We're done with this pattern!  It's eligible for GISel emission; return it.
  ++NumPatternImported;
  return std::move(M);
}

// Emit imm predicate table and an enum to reference them with.
// The 'Predicate_' part of the name is redundant but eliminating it is more
// trouble than it's worth.
void GlobalISelEmitter::emitImmPredicates(
    raw_ostream &OS, StringRef TypeIdentifier, StringRef Type,
    std::function<bool(const Record *R)> Filter) {
  std::vector<const Record *> MatchedRecords;
  const auto &Defs = RK.getAllDerivedDefinitions("PatFrag");
  std::copy_if(Defs.begin(), Defs.end(), std::back_inserter(MatchedRecords),
               [&](Record *Record) {
                 return !Record->getValueAsString("ImmediateCode").empty() &&
                        Filter(Record);
               });

  if (!MatchedRecords.empty()) {
    OS << "// PatFrag predicates.\n"
       << "enum {\n";
    std::string EnumeratorSeparator =
        (" = GIPFP_" + TypeIdentifier + "_Invalid + 1,\n").str();
    for (const auto *Record : MatchedRecords) {
      OS << "  GIPFP_" << TypeIdentifier << "_Predicate_" << Record->getName()
         << EnumeratorSeparator;
      EnumeratorSeparator = ",\n";
    }
    OS << "};\n";
  }

  for (const auto *Record : MatchedRecords)
    OS << "static bool Predicate_" << Record->getName() << "(" << Type
       << " Imm) {" << Record->getValueAsString("ImmediateCode") << "}\n";

  OS << "static InstructionSelector::" << TypeIdentifier
     << "ImmediatePredicateFn " << TypeIdentifier << "ImmPredicateFns[] = {\n"
     << "  nullptr,\n";
  for (const auto *Record : MatchedRecords)
    OS << "  Predicate_" << Record->getName() << ",\n";
  OS << "};\n";
}

void GlobalISelEmitter::run(raw_ostream &OS) {
  if (!UseCoverageFile.empty()) {
    RuleCoverage = CodeGenCoverage();
    auto RuleCoverageBufOrErr = MemoryBuffer::getFile(UseCoverageFile);
    if (!RuleCoverageBufOrErr) {
      PrintWarning(SMLoc(), "Missing rule coverage data");
      RuleCoverage = None;
    } else {
      if (!RuleCoverage->parse(*RuleCoverageBufOrErr.get(), Target.getName())) {
        PrintWarning(SMLoc(), "Ignoring invalid or missing rule coverage data");
        RuleCoverage = None;
      }
    }
  }

  // Track the GINodeEquiv definitions.
  gatherNodeEquivs();

  emitSourceFileHeader(("Global Instruction Selector for the " +
                       Target.getName() + " target").str(), OS);
  std::vector<RuleMatcher> Rules;
  // Look through the SelectionDAG patterns we found, possibly emitting some.
  for (const PatternToMatch &Pat : CGP.ptms()) {
    ++NumPatternTotal;

    auto MatcherOrErr = runOnPattern(Pat);

    // The pattern analysis can fail, indicating an unsupported pattern.
    // Report that if we've been asked to do so.
    if (auto Err = MatcherOrErr.takeError()) {
      if (WarnOnSkippedPatterns) {
        PrintWarning(Pat.getSrcRecord()->getLoc(),
                     "Skipped pattern: " + toString(std::move(Err)));
      } else {
        consumeError(std::move(Err));
      }
      ++NumPatternImportsSkipped;
      continue;
    }

    if (RuleCoverage) {
      if (RuleCoverage->isCovered(MatcherOrErr->getRuleID()))
        ++NumPatternsTested;
      else
        PrintWarning(Pat.getSrcRecord()->getLoc(),
                     "Pattern is not covered by a test");
    }
    Rules.push_back(std::move(MatcherOrErr.get()));
  }

  std::stable_sort(Rules.begin(), Rules.end(),
            [&](const RuleMatcher &A, const RuleMatcher &B) {
              if (A.isHigherPriorityThan(B)) {
                assert(!B.isHigherPriorityThan(A) && "Cannot be more important "
                                                     "and less important at "
                                                     "the same time");
                return true;
              }
              return false;
            });

  std::vector<Record *> ComplexPredicates =
      RK.getAllDerivedDefinitions("GIComplexOperandMatcher");
  std::sort(ComplexPredicates.begin(), ComplexPredicates.end(),
            [](const Record *A, const Record *B) {
              if (A->getName() < B->getName())
                return true;
              return false;
            });
  unsigned MaxTemporaries = 0;
  for (const auto &Rule : Rules)
    MaxTemporaries = std::max(MaxTemporaries, Rule.countRendererFns());

  OS << "#ifdef GET_GLOBALISEL_PREDICATE_BITSET\n"
     << "const unsigned MAX_SUBTARGET_PREDICATES = " << SubtargetFeatures.size()
     << ";\n"
     << "using PredicateBitset = "
        "llvm::PredicateBitsetImpl<MAX_SUBTARGET_PREDICATES>;\n"
     << "#endif // ifdef GET_GLOBALISEL_PREDICATE_BITSET\n\n";

  OS << "#ifdef GET_GLOBALISEL_TEMPORARIES_DECL\n"
     << "  mutable MatcherState State;\n"
     << "  typedef "
        "ComplexRendererFns("
     << Target.getName()
     << "InstructionSelector::*ComplexMatcherMemFn)(MachineOperand &) const;\n"
     << "  const MatcherInfoTy<PredicateBitset, ComplexMatcherMemFn> "
        "MatcherInfo;\n"
     << "  static " << Target.getName()
     << "InstructionSelector::ComplexMatcherMemFn ComplexPredicateFns[];\n"
     << "#endif // ifdef GET_GLOBALISEL_TEMPORARIES_DECL\n\n";

  OS << "#ifdef GET_GLOBALISEL_TEMPORARIES_INIT\n"
     << ", State(" << MaxTemporaries << "),\n"
     << "MatcherInfo({TypeObjects, FeatureBitsets, I64ImmPredicateFns, "
        "APIntImmPredicateFns, APFloatImmPredicateFns, ComplexPredicateFns})\n"
     << "#endif // ifdef GET_GLOBALISEL_TEMPORARIES_INIT\n\n";

  OS << "#ifdef GET_GLOBALISEL_IMPL\n";
  SubtargetFeatureInfo::emitSubtargetFeatureBitEnumeration(SubtargetFeatures,
                                                           OS);

  // Separate subtarget features by how often they must be recomputed.
  SubtargetFeatureInfoMap ModuleFeatures;
  std::copy_if(SubtargetFeatures.begin(), SubtargetFeatures.end(),
               std::inserter(ModuleFeatures, ModuleFeatures.end()),
               [](const SubtargetFeatureInfoMap::value_type &X) {
                 return !X.second.mustRecomputePerFunction();
               });
  SubtargetFeatureInfoMap FunctionFeatures;
  std::copy_if(SubtargetFeatures.begin(), SubtargetFeatures.end(),
               std::inserter(FunctionFeatures, FunctionFeatures.end()),
               [](const SubtargetFeatureInfoMap::value_type &X) {
                 return X.second.mustRecomputePerFunction();
               });

  SubtargetFeatureInfo::emitComputeAvailableFeatures(
      Target.getName(), "InstructionSelector", "computeAvailableModuleFeatures",
      ModuleFeatures, OS);
  SubtargetFeatureInfo::emitComputeAvailableFeatures(
      Target.getName(), "InstructionSelector",
      "computeAvailableFunctionFeatures", FunctionFeatures, OS,
      "const MachineFunction *MF");

  // Emit a table containing the LLT objects needed by the matcher and an enum
  // for the matcher to reference them with.
  std::vector<LLTCodeGen> TypeObjects;
  for (const auto &Ty : LLTOperandMatcher::KnownTypes)
    TypeObjects.push_back(Ty);
  std::sort(TypeObjects.begin(), TypeObjects.end());
  OS << "// LLT Objects.\n"
     << "enum {\n";
  for (const auto &TypeObject : TypeObjects) {
    OS << "  ";
    TypeObject.emitCxxEnumValue(OS);
    OS << ",\n";
  }
  OS << "};\n"
     << "const static LLT TypeObjects[] = {\n";
  for (const auto &TypeObject : TypeObjects) {
    OS << "  ";
    TypeObject.emitCxxConstructorCall(OS);
    OS << ",\n";
  }
  OS << "};\n\n";

  // Emit a table containing the PredicateBitsets objects needed by the matcher
  // and an enum for the matcher to reference them with.
  std::vector<std::vector<Record *>> FeatureBitsets;
  for (auto &Rule : Rules)
    FeatureBitsets.push_back(Rule.getRequiredFeatures());
  std::sort(
      FeatureBitsets.begin(), FeatureBitsets.end(),
      [&](const std::vector<Record *> &A, const std::vector<Record *> &B) {
        if (A.size() < B.size())
          return true;
        if (A.size() > B.size())
          return false;
        for (const auto &Pair : zip(A, B)) {
          if (std::get<0>(Pair)->getName() < std::get<1>(Pair)->getName())
            return true;
          if (std::get<0>(Pair)->getName() > std::get<1>(Pair)->getName())
            return false;
        }
        return false;
      });
  FeatureBitsets.erase(
      std::unique(FeatureBitsets.begin(), FeatureBitsets.end()),
      FeatureBitsets.end());
  OS << "// Feature bitsets.\n"
     << "enum {\n"
     << "  GIFBS_Invalid,\n";
  for (const auto &FeatureBitset : FeatureBitsets) {
    if (FeatureBitset.empty())
      continue;
    OS << "  " << getNameForFeatureBitset(FeatureBitset) << ",\n";
  }
  OS << "};\n"
     << "const static PredicateBitset FeatureBitsets[] {\n"
     << "  {}, // GIFBS_Invalid\n";
  for (const auto &FeatureBitset : FeatureBitsets) {
    if (FeatureBitset.empty())
      continue;
    OS << "  {";
    for (const auto &Feature : FeatureBitset) {
      const auto &I = SubtargetFeatures.find(Feature);
      assert(I != SubtargetFeatures.end() && "Didn't import predicate?");
      OS << I->second.getEnumBitName() << ", ";
    }
    OS << "},\n";
  }
  OS << "};\n\n";

  // Emit complex predicate table and an enum to reference them with.
  OS << "// ComplexPattern predicates.\n"
     << "enum {\n"
     << "  GICP_Invalid,\n";
  for (const auto &Record : ComplexPredicates)
    OS << "  GICP_" << Record->getName() << ",\n";
  OS << "};\n"
     << "// See constructor for table contents\n\n";

  emitImmPredicates(OS, "I64", "int64_t", [](const Record *R) {
    bool Unset;
    return !R->getValueAsBitOrUnset("IsAPFloat", Unset) &&
           !R->getValueAsBit("IsAPInt");
  });
  emitImmPredicates(OS, "APFloat", "const APFloat &", [](const Record *R) {
    bool Unset;
    return R->getValueAsBitOrUnset("IsAPFloat", Unset);
  });
  emitImmPredicates(OS, "APInt", "const APInt &", [](const Record *R) {
    return R->getValueAsBit("IsAPInt");
  });
  OS << "\n";

  OS << Target.getName() << "InstructionSelector::ComplexMatcherMemFn\n"
     << Target.getName() << "InstructionSelector::ComplexPredicateFns[] = {\n"
     << "  nullptr, // GICP_Invalid\n";
  for (const auto &Record : ComplexPredicates)
    OS << "  &" << Target.getName()
       << "InstructionSelector::" << Record->getValueAsString("MatcherFn")
       << ", // " << Record->getName() << "\n";
  OS << "};\n\n";

  OS << "bool " << Target.getName()
     << "InstructionSelector::selectImpl(MachineInstr &I, CodeGenCoverage "
        "&CoverageInfo) const {\n"
     << "  MachineFunction &MF = *I.getParent()->getParent();\n"
     << "  MachineRegisterInfo &MRI = MF.getRegInfo();\n"
     << "  // FIXME: This should be computed on a per-function basis rather "
        "than per-insn.\n"
     << "  AvailableFunctionFeatures = computeAvailableFunctionFeatures(&STI, "
        "&MF);\n"
     << "  const PredicateBitset AvailableFeatures = getAvailableFeatures();\n"
     << "  NewMIVector OutMIs;\n"
     << "  State.MIs.clear();\n"
     << "  State.MIs.push_back(&I);\n\n";

  MatchTable Table(0);
  for (auto &Rule : Rules) {
    Rule.emit(Table);
    ++NumPatternEmitted;
  }
  Table << MatchTable::Opcode("GIM_Reject") << MatchTable::LineBreak;
  Table.emitDeclaration(OS);
  OS << "  if (executeMatchTable(*this, OutMIs, State, MatcherInfo, ";
  Table.emitUse(OS);
  OS << ", TII, MRI, TRI, RBI, AvailableFeatures, CoverageInfo)) {\n"
     << "    return true;\n"
     << "  }\n\n";

  OS << "  return false;\n"
     << "}\n"
     << "#endif // ifdef GET_GLOBALISEL_IMPL\n";

  OS << "#ifdef GET_GLOBALISEL_PREDICATES_DECL\n"
     << "PredicateBitset AvailableModuleFeatures;\n"
     << "mutable PredicateBitset AvailableFunctionFeatures;\n"
     << "PredicateBitset getAvailableFeatures() const {\n"
     << "  return AvailableModuleFeatures | AvailableFunctionFeatures;\n"
     << "}\n"
     << "PredicateBitset\n"
     << "computeAvailableModuleFeatures(const " << Target.getName()
     << "Subtarget *Subtarget) const;\n"
     << "PredicateBitset\n"
     << "computeAvailableFunctionFeatures(const " << Target.getName()
     << "Subtarget *Subtarget,\n"
     << "                                 const MachineFunction *MF) const;\n"
     << "#endif // ifdef GET_GLOBALISEL_PREDICATES_DECL\n";

  OS << "#ifdef GET_GLOBALISEL_PREDICATES_INIT\n"
     << "AvailableModuleFeatures(computeAvailableModuleFeatures(&STI)),\n"
     << "AvailableFunctionFeatures()\n"
     << "#endif // ifdef GET_GLOBALISEL_PREDICATES_INIT\n";
}

void GlobalISelEmitter::declareSubtargetFeature(Record *Predicate) {
  if (SubtargetFeatures.count(Predicate) == 0)
    SubtargetFeatures.emplace(
        Predicate, SubtargetFeatureInfo(Predicate, SubtargetFeatures.size()));
}

TreePatternNode *GlobalISelEmitter::fixupPatternNode(TreePatternNode *N) {
  if (!N->isLeaf()) {
    for (unsigned I = 0, E = N->getNumChildren(); I < E; ++I) {
      TreePatternNode *OrigChild = N->getChild(I);
      TreePatternNode *NewChild = fixupPatternNode(OrigChild);
      if (OrigChild != NewChild)
        N->setChild(I, NewChild);
    }

    if (N->getOperator()->getName() == "ld") {
      // If it's a signext-load we need to adapt the pattern slightly. We need
      // to split the node into (sext (ld ...)), remove the <<signext>> predicate,
      // and then apply the <<signextTY>> predicate by updating the result type
      // of the load.
      //
      // For example:
      //   (ld:[i32] [iPTR])<<unindexed>><<signext>><<signexti16>>
      // must be transformed into:
      //   (sext:[i32] (ld:[i16] [iPTR])<<unindexed>>)
      //
      // Likewise for zeroext-load and anyext-load.

      std::vector<TreePredicateFn> Predicates;
      bool IsSignExtLoad = false;
      bool IsZeroExtLoad = false;
      bool IsAnyExtLoad = false;
      Record *MemVT = nullptr;
      for (const auto &P : N->getPredicateFns()) {
        if (P.isLoad() && P.isSignExtLoad()) {
          IsSignExtLoad = true;
          continue;
        }
        if (P.isLoad() && P.isZeroExtLoad()) {
          IsZeroExtLoad = true;
          continue;
        }
        if (P.isLoad() && P.isAnyExtLoad()) {
          IsAnyExtLoad = true;
          continue;
        }
        if (P.isLoad() && P.getMemoryVT()) {
          MemVT = P.getMemoryVT();
          continue;
        }
        Predicates.push_back(P);
      }

      if ((IsSignExtLoad || IsZeroExtLoad || IsAnyExtLoad) && MemVT) {
        assert((IsSignExtLoad + IsZeroExtLoad + IsAnyExtLoad) == 1 &&
               "IsSignExtLoad, IsZeroExtLoad, IsAnyExtLoad are mutually exclusive");
        TreePatternNode *Ext = new TreePatternNode(
            RK.getDef(IsSignExtLoad ? "sext"
                                    : IsZeroExtLoad ? "zext" : "anyext"),
            {N}, 1);
        Ext->setType(0, N->getType(0));
        N->clearPredicateFns();
        N->setPredicateFns(Predicates);
        N->setType(0, getValueType(MemVT));
        return Ext;
      }
    }
  }

  return N;
}

void GlobalISelEmitter::fixupPatternTrees(TreePattern *P) {
  for (unsigned I = 0, E = P->getNumTrees(); I < E; ++I) {
    TreePatternNode *OrigTree = P->getTree(I);
    TreePatternNode *NewTree = fixupPatternNode(OrigTree);
    if (OrigTree != NewTree)
      P->setTree(I, NewTree);
  }
}

} // end anonymous namespace

//===----------------------------------------------------------------------===//

namespace llvm {
void EmitGlobalISel(RecordKeeper &RK, raw_ostream &OS) {
  GlobalISelEmitter(RK).run(OS);
}
} // End llvm namespace
