//===-------------------------- FunctionDuplicate.cpp ------------------------===//
// This file implements the Scaffold pass of duplicating functions to create
// {fname}_quantum functions.
//
//        This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "FunctionDuplicate"
#include <sstream>
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
#include "llvm/LLVMContext.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Support/CFG.h"
#include "llvm/ADT/SCCIterator.h"

// DEBUG switch
bool debugDuplicate = false;

using namespace llvm;
using namespace std;

// An anonymous namespace for the pass. Things declared inside it are
// only visible to the current file.
namespace {

  vector<Instruction*> vInstRemove;

  vector<BasicBlock::iterator> vectSplitInsts;

  vector<BasicBlock::iterator> vectSplitInsts2;  

  vector<BranchInst*> vBranchReplace;



  // Derived from ModulePass to work on callgraph
  struct FunctionDuplicate : public ModulePass {
    static char ID; // Pass identification

    //external instrumentation function
    Function* qasmResSum; 
    Function* memoize; 
    
    FunctionDuplicate() : ModulePass(ID) {}

    Function *CloneFunctionInfo(const Function *F, ValueMap<const Value*, WeakVH> &VMap, Module *M);

    ArrayRef<Value*> getMemoizeArgs(Function* F, Instruction* I);    

    void visitFunction(Function &F);
    
    void insertNewCallSite(CallInst *CI, std::string specializedName, Module *M);

    bool runOnModule (Module &M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.setPreservesAll();  
        AU.addRequired<CallGraph>();
    }

  }; // End of struct FunctionDuplicate
} // End of anonymous namespace


char FunctionDuplicate::ID = 0;
static RegisterPass<FunctionDuplicate> X("FunctionDuplicate", "Function Duplicating Pass", false, false);


Function *FunctionDuplicate::CloneFunctionInfo(const Function *F, ValueMap<const Value*, WeakVH> &VMap, Module *M) {
  std::vector<Type*> ArgTypes;
  // the user might be deleting arguments to the function by specifying them in the VMap.
  // If so, we need to not add the arguments to the ArgTypes vector

  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end(); I!=E; I++)
    if (VMap.count(I) == 0) // haven't mapped the argument to anything yet?
      ArgTypes.push_back(I->getType());

  // create a new funcion type...
  FunctionType *FTy = FunctionType::get(
      F->getFunctionType()->getReturnType(), ArgTypes, F->getFunctionType()->isVarArg());

  // Create the new function
  Function *NewF = Function::Create(FTy, F->getLinkage(), F->getName(), M);

  // Loop over the arguments, copying the names of the mapped arguments over...
  Function::arg_iterator DestI = NewF->arg_begin();
  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end(); I!=E; ++I)
    if (VMap.count(I) == 0) {     // is this argument preserved?
      DestI->setName(I->getName());   // copy the name over..
      WeakVH wval(DestI++);
      VMap[I] = wval;          // add mapping to VMap
    }
  return NewF;
}


