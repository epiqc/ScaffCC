//===-------------------------- FunctionClone.cpp ------------------------===//
// This file implements the Scaffold pass of sorting integer/double arguments
// in functions that have been cloned based on their constant arguments.
//
//        This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "FunctionClone"
#include <sstream>
#include <iostream>
#include <iomanip>
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
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Support/CFG.h"
#include "llvm/ADT/SCCIterator.h"

#define _MAX_FUNCTION_NAME 32
#define _MAX_INT_PARAMS 4
#define _MAX_DOUBLE_PARAMS 4

// DEBUG switch
bool debugSortArgs = false;

using namespace llvm;

// An anonymous namespace for the pass. Things declared inside it are
// only visible to the current file.
namespace {

  // Derived from ModulePass to work on callgraph
  struct SortCloneArguments : public ModulePass {
    static char ID; // Pass identification
    SortCloneArguments() : ModulePass(ID) {}

    bool runOnModule (Module &M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.setPreservesAll();  
        AU.addRequired<CallGraph>();
    }

  }; // End of struct SortCloneArguments
} // End of anonymous namespace


char SortCloneArguments::ID = 0;
static RegisterPass<SortCloneArguments> X("SortCloneArguments", "Sorting Arguments of Cloned Function", false, false);

bool SortCloneArguments::runOnModule (Module &M) {
   
  for (Module::iterator F = M.begin(); F != M.end(); ++F) {
    if(F->isIntrinsic() || F->isDeclaration() || F->getName()=="main")
      continue;
    //----- Only Modifying Function Names: Therefore the correspondence bewteen name order and
    //----- argument order breaks, but thats not important
      
      std::string originalName = F->getName().str();
      if (debugSortArgs)
        errs() << "Function: " << originalName << "\n";
      std::string::size_type found_pos_begin, found_pos_end, found_pos_begin_new;

      std::vector<int> originalInts;
      std::vector<double> originalDoubles;
      
      // for IntParams
      found_pos_begin = originalName.find("_IP");
      while (found_pos_begin != std::string::npos){
        //because there might be numbers more than 1 digit long, need to find begin and end
        found_pos_end = originalName.find_first_not_of("0123456789",found_pos_begin+3);
        std::string intString = originalName.substr(found_pos_begin+3, found_pos_end-(found_pos_begin+3));
        if (debugSortArgs)
          errs() << "\tint: " << intString << "\n";
        originalInts.push_back(atoi(intString.c_str()));
        //next one
        found_pos_begin_new = originalName.find("_IP",found_pos_end);  
        found_pos_begin = found_pos_begin_new;
      }
      
      // for DoubleParams
      found_pos_begin = originalName.find("_DP");
      while (found_pos_begin != std::string::npos){
        //because there might be numbers more than 1 digit long, need to find begin and end
        found_pos_end = originalName.find_first_not_of("0123456789e.-",found_pos_begin+3);
        std::string doubleString = originalName.substr(found_pos_begin+3, found_pos_end-(found_pos_begin+3));
        if (debugSortArgs)
          errs() << "\tdouble: " << doubleString << "\n";
        originalDoubles.push_back(atof(doubleString.c_str()));
        //next one
        found_pos_begin_new = originalName.find("_DP",found_pos_end);  
        found_pos_begin = found_pos_begin_new;
      }
      
      // concatanate the rest with zero and then sort
      for (int i=originalInts.size(); i<_MAX_INT_PARAMS; i++)
        originalInts.push_back(0);
      for (int i=originalDoubles.size(); i<_MAX_DOUBLE_PARAMS; i++)
        originalDoubles.push_back(0);      

      std::sort(originalInts.begin(), originalInts.end());
      std::sort(originalDoubles.begin(), originalDoubles.end());      

      // what's the name without all the parameters?
      std::string::size_type originalCoreEnd;
      originalCoreEnd = originalName.find(std::string("_IP"));
      if (originalCoreEnd == std::string::npos) // maybe it's all doubles?
        originalCoreEnd = originalName.find(std::string("_DP"));        
      std::string originalCore = originalName.substr(0, originalCoreEnd);
      if(debugSortArgs)
        errs() << "\toriginalCore: " << originalCore << "\n";

      // construct the new clone name with ordered parameters
      std::stringstream ss;
      for (std::vector<int>::iterator i = originalInts.begin(), e = originalInts.end(); i!=e; ++i)
        ss << "_IP" << *i;
      //for (std::vector<double>::iterator i = originalDoubles.begin(), e = originalDoubles.end(); i!=e; ++i)
      //  ss << "_DP" << std::fixed << *i;
      
      std::string specializedName = originalCore + ss.str();
      if(debugSortArgs)
        errs() << "\tspecializedName: " << specializedName << "\n";
      
      F->setName(specializedName);
  }
  return true;
}

