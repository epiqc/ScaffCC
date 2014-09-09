//===- XformCbitStores.cpp - Transform stores to Cbits  -------------------===//
//===------------- into calls to dummy function that cannot ---------------===//
//===--------------  be eliminated by deadcode_elim  ----------------------===//
//
//                     The LLVM Scaffold Compiler Infrastructure
//
//        This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//


#include "llvm/ADT/ArrayRef.h"
#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Intrinsics.h"
#include "llvm/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace std;

namespace {
  struct XformCbitStores : public ModulePass {
    static char ID;
    XformCbitStores() : ModulePass(ID) {}
    

    virtual bool runOnModule(Module &M) {
      
      //create a new function called store_cbit(i1,i1*)      
      string dummyFuncName = "store_cbit";

      vector<Type*> dummyFuncArgs;
      dummyFuncArgs.push_back(Type::getInt1Ty(getGlobalContext()));
      dummyFuncArgs.push_back(Type::getInt1PtrTy(getGlobalContext()));
      ArrayRef<Type*> dummyFuncArgsRef(dummyFuncArgs);

      FunctionType *dummyFuncType = FunctionType::get(
						      Type::getVoidTy(getGlobalContext()),
						      dummyFuncArgsRef,
						      false);      
      
      Function* dummy_store_func = Function::Create(dummyFuncType, GlobalVariable::ExternalLinkage,dummyFuncName, &M);      
      
      for(Module::iterator F = M.begin(), E = M.end(); F!= E; ++F){
	for(inst_iterator I = inst_begin(F), IE = inst_end(F); I != IE; ++I){
	  
	  Instruction *pInst = &*I;
	  
	  if(StoreInst* SI = dyn_cast<StoreInst>(pInst)) {	    
	    Value* valOp = SI->getValueOperand();
	    Value* ptrOp = SI->getPointerOperand();
	    
	    Type* valType = valOp->getType();
	    Type* ptrType = ptrOp->getType();
	    Type* ptrElemType = ptrType->getPointerElementType();
	    
	    if(valType->isIntegerTy(1) && ptrElemType->isIntegerTy(1))
	      {
		//errs()<<"Found a Cbit Store\n";

		//take both arguments and create a new call inst		
		vector<Value*> callArgs;
		callArgs.push_back(valOp);
		callArgs.push_back(ptrOp);
		ArrayRef<Value*> callArgsRef(callArgs);

		//insert the call inst after or before the store inst
		CallInst::Create(dummy_store_func,callArgsRef,"",(Instruction*)SI);		
	      }	    
	  } // dyn_cast<StoreInst>
	}
      }	    
      return true;
    } // runOnModule()    
  }; // struct XformCbitStores
} // namespace

char XformCbitStores::ID = 0;
static RegisterPass<XformCbitStores> X("xform-cbit-stores", "Transform Cbit Stores into a dummy function to avoid deadcode elim", false, false);