ArrayRef<Value*> FunctionDuplicate::getMemoizeArgs(Function* F, Instruction* I) {
  // int memoize (char *function_name, int *int_params, unsigned num_ints, double *double_params, unsigned num_doubles)          
  vector <Value*> vectCallArgs;
  BasicBlock::iterator it(I);

  Constant *StrConstant = ConstantDataArray::getString(I->getContext(), F->getName());           
  ArrayType* strTy = cast<ArrayType>(StrConstant->getType());
  AllocaInst* strAlloc = new AllocaInst(strTy,"",&*it);
  new StoreInst(StrConstant,strAlloc,"",&*it);	  	  
  Value* Idx[2];	  
  Idx[0] = Constant::getNullValue(Type::getInt32Ty(I->getContext()));  
  Idx[1] = ConstantInt::get(Type::getInt32Ty(I->getContext()),0);
  GetElementPtrInst* strPtr = GetElementPtrInst::Create(strAlloc, Idx, "", &*it);
  
  Value *intArgPtr;
  vector<Value*> vIntArgs;
  unsigned num_ints = 0;
  Value *doubleArgPtr;
  vector<Value*> vDoubleArgs;
  unsigned num_doubles = 0;

  for (Function::arg_iterator FuncArg = F->arg_begin(), E = F->arg_end(); FuncArg != E; ++FuncArg) {

    Value *callArg = (Value*)FuncArg;
    // Integer Arguments
    if(ConstantInt *CInt = dyn_cast<ConstantInt>(callArg)){
      intArgPtr = CInt;
      num_ints++;
      vIntArgs.push_back(intArgPtr);          
    }
    else if (callArg->getType() == Type::getInt32Ty(I->getContext())){ //FIXME: make sure it's an integer
      intArgPtr = CastInst::CreateIntegerCast(callArg, Type::getInt32Ty(I->getContext()), false, "", &*it);
      num_ints++;
      vIntArgs.push_back(intArgPtr);          
    }

    // Double Arguments
    if(ConstantFP *CDouble = dyn_cast<ConstantFP>(callArg)){ 
      doubleArgPtr = CDouble;
      vDoubleArgs.push_back(doubleArgPtr);          
      num_doubles++;
    }
    else if (callArg->getType() == Type::getDoubleTy(I->getContext())){ //FIXME: make sure it's an integer
      doubleArgPtr = CastInst::CreateFPCast(callArg, Type::getDoubleTy(I->getContext()), "", &*it);          
      num_doubles++;
      vDoubleArgs.push_back(doubleArgPtr);          
    }
  }
  
  ArrayType *intArrTy = ArrayType::get(Type::getInt32Ty(I->getContext()), num_ints);
  AllocaInst *intArrAlloc = new AllocaInst(intArrTy, "", &*it);
  for (unsigned i=0; i<num_ints; i++) {
    Value *Int = vIntArgs[i];        
    Idx[1] = ConstantInt::get(Type::getInt32Ty(I->getContext()),i);        
    Value *intPtr = GetElementPtrInst::CreateInBounds(intArrAlloc, Idx, "", &*it);        
    new StoreInst(Int, intPtr, "", &*it);
  }
  Idx[1] = ConstantInt::get(Type::getInt32Ty(I->getContext()),0);        
  GetElementPtrInst* intArrPtr = GetElementPtrInst::CreateInBounds(intArrAlloc, Idx, "", &*it);

  ArrayType *doubleArrTy = ArrayType::get(Type::getDoubleTy(getGlobalContext()), num_doubles);        
  AllocaInst *doubleArrAlloc = new AllocaInst(doubleArrTy,"", &*it);
  for (unsigned i=0; i<num_doubles; i++) {
    Value *Double = vDoubleArgs[i];     
    Idx[1] = ConstantInt::get(Type::getInt32Ty(getGlobalContext()),i);        
    Value *doublePtr = GetElementPtrInst::CreateInBounds(doubleArrAlloc, Idx, "", &*it);        
    new StoreInst(Double, doublePtr, "", &*it);          
  }
  GetElementPtrInst* doubleArrPtr = GetElementPtrInst::CreateInBounds(doubleArrAlloc, Idx, "", &*it);

  Constant *IntNumConstant = ConstantInt::get(Type::getInt32Ty(getGlobalContext()) , num_ints, false);       
  Constant *DoubleNumConstant = ConstantInt::get(Type::getInt32Ty(getGlobalContext()) , num_doubles, false);          

  vectCallArgs.push_back(cast<Value>(strPtr));
  vectCallArgs.push_back(cast<Value>(intArrPtr));
  vectCallArgs.push_back(IntNumConstant);          
  vectCallArgs.push_back(cast<Value>(doubleArrPtr));
  vectCallArgs.push_back(DoubleNumConstant);          

  ArrayRef<Value*> call_args(vectCallArgs);  
  return call_args;
}

void FunctionDuplicate::visitFunction(Function &F) {
  // insert initialization and termination functions in "main"
  if(F.getName() == "main"){
    BasicBlock* BB_last = &(F.back());
    TerminatorInst *BBTerm = BB_last->getTerminator();
    CallInst::Create(qasmResSum, "",(Instruction*)BBTerm);	
    return;
  }
}

