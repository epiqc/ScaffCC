//===------------------ RuntimeResourceEst-Memoized.cpp  ------------------===//
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
#include "llvm/LLVMContext.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace std;

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
#define _Rz 14

bool debugMemoInstrumentation = false;


namespace {

  vector<Instruction*> vInstRemove;

  vector<BasicBlock::iterator> vectSplitInsts;

  vector<BasicBlock::iterator> vectSplitInsts2;  

  vector<BranchInst*> vBranchReplace;

  struct RTResourceEst_Mem : public ModulePass {

    static char ID;  // Pass identification, replacement for typeid

    //external instrumentation function
    Function* qasmGate; 
    Function* qasmQbitDecl; 
    Function* qasmCbitDecl; 
    Function* qasmResSum; 
    Function* memoize; 
    Function* qasmInitialize; 
    Function* exit_scope;

    RTResourceEst_Mem() : ModulePass(ID) {  }

    void instrumentInst(Function* F, Instruction* pInst, int intParam, bool toDel){
      SmallVector<Value*,16> call_args;
      Value* intArg = ConstantInt::get(Type::getInt32Ty(pInst->getContext()),intParam);	
      call_args.push_back(intArg);

      CallInst::Create(F, call_args, "", (Instruction*)pInst);

      if(toDel)
	      vInstRemove.push_back(pInst);
    }
    

    void visitCallInst (BasicBlock::iterator I) {
      CallInst *CI = dyn_cast<CallInst>(&*I);

      Function* CF = CI->getCalledFunction();

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
      
      if(CF->isIntrinsic()) {
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
        else if(CF->getIntrinsicID() == Intrinsic::Z) gateIndex = _Z;
        else { isIntrinsicQuantum = false; delAfterInst = false; }
        if (isIntrinsicQuantum)
          instrumentInst(qasmGate,CI,gateIndex,delAfterInst);

      }
      
      else if (!CF->isDeclaration() && isQuantumModuleCall){
        // insert memoize call before this function call
        // int memoize ( char *function_name, int *int_params, unsigned num_ints, double *double_params, unsigned num_doubles)          
        vector <Value*> vectCallArgs;
        
        Constant *StrConstant = ConstantDataArray::getString(CI->getContext(), CF->getName());           
        ArrayType* strTy = cast<ArrayType>(StrConstant->getType());
        AllocaInst* strAlloc = new AllocaInst(strTy,"",(Instruction*)CI);
        new StoreInst(StrConstant,strAlloc,"",(Instruction*)CI);	  	  
        Value* Idx[2];	  
        Idx[0] = Constant::getNullValue(Type::getInt32Ty(CI->getContext()));  
        Idx[1] = ConstantInt::get(Type::getInt32Ty(CI->getContext()),0);
        GetElementPtrInst* strPtr = GetElementPtrInst::Create(strAlloc, Idx, "", (Instruction*)CI);
        
        vector<uint32_t> valueOfInt;
        unsigned num_ints = 0;
        vector<double> valueOfDouble;
        unsigned num_doubles = 0;

        for(unsigned iop=0;iop < CI->getNumArgOperands(); iop++) {
          if(ConstantInt *CInt = dyn_cast<ConstantInt>(CI->getArgOperand(iop))){
            valueOfInt.push_back((uint32_t)CInt->getZExtValue());
            num_ints++;
          }
          if(ConstantFP *CDouble = dyn_cast<ConstantFP>(CI->getArgOperand(iop))){ 
            valueOfDouble.push_back(CDouble->getValueAPF().convertToDouble());
            num_doubles++;
          }
        }

        Constant *IntArrayConstant = ConstantDataArray::get(CI->getContext(), ArrayRef<uint32_t>(valueOfInt));
        ArrayType *intArrTy = cast<ArrayType>(IntArrayConstant->getType());
        AllocaInst *intArrAlloc = new AllocaInst(intArrTy,"",(Instruction*)CI);
        new StoreInst(IntArrayConstant,intArrAlloc,"",(Instruction*)CI);
        GetElementPtrInst* intArrPtr = GetElementPtrInst::CreateInBounds(intArrAlloc, Idx, "", (Instruction*)CI);

        Constant *DoubleArrayConstant = ConstantDataArray::get(CI->getContext(), ArrayRef<double>(valueOfDouble));
        ArrayType *doubleArrTy = cast<ArrayType>(DoubleArrayConstant->getType());
        AllocaInst *doubleArrAlloc = new AllocaInst(doubleArrTy,"",(Instruction*)CI);
        new StoreInst(DoubleArrayConstant,doubleArrAlloc,"",(Instruction*)CI);	  	  
        GetElementPtrInst* doubleArrPtr = GetElementPtrInst::CreateInBounds(doubleArrAlloc, Idx, "", (Instruction*)CI);
        
        Constant *IntNumConstant = ConstantInt::get(Type::getInt32Ty(getGlobalContext()) , num_ints, false);       
        Constant *DoubleNumConstant = ConstantInt::get(Type::getInt32Ty(getGlobalContext()) , num_doubles);          

        vectCallArgs.push_back(cast<Value>(strPtr));
        vectCallArgs.push_back(cast<Value>(intArrPtr));
        vectCallArgs.push_back(IntNumConstant);          
        vectCallArgs.push_back(cast<Value>(doubleArrPtr));
        vectCallArgs.push_back(DoubleNumConstant);          

        ArrayRef<Value*> call_args(vectCallArgs);   

        CallInst::Create(memoize, call_args, "", (Instruction*)CI);      
        //instrumentInst(memoize, CI, call_args, false);
        
        // mark instruction for split basicblock 
        vectSplitInsts.push_back(I);
        vectSplitInsts2.push_back(++I);                    
        isIntrinsicQuantum = false;
        delAfterInst = false;
      }
      
    }
    
