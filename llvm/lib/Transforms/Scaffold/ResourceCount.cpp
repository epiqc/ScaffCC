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
#include <map>
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

#include "llvm/Constants.h"

#define NCOUNTS 14

using namespace llvm;

// An anonymous namespace for the pass. Things declared inside it are
// only visible to the current file.
namespace {
  unsigned long long app_total_gates;
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
          Type *intType=NULL;
          uint64_t arraySize=0;
          //errs() << allocatedType->getTypeID() << " allocated: ";
          //errs() << "Does it have a name? " << AI->getName() << "\n";
          
          if (ArrayType *arrayType = dyn_cast<ArrayType>(allocatedType)) { // Filter allocation of arrays
            Type *elementType = arrayType->getElementType();
            //errs() << "dyncasted to array\n";
            if(elementType->isIntegerTy(8) || elementType->isIntegerTy(16)){
              //errs() << "inside ifstatement\n";
              intType = elementType;
              arraySize = arrayType->getNumElements();
              //errs() << arraySize << "elements\n";
            }
          }
          else{
            if(allocatedType->isIntegerTy(8) || allocatedType->isIntegerTy(16)){
              intType = allocatedType;
              arraySize = 1;
            }
          }

          if(intType){

            if (intType->isIntegerTy(16)) {                           // Filter allocation Type (qbit=i16)
              FunctionResources[F][0] += arraySize;                
            }
            if(intType->isIntegerTy(8)){
              FunctionResources[F][1] += arraySize;
              FunctionResources[F][2] += arraySize;
              if(FunctionResources[F][3] < FunctionResources[F][2])
                FunctionResources[F][3] = FunctionResources[F][2];
            }
          }

        } // end of if allocated instruction

        // Gates?
        else if (CallInst *CI = dyn_cast<CallInst>(Inst)) {      // Filter Call Instructions
          Function *callee = CI->getCalledFunction();
          if (callee->isIntrinsic()) {                      // Intrinsic (Gate) Functions calls
			FunctionResources[F][14]++;
            if (callee->getName().startswith("llvm.X"))
              FunctionResources[F][4]++;
            else if (callee->getName().startswith("llvm.Z"))
              FunctionResources[F][5]++;
            else if (callee->getName().startswith("llvm.H"))
              FunctionResources[F][6]++;
            else if (callee->getName().startswith("llvm.Tdag")) {
              FunctionResources[F][8]++;
	      //errs() << "CI: " << *CI << "\n";
	      //errs() << "Tcount = " << FunctionResources[F][4] << "\n";
	        }
            else if (callee->getName().startswith("llvm.T"))
              FunctionResources[F][7]++;
            else if (callee->getName().startswith("llvm.Sdag"))
              FunctionResources[F][10]++;
            else if (callee->getName().startswith("llvm.S"))
              FunctionResources[F][9]++;
            else if (callee->getName().startswith("llvm.CNOT"))
              FunctionResources[F][11]++;            
            else if (callee->getName().startswith("llvm.PrepZ"))
              FunctionResources[F][12]++;
            else if (callee->getName().startswith("llvm.MeasZ"))
              FunctionResources[F][13]++;
          }

          else if(CI->getCalledFunction()->getName().startswith("afree")){
            //errs() << "begin free\n";
            Type *toAdd = (CI -> getArgOperand(0) -> getType());
            //errs() << "name: " << CI->getArgOperand(0)->getName() << "\n";
            uint64_t addNum=0;
            if (llvm::ConstantInt* consInt = dyn_cast<llvm::ConstantInt>(CI->getArgOperand(1))){
                addNum = consInt -> getLimitedValue();
                errs() << addNum << "this is the value\n";
            }
            else{
              addNum = 1;
            }
            FunctionResources[F][2] -= addNum;
            errs() << "net then width" << FunctionResources[F][2] << FunctionResources[F][3] << "\n"; 
            //errs() << "finish free\n";
          }

          else {  // Non-intrinsic Function Calls
            // Resource numbers must be previously entered
            // for this call. Look them up and add to this function's numbers.
            if (FunctionResources.find(callee) != FunctionResources.end()) {
              unsigned long long* callee_numbers = FunctionResources.find(callee)->second;
			  FunctionResources[F][14] += callee_numbers[14];
              if(callee_numbers[3] > FunctionResources[F][3] - FunctionResources[F][2])
                FunctionResources[F][3] = FunctionResources[F][2] + callee_numbers[3];
              for (int l=0; l<NCOUNTS; l++)
                if(l!=3)
                  FunctionResources[F][l] += callee_numbers[l];
            }
          } // else a non-intrinsic function call
        } // end of call instruction
        // here, we would want to detect the getelement
      } // end of for each instruction
    } // end of procedure

    virtual bool runOnModule (Module &M) {
      // Function* ---> Qubits | Gross Abits | Net Abits | Max Abit Width | X | Z | H | T | CNOT | Toffoli | PrepZ | MeasZ | Rz | Ry
      std::map <Function*, unsigned long long*> FunctionResources;
      std::map <Function*, unsigned long long> FunctionTotals;
  	  std::vector<Function*> callStack;
	  app_total_gates = 0;

      // iterate over all functions, and over all instructions in those functions
      // find call sites that have constant integer values. In Post-Order.
      CallGraphNode* rootNode = getAnalysis<CallGraph>().getRoot();
      
      //fill in the gate count bottom-up in the call graph
      for (scc_iterator<CallGraphNode*> sccIb = scc_begin(rootNode), E = scc_end(rootNode); sccIb != E; ++sccIb) {
        const std::vector<CallGraphNode*> &nextSCC = *sccIb;
        for (std::vector<CallGraphNode*>::const_iterator nsccI = nextSCC.begin(), E = nextSCC.end(); nsccI != E; ++nsccI) {
          Function *F = (*nsccI)->getFunction();	  
          if (F && !F->isDeclaration()) {
            // dynamically create array holding gate numbers for this function
            unsigned long long* ResourceNumbers = new unsigned long long[NCOUNTS];
            for (int k=0; k<NCOUNTS+1; k++)
              ResourceNumbers[k] = 0;
            FunctionResources.insert(std::make_pair(F, ResourceNumbers));
            // count the gates of this function 
            CountFunctionResources(F, FunctionResources);
          }
        }
      }
      
      // print results     
      errs() << "\tQubit\tGross A\tNet A\tWidth\tX\tZ\tH\tT\tT_dag\tS\tS_dag\tCNOT\tPrepZ\tMeasZ\n";
      for (std::map<Function*, unsigned long long*>::iterator i = FunctionResources.begin(), e = FunctionResources.end(); i!=e; ++i) {
        errs() << "Function: " << i->first->getName() << "\n";
		unsigned long long function_total_gates = 0;
        for (int j=0; j<NCOUNTS; j++){
		  if(j > 3){
          	function_total_gates += (i->second)[j];
		  }
          errs() << "\t" << (i->second)[j];
	  	}
		if(i->first->getName() == "main") app_total_gates = function_total_gates;
        errs() << "\n";
        errs() << function_total_gates <<"\n"; //<< " \t";
      }
	  errs() << "total_gates = "<< app_total_gates << "\n";

      // free memory
      for (std::map<Function*, unsigned long long*>::iterator i = FunctionResources.begin(), e = FunctionResources.end(); i!=e; ++i)
        delete [] i->second;


      return false;
    } // End runOnModule
  }; // End of struct ResourceCount2
} // End of anonymous namespace



char ResourceCount::ID = 0;
static RegisterPass<ResourceCount> X("ResourceCount", "Resource Counter Pass");


