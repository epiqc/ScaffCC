//===- DynCritPath.cpp - Instrument binary to generate flat qasm at runtime -------------------===//
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
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Instructions.h"
#include <map>

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


bool debugDynCritPath = false;

namespace {

  struct DynCritPath : public ModulePass, public InstVisitor<DynCritPath> {
    friend class InstVisitor<DynCritPath>;

    static char ID;  // Pass identification, replacement for typeid

    //external instrumentation functions
    Function* dcpGate; 
    Function* dcpGate2;
    Function* dcpGate3;
    Function* dcpQbitInit;
    Function* dcpAncReset;
    Function* dcpSumm;
    Function* dcpInitAlgo;

    map<AllocaInst*, int> mapAllocaInst;
    //bool isMainFunction;

    vector<Instruction*> vInstRemove;

    DynCritPath() : ModulePass(ID) {  }

    void instrumentInst(CallInst* CI, int id){

	SmallVector<Value*, 16> call_args;
	Value* intArg = ConstantInt::get(Type::getInt32Ty(CI->getContext()),id);	
	call_args.push_back(intArg);

	Value* qbitArg = CI->getArgOperand(0);
	call_args.push_back(qbitArg);

	if(id==0){ //CNOT
	  Value* qbitArg2 = CI->getArgOperand(1);
	  call_args.push_back(qbitArg2);
	  CallInst::Create(dcpGate2,call_args,"",(Instruction*)CI);
	}
	else if(id==1 || id==11){ //Fredkin or Toffoli
	  Value* qbitArg2 = CI->getArgOperand(1);
	  call_args.push_back(qbitArg2);
	  Value* qbitArg3 = CI->getArgOperand(2);
	  call_args.push_back(qbitArg3);
	  CallInst::Create(dcpGate3,call_args,"",(Instruction*)CI);
	}
	else
	  CallInst::Create(dcpGate,call_args,"",(Instruction*)CI);	

	if(id!=3 && id!=4)
	  vInstRemove.push_back((Instruction*)CI);
    }


    bool analyzeAllocInst(Instruction* I) {
      if(AllocaInst *AI = dyn_cast<AllocaInst>(I)){

	//errs() << "AllocInst = " << *AI << "\n";
	
	Type *allocatedType = AI->getAllocatedType();
	
	if(ArrayType *arrayType = dyn_cast<ArrayType>(allocatedType)) {      
	  
	  Type *elementType = arrayType->getElementType();
	  uint64_t arraySize = arrayType->getNumElements();
	  if (elementType->isIntegerTy(16)){
	    if(debugDynCritPath)
	      errs() << "New QBit Allocation Found: " << AI->getName() <<"\n";
	    
	    mapAllocaInst[AI] = arraySize;
	  }	  
	}
	return true;
      }
      else return false;
      
    } // analyzeAllocInst
      

    void initializeQbits(Instruction* CI){

      for(map<AllocaInst*, int>::iterator mit = mapAllocaInst.begin(); mit!=mapAllocaInst.end(); ++mit){
	int arrSize = (*mit).second;

	//generate getelementptr inst
	SmallVector<Value*,16> idxVect;
	idxVect.push_back(ConstantInt::get(Type::getInt32Ty(CI->getContext()),0));
	idxVect.push_back(ConstantInt::get(Type::getInt32Ty(CI->getContext()),0));
	GetElementPtrInst *arrPtr = GetElementPtrInst::Create((Value*)(*mit).first, idxVect, "", (Instruction*) CI);

	//generate call inst to external store function
	SmallVector<Value*,16> call_args;
	call_args.push_back(arrPtr);
	Value* intArg = ConstantInt::get(Type::getInt32Ty(CI->getContext()),arrSize);	
	call_args.push_back(intArg);
	CallInst::Create(dcpQbitInit,call_args,"",(Instruction*)CI);
      }
      //mapAllocaInst.clear();
    }

    void resetAncillaData(Function* F){
      BasicBlock* myBB = &(F->back());
      TerminatorInst *BBTerm = myBB->getTerminator();

      for(map<AllocaInst*, int>::iterator mit = mapAllocaInst.begin(); mit!=mapAllocaInst.end(); ++mit){
	int arrSize = (*mit).second;

	//generate getelementptr inst
	SmallVector<Value*,16> idxVect;
	idxVect.push_back(ConstantInt::get(Type::getInt32Ty(BBTerm->getContext()),0));
	idxVect.push_back(ConstantInt::get(Type::getInt32Ty(BBTerm->getContext()),0));
	GetElementPtrInst *arrPtr = GetElementPtrInst::Create((Value*)(*mit).first, idxVect, "", (Instruction*)BBTerm);

	//generate call inst to external store function
	SmallVector<Value*,16> call_args;
	call_args.push_back(arrPtr);
	Value* intArg = ConstantInt::get(Type::getInt32Ty(BBTerm->getContext()),arrSize);	
	call_args.push_back(intArg);
	CallInst::Create(dcpAncReset,call_args,"",(Instruction*)BBTerm);
      }
      mapAllocaInst.clear();

    }