    /*void visitAllocaInst(AllocaInst *AI) {
	
      Type *allocatedType = AI->getAllocatedType();
	
      if(ArrayType *arrayType = dyn_cast<ArrayType>(allocatedType)) {      
        
        Type *elementType = arrayType->getElementType();
        uint64_t arraySize = arrayType->getNumElements();
        if (elementType->isIntegerTy(16)){
            errs() << "New QBit Allocation Found: " << AI->getName() <<"\n";
          
          //instrumentation
          //instrumentInst(qasmQbitDecl,AI,arraySize,false);
        }
          
        if (elementType->isIntegerTy(1)){
            errs() << "New CBit Allocation Found: " << AI->getName() <<"\n";
          
          //instrumentation
          //instrumentInst(qasmCbitDecl,AI,arraySize,false);
        }
      }
    } // visitAllocaInst/
    */

    void visitFunction(Function &F) {
      // insert initialization and termination functions in "main"
      if(F.getName() == "main"){
        BasicBlock* BB_last = &(F.back());
        TerminatorInst *BBTerm = BB_last->getTerminator();
        CallInst::Create(qasmResSum, "",(Instruction*)BBTerm);	

        BasicBlock* BB_first = &(F.front());
        BasicBlock::iterator BBiter = BB_first->getFirstNonPHI();
        while(isa<AllocaInst>(BBiter))
          ++BBiter;
        CallInst::Create(qasmInitialize, "", (Instruction*)&(*BBiter));
        return;
      }

      // insert exit_scope instruction in other quantum modules for end of scope indication      
      bool isQuantumModule = false;
      for(Function::arg_iterator ait=F.arg_begin();ait!=F.arg_end();++ait) {
        if (ait->getType()->isPointerTy())
          if(ait->getType()->getPointerElementType()->isIntegerTy(16))
            isQuantumModule = true;
        if (ait->getType()->isIntegerTy(16))
          isQuantumModule = true;        
      }
      if(!F.isDeclaration() && isQuantumModule){
        BasicBlock* BB_last = &(F.back());
        TerminatorInst *BBTerm = BB_last->getTerminator();
        CallInst::Create(exit_scope, "",(Instruction*)BBTerm);	
      }
    }
    
    bool runOnModule(Module &M) {
      // void qasm_qbit_decl (int)
      qasmQbitDecl = cast<Function>(M.getOrInsertFunction("qasm_qbit_decl", 
                                    Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), 
                                    (Type*)0));
      
      // void qasm_cbit_decl (int)
      qasmCbitDecl = cast<Function>(M.getOrInsertFunction("qasm_cbit_decl", 
                                    Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), 
                                    (Type*)0));

      // void exit_scope ()      
      exit_scope = cast<Function>(M.getOrInsertFunction("exit_scope", Type::getVoidTy(M.getContext()), (Type*)0));

      //void initialize ()
      qasmInitialize = cast<Function>(M.getOrInsertFunction("qasm_initialize", Type::getVoidTy(M.getContext()), (Type*)0));
      
      //void qasm_resource_summary ()
      qasmResSum = cast<Function>(M.getOrInsertFunction("qasm_resource_summary", Type::getVoidTy(M.getContext()), (Type*)0));

