//===----------------------------- GateCount.cpp --------------------------===//
// This file implements the Scaffold Pass of counting the number of qbits and
// gates in a program in callgraph post-order.
//
//        This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "GateCount"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include <limits>
#include <vector>

using namespace llvm;

// An anonymous namespace for the pass. Things declared inside it are
// only visible to the current file.
namespace {

// Derived from ModulePass to count qbits in functions
struct GateCount : public ModulePass {
  static char ID; // Pass identification
  GateCount() : ModulePass(ID) {}

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequired<CallGraphWrapperPass>();
  }

  void CountFunctionGates(
      Function *F,
      std::map<Function *, unsigned long long *> FunctionGates) const {
    // Traverse instruction by instruction
    for (inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E; ++I) {

      Instruction *Inst = &*I; // Grab pointer to instruction reference

      if (CallInst *CI = dyn_cast<CallInst>(Inst)) { // Filter Call Instructions
        Function *callee = CI->getCalledFunction();
        if (callee->isIntrinsic()) { // Intrinsic (Gate) Functions calls
          if (callee->getName().startswith("llvm.X"))
            FunctionGates[F][0]++;
          else if (callee->getName().startswith("llvm.Z"))
            FunctionGates[F][1]++;
          else if (callee->getName().startswith("llvm.H"))
            FunctionGates[F][2]++;
          else if (callee->getName().startswith("llvm.T"))
            FunctionGates[F][3]++;
          else if (callee->getName().startswith("llvm.CNOT"))
            FunctionGates[F][4]++;
          else if (callee->getName().startswith("llvm.Toffoli"))
            FunctionGates[F][5]++;
          else if (callee->getName().startswith("llvm.RZ"))
            FunctionGates[F][6]++;
          else if (callee->getName().startswith("llvm.PrepZ"))
            FunctionGates[F][7]++;
          else if (callee->getName().startswith("llvm.MeasZ"))
            FunctionGates[F][8]++;
        }

        else { // Non-intrinsic Function Calls
          // Gate numbers must be previously entered
          // for this call. Look them up and add to this function's numbers.
          unsigned long long *callee_numbers =
              FunctionGates.find(callee)->second;
          for (int l = 0; l < 9; l++)
            FunctionGates[F][l] += callee_numbers[l];
        }
      }
    }
  }

  virtual bool runOnModule(Module &M) {
    // Function* ---> X | Z | H | T | CNOT | Toffoli | Rz | PrepZ | MeasZ
    std::map<Function *, unsigned long long *> FunctionGates;

    // unsigned long long is 18x10^18 digits longs. good enough.
    errs()
        << "\t\tX\t\tZ\t\tH\t\tT\t\tCNOT\t\tToffoli\t\tRz\t\tPrepZ\t\tMeasZ\n";

    // iterate over all functions, and over all instructions in those functions
    // find call sites that have constant integer values. In Post-Order.
    CallGraph &CG = getAnalysis<CallGraphWrapperPass>().getCallGraph();

    // fill in the gate count bottom-up in the call graph
    for (scc_iterator<CallGraph *> sccIb = scc_begin(&CG);
         !sccIb.isAtEnd(); ++sccIb) {
      const std::vector<CallGraphNode *> &nextSCC = *sccIb;
      for (std::vector<CallGraphNode *>::const_iterator nsccI = nextSCC.begin(),
                                                        E = nextSCC.end();
           nsccI != E; ++nsccI) {
        Function *F = (*nsccI)->getFunction();

        // dynamically create array holding gate numbers for this function
        unsigned long long *GateNumbers = new unsigned long long[9];
        for (int k = 0; k < 9; k++)
          GateNumbers[k] = 0;
        FunctionGates.insert(std::make_pair(F, GateNumbers));

        // count the gates of this function
        CountFunctionGates(F, FunctionGates);
      }
    }

    // print results
    for (std::map<Function *, unsigned long long *>::iterator
             i = FunctionGates.begin(),
             e = FunctionGates.end();
         i != e; ++i) {
      errs() << "Function: " << i->first->getName() << "\n";
      for (int j = 0; j < 9; j++)
        errs() << "\t" << (i->second)[j];
      errs() << "\n";
    }

    unsigned long long total_gates = 0;
    for (int j = 0; j < 9; j++)
      total_gates += FunctionGates.find(M.getFunction("main"))->second[j];
    errs() << "\ntotal_gates = " << total_gates << "\n";

    // free memory
    for (std::map<Function *, unsigned long long *>::iterator
             i = FunctionGates.begin(),
             e = FunctionGates.end();
         i != e; ++i)
      delete[] i->second;

    return false;
  } // End runOnModule
};  // End of struct GateCount
} // End of anonymous namespace

char GateCount::ID = 0;
static RegisterPass<GateCount> X("GateCount", "Gate Counter Pass");
