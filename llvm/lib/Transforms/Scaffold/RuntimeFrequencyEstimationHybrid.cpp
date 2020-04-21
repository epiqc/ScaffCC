//===------------------ RuntimeResourceEst-Memoized.cpp  ------------------===//
//
//                     The LLVM Scaffold Compiler Infrastructure
//
// This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#include <sstream>
#include <iomanip>
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
#include "llvm/IR/LLVMContext.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace std;

#define _MAX_FUNCTION_NAME 90
#define _MAX_INT_PARAMS 4
#define _MAX_DOUBLE_PARAMS 4

#define _X 0
#define _Z 1
#define _H 2
#define _T 3
#define _Tdag 4
#define _S 5
#define _Sdag 6
#define _CNOT 7
#define _PrepZ 8
#define _MeasZ 9
#define _PrepX 10
#define _MeasX 11
#define _Fredkin 12
#define _Toffoli 13
#define _Rx 14
#define _Ry 15
#define _Rz 16

bool debugRTFreqEstHyb = false;


namespace {

  vector<Instruction*> vInstRemove;

  struct RTFreqEstHyb : public ModulePass {

    static char ID;  // Pass identification, replacement for typeid

    //external instrumentation function
    FunctionCallee qasmGate; 
    FunctionCallee qasmResSum; 
    FunctionCallee memoize; 
    FunctionCallee qasmInitialize; 
    FunctionCallee exit_scope;

    //uint32_t rep_val;
    Value* rep_val;

    RTFreqEstHyb() : ModulePass(ID) {  }

