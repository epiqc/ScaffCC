//===-------------------------- FlattenModule.cpp ------------------------===//
// This file implements the Scaffold pass of flattening modules whose gate 
// counts are below the threshold. These modules' names have been previously 
// written to the file "flat_info.txt".
//
//        This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

//#define DEBUG_TYPE "FlattenModule"
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/CallSite.h"
#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/Support/CommandLine.h"

// DEBUG switch
bool debugFlattening = false;

using namespace llvm;

static cl::opt<unsigned>
FLAT("all", cl::init(0), cl::Hidden,
    cl::desc("Toggle for complete inlining"));

// An anonymous namespace for the pass. Things declared inside it are
// only visible to the current file.
namespace {

  // Derived from ModulePass to work on callgraph
  struct FlattenModule : public ModulePass {
    static char ID; // Pass identification
    FlattenModule() : ModulePass(ID) {}

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
        AU.addRequired<CallGraphWrapperPass>();
        // Could be important AU.addRequired<DataLayout>();
    }

  }; // End of struct FlattenModule
} // End of anonymous namespace


char FlattenModule::ID = 0;
static RegisterPass<FlattenModule> X("FlattenModule", "Quantum Module Flattening Pass", false, false);

bool FlattenModule::runOnModule( Module & M ) {

  const char *debug_val = getenv("DEBUG_FLATTENMODULE");
  if(debug_val){
    if(!strncmp(debug_val, "1", 1)) debugFlattening = true;
    else debugFlattening = false;
  }

  debug_val = getenv("DEBUG_SCAFFOLD");
  if(debug_val && !debugFlattening){
    if(!strncmp(debug_val, "1", 1)) debugFlattening = true;
    else debugFlattening = false;
  }
  
  std::vector<std::string> leafNames;
  
  std::string line;
  std::ifstream file ("flat_info.txt");
  if(file.is_open()) {
    while(std::getline(file, line))
      leafNames.push_back(line);
    file.close();
  }
  else{
    if(debugFlattening) 
        errs() << "Error: Could not open flat_info file.\n";
  }
  for (std::vector<std::string>::iterator i = leafNames.begin(), e = leafNames.end();
      i!=e; ++i) {
    if (debugFlattening)
      errs() << "flat_info: " << *i << "\n";
    makeLeaf.push_back(M.getFunction(*i));
  }

  // First, get a pointer to previous analysis results
  //CallGraph cg = getAnalysis<CallGraphWrapperPass>().getCallGraph();

  CallGraph CG = CallGraph(M);

  CallGraphNode *entry = nullptr;

  for(auto it = CG.begin();it != CG.end();it++){
    if(!(it->second->getFunction())) continue;
    if(it->second->getFunction()->getName() == "main"){
      entry = &(*it->second);
      break;
    }
  }
  if( entry && entry->getFunction() && debugFlattening)
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
  const DataLayout TD = M.getDataLayout();
  InlineFunctionInfo InlineInfo(&CG);//, TD);  

  std::reverse(inlineCallInsts.begin(),inlineCallInsts.end());
  for (std::vector<CallInst*>::iterator i = inlineCallInsts.begin(), e = inlineCallInsts.end();
      i!=e; ++i) {
    CallInst* CI = *i;
    bool success = InlineFunction(CI, InlineInfo, nullptr, false);
    if(!success) {
      if (debugFlattening)
        errs() << "Error: Could not inline callee function " << CI->getCalledFunction()->getName()
                 << " into caller function " << "\n";
      continue;
    }
    if (debugFlattening)    
      errs() << "Successfully inlined callee function " << CI->getCalledFunction()->getName()
                 << "into caller function " << "\n";
  }  
  
  return false;
}

bool FlattenModule::runOnSCC( const std::vector<CallGraphNode *> &scc ) {
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

bool FlattenModule::runOnFunction( Function & F ) {

  if (debugFlattening)  
    errs() << "run on function: " << F.getName() << "\n";
  // only continue if this function is part of makeLeaf and complete inlining is not toggled on
  if (FLAT == 0){
    if (std::find(makeLeaf.begin(), makeLeaf.end(), &F) == makeLeaf.end())
        return false;
  }
  if (debugFlattening)  
    errs() << "makeLeaf: " << F.getName() << "\n";
  
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *pInst = &*I;          
    if(CallInst *CI = dyn_cast<CallInst>(pInst)) {
      if (!CI->getCalledFunction()->isIntrinsic() && !CI->getCalledFunction()->isDeclaration()) {
        makeLeaf.push_back(CI->getCalledFunction());
        inlineCallInsts.push_back(CI);
      }
    }
  }

  return true;
}