    void visitCallInst(CallInst &I) {
      CallInst *CI = dyn_cast<CallInst>(&I);

      //errs() << "CallInst = " << *CI << "\n";

      Function* CF = CI->getCalledFunction();

      int gateIndex = -1;
      bool isIntrinsicQuantum = true;

      if(CF->isIntrinsic()){
	  if(CF->getIntrinsicID() == Intrinsic::CNOT) gateIndex = _CNOT;
	  else if(CF->getIntrinsicID() == Intrinsic::Fredkin) gateIndex = _Fredkin;
	  else if(CF->getIntrinsicID() == Intrinsic::H) gateIndex = _H;
	  else if(CF->getIntrinsicID() == Intrinsic::MeasX) { gateIndex = _MeasX; }
	  else if(CF->getIntrinsicID() == Intrinsic::MeasZ) { gateIndex = _MeasZ; }
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
	  else { isIntrinsicQuantum = false; }
      }
      else{
	isIntrinsicQuantum = false;
      }
            
      if(isIntrinsicQuantum){
	instrumentInst(CI, gateIndex);
      }
    }
        
    bool runOnModule(Module &M){

      dcpGate = cast<Function>(M.getOrInsertFunction("dcp_qgate", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), Type::getInt16Ty(M.getContext()), (Type*)0));
      
      dcpGate2 = cast<Function>(M.getOrInsertFunction("dcp_qgate2", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), Type::getInt16Ty(M.getContext()), Type::getInt16Ty(M.getContext()), (Type*)0));

      dcpGate3 = cast<Function>(M.getOrInsertFunction("dcp_qgate3", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), Type::getInt16Ty(M.getContext()), Type::getInt16Ty(M.getContext()), Type::getInt16Ty(M.getContext()), (Type*)0));

      dcpQbitInit = cast<Function>(M.getOrInsertFunction("dcp_qbit_init", Type::getVoidTy(M.getContext()), Type::getInt16Ty(M.getContext())->getPointerTo(), Type::getInt32Ty(M.getContext()), (Type*)0));

      dcpAncReset = cast<Function>(M.getOrInsertFunction("dcp_anc_reset", Type::getVoidTy(M.getContext()), Type::getInt16Ty(M.getContext())->getPointerTo(), Type::getInt32Ty(M.getContext()), (Type*)0));
      
      dcpSumm = cast<Function>(M.getOrInsertFunction("dcp_summary", Type::getVoidTy(M.getContext()), (Type*)0));

      dcpInitAlgo = cast<Function>(M.getOrInsertFunction("dcp_init_algo", Type::getVoidTy(M.getContext()), (Type*)0));
        
      for(Module::iterator F = M.begin(); F!=M.end(); ++F){
	if(F && !F->isDeclaration()){
	  //errs() << "Func : " << F->getName() << "\n";	  	  

	  //analyze Alloc Insts first
	  for(inst_iterator instIb = inst_begin(F); instIb!=inst_end(F); ++instIb){
	    Instruction* pInst = &*instIb;
	    bool isAlloc = analyzeAllocInst(pInst);
	    if(!isAlloc){

	      if(F->getName() == "main")
	      {
		  //add call to init_algo
	        CallInst::Create(dcpInitAlgo,"",pInst);
	      }

	      //add store functions for alloc insts
	      initializeQbits(pInst); 
	      break;
	    }
	  }

	  //analyze Call insts now
	  visit(F);
	  
	  //reset data for ancilla qubits to save qubits
	  resetAncillaData(F);

	  if(F->getName() == "main"){
	    BasicBlock* myBB = &(F->back());
	    TerminatorInst *BBTerm = myBB->getTerminator();
	    
	    CallInst::Create(dcpSumm,"",(Instruction*)BBTerm);
	  }   	
	}      
      }
      //errs() << "Removing instructions:\n";
      for(vector<Instruction*>::iterator iterInst = vInstRemove.begin(); iterInst != vInstRemove.end(); ++iterInst){
	//errs() << *(*iterInst) << "\n";
	(*iterInst)->eraseFromParent();
      }     

      return true;      
    }
    
    void print(raw_ostream &O, const Module* = 0) const { 
      errs() << "Instrumented program with blocks for circuit split analysis \n";
    }  
  };
}

char DynCritPath::ID = 0;
static RegisterPass<DynCritPath>
X("dyn-critical-path", "Instrument binary for dynamic estimation of critical path");
  