    void visitCallInst (BasicBlock::iterator I, AllocaInst* strAlloc, AllocaInst* intArrAlloc, AllocaInst* doubleArrAlloc) {
      CallInst *CI = dyn_cast<CallInst>(&*I);

      Function* CF = CI->getCalledFunction();

      if(debugRTFreqEstHyb)
        errs() << "\tCalls: " << CF->getName() << "\n";

      int gateIndex = 0;
      bool isIntrinsicQuantum = true;
      bool delAfterInst = true; // do not delete Meas gates because it will invalidate cbit stores

      bool isQuantumModuleCall = false;

      // is this a call to a quantum module? Only those should be instrumented
      // quantum modules arguments are either qbit or qbit* type
      for(unsigned iop=0;iop < CI->getNumArgOperands(); iop++) {
        if (CI->getArgOperand(iop)->getType()->isPointerTy())
          if(CI->getArgOperand(iop)->getType()->getPointerElementType()->isIntegerTy(16))
            isQuantumModuleCall = true;
        if (CI->getArgOperand(iop)->getType()->isIntegerTy(16))
          isQuantumModuleCall = true;
      }
      
      if(CF->getName().find("store_cbit") != std::string::npos)
        vInstRemove.push_back((Instruction*)CI);
      
      if(CF->isIntrinsic()) {
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
        else if(CF->getIntrinsicID() == Intrinsic::Z) gateIndex = _Z;
        else { isIntrinsicQuantum = false; delAfterInst = false; }
        if (isIntrinsicQuantum) {
          vector <Value*> vectCallArgs;

          Constant* gateID = ConstantInt::get(Type::getInt32Ty(CI->getContext()), gateIndex, false);	
          //Constant* RepeatConstant = ConstantInt::get(Type::getInt32Ty(getGlobalContext()) , rep_val, false);
          
          vectCallArgs.push_back(gateID);
          //vectCallArgs.push_back(RepeatConstant);
          vectCallArgs.push_back(rep_val);       
          
          ArrayRef<Value*> call_args(vectCallArgs);  
          
          //CallInst::Create(qasmGate, "", (Instruction*)CI);
          
          if(delAfterInst)
            vInstRemove.push_back((Instruction*)CI);

        }
      }
      
      else if (!CF->isDeclaration() && isQuantumModuleCall){
        // insert memoize call before this function call
        // int memoize ( char *function_name, int *int_params, unsigned num_ints, double *double_params, unsigned num_doubles, unsigned repeat)          
        
        vector <Value*> vectCallArgs;
        
        std::stringstream ss;
        ss << std::left << std::setw (_MAX_FUNCTION_NAME-1) << std::setfill(' ') << CF->getName().str();
        Constant *StrConstant = ConstantDataArray::getString(CI->getContext(), ss.str());                   
        
        new StoreInst(StrConstant,strAlloc,false,(Instruction*)CI);	  	  
        Value* Idx[2];	  
        Idx[0] = Constant::getNullValue(Type::getInt32Ty(CI->getContext()));  
        Idx[1] = ConstantInt::get(Type::getInt32Ty(CI->getContext()),0);
        GetElementPtrInst* strPtr = GetElementPtrInst::Create(strAlloc->getAllocatedType(), strAlloc, Idx, "", (Instruction*)CI);
        
        Value *intArgPtr;
        vector<Value*> vIntArgs;
        unsigned num_ints = 0;
        Value *doubleArgPtr;
        vector<Value*> vDoubleArgs;
        unsigned num_doubles = 0;

        for(unsigned iop=0; iop < CI->getNumArgOperands(); iop++) {
          Value *callArg = CI->getArgOperand(iop);
          // Integer Arguments
          if(ConstantInt *CInt = dyn_cast<ConstantInt>(callArg)){
            intArgPtr = CInt;
            num_ints++;
            vIntArgs.push_back(intArgPtr);          
          }
          else if (callArg->getType() == Type::getInt32Ty(CI->getContext())){ //FIXME: make sure it's an integer
            intArgPtr = CastInst::CreateIntegerCast(callArg, Type::getInt32Ty(CI->getContext()), false, "", (Instruction*)CI);
            num_ints++;
            vIntArgs.push_back(intArgPtr);          
          }

          // Double Arguments
          if(ConstantFP *CDouble = dyn_cast<ConstantFP>(CI->getArgOperand(iop))){ 
            doubleArgPtr = CDouble;
            vDoubleArgs.push_back(doubleArgPtr);          
            num_doubles++;
          }
          else if (callArg->getType() == Type::getDoubleTy(CI->getContext())){ //FIXME: make sure it's an integer
            doubleArgPtr = CastInst::CreateFPCast(callArg, Type::getDoubleTy(CI->getContext()), "", (Instruction*)CI);          
            num_doubles++;
            vDoubleArgs.push_back(doubleArgPtr);          
          }
        }
        
        for (unsigned i=0; i<num_ints; i++) {
          Value *Int = vIntArgs[i];        
          Idx[1] = ConstantInt::get(Type::getInt32Ty(CI->getContext()),i);        
          Value *intPtr = GetElementPtrInst::CreateInBounds(intArrAlloc, Idx, "", (Instruction*)CI);        
          new StoreInst(Int, intPtr, false, (Instruction*)CI);
        }
        Idx[1] = ConstantInt::get(Type::getInt32Ty(CI->getContext()),0);        
        GetElementPtrInst* intArrPtr = GetElementPtrInst::CreateInBounds(intArrAlloc, Idx, "", (Instruction*)CI);

        for (unsigned i=0; i<num_doubles; i++) {
          Value *Double = vDoubleArgs[i];     
          Idx[1] = ConstantInt::get(Type::getInt32Ty(CI->getContext()),i);        
          Value *doublePtr = GetElementPtrInst::CreateInBounds(doubleArrAlloc, Idx, "", (Instruction*)CI);        
          new StoreInst(Double, doublePtr, false, (Instruction*)CI);          
        }
        GetElementPtrInst* doubleArrPtr = GetElementPtrInst::CreateInBounds(doubleArrAlloc, Idx, "", (Instruction*)CI);

        Constant *IntNumConstant = ConstantInt::get(Type::getInt32Ty(CI->getContext()) , num_ints, false);       
        Constant *DoubleNumConstant = ConstantInt::get(Type::getInt32Ty(CI->getContext()) , num_doubles, false);          

        //Constant *RepeatConstant = ConstantInt::get(Type::getInt32Ty(getGlobalContext()) , rep_val, false);

        vectCallArgs.push_back(cast<Value>(strPtr));
        vectCallArgs.push_back(cast<Value>(intArrPtr));
        vectCallArgs.push_back(IntNumConstant);          
        vectCallArgs.push_back(cast<Value>(doubleArrPtr));
        vectCallArgs.push_back(DoubleNumConstant);          
        //vectCallArgs.push_back(RepeatConstant);       
        vectCallArgs.push_back(rep_val);       

        ArrayRef<Value*> call_args(vectCallArgs);  
        
        CallInst::Create(memoize, call_args, "", (Instruction*)CI);      

        //CallInst::Create(memoize, getMemoizeArgs(CI, strAlloc, intArrAlloc, doubleArrAlloc), "", (Instruction*)CI);
        
        //vector <Value*> vectCallArgs2;              
        //vectCallArgs2.push_back(rep_val);       
        //ArrayRef<Value*> call_args2(vectCallArgs2);   

        CallInst::Create(exit_scope, "", (&*++I));
        
        isIntrinsicQuantum = false;
        delAfterInst = false;
      }

      else if (CF->getName().find("qasmRepLoopStart") != string::npos) {
        rep_val = CI->getArgOperand(0);
        vInstRemove.push_back((Instruction*)CI);
      }
      
      else if (CF->getName().find("qasmRepLoopEnd") != string::npos) {
        rep_val = ConstantInt::get(Type::getInt32Ty(CI->getContext()), 1, false);
        vInstRemove.push_back((Instruction*)CI);        
      }           
      
    }
    
