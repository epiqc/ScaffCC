//===- QBitDataPath.cpp - Generate datapaths for qubits in code -----------===//
//
//                     The LLVM Scaffold Compiler Infrastructure
//
// This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instruction.h"
#include "llvm/Constants.h"
#include "llvm/Intrinsics.h"
#include "llvm/Support/InstVisitor.h" 
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace std;

#define _CNOT 0
#define _Fredkin 1
#define _H 2
#define _MeasX 3
#define _MeasZ 4
#define _PrepX 5
#define _PrepZ 6
#define _S 7
#define _T 8
#define _Sdag 9
#define _Tdag 10
#define _Toffoli 11
#define _X 12
#define _Y 13
#define _Z 14
#define _Rz 15

bool debugRTResourceEst = false;

namespace {

  vector<Instruction*> vInstRemove;

  struct RTResourceEst : public ModulePass, public InstVisitor<RTResourceEst> {
    friend class InstVisitor<RTResourceEst>;

    static char ID;  // Pass identification, replacement for typeid

    Function* qasmGate; //external instrumentation function
    Function* qasmQbitDecl; //external instrumentation function    
    Function* qasmCbitDecl; //external instrumentation function    
    Function* qasmResSum; //external instrumentation function    

    RTResourceEst() : ModulePass(ID) {  }

    void instrumentInst(Function* F,Instruction* pInst, int intParam, bool isCall){
      SmallVector<Value*,16> call_args;
      Value* intArg = ConstantInt::get(Type::getInt32Ty(pInst->getContext()),intParam);	
      call_args.push_back(intArg);

      CallInst::Create(F, call_args,"",(Instruction*)pInst);

      if(isCall)
	vInstRemove.push_back(pInst);
    }
    
    void visitCallInst(CallInst &I) {
      CallInst *CI = dyn_cast<CallInst>(&I);

      Function* CF = CI->getCalledFunction();

      int gateIndex = 14;
      bool isIntrinsicQuantum = true;

      bool delAfterInst = true;

      if(CF->isIntrinsic()){
	  if(CF->getIntrinsicID() == Intrinsic::CNOT) gateIndex = _CNOT;
	  else if(CF->getIntrinsicID() == Intrinsic::Fredkin) gateIndex = _Fredkin;
	  else if(CF->getIntrinsicID() == Intrinsic::H) gateIndex = _H;
	  else if(CF->getIntrinsicID() == Intrinsic::MeasX) { gateIndex = _MeasX; delAfterInst = false; }
	  else if(CF->getIntrinsicID() == Intrinsic::MeasZ) { gateIndex = _MeasZ; delAfterInst = false; }
	  else if(CF->getIntrinsicID() == Intrinsic::PrepX) gateIndex = _PrepX;
	  else if(CF->getIntrinsicID() == Intrinsic::PrepZ) gateIndex = _PrepZ;
	  else if(CF->getIntrinsicID() == Intrinsic::Rz) gateIndex = _Rz;
	  else if(CF->getIntrinsicID() == Intrinsic::S) gateIndex = _S;
	  else if(CF->getIntrinsicID() == Intrinsic::T) gateIndex = _T;
	  else if(CF->getIntrinsicID() == Intrinsic::Sdag) gateIndex = _Sdag;
	  else if(CF->getIntrinsicID() == Intrinsic::Tdag) gateIndex = _Tdag;
	  else if(CF->getIntrinsicID() == Intrinsic::Toffoli) gateIndex = _Toffoli;
	  else if(CF->getIntrinsicID() == Intrinsic::X) gateIndex = _X;
	  else if(CF->getIntrinsicID() == Intrinsic::Y) gateIndex = _Y;
	  else if(CF->getIntrinsicID() == Intrinsic::Z) gateIndex = _Z;
	  else { isIntrinsicQuantum = false; delAfterInst = false; }
      }
      else{
	isIntrinsicQuantum = false;
	delAfterInst = false;
      }
      
      if(isIntrinsicQuantum)
	instrumentInst(qasmGate,CI,gateIndex,delAfterInst);
      
    }
    
    void visitAllocaInst(AllocaInst &I) {
      AllocaInst *AI = dyn_cast<AllocaInst>(&I);
	
      Type *allocatedType = AI->getAllocatedType();
	
      if(ArrayType *arrayType = dyn_cast<ArrayType>(allocatedType)) {      
	
	Type *elementType = arrayType->getElementType();
	uint64_t arraySize = arrayType->getNumElements();
	if (elementType->isIntegerTy(16)){
	  if(debugRTResourceEst)
	    errs() << "New QBit Allocation Found: " << AI->getName() <<"\n";
	  
	  //instrumentation
	  instrumentInst(qasmQbitDecl,AI,arraySize,false);
	  }
	  
	if (elementType->isIntegerTy(1)){
	  if(debugRTResourceEst)
	    errs() << "New CBit Allocation Found: " << AI->getName() <<"\n";
	  
	  //instrumentation
	  instrumentInst(qasmCbitDecl,AI,arraySize,false);
	}
      }
    } // visitAllocaInst
    
    void visitFunction(Function &F){
      if(F.getName() == "main"){
	BasicBlock* BB = &F.back();
	TerminatorInst *BBTerm = BB->getTerminator();
	CallInst::Create(qasmResSum, "",(Instruction*)BBTerm);	
      }
    }
    
    bool runOnModule(Module &M){
      qasmGate = cast<Function>(M.getOrInsertFunction("qasm_gate", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), (Type*)0));
      
      qasmQbitDecl = cast<Function>(M.getOrInsertFunction("qasm_qbit_decl", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), (Type*)0));
      
      qasmCbitDecl = cast<Function>(M.getOrInsertFunction("qasm_cbit_decl", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), (Type*)0));
      
      qasmResSum = cast<Function>(M.getOrInsertFunction("qasm_resource_summary", Type::getVoidTy(M.getContext()), (Type*)0));
            
      visit(M);


      //errs() << "Removing instructions:\n";
      for(vector<Instruction*>::iterator iterInst = vInstRemove.begin(); iterInst != vInstRemove.end(); ++iterInst){
	//errs() << *(*iterInst) << "\n";
	(*iterInst)->eraseFromParent();
      }     
      
      return true;      
    }
    
    void print(raw_ostream &O, const Module* = 0) const { 
      errs() << "Ran Runtime Resource Estimator Validator \n";
    }  
  };
}

char RTResourceEst::ID = 0;
static RegisterPass<RTResourceEst>
X("runtime-resource-estimation", "Estimate qbits and qgates at runtime");
  
