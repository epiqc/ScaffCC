//===----------------------------- GateCount.cpp -------------------------===//
// This file implements the Scaffold Pass of counting the number of qbits and
// gates in a program in callgraph post-order.
//
//        This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "ResourceCount"
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
  struct ResourceCount : public ModulePass {
    static char ID; // Pass identification
    ResourceCount() : ModulePass(ID) {}

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();  
      AU.addRequired<CallGraph>();    
    }
    
    void CountFunctionResources (Function *F, std::map <Function*, unsigned long long* > FunctionResources) const {
      // Traverse instruction by instruction
      for (inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E; ++I) {
        Instruction *Inst = &*I;                            // Grab pointer to instruction reference

        // Qubits?
        if (AllocaInst *AI = dyn_cast<AllocaInst>(Inst)) {                  // Filter Allocation Instructions
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
            if (callee->getName().str() == "llvm.X") 
              FunctionResources[F][1]++;
            else if (callee->getName().str() == "llvm.Z") 
              FunctionResources[F][2]++;
            else if (callee->getName().str() == "llvm.H") 
              FunctionResources[F][3]++;
            else if (callee->getName().str() == "llvm.T") 
              FunctionResources[F][4]++;
            else if (callee->getName().str() == "llvm.Tdag")
              FunctionResources[F][5]++;
            else if (callee->getName().str() == "llvm.S") 
              FunctionResources[F][6]++;
            else if (callee->getName().str() == "llvm.Sdag")
              FunctionResources[F][7]++;
            else if (callee->getName().str() == "llvm.CNOT")
              FunctionResources[F][8]++;            
            else if (callee->getName().str() == "llvm.PrepZ") 
              FunctionResources[F][9]++;
            else if (callee->getName().str() == "llvm.MeasZ") 
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
      // Function* ---> Qubits | X | Z | H | T | CNOT | Toffoli | PrepZ | MeasZ
      std::map <Function*, unsigned long long*> FunctionResources;

      // unsigned long long is 18x10^18 digits longs. good enough.
      // errs() << "LONG LONG LIMIT: " << std::numeric_limits<unsigned long long>::max() << "\n";

      errs() << "\tQubit\tX\tZ\tH\tT\tT_dag\tS\tS_dag\tCNOT\tPrepZ\tMeasZ\n";

      // iterate over all functions, and over all instructions in those functions
      // find call sites that have constant integer values. In Post-Order.
      CallGraphNode* rootNode = getAnalysis<CallGraph>().getRoot();
      
      //fill in the gate count bottom-up in the call graph
      for (scc_iterator<CallGraphNode*> sccIb = scc_begin(rootNode), E = scc_end(rootNode); sccIb != E; ++sccIb) {
        const std::vector<CallGraphNode*> &nextSCC = *sccIb;
        //errs() << "\nSCC #" << ++sccNum << " : ";      
        for (std::vector<CallGraphNode*>::const_iterator nsccI = nextSCC.begin(), E = nextSCC.end(); nsccI != E; ++nsccI) {
          Function *F = (*nsccI)->getFunction();	  
          if (F && !F->isDeclaration()) {
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
      for (std::map<Function*, unsigned long long*>::iterator i = FunctionResources.begin(), e = FunctionResources.end(); i!=e; ++i) {
        errs() << "Function: " << i->first->getName() << "\n";
        for (int j=0; j<11; j++)
          errs() << "\t" << (i->second)[j];
        errs() << "\n";
      }

      unsigned long long total_gates = 0;
      for (int j=1; j<11;j++)
        total_gates += FunctionResources.find(M.getFunction("main"))->second[j];
      errs() << "\ntotal_gates = " << total_gates << "\n";

      // free memory
      for (std::map<Function*, unsigned long long*>::iterator i = FunctionResources.begin(), e = FunctionResources.end(); i!=e; ++i)
        delete [] i->second;


      return false;
    } // End runOnModule
  }; // End of struct ResourceCount
} // End of anonymous namespace



char ResourceCount::ID = 0;
static RegisterPass<ResourceCount> X("ResourceCount", "Resource Counter Pass");


