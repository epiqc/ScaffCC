//===----------------------------- ResourceCount2.cpp -------------------------===//
// This file implements the Scaffold Pass of counting the number of qbits and
// gates in a program in callgraph post-order. Printing total for every function.
//
//        This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "ResourceCount2"
#include <vector>
#include <limits>
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instruction.h"
#include "llvm/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Support/CFG.h"
#include "llvm/ADT/SCCIterator.h"


using namespace llvm;

// An anonymous namespace for the pass. Things declared inside it are
// only visible to the current file.
namespace {

  // Derived from ModulePass to count qbits in functions
  struct ResourceCount2 : public ModulePass {
    static char ID; // Pass identification
    ResourceCount2() : ModulePass(ID) {}

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();  
      AU.addRequired<CallGraph>();    
    }
    
    void CountFunctionResources (Function *F, 
        std::map <Function*, unsigned long long* > FunctionResources) const {
      // Traverse instruction by instruction
      for (inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E; ++I) {        
        Instruction *Inst = &*I;     // Grab pointer to instruction reference

        // Qubits?
        if (AllocaInst *AI = dyn_cast<AllocaInst>(Inst)) {                // Filter Allocation Instructions
          Type *allocatedType = AI->getAllocatedType();

          if (ArrayType *arrayType = dyn_cast<ArrayType>(allocatedType)) { // Filter allocation of arrays
            Type *elementType = arrayType->getElementType();
            if (elementType->isIntegerTy(16)) {                           // Filter allocation Type (qbit=i16)
              uint64_t arraySize = arrayType->getNumElements();            
              FunctionResources[F][0] += arraySize;                
            }
          }
        }

        // Gates?
        if (CallInst *CI = dyn_cast<CallInst>(Inst)) {      // Filter Call Instructions
          Function *callee = CI->getCalledFunction();
          if (callee->isIntrinsic()) {                      // Intrinsic (Gate) Functions calls
            if (callee->getName().str().find("llvm.X") != std::string::npos) 
              FunctionResources[F][1]++;
            else if (callee->getName().str().find("llvm.Z") != std::string::npos) 
              FunctionResources[F][2]++;
            else if (callee->getName().str().find("llvm.H") != std::string::npos) 
              FunctionResources[F][3]++;
            else if (callee->getName().str().find("llvm.T") != std::string::npos)
              FunctionResources[F][4]++;
            else if (callee->getName().str().find("llvm.Tdag") != std::string::npos)
              FunctionResources[F][5]++;
            else if (callee->getName().str().find("llvm.S") != std::string::npos) 
              FunctionResources[F][6]++;
            else if (callee->getName().str().find("llvm.Sdag") != std::string::npos)
              FunctionResources[F][7]++;
            else if (callee->getName().str().find("llvm.CNOT") != std::string::npos)
              FunctionResources[F][8]++;            
            else if (callee->getName().str().find("llvm.PrepZ")!= std::string::npos)
              FunctionResources[F][9]++;
            else if (callee->getName().str().find("llvm.MeasZ")!= std::string::npos)
              FunctionResources[F][10]++;
          }

          else {                                              // Non-intrinsic Function Calls
            // Resource numbers must be previously entered
            // for this call. Look them up and add to this function's numbers.
            if (FunctionResources.find(callee) != FunctionResources.end()) {
              unsigned long long* callee_numbers = FunctionResources.find(callee)->second;
              for (int l=0; l<11; l++)
                FunctionResources[F][l] += callee_numbers[l];
            }
          }

        }

      }
    }

    virtual bool runOnModule (Module &M) {
      // Function* ---> Qubits | X | Z | H | T | CNOT | Toffoli | PrepZ | MeasZ | Rz | Ry
      std::map <Function*, unsigned long long*> FunctionResources;

      // iterate over all functions, and over all instructions in those functions
      // find call sites that have constant integer values. In Post-Order.
      CallGraphNode* rootNode = getAnalysis<CallGraph>().getRoot();
      
      //fill in the gate count bottom-up in the call graph
      for (scc_iterator<CallGraphNode*> sccIb = scc_begin(rootNode), 
          E = scc_end(rootNode); sccIb != E; ++sccIb) {
        const std::vector<CallGraphNode*> &nextSCC = *sccIb;
        for (std::vector<CallGraphNode*>::const_iterator nsccI = nextSCC.begin(), 
            E = nextSCC.end(); nsccI != E; ++nsccI) {
          Function *F = (*nsccI)->getFunction();	  
          if (F && !F->isDeclaration()) {
            
            //errs() << "--Function: " << F->getName() << "\n";
            // dynamically create array holding gate numbers for this function
            unsigned long long* ResourceNumbers = new unsigned long long[11];
            for (int k=0; k<11; k++)
              ResourceNumbers[k] = 0;
            FunctionResources.insert(std::make_pair(F, ResourceNumbers));

            // count the gates of this function 
            CountFunctionResources(F, FunctionResources);
          }
        }
      }

      // print results      
      for (std::map<Function*, 
          unsigned long long*>::iterator i = FunctionResources.begin(), 
          e = FunctionResources.end(); i!=e; ++i) {
        errs() << i->first->getName() << ": \t";
        unsigned long long function_total_gates = 0;
        for (int j=1; j<11; j++)
          function_total_gates += (i->second)[j];
        errs() << function_total_gates <<"\n"; //<< " \t";
        //errs() << (i->second)[4] << "\n"; // T Gates
      }

      // free memory
      for (std::map<Function*, 
          unsigned long long*>::iterator i = FunctionResources.begin(), 
          e = FunctionResources.end(); i!=e; ++i)
        delete [] i->second;


      return false;
    } // End runOnModule
  }; // End of struct ResourceCount2
} // End of anonymous namespace



char ResourceCount2::ID = 0;
static RegisterPass<ResourceCount2> X("ResourceCount2", "Resource Counter Pass");