    void visitFunction(Function &F) {
      // insert alloca instructions at the beginning for subsequent memoize calls   
      bool isQuantumModule = false;
      for(Function::arg_iterator ait=F.arg_begin();ait!=F.arg_end();++ait) {
        if (ait->getType()->isPointerTy())
          if(ait->getType()->getPointerElementType()->isIntegerTy(16))
            isQuantumModule = true;
        if (ait->getType()->isIntegerTy(16))
          isQuantumModule = true;       
      }
      if (F.getName() == "main") 
          isQuantumModule = true;      
      if(!F.isDeclaration() && isQuantumModule){
        BasicBlock* BB_first = &(F.front());
       Instruction *BBiter = BB_first->getFirstNonPHI();
        while(isa<AllocaInst>(BBiter))
          ++BBiter;
        Instruction* pInst = &(*BBiter);
  
        ArrayType *strTy = ArrayType::get(Type::getInt8Ty(pInst->getContext()), _MAX_FUNCTION_NAME);
        AllocaInst *strAlloc = new AllocaInst(strTy,0,"",pInst);
        
        ArrayType *intArrTy = ArrayType::get(Type::getInt32Ty(pInst->getContext()), _MAX_INT_PARAMS);
        AllocaInst *intArrAlloc = new AllocaInst(intArrTy, 0, "", pInst);

        ArrayType *doubleArrTy = ArrayType::get(Type::getDoubleTy(pInst->getContext()), _MAX_DOUBLE_PARAMS);        
        AllocaInst *doubleArrAlloc = new AllocaInst(doubleArrTy,0,"",pInst);

        if(debugRTFreqEstHyb)
          errs() << "Function: " << F.getName() << "\n";
        for (Function::iterator BB = F.begin(); BB != F.end(); ++BB) {
          for (BasicBlock::iterator I = (*BB).begin(); I != (*BB).end(); ++I) {
            if (dyn_cast<CallInst>(&*I))
              visitCallInst(I, strAlloc, intArrAlloc, doubleArrAlloc);
          }
        }
      }
    }
    
