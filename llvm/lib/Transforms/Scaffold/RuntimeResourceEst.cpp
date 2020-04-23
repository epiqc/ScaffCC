//===- QBitDataPath.cpp - Generate datapaths for qubits in code -----------===//
//
//                     The LLVM Scaffold Compiler Infrastructure
//
// This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#include <cstring>
#include <cstdlib>
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/InstVisitor.h" 
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
#define _Rx 15
#define _Ry 16
#define _Rz 17

bool debugRTResourceEst = false;

namespace {

  vector<Instruction*> vInstRemove;

  struct RTResourceEst : public ModulePass, public InstVisitor<RTResourceEst> {
    friend class InstVisitor<RTResourceEst>;

    static char ID;  // Pass identification, replacement for typeid

    FunctionCallee qasmGate; //external instrumentation function
    FunctionCallee qasmQbitDecl; //external instrumentation function    
    FunctionCallee qasmCbitDecl; //external instrumentation function    
    FunctionCallee qasmResSum; //external instrumentation function    

    RTResourceEst() : ModulePass(ID) {  }

    void instrumentInst(FunctionCallee F,Instruction* pInst, int intParam, bool isCall){
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
	  else if(CF->getIntrinsicID() == Intrinsic::Rx) gateIndex = _Rx;
	  else if(CF->getIntrinsicID() == Intrinsic::Ry) gateIndex = _Ry;
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
	Instruction *BBTerm = BB->getTerminator();
	CallInst::Create(qasmResSum, "",(Instruction*)BBTerm);	
      }
    }
    
    bool runOnModule(Module &M){
      const char *debug_val = getenv("DEBUG_RUNTIMERESOURCEST");
      if(debug_val){
        if(!strncmp(debug_val, "1", 1)) debugRTResourceEst = true;
        else debugRTResourceEst = false;
      }

      debug_val = getenv("DEBUG_SCAFFOLD");
      if(debug_val && !debugRTResourceEst){
        if(!strncmp(debug_val, "1", 1)) debugRTResourceEst = true;
        else debugRTResourceEst = false;
      }

      qasmGate = M.getOrInsertFunction("qasm_gate", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), (Type*)0);
      
      qasmQbitDecl = M.getOrInsertFunction("qasm_qbit_decl", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), (Type*)0);
      
      qasmCbitDecl = M.getOrInsertFunction("qasm_cbit_decl", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), (Type*)0);
      
      qasmResSum = M.getOrInsertFunction("qasm_resource_summary", Type::getVoidTy(M.getContext()), (Type*)0);
            
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
  