bool FunctionDuplicate::runOnModule (Module &M) {
    
  //void qasm_resource_summary ()
  qasmResSum = cast<Function>(M.getOrInsertFunction("summary", Type::getVoidTy(M.getContext()), (Type*)0));
  
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

  
  // iterate over all functions, and over all instructions in those functions
  // find call sites that have constant integer or double values. In Post-Order.
  CallGraphNode* rootNode = getAnalysis<CallGraph>().getRoot();
  
  std::vector<Function*> vectPostOrder;
  
  for (scc_iterator<CallGraphNode*> sccIb = scc_begin(rootNode), E = scc_end(rootNode); sccIb != E; ++sccIb) {
    const std::vector<CallGraphNode*> &nextSCC = *sccIb;
    for (std::vector<CallGraphNode*>::const_iterator nsccI = nextSCC.begin(), E = nextSCC.end(); nsccI != E; ++nsccI) {
      Function *F = (*nsccI)->getFunction();	  
      
      if (F==NULL)
        continue;

      // is this a call to a quantum module? Only those should be instrumented
      // quantum modules arguments are either qbit or qbit* type
      bool isQuantumModuleCall = false;
      for (Function::arg_iterator FuncArg = F->arg_begin(), E = F->arg_end(); FuncArg != E; ++FuncArg) {
        if ((&*FuncArg)->getType()->isPointerTy())
          if((&*FuncArg)->getType()->getPointerElementType()->isIntegerTy(16))
            isQuantumModuleCall = true;
        if ((&*FuncArg)->getType()->isIntegerTy(16))
          isQuantumModuleCall = true;      
      }

      if((F->getName().find("main")!=string::npos) || (!F->isDeclaration() && isQuantumModuleCall))
        vectPostOrder.push_back(F);
    }
  }
  
  //reverse the vector
  std::reverse(vectPostOrder.begin(),vectPostOrder.end());

  // Start traversing in reverse order for a pre-order

  errs()<<"Functions to be duplicated with _qtm appendix:\n";
  for(std::vector<Function*>::iterator vit = vectPostOrder.begin(), vitE = vectPostOrder.end();
      vit!=vitE; ++vit) { 
    Function *F = *vit;      

    // for each quantum function create one with a _qtm appendix
    // which does the job of printing out qasm
    errs()<<F->getName()<<"\n";
    
    std::stringstream ss; 
    ss << "_qtm";
    std::string specializedName = F->getName().str() + ss.str();

    ValueMap<const Value*, WeakVH> VMap;
    Function *specializedFunction = CloneFunctionInfo(F, VMap, &M); 
    specializedFunction->setName(specializedName);

    SmallVector<ReturnInst*,1> Returns; // FIXME: what is the length of this vector?
    ClonedCodeInfo SpecializedFunctionInfo;

    CloneAndPruneFunctionInto (specializedFunction,   // NewFunc
                                F,                    // OldFunc
                                VMap,                 // ValueMap
                                0,                    // ModuleLevelChanges
                                Returns,              // Returns
                                ".",                  // NameSuffix
                                &SpecializedFunctionInfo,  // CodeInfo
                                0);                   // TD            
  
    // after all "alloca" instructions, insert calls to the "memoize" and {fname}_qtm functions
    // use exactly the same arguments as those input arguments to the function declaration 
    inst_iterator instIter = inst_begin(F);
    while(isa<AllocaInst>(*instIter))
      ++instIter;

    // int memoize (char *function_name, int *int_params, unsigned num_ints, double *double_params, unsigned num_doubles)                  
    //---errs() << "1\n";
    //--CallInst::Create(memoize, getMemoizeArgs(F, &*instIter), "", &*instIter);      
    //ArrayRef<Value*> FunctionDuplicate::getMemoizeArgs(Function* F, Instruction* I){
    Instruction* myI = &*instIter;

      // int memoize (char *function_name, int *int_params, unsigned num_ints, double *double_params, unsigned num_doubles)          
      vector <Value*> vectCallArgs;
      BasicBlock::iterator it(myI);

      Constant *StrConstant = ConstantDataArray::getString(myI->getContext(), F->getName());           
      ArrayType* strTy = cast<ArrayType>(StrConstant->getType());
      AllocaInst* strAlloc = new AllocaInst(strTy,"",&*it);
      new StoreInst(StrConstant,strAlloc,"",&*it);    
      Value* Idx[2];  
      Idx[0] = Constant::getNullValue(Type::getInt32Ty(myI->getContext()));  
      Idx[1] = ConstantInt::get(Type::getInt32Ty(myI->getContext()),0);
      GetElementPtrInst* strPtr = GetElementPtrInst::Create(strAlloc, Idx, "", &*it);
  
      Value *intArgPtr;
      vector<Value*> vIntArgs;
      unsigned num_ints = 0;
      Value *doubleArgPtr;
      vector<Value*> vDoubleArgs;
      unsigned num_doubles = 0;

      for (Function::arg_iterator FuncArg = F->arg_begin(), E = F->arg_end(); FuncArg != E; ++FuncArg) {

	Value *callArg = (Value*)FuncArg;
	// Integer Arguments
	if(ConstantInt *CInt = dyn_cast<ConstantInt>(callArg)){
	  intArgPtr = CInt;
	  num_ints++;
	  vIntArgs.push_back(intArgPtr);          
	}
	else if (callArg->getType() == Type::getInt32Ty(myI->getContext())){ //FIXME: make sure it's an integer
	  intArgPtr = CastInst::CreateIntegerCast(callArg, Type::getInt32Ty(myI->getContext()), false, "", &*it);
	  num_ints++;
	  vIntArgs.push_back(intArgPtr);          
	}

	// Double Arguments
	if(ConstantFP *CDouble = dyn_cast<ConstantFP>(callArg)){ 
	  doubleArgPtr = CDouble;
	  vDoubleArgs.push_back(doubleArgPtr);          
	  num_doubles++;
	}
	else if (callArg->getType() == Type::getDoubleTy(myI->getContext())){ //FIXME: make sure it's an integer
	  doubleArgPtr = CastInst::CreateFPCast(callArg, Type::getDoubleTy(myI->getContext()), "", &*it);          
	  num_doubles++;
	  vDoubleArgs.push_back(doubleArgPtr);          
	}
      }
  
      ArrayType *intArrTy = ArrayType::get(Type::getInt32Ty(myI->getContext()), num_ints);
      AllocaInst *intArrAlloc = new AllocaInst(intArrTy, "", &*it);
      for (unsigned i=0; i<num_ints; i++) {
	Value *Int = vIntArgs[i];        
	Idx[1] = ConstantInt::get(Type::getInt32Ty(myI->getContext()),i);        
	Value *intPtr = GetElementPtrInst::CreateInBounds(intArrAlloc, Idx, "", &*it);        
	new StoreInst(Int, intPtr, "", &*it);
      }
      Idx[1] = ConstantInt::get(Type::getInt32Ty(myI->getContext()),0);        
      GetElementPtrInst* intArrPtr = GetElementPtrInst::CreateInBounds(intArrAlloc, Idx, "", &*it);

      ArrayType *doubleArrTy = ArrayType::get(Type::getDoubleTy(getGlobalContext()), num_doubles);        
      AllocaInst *doubleArrAlloc = new AllocaInst(doubleArrTy,"", &*it);
      for (unsigned i=0; i<num_doubles; i++) {
	Value *Double = vDoubleArgs[i];     
	Idx[1] = ConstantInt::get(Type::getInt32Ty(getGlobalContext()),i);        
	Value *doublePtr = GetElementPtrInst::CreateInBounds(doubleArrAlloc, Idx, "", &*it);        
	new StoreInst(Double, doublePtr, "", &*it);          
      }
      GetElementPtrInst* doubleArrPtr = GetElementPtrInst::CreateInBounds(doubleArrAlloc, Idx, "", &*it);

      Constant *IntNumConstant = ConstantInt::get(Type::getInt32Ty(getGlobalContext()) , num_ints, false);       
      Constant *DoubleNumConstant = ConstantInt::get(Type::getInt32Ty(getGlobalContext()) , num_doubles, false);          

      vectCallArgs.push_back(cast<Value>(strPtr));
      vectCallArgs.push_back(cast<Value>(intArrPtr));
      vectCallArgs.push_back(IntNumConstant);          
      vectCallArgs.push_back(cast<Value>(doubleArrPtr));
      vectCallArgs.push_back(DoubleNumConstant);          

      ArrayRef<Value*> call_args(vectCallArgs);  
      //return call_args;
      //}
      CallInst::Create(memoize, call_args, "", &*instIter); 

      //errs() << "2\n";
    // void {fname}_qtm (qbit* ..., int* ..., double* ...)
    std::vector<Value*> Args;
    for (Function::arg_iterator FuncArg = F->arg_begin(), e = F->arg_end(); FuncArg!=e; ++FuncArg)
      Args.push_back(cast<Value>(FuncArg));         
    ArrayRef<Value*> ArgsRef(Args);
    CallInst::Create(M.getFunction(specializedName), ArgsRef, "", &*instIter);


  } // end function iterator

  // iterate a second time to mark places for splitting basic blocks
  for (Module::iterator F = M.begin(); F != M.end(); ++F) {
    for (Function::iterator BB = (*F).begin(); BB != (*F).end(); ++BB) {
      for (BasicBlock::iterator I = (*BB).begin(); I != (*BB).end(); ++I) {
        if (CallInst *CI = dyn_cast<CallInst>(&*I)) {
          if (CI->getCalledFunction()->getName().find("memoize")!=string::npos)
            vectSplitInsts.push_back(++I);
        }
      }
    }
  }   

  // iterate a third time to mark places for splitting basic blocks
  for (Module::iterator F = M.begin(); F != M.end(); ++F) {
    for (Function::iterator BB = (*F).begin(); BB != (*F).end(); ++BB) {
      for (BasicBlock::iterator I = (*BB).begin(); I != (*BB).end(); ++I) {
        if (CallInst *CI = dyn_cast<CallInst>(&*I)) {
          if (CI->getCalledFunction()->getName().find("_qtm")!=string::npos)
            vectSplitInsts2.push_back(++I);
        }
      }
    }
  }  

  // split the if.then part
  if (debugDuplicate)
    errs()<<"if.then part:\n";
  for(vector<BasicBlock::iterator>::iterator v = vectSplitInsts.begin(); v != vectSplitInsts.end(); ++v) {
    Instruction *pInst = &*(*v);
    if (debugDuplicate)
      pInst->dump();
    BasicBlock *BB = pInst->getParent();
    BB->splitBasicBlock(*v, Twine("memoize.if.then"));
  }

  // split the if.end part      
  if (debugDuplicate)      
    errs()<<"if.end part:\n";
  for(vector<BasicBlock::iterator>::iterator v = vectSplitInsts2.begin(); v != vectSplitInsts2.end(); ++v) {
    Instruction *pInst = &*(*v);
    if (debugDuplicate)
      pInst->dump();
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
    if (debugDuplicate)
      errs() << "removing call to: " << (dyn_cast<CallInst>(*iterInst))->getCalledFunction()->getName() << "\n";
    (*iterInst)->eraseFromParent();
  }

  // replacing branches ...
  for(vector<BranchInst*>::iterator iterInst = vBranchReplace.begin(); iterInst != vBranchReplace.end(); ++iterInst) {
    BranchInst* BI = (*iterInst);
    if (debugDuplicate){
      errs() << "replacing branch: \n";
      BI->dump();
    }

    BasicBlock::iterator ii(BI);
    Instruction *memoizeInstruction = &*(--ii);                              // the call to memoize
           
    BasicBlock *currentBlock = BI->getParent();
    BasicBlock *trueBlock = BI->getSuccessor(0)->getTerminator()->getSuccessor(0);
    BasicBlock *falseBlock = BI->getSuccessor(0);

    if (debugDuplicate) {
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

} // End runOnModule