    bool runOnModule(Module &M) {
      const char *debug_val = getenv("DEBUG_RUNTIMEFREQUENCYESTIMATIONHYBRID");
      if(debug_val){
        if(!strncmp(debug_val, "1", 1)) debugRTFreqEstHyb = true;
        else debugRTFreqEstHyb = false;
      }

      debug_val = getenv("DEBUG_SCAFFOLD");
      if(debug_val && !debugRTFreqEstHyb){
        if(!strncmp(debug_val, "1", 1)) debugRTFreqEstHyb = true;
        else debugRTFreqEstHyb = false;
      }

      //rep_val = 1;  
      rep_val = ConstantInt::get(Type::getInt32Ty(M.getContext()), 1, false);

      // void exit_scope ()      
      exit_scope = M.getOrInsertFunction("exit_scope", Type::getVoidTy(M.getContext()), (Type*)0);

      //void initialize ()
      qasmInitialize = M.getOrInsertFunction("qasm_initialize", Type::getVoidTy(M.getContext()), (Type*)0);
      
      //void qasm_resource_summary ()
      qasmResSum = M.getOrInsertFunction("qasm_resource_summary", Type::getVoidTy(M.getContext()), (Type*)0);

      // void qasmGate ()      
      qasmGate = M.getOrInsertFunction("qasm_gate", Type::getVoidTy(M.getContext()), (Type*)0);      

      // int memoize (char*, int*, unsigned, double*, unsigned, unsigned)
      vector <Type*> vectParamTypes2;
      vectParamTypes2.push_back(Type::getInt8Ty(M.getContext())->getPointerTo());      
      vectParamTypes2.push_back(Type::getInt32Ty(M.getContext())->getPointerTo());
      vectParamTypes2.push_back(Type::getInt32Ty(M.getContext()));
      vectParamTypes2.push_back(Type::getDoubleTy(M.getContext())->getPointerTo());
      vectParamTypes2.push_back(Type::getInt32Ty(M.getContext()));
      vectParamTypes2.push_back(Type::getInt32Ty(M.getContext()));
      ArrayRef<Type*> Param_Types2(vectParamTypes2);
      Type* Result_Type2 = Type::getInt32Ty(M.getContext());
      memoize = M.getOrInsertFunction(
            "memoize",                          /* Name of Function */
            FunctionType::get(                  /* Type of Function */
              Result_Type2,                     /* Result */
              Param_Types2,                     /* Params */
              false                             /* isVarArg */
              )
            );

      // iterate over instructions to instrument the initialize and exit scope calls
      // insert alloca instructions at the beginning for subsequent memoize calls         
      for (Module::iterator F = M.begin(); F != M.end(); ++F) {
        visitFunction(*F);
      }      

      // insert initialization and termination functions in "main"
      Function* F = M.getFunction("main");
      if(F){
        BasicBlock* BB_last = &(F->back());
        Instruction *BBTerm = BB_last->getTerminator();
        CallInst::Create(qasmResSum, "",(Instruction*)BBTerm);	

        BasicBlock* BB_first = &(F->front());
        Instruction *BBiter = BB_first->getFirstNonPHI();
        while(isa<AllocaInst>(BBiter))
          ++BBiter;
        CallInst::Create(qasmInitialize, "", (Instruction*)&(*BBiter));
      }

      // removing instructions that were marked for deletion
      for(vector<Instruction*>::iterator iterInst = vInstRemove.begin(); iterInst != vInstRemove.end(); ++iterInst) {      
        if (debugRTFreqEstHyb)
          errs() << "removing call to: " << (dyn_cast<CallInst>(*iterInst))->getCalledFunction()->getName() << "\n";
        (*iterInst)->eraseFromParent();
      }

      return true;      
    }
    
    void print(raw_ostream &O, const Module* = 0) const { 
      errs() << "Ran Runtime Resource Estimator Validator \n";
    }  
  };
}

char RTFreqEstHyb::ID = 0;
static RegisterPass<RTFreqEstHyb>
X("runtime-frequency-estimation-hybrid", "Estimate frequency of modules at runtime");
  

