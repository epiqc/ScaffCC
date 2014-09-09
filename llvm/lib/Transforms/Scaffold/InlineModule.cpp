//===-------------------------- InlineModule.cpp ------------------------===//
// This file implements the Scaffold pass of cloning functions with constant 
// integer or double arguments.
//
//        This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "InlineModule"
#include <sstream>
#include <fstream>
#include <string>
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instruction.h"
#include "llvm/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Transforms/IPO/InlinerPass.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Support/CFG.h"
#include "llvm/Target/TargetData.h"
#include "llvm/ADT/SCCIterator.h"

// DEBUG switch
bool debugInlining = false;

using namespace llvm;

// An anonymous namespace for the pass. Things declared inside it are
// only visible to the current file.
namespace {

  // Derived from ModulePass to work on callgraph
  struct InlineModule : public ModulePass {
    static char ID; // Pass identification
    InlineModule() : ModulePass(ID) {}

    // what functions to make leaves
    std::vector <Function*> makeLeaf;
    
    // mark those call sites to be inlined
    std::vector<CallInst*> inlineCallInsts; 

    // functions in post-order
    std::vector<Function*> vectPostOrder;

    bool runOnModule (Module &M);
    bool runOnSCC( const std::vector<CallGraphNode *> &scc );
    bool runOnFunction( Function & F );    

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.setPreservesCFG();  
        AU.addRequired<CallGraph>();
        AU.addRequired<TargetData>();
    }

  }; // End of struct InlineModule
} // End of anonymous namespace


char InlineModule::ID = 0;
static RegisterPass<InlineModule> X("InlineModule", "Quantum Module Inlining Pass", false, false);

bool InlineModule::runOnModule( Module & M ) {
  
  std::vector<std::string> leafNames;
  //File *file = fopen("inline_info.txt", "r");
  //if (!file) {
  //  errs() << "Error: Could not open inline_info file.\n";
  //  retrun true;
  //}
  std::string line;
  std::ifstream file ("inline_info.txt");
  if(file.is_open()) {
    while(std::getline(file, line))
      leafNames.push_back(line);
    file.close();
  }
  else
    errs() << "Error: Could not open inline_info file.\n";

  //makeLeaf.push_back(M.getFunction("ORACLE_0"));
  //makeLeaf.push_back(M.getFunction("ORACLE_1"));
  //makeLeaf.push_back(M.getFunction("ORACLE_2"));
  //makeLeaf.push_back(M.getFunction("ORACLE_3"));

  for (std::vector<std::string>::iterator i = leafNames.begin(), e = leafNames.end();
      i!=e; ++i) {
    if (debugInlining)
      errs() << "inline_info: " << *i << "\n";
    makeLeaf.push_back(M.getFunction(*i));
  }
  

  // First, get a pointer to previous analysis results
  CallGraph & CG = getAnalysis<CallGraph>();

  CallGraphNode * entry = CG.getRoot();
  if( entry && entry->getFunction() && debugInlining)
    errs() << "Entry is function: " << entry->getFunction()->getName() << "\n";

  // Iterate over all SCCs in the module in bottom-up order
  for( scc_iterator<CallGraph*>
   si=scc_begin( &CG ), se=scc_end( &CG ); si != se; ++si ) {
    runOnSCC( *si );
  }

  //reverse the vector for preorder
  std::reverse(vectPostOrder.begin(),vectPostOrder.end());

  for(std::vector<Function*>::iterator vit = vectPostOrder.begin(), vitE = vectPostOrder.end();
      vit!=vitE; ++vit) { 
    Function *f = *vit;      
    runOnFunction(*f);    
  }

  
  // now we have all the call sites which need to be inlined
  // inline from the leaves all the way up
  const TargetData *TD = getAnalysisIfAvailable<TargetData>();
  InlineFunctionInfo InlineInfo(&CG, TD);  

  std::reverse(inlineCallInsts.begin(),inlineCallInsts.end());
  for (std::vector<CallInst*>::iterator i = inlineCallInsts.begin(), e = inlineCallInsts.end();
      i!=e; ++i) {
    CallInst* CI = *i;
    bool success = InlineFunction(CI, InlineInfo, false);
    if(!success) {
      if (debugInlining)
        errs() << "Error: Could not inline callee function " << CI->getCalledFunction()->getName()
                 << " into caller function " << "\n";
      continue;
    }
    if (debugInlining)    
      errs() << "Successfully inlined callee function " << CI->getCalledFunction()->getName()
                 << "into caller function " << "\n";
  }  
  
  return false;
}

bool InlineModule::runOnSCC( const std::vector<CallGraphNode *> &scc ) {
  for( std::vector<CallGraphNode *>::const_iterator
   i = scc.begin(), e = scc.end(); i != e; ++i ) {
    if( *i && (*i)->getFunction() ) {
      Function & F = *(*i)->getFunction();
      //runOnFunction( F, *i );
      vectPostOrder.push_back(&F);
    }
  }
  return false;
}

bool InlineModule::runOnFunction( Function & F ) {
  if (debugInlining)  
    errs() << "run on function: " << F.getName() << "\n";
  // only continue if this function is part of makeLeaf
  if (std::find(makeLeaf.begin(), makeLeaf.end(), &F) == makeLeaf.end())
    return false;
  if (debugInlining)  
    errs() << "makeLeaf: " << F.getName() << "\n";

  if(F.getName() != "main"){
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *pInst = &*I;          
    if(CallInst *CI = dyn_cast<CallInst>(pInst)) {
      if (!CI->getCalledFunction()->isIntrinsic() && !CI->getCalledFunction()->isDeclaration()) {
        makeLeaf.push_back(CI->getCalledFunction());
        inlineCallInsts.push_back(CI);
      }
    }
  }
  }

  return true;
}



