//===-------------------------- InlineModule.cpp --------------------------===//
// This file implements the Scaffold pass of inlining modules whose gate
// counts are below the threshold. These modules' names have been previously
// written to the file "inline_info.txt".
//
//        This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "InlineModule"
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
#include "llvm/IR/CallSite.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include <fstream>
#include <sstream>
#include <string>

// DEBUG switch
bool debugInlining = true;

using namespace llvm;

// An anonymous namespace for the pass. Things declared inside it are
// only visible to the current file.
namespace {

// Derived from ModulePass to work on callgraph
struct InlineModule : public ModulePass {
  static char ID; // Pass identification
  InlineModule() : ModulePass(ID) {}

  // what functions to make inlined
  std::vector<Function *> makeInlined;

  // mark those call sites to be inlined
  std::vector<CallInst *> inlineCallInsts;

  // functions in post-order
  std::vector<Function *> vectPostOrder;

  bool runOnModule(Module &M);
  bool runOnSCC(const std::vector<CallGraphNode *> &scc);
  bool runOnFunction(Function &F);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();
    AU.addRequired<CallGraphWrapperPass>();
  }

}; // End of struct InlineModule
} // End of anonymous namespace

char InlineModule::ID = 0;
static RegisterPass<InlineModule>
    X("InlineModule", "Quantum Module Inlining Pass", false, false);

bool InlineModule::runOnModule(Module &M) {
  std::vector<std::string> inlinedNames;
  std::string line;
  std::ifstream file("inline_info.txt");
  if (file.is_open()) {
    while (std::getline(file, line))
      inlinedNames.push_back(line);
    file.close();
  } else
    errs() << "Error: Could not open inline_info file.\n";

  for (std::vector<std::string>::iterator i = inlinedNames.begin(),
                                          e = inlinedNames.end();
       i != e; ++i) {
    if (debugInlining)
      errs() << "inline_info: " << *i << "\n";
    makeInlined.push_back(M.getFunction(*i));
  }

  // First, get a pointer to previous analysis results
  CallGraph &CG = getAnalysis<CallGraphWrapperPass>().getCallGraph();
  scc_iterator<CallGraph *> SCCIter = scc_begin(&CG);
  const std::vector<CallGraphNode *> &NextSCC = *SCCIter;
  CallGraphNode *entry = *NextSCC.begin();
  if (entry && entry->getFunction() && debugInlining)
    errs() << "Entry is function: " << entry->getFunction()->getName() << "\n";

  // Iterate over all SCCs in the module in bottom-up order
  for (scc_iterator<CallGraph *> si = scc_begin(&CG), se = scc_end(&CG);
       si != se; ++si) {
    runOnSCC(*si);
  }

  // reverse the vector for preorder
  std::reverse(vectPostOrder.begin(), vectPostOrder.end());

  for (std::vector<Function *>::iterator vit = vectPostOrder.begin(),
                                         vitE = vectPostOrder.end();
       vit != vitE; ++vit) {
    Function *f = *vit;
    runOnFunction(*f);
  }

  // now we have all the call sites which need to be inlined
  // inline from the leaves all the way up
  InlineFunctionInfo InlineInfo(&CG);

  std::reverse(inlineCallInsts.begin(), inlineCallInsts.end());
  for (std::vector<CallInst *>::iterator i = inlineCallInsts.begin(),
                                         e = inlineCallInsts.end();
       i != e; ++i) {
    CallInst *CI = *i;
    bool success = InlineFunction(CI, InlineInfo);
    if (!success) {
      if (debugInlining)
        errs() << "Error: Could not inline callee function "
               << CI->getCalledFunction()->getName() << " into caller function "
               << "\n";
      continue;
    }
    if (debugInlining)
      errs() << "Successfully inlined callee function "
             << CI->getCalledFunction()->getName() << " into caller function "
             << "\n";
  }

  return false;
}

bool InlineModule::runOnSCC(const std::vector<CallGraphNode *> &scc) {
  for (std::vector<CallGraphNode *>::const_iterator i = scc.begin(),
                                                    e = scc.end();
       i != e; ++i) {
    if (*i && (*i)->getFunction()) {
      Function &F = *(*i)->getFunction();
      vectPostOrder.push_back(&F);
    }
  }
  return false;
}

bool InlineModule::runOnFunction(Function &F) {
  if (debugInlining)
    errs() << "run on function: " << F.getName() << "\n";

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *pInst = &*I;
    if (CallInst *CI = dyn_cast<CallInst>(pInst)) {
      if (std::find(makeInlined.begin(), makeInlined.end(),
                    CI->getCalledFunction()) != makeInlined.end()) {
        if (debugInlining)
          errs() << "makeInlined: " << CI->getCalledFunction()->getName()
                 << "\n";
        inlineCallInsts.push_back(CI);
      }
    }
  }

  return true;
}