      // void qasmGate (int gate_id)      
      qasmGate = cast<Function>(M.getOrInsertFunction("qasm_gate", 
            Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), (Type*)0));      


      // int memoize (char*, int*, unsigned, double*, unsigned)
      vector <Type*> vectParamTypes2;
      vectParamTypes2.push_back(Type::getInt8Ty(M.getContext())->getPointerTo());      
      vectParamTypes2.push_back(Type::getInt32Ty(M.getContext())->getPointerTo());
      vectParamTypes2.push_back(Type::getInt32Ty(M.getContext()));
      vectParamTypes2.push_back(Type::getDoubleTy(M.getContext())->getPointerTo());
      vectParamTypes2.push_back(Type::getInt32Ty(M.getContext()));
      ArrayRef<Type*> Param_Types2(vectParamTypes2);
      Type* Result_Type2 = Type::getInt32Ty(M.getContext());
      memoize = cast<Function> (  
          M.getOrInsertFunction(
            "memoize",                          /* Name of Function */
            FunctionType::get(                  /* Type of Function */
              Result_Type2,                     /* Result */
              Param_Types2,                     /* Params */
              false                             /* isVarArg */
              )
            )
          );
      
      // iterate over instructions to instrument
      for (Module::iterator F = M.begin(); F != M.end(); ++F) {
        for (Function::iterator BB = (*F).begin(); BB != (*F).end(); ++BB) {
          for (BasicBlock::iterator I = (*BB).begin(); I != (*BB).end(); ++I) {
            if (dyn_cast<CallInst>(&*I))
              visitCallInst(I); 
            //else if (AllocaInst *AI = dyn_cast<AllocaInst>(&*I))
            //  visitAllocaInst(AI);
          }
        }
      }
       

      // split the if.then part
      for(vector<BasicBlock::iterator>::iterator v = vectSplitInsts.begin(); v != vectSplitInsts.end(); ++v) {
        Instruction *pInst = &*(*v);
        BasicBlock *BB = pInst->getParent();
        BB->splitBasicBlock(*v, Twine("memoize.if.then"));
      }

      // split the if.end part
      for(vector<BasicBlock::iterator>::iterator v = vectSplitInsts2.begin(); v != vectSplitInsts2.end(); ++v) {
        Instruction *pInst = &*(*v);
        BasicBlock *BB = pInst->getParent();
        BB->splitBasicBlock(*v, Twine("memoize.if.end"));
      }

      // second iteration over instructions -- to change branches to conditional branch
      for (Module::iterator F = M.begin(); F != M.end(); ++F) {
        for (Function::iterator BB = (*F).begin(); BB != (*F).end(); ++BB) {
          for (BasicBlock::iterator I = (*BB).begin(); I != (*BB).end(); ++I) {
            if (BranchInst *BI = dyn_cast<BranchInst>(&*I)) {
              if (BI->isUnconditional() && BI->getSuccessor(0)->getName().find("memoize.if.then")!=string::npos) {
                vBranchReplace.push_back(BI);
                              }
            }
          }
        }
        visitFunction(*F); // visit function must happen after Basic Block splittings since it creates
                          // the "exit_scope()" call for all functions, and they must always be at the end        
      }
      

      // removing instructions that were marked for deletion
      for(vector<Instruction*>::iterator iterInst = vInstRemove.begin(); iterInst != vInstRemove.end(); ++iterInst) {
        if (debugMemoInstrumentation)
          errs() << "removing call to: " << (dyn_cast<CallInst>(*iterInst))->getCalledFunction()->getName() << "\n";
        (*iterInst)->eraseFromParent();
      }

      // replacing branches ...
      for(vector<BranchInst*>::iterator iterInst = vBranchReplace.begin(); iterInst != vBranchReplace.end(); ++iterInst) {
        BranchInst* BI = (*iterInst);
        if (debugMemoInstrumentation){
          errs() << "replacing branch: \n";
          BI->dump();
        }

        BasicBlock::iterator ii(BI);
        Instruction *memoizeInstruction = &*(--ii);                              // the call to memoize
               
        BasicBlock *currentBlock = BI->getParent();
        BasicBlock *trueBlock = BI->getSuccessor(0)->getTerminator()->getSuccessor(0);
        BasicBlock *falseBlock = BI->getSuccessor(0);

        if (debugMemoInstrumentation) {
          errs() << "current block: " << currentBlock->getName() << "\n";
          errs() << "true block: " << trueBlock->getName() << "\n";
          errs() << "false block: " << falseBlock->getName() << "\n";
        }

        // erasing this currentBlock's unconditional branch instruction
        currentBlock->getTerminator()->eraseFromParent();

        // inserting ICmpInst at the end of currentBlock
        ICmpInst *test = new ICmpInst(*currentBlock, CmpInst::ICMP_EQ, 
            memoizeInstruction, ConstantInt::get(Type::getInt32Ty(M.getContext()), 1), "shadow check");
                
        BranchInst::Create (trueBlock, falseBlock, test, currentBlock);
        
      }
      
      return true;      
    }
    
    void print(raw_ostream &O, const Module* = 0) const { 
      errs() << "Ran Runtime Resource Estimator Validator \n";
    }  
  };
}

char RTResourceEst_Mem::ID = 0;
static RegisterPass<RTResourceEst_Mem>
X("runtime-resource-estimation-memoized", "Estimate qbits and qgates at runtime with memoization");
  
