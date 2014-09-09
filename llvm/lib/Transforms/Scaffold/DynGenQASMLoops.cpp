//===- DynGenQASMLoops.cpp - Generate qasm output with loops -------------------===//
//
//                     The LLVM Scaffold Compiler Infrastructure
//
// This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#include <sstream>
#include <iomanip>
#include "llvm/Argument.h"
#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instruction.h"
#include "llvm/Instructions.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/ilist.h"
#include "llvm/Constants.h"
#include "llvm/Analysis/DebugInfo.h"
#include "llvm/IntrinsicInst.h"

using namespace llvm;
using namespace std;

#define MAX_BT_COUNT 15 //max backtrace allowed - to avoid infinite recursive loops
#define MAX_QBIT_ARR_DIM 5 //max dimensions allowed for qbit arrays
#define MAX_FUNCTION_NAME 32

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
#define _Ry 17
#define _Rx 18


bool debugDynGenQASMLoops = false;

namespace {

  struct qGateArg{ //arguments to qgate calls
    Value* argPtr;
    int argNum;
    bool isQbit;
    bool isCbit;
    bool isUndef;
    bool isPtr;
    bool isDouble;
    int numDim; //number of dimensions of qbit array  
    int dimSize[MAX_QBIT_ARR_DIM]; //sizes of dimensions of array for qbit declarations OR indices of specific qbit for gate arguments
    int valOrIndex; //Value if not Qbit, Index if Qbit & not a Ptr
    double val;
    //Note: valOrIndex is of type integer. Assumes that quantities will be int in the program.
    qGateArg(): argPtr(NULL), argNum(-1), isQbit(false), isCbit(false), isUndef(false), isPtr(false), isDouble(false), numDim(0), valOrIndex(-1), val(0.0){ }
  };
  
  struct FnCall{ //datapath sequence
    Function* func;
    Value* instPtr;
    std::vector<qGateArg> qArgs;
  };  
  
  struct DynGenQASMLoops : public ModulePass {
    static char ID;  // Pass identification, replacement for typeid
    std::vector<Value*> vectQbit;

    std::vector<qGateArg> tmpDepQbit;
    std::vector<qGateArg> allDepQbit;

    std::map<Function*, vector<qGateArg> > qbitsInFunc; //qbits in function
    std::map<Function*, vector<qGateArg> > qbitsInitInFunc; //new qbits declared in function
    std::map<Function*, vector<qGateArg> > funcArgList; //function arguments

    std::map<Function*, std::vector<FnCall> > mapFunction; //trace sequence of qgate calls
    std::map<Value*, string> mapInstRtn;    //traces return cbits for Meas Inst

    vector<Instruction*> vRemoveInst;
    
    int btCount; //backtrace count
    string forallStr;


    Instruction* FirstInst;

    Function* qasmAllocQbit; //alloc function
    Function* qasmAllocCbit; //alloc function
    Function* qasmAllocQbitExec; //alloc function
    Function* qasmAllocCbitExec; //alloc function
    Function* qasmHeader; //func decl and opening brace
    Function* qasmHeaderQbit; //quantum call arg
    Function* qasmHeaderQbitPtr; //quantum call arg
    Function* qasmHeaderCbitPtr; //quantum call arg
    Function* qasmHeaderEnd; //quantum call arg
    Function* qasmHeaderIntArg;
    Function* qasmHeaderDoubleArg;
    Function* qasmFooter; //closing brace
    Function* qasmQCall; //quantum call
    Function* qasmQCallArgQbit; //quantum call arg
    Function* qasmQCallArgQbitPtr; //quantum call arg
    Function* qasmQCallEnd; //quantum call arg
    Function* qasmGate;
    Function* qasmGate2;
    Function* qasmGate3;
    Function* qasmRot;
    Function* qasmCallInstIntArg;
    Function* qasmCallInstDoubleArg;

    DynGenQASMLoops() : ModulePass(ID) {  }

    bool getQbitArrDim(Type* instType, qGateArg* qa);
    bool backtraceOperand(Value* opd, int opOrIndex);
    void analyzeAllocInst(Function* F,Instruction* pinst, AllocaInst* strAlloc);

    void analyzeAllocInstExec(Function* F,Instruction* pinst,AllocaInst* strAlloc);

    void analyzeCallInst(Function* F,Instruction* pinst,AllocaInst* strAlloc);
    void analyzeInst(Function* F,Instruction* pinst,AllocaInst* strAlloc);


    void insertCallToAllocQbit(AllocaInst* AI, int varSize, AllocaInst* strAlloc);
    void insertCallToAllocCbit(AllocaInst* AI, int varSize, AllocaInst* strAlloc);

    void insertCallToAllocQbitExec(AllocaInst* AI, int varSize, AllocaInst* strAlloc);
    void insertCallToAllocCbitExec(AllocaInst* AI, int varSize, AllocaInst* strAlloc);

    void insertCallToHeader(Function* F, Instruction* I, AllocaInst* strAlloc);
    void instrumentIntrinsicInst(CallInst* CI, int id, map<unsigned, pair<string,bool> > nameOfQbit, AllocaInst* strAlloc);
    void instrumentNonIntrinsicInst(CallInst* CI, map<unsigned, pair<string,bool> > nameOfQbit, AllocaInst* strAlloc);

    void analyzeStoreCbitInst(Function* F, Instruction* pInst);
    void cleanup_store_cbits(Function* F);
    void removeIntrinsicQtmExec(Function* F,Instruction* I);

    // run - Print out SCCs in the call graph for the specified module.
    bool runOnModule(Module &M);

    void printFuncHeader(Function* F);

    string printVarName(StringRef s)
    {
      std::string sName = s.str();

      unsigned pos = sName.rfind("..");

      if(pos == sName.length()-2){
	std::string s1 = sName.substr(0,pos);
	return s1;
      }
      else{
	unsigned pos1 = sName.rfind(".");
	
	if(pos1 == sName.length()-1){
	  std::string s1 = sName.substr(0,pos1);
	  return s1;
	}
	else{
	  pos = sName.find(".addr");
	  std::string s1 = sName.substr(0,pos);     
	  return s1;
	}
      }
    }
    

    void print_qgateArg(qGateArg qg)
    {
      errs()<< "Printing QGate Argument:\n";
      if(qg.argPtr) errs() << "  Name: "<<qg.argPtr->getName()<<"\n";
      errs() << "  Arg Num: "<<qg.argNum<<"\n"
	     << "  isUndef: "<<qg.isUndef
	     << "  isQbit: "<<qg.isQbit
	     << "  isCbit: "<<qg.isCbit
	     << "  isPtr: "<<qg.isPtr << "\n"
	     << "  Value or Index: "<<qg.valOrIndex<<"\n"
	     << "  Num of Dim: "<<qg.numDim<<"\n";
      for(int i = 0; i<qg.numDim; i++)
	errs() << "     dimSize ["<<i<<"] = "<<qg.dimSize[i] << "\n";
    }

    void genQASM(Function* F);
    void getFunctionArguments(Function* F);
    
    void print(raw_ostream &O, const Module* = 0) const { 
      errs() << "Qbits found: ";
      for(unsigned int vb=0; vb<vectQbit.size(); vb++){
	errs() << vectQbit[vb]->getName() <<" ";
      }
      errs()<<"\n";      
    }
  

    // getAnalysisUsage - This pass requires the CallGraph.
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequired<CallGraph>();
    }
  };
}

char DynGenQASMLoops::ID = 0;
static RegisterPass<DynGenQASMLoops>
X("dyn-gen-qasm-with-loops", "Generate QASM output code with repeat statements");


void DynGenQASMLoops::insertCallToHeader(Function* F, Instruction* I,  AllocaInst* strAlloc){

  //errs() << "Insert Call to Header \n";
  string fnName = F->getName().str();

  //arguments are varType, size, name
  SmallVector<Value*,16> call_args;

  //insert argument1: fnName
  //Constant *StrConstant = ConstantDataArray::getString(I->getContext(), fnName);
  //ArrayType* strTy = cast<ArrayType>(StrConstant->getType());
  //AllocaInst* strAlloc = new AllocaInst(strTy,"",FirstInst);
  //StoreInst* strInit = new StoreInst(StrConstant,strAlloc,"",FirstInst);

  std::stringstream ss;
  ss << std::left << std::setw (MAX_FUNCTION_NAME-1) << std::setfill('.') << fnName;
  Constant *StrConstant = ConstantDataArray::getString(I->getContext(), ss.str());                   
  new StoreInst(StrConstant,strAlloc,"",FirstInst); 
  Value* Idx[2];
  Idx[0] = Constant::getNullValue(Type::getInt32Ty(I->getContext()));
  Idx[1] = ConstantInt::get(Type::getInt32Ty(I->getContext()),0);
  GetElementPtrInst* strPtr = GetElementPtrInst::Create(strAlloc, Idx, "", FirstInst);

  call_args.push_back(strPtr);

  CallInst::Create(qasmHeader,call_args,"",FirstInst);
  /*----
  std::map<unsigned, pair<string,int> > nameOfQbit; // string:isPtr
  //pair<string, int> encoding:
  //00 => qbit ptr
  //01 => qbit
  //10 => cbit ptr
  //11 => cbit

  unsigned iop =0;

  //iterate over int and double args
 //iterate over int and double args and send
  //errs() << "Insert int/double args \n";
  //for(unsigned iop=0;iop < CI->getNumArgOperands(); iop++) {
  for(Function::arg_iterator ait=F->arg_begin();ait!=F->arg_end();++ait) {
    //Value* arg = CI->getArgOperand(iop);
    Value* arg = (Value*)(ait);
    Type* argType = ait->getType();
    if(argType->isIntegerTy(32)){
      //errs() << "Inserting Int \n";
      CallInst::Create(qasmHeaderIntArg, arg, "", FirstInst);
      //errs() << "Done inserting Int Arg \n";
    }
    else if(argType->isDoubleTy()){
      CallInst::Create(qasmHeaderDoubleArg, arg, "", FirstInst);
    }
    else{	          	
      //errs() << "Inserting Non-Int \n";
	  if(argType->isPointerTy()){	 
	    Type *argElemType = argType->getPointerElementType();
	    if(argElemType->isIntegerTy(16))	     
	      nameOfQbit[iop++] = make_pair(ait->getName().str(),0);

	    if(argElemType->isIntegerTy(1))
	      //assert(false && "Should not find cbit as function argument");
	      nameOfQbit[iop++] = make_pair(ait->getName().str(),2);
	  }
	  else if(argType->isIntegerTy(16)){
	    //errs() << "\t Arg is qbit \n";        
            nameOfQbit[iop++] = make_pair(ait->getName().str(),1);
	    //errs() << "\t Arg Name = " << arg->getName() << "\n";
	  }	  	
	  else if(argType->isIntegerTy(1)){
            //assert(false && "Should not find cbit as function argument");
            nameOfQbit[iop++] = make_pair(ait->getName().str(),3);
	  }     	            
    }
  }

  //send call to print opening brace for function
  Value* endVal1 = ConstantInt::get(Type::getInt32Ty(I->getContext()),0);
  CallInst::Create(qasmHeaderEnd, endVal1, "", FirstInst);      
  
  //errs() << "Insert qbit args \n";

  //iterate over map and send one argument at a time based on whether ptr or not
  for (std::map<unsigned, pair<string,int> >::iterator i = nameOfQbit.begin(); i!=nameOfQbit.end(); ++i){
    
    string qname = i->second.first;
    
    std::stringstream ss;
    ss << std::left << std::setw (MAX_FUNCTION_NAME-1) << std::setfill('.') << qname;
    Constant *StrConstant = ConstantDataArray::getString(I->getContext(), ss.str());                  
    new StoreInst(StrConstant,strAlloc,"",FirstInst);     
    Value* Idx[2];
    Idx[0] = Constant::getNullValue(Type::getInt32Ty(I->getContext()));
    Idx[1] = ConstantInt::get(Type::getInt32Ty(I->getContext()),0);
  
    GetElementPtrInst* strPtr = GetElementPtrInst::Create(strAlloc, Idx, "", FirstInst);
    
    if(i->second.second == 0){ //is ptr to qbit            
      CallInst::Create(qasmHeaderQbitPtr,strPtr, "", FirstInst);      
    } //is qbit ptr
    else if(i->second.second == 2){ //is ptr to cbit            
      CallInst::Create(qasmHeaderCbitPtr,strPtr, "", FirstInst);      
    } //is qbit ptr
    else if(i->second.second == 1){ //is qbit
      //SmallVector<Value*,16> call_args;
      //call_args.push_back(strPtr);
      
      //Value* qbitArg = CI->getOperand(i->first);
      //call_args.push_back(qbitArg);
      CallInst::Create(qasmHeaderQbit,strPtr, "", FirstInst);      
    }

    else if(i->second.second == 3){ //is cbit
      //SmallVector<Value*,16> call_args;
      //call_args.push_back(strPtr);
      
      //Value* qbitArg = CI->getOperand(i->first);
      //call_args.push_back(qbitArg);
      //CallInst::Create(qasmHeaderQbit,call_args, "", CI);
    }

  } // end of map iterator
  ---*/
 // send call_end
  Value* endVal2 = ConstantInt::get(Type::getInt32Ty(I->getContext()),1);
  CallInst::Create(qasmHeaderEnd, endVal2, "", FirstInst);      

}

bool DynGenQASMLoops::backtraceOperand(Value* opd, int opOrIndex)
{
  bool foundOne = false;
      //search for opd in qbit/cbit vector
      std::vector<Value*>::iterator vIter=std::find(vectQbit.begin(),vectQbit.end(),opd);
      if(vIter != vectQbit.end()){
	if(debugDynGenQASMLoops)
	  errs()<<"Found qubit associated: "<< opd->getName() << "\n";
	
	tmpDepQbit[0].argPtr = opd;
	
	return true;
      }
      
      if(btCount>MAX_BT_COUNT)
	return false;
        
      if(GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(opd))
	{
	  //if(debugDynGenQASMLoops)
	  //errs() << "Get Elem Ptr Inst Found: " << *GEPI <<"\n";
	  foundOne = backtraceOperand(GEPI->getPointerOperand(),0);
	  
	}
      else if(Instruction* pInst = dyn_cast<Instruction>(opd)){
	//errs() << "In backtrace. Inst = " << *pInst << "\n";
	unsigned numOps = pInst->getNumOperands();
	//bool foundOne = false;
	for(unsigned iop=0;(iop<numOps && !foundOne);iop++){
	  btCount++;
	  foundOne = foundOne || backtraceOperand(pInst->getOperand(iop),0);
	  //errs() << "Found one = " <<foundOne << "\n";
	  btCount--;
	}
	//errs() << "Going back\n";
	return foundOne;
      }
      else{
	//if(debugDynGenQASMLoops)
	//errs() << "Ending Recursion\n";
	return false;
      }        
      return foundOne;
}

void DynGenQASMLoops::insertCallToAllocQbit(AllocaInst* AI, int varSize, AllocaInst* strAlloc){
 
  assert(FirstInst != NULL && "NULL FirstInst");

  //arguments are qbit array, size, name
  SmallVector<Value*,16> call_args;

  //insert argument1: qbit array
  //generate getelementptr inst
  SmallVector<Value*,16> idxVect;
  idxVect.push_back(ConstantInt::get(Type::getInt32Ty(AI->getContext()),0));
  idxVect.push_back(ConstantInt::get(Type::getInt32Ty(AI->getContext()),0));
  GetElementPtrInst *arrPtr = GetElementPtrInst::Create((Value*)AI, idxVect, "", FirstInst);
  
  call_args.push_back(arrPtr);

  //insert argument2: varSize
  Value* arg2 = ConstantInt::get(Type::getInt32Ty(AI->getContext()),varSize);
  call_args.push_back(arg2);

  //insert argument3: varName
  //Constant *StrConstant = ConstantDataArray::getString(AI->getContext(), AI->getName().str());
  //ArrayType* strTy = cast<ArrayType>(StrConstant->getType());
  //AllocaInst* strAlloc = new AllocaInst(strTy,"",FirstInst);
  //StoreInst* strInit = new StoreInst(StrConstant,strAlloc,"",FirstInst);

  std::stringstream ss;
  ss << std::left << std::setw (MAX_FUNCTION_NAME-1) << std::setfill('.') << AI->getName().str();
  Constant *StrConstant = ConstantDataArray::getString(AI->getContext(), ss.str());                  
  new StoreInst(StrConstant,strAlloc,"",FirstInst); 
  Value* Idx[2];
  Idx[0] = Constant::getNullValue(Type::getInt32Ty(AI->getContext()));
  Idx[1] = ConstantInt::get(Type::getInt32Ty(AI->getContext()),0);
  
  GetElementPtrInst* strPtr = GetElementPtrInst::Create(strAlloc, Idx, "", FirstInst);

  call_args.push_back(strPtr);

  CallInst::Create(qasmAllocQbit,call_args,"",FirstInst);
}

void DynGenQASMLoops::insertCallToAllocCbit(AllocaInst* AI, int varSize,AllocaInst* strAlloc){
 
  assert(FirstInst != NULL && "NULL FirstInst");

  //arguments are qbit array, size, name
  SmallVector<Value*,16> call_args;

  //insert argument1: varSize
  Value* arg2 = ConstantInt::get(Type::getInt32Ty(AI->getContext()),varSize);
  call_args.push_back(arg2);

  //insert argument2: varName
  //Constant *StrConstant = ConstantDataArray::getString(AI->getContext(), AI->getName().str());
  //ArrayType* strTy = cast<ArrayType>(StrConstant->getType());
  //AllocaInst* strAlloc = new AllocaInst(strTy,"",FirstInst);
  //StoreInst* strInit = new StoreInst(StrConstant,strAlloc,"",FirstInst);

  std::stringstream ss;
  ss << std::left << std::setw (MAX_FUNCTION_NAME-1) << std::setfill('.') << AI->getName().str();
  Constant *StrConstant = ConstantDataArray::getString(AI->getContext(), ss.str());                  
  new StoreInst(StrConstant,strAlloc,"",FirstInst);  
  Value* Idx[2];
  Idx[0] = Constant::getNullValue(Type::getInt32Ty(AI->getContext()));
  Idx[1] = ConstantInt::get(Type::getInt32Ty(AI->getContext()),0);
  
  GetElementPtrInst* strPtr = GetElementPtrInst::Create(strAlloc, Idx, "", FirstInst);

  call_args.push_back(strPtr);

  CallInst::Create(qasmAllocCbit,call_args,"",FirstInst);
} //end insertCallToAllocCbit


void DynGenQASMLoops::insertCallToAllocQbitExec(AllocaInst* AI, int varSize,AllocaInst* strAlloc){
 
  assert(FirstInst != NULL && "NULL FirstInst");

  //arguments are qbit array, size, name
  SmallVector<Value*,16> call_args;

//insert argument1: qbit array
//generate getelementptr inst
    SmallVector<Value*,16> idxVect;
	idxVect.push_back(ConstantInt::get(Type::getInt32Ty(AI->getContext()),0));
	idxVect.push_back(ConstantInt::get(Type::getInt32Ty(AI->getContext()),0));
	GetElementPtrInst *arrPtr = GetElementPtrInst::Create((Value*)AI, idxVect, "", FirstInst);

  call_args.push_back(arrPtr);

  //insert argument2: varSize
  Value* arg2 = ConstantInt::get(Type::getInt32Ty(AI->getContext()),varSize);
  call_args.push_back(arg2);

  //insert argument3: varName
  //Constant *StrConstant = ConstantDataArray::getString(AI->getContext(), AI->getName().str());
  //ArrayType* strTy = cast<ArrayType>(StrConstant->getType());
  //AllocaInst* strAlloc = new AllocaInst(strTy,"",FirstInst);
  //StoreInst* strInit = new StoreInst(StrConstant,strAlloc,"",FirstInst);

  std::stringstream ss;
  ss << std::left << std::setw (MAX_FUNCTION_NAME-1) << std::setfill('.') << AI->getName().str();
  Constant *StrConstant = ConstantDataArray::getString(AI->getContext(), ss.str());                  
  new StoreInst(StrConstant,strAlloc,"",FirstInst); 
  Value* Idx[2];
  Idx[0] = Constant::getNullValue(Type::getInt32Ty(AI->getContext()));
  Idx[1] = ConstantInt::get(Type::getInt32Ty(AI->getContext()),0);
  
  GetElementPtrInst* strPtr = GetElementPtrInst::Create(strAlloc, Idx, "", FirstInst);

  call_args.push_back(strPtr);

  CallInst::Create(qasmAllocQbitExec,call_args,"",FirstInst);
}

void DynGenQASMLoops::insertCallToAllocCbitExec(AllocaInst* AI, int varSize,AllocaInst* strAlloc){
 
  assert(FirstInst != NULL && "NULL FirstInst");

  //arguments are qbit array, size, name
  SmallVector<Value*,16> call_args;

  //insert argument1: varSize
  Value* arg2 = ConstantInt::get(Type::getInt32Ty(AI->getContext()),varSize);
  call_args.push_back(arg2);

  //insert argument2: varName
  //Constant *StrConstant = ConstantDataArray::getString(AI->getContext(), AI->getName().str());
  //ArrayType* strTy = cast<ArrayType>(StrConstant->getType());
  //AllocaInst* strAlloc = new AllocaInst(strTy,"",FirstInst);
  //StoreInst* strInit = new StoreInst(StrConstant,strAlloc,"",FirstInst);
  std::stringstream ss;
  ss << std::left << std::setw (MAX_FUNCTION_NAME-1) << std::setfill('.') << AI->getName().str();
  Constant *StrConstant = ConstantDataArray::getString(AI->getContext(), ss.str());                  
  new StoreInst(StrConstant,strAlloc,"",FirstInst); 
  Value* Idx[2];
  Idx[0] = Constant::getNullValue(Type::getInt32Ty(AI->getContext()));
  Idx[1] = ConstantInt::get(Type::getInt32Ty(AI->getContext()),0);
  
  GetElementPtrInst* strPtr = GetElementPtrInst::Create(strAlloc, Idx, "", FirstInst);

  call_args.push_back(strPtr);

  CallInst::Create(qasmAllocCbitExec,call_args,"",FirstInst);
} //end insertCallToAllocCbitExec



void DynGenQASMLoops::instrumentIntrinsicInst(CallInst* CI, int id, map<unsigned, pair<string,bool> > nameOfQbit, AllocaInst* strAlloc ){
  
  SmallVector<Value*,16> call_args;

  Value* tmpStrArg;

  //insert first argument => gateID
  Value* intArg = ConstantInt::get(Type::getInt32Ty(CI->getContext()),id);	
  call_args.push_back(intArg);
  
  //insert string args => qbit names
  for (std::map<unsigned, pair<string,bool> >::iterator iter = nameOfQbit.begin(); iter!=nameOfQbit.end(); ++iter){
    
    string qname = iter->second.first;
    
    //Constant *StrConstant = ConstantDataArray::getString(CI->getContext(), qname);
    //ArrayType* strTy = cast<ArrayType>(StrConstant->getType());
    //AllocaInst* strAlloc = new AllocaInst(strTy,"",CI);
    //StoreInst* strInit = new StoreInst(StrConstant,strAlloc,"",CI);
    
  std::stringstream ss;
  ss << std::left << std::setw (MAX_FUNCTION_NAME-1) << std::setfill('.') << qname;
  Constant *StrConstant = ConstantDataArray::getString(CI->getContext(), ss.str());                  
  new StoreInst(StrConstant,strAlloc,"",CI); 
    Value* Idx[2];
    Idx[0] = Constant::getNullValue(Type::getInt32Ty(CI->getContext()));
    Idx[1] = ConstantInt::get(Type::getInt32Ty(CI->getContext()),0);
    
    GetElementPtrInst* strPtr = GetElementPtrInst::Create(strAlloc, Idx, "", CI);

    call_args.push_back(strPtr);              

    tmpStrArg = strPtr; //to remember last str arg for Prep Insts

  }


  switch(id){
  case _H:
  case _S:
  case _T:
  case _Sdag:
  case _Tdag:
  case _X:
  case _Y:
  case _Z:
    {      
      Value* qbitArg = CI->getArgOperand(0);
      call_args.push_back(qbitArg);

      CallInst::Create(qasmGate,call_args,"",CI);
      break;
    }

  case _Rx:
  case _Ry:
  case _Rz:
    {          
      Value* qbitArg = CI->getArgOperand(0);
      call_args.push_back(qbitArg);
      
      Value* qbitArg1 = CI->getArgOperand(1); //double or const double
      call_args.push_back(qbitArg1);      

      CallInst::Create(qasmRot,call_args,"",CI);            
      break;    
    }

  case _CNOT:
    {
      
      Value* qbitArg = CI->getArgOperand(0);
      call_args.push_back(qbitArg);
      
      Value* qbitArg1 = CI->getArgOperand(1);
      call_args.push_back(qbitArg1);      
      
      CallInst::Create(qasmGate2,call_args,"",CI); 
      
      break;
    }
  case _PrepX:
  case _PrepZ:
    {
      //insert dummy string argument
      call_args.push_back(tmpStrArg);

      Value* qbitArg = CI->getArgOperand(0);
      call_args.push_back(qbitArg);
      
      ConstantInt *Cint = dyn_cast<ConstantInt>(CI->getArgOperand(1));
      Value* prepVal;
      if(Cint->getZExtValue()==0)
	prepVal= ConstantInt::get(Type::getInt16Ty(CI->getContext()),0);	
      else
	prepVal = ConstantInt::get(Type::getInt16Ty(CI->getContext()),1);		  
      call_args.push_back(prepVal);
      CallInst::Create(qasmGate2,call_args,"",(Instruction*)CI);
      break;
    }
  case _MeasX:
  case _MeasZ:
    {

      //errs() << "Meas found: " << *CI <<"\n"; 
      //add cbit string
      map<Value*, string>::iterator mit = mapInstRtn.find((Value*)CI);
      assert(mit!=mapInstRtn.end() && "Cbit Name Not Found in Map");

      //Constant *StrConstant = ConstantDataArray::getString(CI->getContext(), (*mit).second);
      //ArrayType* strTy = cast<ArrayType>(StrConstant->getType());
      //AllocaInst* strAlloc = new AllocaInst(strTy,"",CI);
      //StoreInst* strInit = new StoreInst(StrConstant,strAlloc,"",CI);

      std::stringstream ss;
      ss << std::left << std::setw (MAX_FUNCTION_NAME-1) << std::setfill('.') << (*mit).second;
      Constant *StrConstant = ConstantDataArray::getString(CI->getContext(), ss.str());                  
      new StoreInst(StrConstant,strAlloc,"",CI);       
      Value* Idx[2];
      Idx[0] = Constant::getNullValue(Type::getInt32Ty(CI->getContext()));
      Idx[1] = ConstantInt::get(Type::getInt32Ty(CI->getContext()),0);
      
      GetElementPtrInst* strPtr = GetElementPtrInst::Create(strAlloc, Idx, "", CI);
      
      call_args.push_back(strPtr);              
      

      Value* qbitArg = CI->getArgOperand(0);
      call_args.push_back(qbitArg);
      call_args.push_back(qbitArg);
      CallInst::Create(qasmGate2,call_args,"",(Instruction*)CI);   
      break;
    }
    
  case _Fredkin:
  case _Toffoli:
    {        
      Value* qbitArg = CI->getArgOperand(0);
      call_args.push_back(qbitArg);
      
      Value* qbitArg1 = CI->getArgOperand(1);
      call_args.push_back(qbitArg1);      
      
      Value* qbitArg2 = CI->getArgOperand(2);
      call_args.push_back(qbitArg2);  
      
      CallInst::Create(qasmGate3,call_args,"",CI); 
      
      break;
    }
  }
} //end instrumentIntrinsicInst



void DynGenQASMLoops::instrumentNonIntrinsicInst(CallInst* CI, map<unsigned, pair<string,bool> > nameOfQbit, AllocaInst* strAlloc){
  //errs() << "In InstrumentNonIntrinsicInst \n";
  
    //print_call_start
    
    //insert argument: called func name

  //errs() << "Insert CF name \n";
  //Constant *StrConstant = ConstantDataArray::getString(CI->getContext(), CI->getCalledFunction()->getName().str());
  //ArrayType* strTy = cast<ArrayType>(StrConstant->getType());
  //AllocaInst* strAlloc = new AllocaInst(strTy,"",CI);
  //StoreInst* strInit = new StoreInst(StrConstant,strAlloc,"",CI);
  std::stringstream ss;
  ss << std::left << std::setw (MAX_FUNCTION_NAME-1) << std::setfill('.') << CI->getCalledFunction()->getName().str();
  Constant *StrConstant = ConstantDataArray::getString(CI->getContext(), ss.str());                  
  new StoreInst(StrConstant,strAlloc,"",CI); 

  Value* Idx[2];
  Idx[0] = Constant::getNullValue(Type::getInt32Ty(CI->getContext()));
  Idx[1] = ConstantInt::get(Type::getInt32Ty(CI->getContext()),0);
  
  GetElementPtrInst* strPtr = GetElementPtrInst::Create(strAlloc, Idx, "", CI);

  //call_args.push_back(strPtr);
    
  CallInst::Create(qasmQCall,strPtr, "", CI);
  
  //iterate over int and double args and send
  //errs() << "Insert int/double args \n";
  for(unsigned iop=0;iop < CI->getNumArgOperands(); iop++) {
    Value* arg = CI->getArgOperand(iop);
    Type* argType = arg->getType();
    if(argType->isIntegerTy(32))
      CallInst::Create(qasmCallInstIntArg, arg, "", CI);
    else if(argType->isDoubleTy())
      CallInst::Create(qasmCallInstDoubleArg, arg, "", CI);
  }

  //send call to print opening brace for function
  Value* endVal1 = ConstantInt::get(Type::getInt32Ty(CI->getContext()),0);
  CallInst::Create(qasmQCallEnd, endVal1, "", CI);      
 /*--- 
  //errs() << "Insert qbit args \n";

  //iterate over map and send one argument at a time based on whether ptr or not
  for (std::map<unsigned, pair<string,bool> >::iterator i = nameOfQbit.begin(); i!=nameOfQbit.end(); ++i){
    
    string qname = i->second.first;
    
    //Constant *StrConstant = ConstantDataArray::getString(CI->getContext(), qname);
    //ArrayType* strTy = cast<ArrayType>(StrConstant->getType());
    //AllocaInst* strAlloc = new AllocaInst(strTy,"",CI);
    //StoreInst* strInit = new StoreInst(StrConstant,strAlloc,"",CI);

    std::stringstream ss;
    ss << std::left << std::setw (MAX_FUNCTION_NAME-1) << std::setfill('.') << qname;
    Constant *StrConstant = ConstantDataArray::getString(CI->getContext(), ss.str());                  
    new StoreInst(StrConstant,strAlloc,"",CI); 
    Value* Idx[2];
    Idx[0] = Constant::getNullValue(Type::getInt32Ty(CI->getContext()));
    Idx[1] = ConstantInt::get(Type::getInt32Ty(CI->getContext()),0);
  
    GetElementPtrInst* strPtr = GetElementPtrInst::Create(strAlloc, Idx, "", CI);
    
    if(i->second.second == true){ //is ptr to qbit            
      CallInst::Create(qasmQCallArgQbitPtr,strPtr, "", CI);      
    } //is qbit ptr
    else{ //is qbit
      SmallVector<Value*,16> call_args;
      call_args.push_back(strPtr);
      
      Value* qbitArg = CI->getOperand(i->first);
      call_args.push_back(qbitArg);
      CallInst::Create(qasmQCallArgQbit,call_args, "", CI);
      
    }
  } // end of map iterator
  ----*/
  
    // send call_end

  Value* endVal2 = ConstantInt::get(Type::getInt32Ty(CI->getContext()),1);
  CallInst::Create(qasmQCallEnd, endVal2, "", CI);      
}

void DynGenQASMLoops::analyzeAllocInst(Function* F, Instruction* pInst, AllocaInst* strAlloc){
  if (AllocaInst *AI = dyn_cast<AllocaInst>(pInst)) {
    Type *allocatedType = AI->getAllocatedType();
    
    if(ArrayType *arrayType = dyn_cast<ArrayType>(allocatedType)) {      
      qGateArg tmpQArg;

      Type *elementType = arrayType->getElementType();
      uint64_t arraySize = arrayType->getNumElements();
      if (elementType->isIntegerTy(16)){
	if(debugDynGenQASMLoops)
	  errs() << "New QBit Allocation Found: " << AI->getName() <<"\n";
	insertCallToAllocQbit(AI,arraySize,strAlloc);   
	vectQbit.push_back(AI);
	tmpQArg.isQbit = true;
	tmpQArg.argPtr = AI;
	tmpQArg.numDim = 1;
	tmpQArg.dimSize[0] = arraySize;
	tmpQArg.valOrIndex = arraySize;
	(qbitsInFunc.find(F))->second.push_back(tmpQArg);
	(qbitsInitInFunc.find(F))->second.push_back(tmpQArg);	
      }
      
      else if (elementType->isIntegerTy(1)){
	if(debugDynGenQASMLoops)
	  errs() << "New CBit Allocation Found: " << AI->getName() <<"\n";

	insertCallToAllocCbit(AI,arraySize, strAlloc);

	vectQbit.push_back(AI); //Cbit added here
	tmpQArg.isCbit = true;
	tmpQArg.argPtr = AI;
	tmpQArg.numDim = 1;
	tmpQArg.dimSize[0] = arraySize;
	tmpQArg.valOrIndex = arraySize;
	(qbitsInFunc.find(F))->second.push_back(tmpQArg);
	(qbitsInitInFunc.find(F))->second.push_back(tmpQArg);	
      }

      else if(elementType->isArrayTy()){
	  errs() << "Multidimensional array\n";
        assert(false && "Multidimensional array");
/*
	  tmpQArg.dimSize[0] = arraySize;
	  tmpQArg.numDim++;
	  tmpQArg.valOrIndex = arraySize;

	  //recurse on multi-dimensional array
	  bool isQAlloc = getQbitArrDim(elementType,&tmpQArg);

	  if(isQAlloc){
	    vectQbit.push_back(AI);
	    tmpQArg.argPtr = AI;
	    (qbitsInFunc.find(F))->second.push_back(tmpQArg);
	    (qbitsInitInFunc.find(F))->second.push_back(tmpQArg);

	    if(debugDynGenQASMLoops)
	      print_qgateArg(tmpQArg);
	  }
          */	  
      }

    }
    else if(allocatedType->isPointerTy()){
      
      /*Note: this is necessary if -mem2reg is not run on LLVM IR before.
	Eg without -mem2reg
	module(i8* %q){
	%q.addr = alloca i8*, align 8
	...
	}
	qbit q.addr must be mapped to argument q. Hence the following code.
	If it is known that -O1 will be run, then this can be removed.
      */
      
      Type *elementType = allocatedType->getPointerElementType();
      if(debugDynGenQASMLoops)
	errs() << "\tIs a Pointer of Type: " << *elementType << "\n";
      
      if (elementType->isIntegerTy(16)){
	vectQbit.push_back(AI);
	
	qGateArg tmpQArg;
	tmpQArg.isPtr = true;
	tmpQArg.isQbit = true;
	tmpQArg.argPtr = AI;
	
	(qbitsInFunc.find(F))->second.push_back(tmpQArg);
	
	std::string argName = AI->getName();
	unsigned pos = argName.find(".addr");
	std::string argName2 = argName.substr(0,pos);

	//find argName2 in funcArgList - avoid printing out qbit declaration twice
	std::map<Function*, vector<qGateArg> >::iterator mIter = funcArgList.find(F);
	if(mIter != funcArgList.end()){
	  bool foundit = false;
	  for(vector<qGateArg>::iterator vParamIter = (*mIter).second.begin();(vParamIter!=(*mIter).second.end() && !foundit);++vParamIter){
	    if((*vParamIter).argPtr->getName() == argName2){ 
	      foundit = true;
	    }
	  }
	  if(!foundit) //do not add duplicate declaration
	    (qbitsInitInFunc.find(F))->second.push_back(tmpQArg);
	}
      }
    }
    return;
  }

}


void DynGenQASMLoops::analyzeAllocInstExec(Function* F, Instruction* pInst, AllocaInst* strAlloc){
  if (AllocaInst *AI = dyn_cast<AllocaInst>(pInst)) {
    Type *allocatedType = AI->getAllocatedType();
    
    if(ArrayType *arrayType = dyn_cast<ArrayType>(allocatedType)) {      

      Type *elementType = arrayType->getElementType();
      uint64_t arraySize = arrayType->getNumElements();
      if (elementType->isIntegerTy(16)){
	if(debugDynGenQASMLoops)
	  errs() << "New QBit Allocation Found: " << AI->getName() <<"\n";
	insertCallToAllocQbitExec(AI,arraySize, strAlloc);   
      }      
      else if (elementType->isIntegerTy(1)){
	if(debugDynGenQASMLoops)
	  errs() << "New CBit Allocation Found: " << AI->getName() <<"\n";
	insertCallToAllocCbitExec(AI,arraySize, strAlloc);
      }
    }
  }
} //end analyzeAllocExec functions



void DynGenQASMLoops::analyzeCallInst(Function* F, Instruction* pInst, AllocaInst* strAlloc){
  if(CallInst *CI = dyn_cast<CallInst>(pInst))
    {
      if(debugDynGenQASMLoops)
      //if(F->getName() == "main_qtm")
	errs() << "Call inst: " << CI->getCalledFunction()->getName() << "\n";
      
      Function* CF = CI->getCalledFunction();

      if(CF->getName().find("qasm_print_")!=string::npos) return;
      
      if(CF->getName() == "store_cbit"){	//trace return values
	return;
      }

      /*
      if(((CF->getName()).find("qasm_print_RepLoopStart")!=string::npos) || ((CF->getName()).find("qasm_print_RepLoopEnd")!=string::npos)){
	vRemoveInst.push_back(CI);       
	return; //spatil: should this be here?
	}
      */

      std::map<unsigned, int> valueOfInt; // map argument index to const int value
      std::map<unsigned, double> valueOfDouble; // map argument index to const double value  
      std::map<unsigned, pair<string,bool> > nameOfQbit; // string:isPtr
      
      // scan for constant int or double arguments and save them
      for(unsigned iop=0;iop < CI->getNumArgOperands(); iop++) {
	if(ConstantInt *CInt = dyn_cast<ConstantInt>(CI->getArgOperand(iop)))
	  valueOfInt[iop] = CInt->getZExtValue();
	else if(ConstantFP *CDouble = dyn_cast<ConstantFP>(CI->getArgOperand(iop)))
	  valueOfDouble[iop] = CDouble->getValueAPF().convertToDouble();    
	else{
	  
	  tmpDepQbit.clear();
	  btCount = 0;
	  qGateArg tmpQGateArg1;
	  tmpDepQbit.push_back(tmpQGateArg1);
          
	  Type* argType = CI->getArgOperand(iop)->getType();
	  if(argType->isPointerTy()){
	    //tmpQGateArg.isPtr = true;
	    Type *argElemType = argType->getPointerElementType();
	    if(argElemType->isIntegerTy(16)){
	      backtraceOperand(CI->getArgOperand(iop),0);
	      assert(tmpDepQbit[0].argPtr!=NULL && "Expected to find qbit ptr");
	      nameOfQbit[iop] = make_pair(tmpDepQbit[0].argPtr->getName().str(),true);
	    //tmpQGateArg.isQbit = true;
	    }
	    else if(argElemType->isIntegerTy(1)){
	      backtraceOperand(CI->getArgOperand(iop),0);
	      assert(tmpDepQbit[0].argPtr!=NULL && "Expected to find cbit ptr as function argument");
            nameOfQbit[iop] = make_pair(tmpDepQbit[0].argPtr->getName().str(),true);
	    //tmpQGateArg.isCbit = true;
	    }
	  }
	  else if(argType->isIntegerTy(16)){
	    //errs() << "\t Arg is qbit \n";
	    
            backtraceOperand(CI->getArgOperand(iop),0);
	    
            assert(tmpDepQbit[0].argPtr!=NULL && "Expected to find qbit");
            nameOfQbit[iop] = make_pair(tmpDepQbit[0].argPtr->getName().str(),false);
	    //errs() << "\t Arg Name = " << tmpDepQbit[0].argPtr->getName() << "\n";
	    //tmpQGateArg.isQbit = true;
	    //tmpQGateArg.valOrIndex = 0;	 
	  }	  	
	  else if(argType->isIntegerTy(1)){
            assert(false && "Should not find cbit as function argument");
	    //tmpQGateArg.isCbit = true;
	    //tmpQGateArg.valOrIndex = 0;	 
	  }     
	  
          
	}
      }
            
      //check if Intrinsic Function
      
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
	//errs() << "Is Intrinsic. GateID = " << gateIndex << "\n";
	instrumentIntrinsicInst(CI, gateIndex, nameOfQbit, strAlloc);
	return;
      }

      if(CF->isDeclaration()) return;

      //NonIntrinsicCall Found
          
      instrumentNonIntrinsicInst(CI, nameOfQbit, strAlloc);
      vRemoveInst.push_back(CI);

    } //if CallInst
}


void DynGenQASMLoops::cleanup_store_cbits(Function* F){
  //iterate over instructions
   for(inst_iterator instIb = inst_begin(F),instIe=inst_end(F); instIb!=instIe;++instIb){
     Instruction* pInst = &*instIb;
     //if store_cbit found, mark for removal 
     if(CallInst *CI = dyn_cast<CallInst>(pInst))
       {      
	 if(CI->getCalledFunction()->getName() == "store_cbit"){
	   vRemoveInst.push_back(CI); 
	 }

	 else if(((CI->getCalledFunction()->getName()).find("qasm_print_RepLoopStart")!=string::npos) || ((CI->getCalledFunction()->getName()).find("qasm_print_RepLoopEnd")!=string::npos)){
	   vRemoveInst.push_back(CI);       
	 }

       }    
   }
}

void DynGenQASMLoops::analyzeStoreCbitInst(Function* F, Instruction* pInst){

  if(CallInst *CI = dyn_cast<CallInst>(pInst))
    {
      
      Function* CF = CI->getCalledFunction();
      
      if(CF->getName() == "store_cbit"){	//trace return values

	if(debugDynGenQASMLoops)      
	  errs() << "Call inst: " << CF->getName() << "\n";

	Value* rtnVal = CI->getArgOperand(0); //cbit
	//tmpDepQbit.clear();

	qGateArg tmpQGateArg2;
	tmpQGateArg2.isCbit = true;
	tmpQGateArg2.isPtr = true;
	tmpDepQbit.push_back(tmpQGateArg2);	
	backtraceOperand(CI->getArgOperand(1),0); //pointer Operand

	//insert info in map here
	mapInstRtn[rtnVal] = tmpDepQbit[0].argPtr->getName().str();

	tmpDepQbit.clear();
	vRemoveInst.push_back(CI);       
	return;
      }

    }

}

void DynGenQASMLoops::analyzeInst(Function* F, Instruction* pInst, AllocaInst* strAlloc){
  if(debugDynGenQASMLoops)
    errs() << "--Processing Inst: "<<*pInst << '\n';

  //analyzeAllocInst(F,pInst);

  analyzeCallInst(F,pInst, strAlloc);
    
  if(debugDynGenQASMLoops)
    {
      errs() << "Opcode: "<<pInst->getOpcodeName() << "\n";
      
      unsigned numOps = pInst->getNumOperands();
      errs() << "Num Operands: " << numOps << ": ";
      
      for(unsigned iop=0;iop<numOps;iop++){
	errs() << pInst->getOperand(iop)->getName() << "; ";
      }
      errs() << "\n";		
      return;
    }
    
  return;
}

void DynGenQASMLoops::printFuncHeader(Function* F)
{

  map<Function*, vector<qGateArg> >::iterator mpItr;
  map<Function*, vector<qGateArg> >::iterator mpItr2;
  map<Function*, vector<qGateArg> >::iterator mvpItr;

  mpItr = qbitsInFunc.find(F);

  //print name of function
  errs()<<"\nmodule "<<F->getName();
  
  //print arguments of function
  mpItr2=funcArgList.find(F);    
  errs()<<" ( ";    
  unsigned tmp_num_elem = (*mpItr2).second.size();
  
  if(tmp_num_elem > 0){
    for(unsigned tmp_i=0;tmp_i<tmp_num_elem - 1;tmp_i++){
      
      qGateArg tmpQA = (*mpItr2).second[tmp_i];
      
      if(debugDynGenQASMLoops)
	print_qgateArg(tmpQA);
      
      if(tmpQA.isQbit)
	errs()<<"qbit";
      else if(tmpQA.isCbit)
	errs()<<"cbit";
      else{
	Type* argTy = tmpQA.argPtr->getType();
	if(argTy->isDoubleTy()) errs() << "double";
	else if(argTy->isFloatTy()) errs() << "float";
	else
	  errs()<<"UNRECOGNIZED "<<argTy<<" ";
      }
      
      if(tmpQA.isPtr)
	errs()<<"*";	  
      
      errs()<<" "<<printVarName(tmpQA.argPtr->getName())<<" , ";
    }
    
    if(debugDynGenQASMLoops)
      print_qgateArg((*mpItr2).second[tmp_num_elem-1]);
    
    
    if(((*mpItr2).second[tmp_num_elem-1]).isQbit)
      errs()<<"qbit";
    else if(((*mpItr2).second[tmp_num_elem-1]).isCbit)
      errs()<<"cbit";
    else{
      Type* argTy = ((*mpItr2).second[tmp_num_elem-1]).argPtr->getType();
      if(argTy->isDoubleTy()) errs() << "double";
      else if(argTy->isFloatTy()) errs() << "float";
      else
	errs()<<"UNRECOGNIZED "<<argTy<<" ";
    }
    
    if(((*mpItr2).second[tmp_num_elem-1]).isPtr)
      errs()<<"*";
    
    errs() <<" "<<printVarName(((*mpItr2).second[tmp_num_elem-1]).argPtr->getName());
  }
  
  errs()<<" ) {\n ";   
  
  //print qbits declared in function
  mvpItr=qbitsInitInFunc.find(F);	    
  for(vector<qGateArg>::iterator vvit=(*mvpItr).second.begin(),vvitE=(*mvpItr).second.end();vvit!=vvitE;++vvit)
    {	
      if((*vvit).isQbit)
	errs()<<"\tqbit "<<printVarName((*vvit).argPtr->getName());
      if((*vvit).isCbit)
	errs()<<"\tcbit "<<printVarName((*vvit).argPtr->getName());

      //if only single-dimensional qbit arrays expected
      //errs()<<"["<<(*vvit).valOrIndex<<"];\n ";

      //if n-dimensional qbit arrays expected 
      for(int ndim = 0; ndim < (*vvit).numDim; ndim++)
	errs()<<"["<<(*vvit).dimSize[ndim]<<"]";
      errs() << ";\n";
    }
  
}

void DynGenQASMLoops::getFunctionArguments(Function* F)
{
  //std::vector<unsigned> qGateArgs;  

  for(Function::arg_iterator ait=F->arg_begin();ait!=F->arg_end();++ait)
    {    
      std::string argName = (ait->getName()).str();
      Type* argType = ait->getType();
      unsigned int argNum=ait->getArgNo();         

      qGateArg tmpQArg;
      tmpQArg.argPtr = ait;
      tmpQArg.argNum = argNum;

      if(argType->isPointerTy()){
	if(debugDynGenQASMLoops)
	  errs()<<"Argument Type: " << *argType <<"\n";

	tmpQArg.isPtr = true;

	Type *elementType = argType->getPointerElementType();
	if (elementType->isIntegerTy(16)){
	  tmpQArg.isQbit = true;
	  vectQbit.push_back(ait);
	  (qbitsInFunc.find(F))->second.push_back(tmpQArg);
	  (funcArgList.find(F))->second.push_back(tmpQArg);
	}
	else if (elementType->isIntegerTy(1)){
	  tmpQArg.isCbit = true;
	  vectQbit.push_back(ait);
	  (qbitsInFunc.find(F))->second.push_back(tmpQArg);
	  (funcArgList.find(F))->second.push_back(tmpQArg);
	}
      }
      else if (argType->isIntegerTy(16)){
	tmpQArg.isQbit = true;
	vectQbit.push_back(ait);
	(qbitsInFunc.find(F))->second.push_back(tmpQArg);
	(funcArgList.find(F))->second.push_back(tmpQArg);
      }
      else if (argType->isIntegerTy(1)){
	tmpQArg.isCbit = true;
	vectQbit.push_back(ait);
	(qbitsInFunc.find(F))->second.push_back(tmpQArg);
	(funcArgList.find(F))->second.push_back(tmpQArg);
      }
      else if(argType->isDoubleTy())     
	(funcArgList.find(F))->second.push_back(tmpQArg);

      if(debugDynGenQASMLoops)
	print_qgateArg(tmpQArg);
    }
}

void DynGenQASMLoops::removeIntrinsicQtmExec(Function* F,Instruction* I){
  if(CallInst *CI = dyn_cast<CallInst>(I)){
    Function* CF = CI->getCalledFunction();

  if(CF->isIntrinsic()){
    if((CF->getIntrinsicID() == Intrinsic::CNOT)
       || (CF->getIntrinsicID() == Intrinsic::Fredkin)
       || (CF->getIntrinsicID() == Intrinsic::H)
       //|| (CF->getIntrinsicID() == Intrinsic::MeasX)
       //|| (CF->getIntrinsicID() == Intrinsic::MeasZ)
       || (CF->getIntrinsicID() == Intrinsic::PrepX)
       || (CF->getIntrinsicID() == Intrinsic::PrepZ)
       || (CF->getIntrinsicID() == Intrinsic::Rz)
       || (CF->getIntrinsicID() == Intrinsic::S)
       || (CF->getIntrinsicID() == Intrinsic::T)
       || (CF->getIntrinsicID() == Intrinsic::Sdag)
       || (CF->getIntrinsicID() == Intrinsic::Tdag)
       || (CF->getIntrinsicID() == Intrinsic::Toffoli)
       || (CF->getIntrinsicID() == Intrinsic::X)
       || (CF->getIntrinsicID() == Intrinsic::Y)
       || (CF->getIntrinsicID() == Intrinsic::Z)){
      vRemoveInst.push_back(CI);
    }
  }
  }
}


// run - Find datapaths for qubits
bool DynGenQASMLoops::runOnModule(Module &M) {

  qasmAllocQbit = cast<Function>(M.getOrInsertFunction("qasm_print_qbit_alloc", Type::getVoidTy(M.getContext()), Type::getInt16Ty(M.getContext())->getPointerTo(), Type::getInt32Ty(M.getContext()), Type::getInt8Ty(M.getContext())->getPointerTo(), (Type*)0));

  qasmAllocCbit = cast<Function>(M.getOrInsertFunction("qasm_print_cbit_alloc", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), Type::getInt8Ty(M.getContext())->getPointerTo(), (Type*)0));

  qasmAllocQbitExec = cast<Function>(M.getOrInsertFunction("qasm_print_qbit_alloc_exec", Type::getVoidTy(M.getContext()), Type::getInt16Ty(M.getContext())->getPointerTo(), Type::getInt32Ty(M.getContext()), Type::getInt8Ty(M.getContext())->getPointerTo(), (Type*)0));

  qasmAllocCbitExec = cast<Function>(M.getOrInsertFunction("qasm_print_cbit_alloc_exec", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), Type::getInt8Ty(M.getContext())->getPointerTo(), (Type*)0));
  
  qasmHeader = cast<Function>(M.getOrInsertFunction("qasm_print_header", Type::getVoidTy(M.getContext()),  Type::getInt8Ty(M.getContext())->getPointerTo(), (Type*)0));       

  qasmHeaderQbit = cast<Function>(M.getOrInsertFunction("qasm_print_header_qbit_arg", Type::getVoidTy(M.getContext()), Type::getInt8Ty(M.getContext())->getPointerTo(), (Type*)0));

    qasmHeaderQbitPtr = cast<Function>(M.getOrInsertFunction("qasm_print_header_qbitptr_arg", Type::getVoidTy(M.getContext()), Type::getInt8Ty(M.getContext())->getPointerTo(),(Type*)0));

    qasmHeaderCbitPtr = cast<Function>(M.getOrInsertFunction("qasm_print_header_cbitptr_arg", Type::getVoidTy(M.getContext()), Type::getInt8Ty(M.getContext())->getPointerTo(),(Type*)0));

    qasmHeaderEnd = cast<Function>(M.getOrInsertFunction("qasm_print_header_end", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), (Type*)0));

      qasmHeaderIntArg = cast<Function>(M.getOrInsertFunction("qasm_print_header_int_arg", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()),(Type*)0));

      qasmHeaderDoubleArg = cast<Function>(M.getOrInsertFunction("qasm_print_header_double_arg", Type::getVoidTy(M.getContext()), Type::getDoubleTy(M.getContext()),(Type*)0));

  qasmQCall = cast<Function>(M.getOrInsertFunction("qasm_print_call_start", Type::getVoidTy(M.getContext()),  Type::getInt8Ty(M.getContext())->getPointerTo(), (Type*)0));       
 
       qasmQCallArgQbit = cast<Function>(M.getOrInsertFunction("qasm_print_call_qbit_arg", Type::getVoidTy(M.getContext()), Type::getInt8Ty(M.getContext())->getPointerTo(), Type::getInt16Ty(M.getContext()), (Type*)0));

    qasmQCallArgQbitPtr = cast<Function>(M.getOrInsertFunction("qasm_print_call_qbitptr_arg", Type::getVoidTy(M.getContext()), Type::getInt8Ty(M.getContext())->getPointerTo(),(Type*)0));

    qasmQCallEnd = cast<Function>(M.getOrInsertFunction("qasm_print_call_end", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), (Type*)0));

    qasmGate = cast<Function>(M.getOrInsertFunction("qasm_print_qgate", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), Type::getInt8Ty(M.getContext())->getPointerTo(),  Type::getInt16Ty(M.getContext()), (Type*)0));
      
      qasmGate2 = cast<Function>(M.getOrInsertFunction("qasm_print_qgate2", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), Type::getInt8Ty(M.getContext())->getPointerTo(), Type::getInt8Ty(M.getContext())->getPointerTo(), Type::getInt16Ty(M.getContext()), Type::getInt16Ty(M.getContext()), (Type*)0));

      qasmGate3 = cast<Function>(M.getOrInsertFunction("qasm_print_qgate3", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), Type::getInt8Ty(M.getContext())->getPointerTo(),Type::getInt8Ty(M.getContext())->getPointerTo(),Type::getInt8Ty(M.getContext())->getPointerTo(),  Type::getInt16Ty(M.getContext()), Type::getInt16Ty(M.getContext()), Type::getInt16Ty(M.getContext()),  (Type*)0));

      qasmRot = cast<Function>(M.getOrInsertFunction("qasm_print_rot", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()),  Type::getInt8Ty(M.getContext())->getPointerTo(), Type::getInt16Ty(M.getContext()), Type::getDoubleTy(M.getContext()),(Type*)0));
      
      qasmCallInstIntArg = cast<Function>(M.getOrInsertFunction("qasm_print_call_int_arg", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()),(Type*)0));

      qasmCallInstDoubleArg = cast<Function>(M.getOrInsertFunction("qasm_print_call_double_arg", Type::getVoidTy(M.getContext()), Type::getDoubleTy(M.getContext()),(Type*)0));


  FirstInst = NULL; //reset firstInst
  
  CallGraphNode* rootNode = getAnalysis<CallGraph>().getRoot();
  unsigned sccNum = 0;
  forallStr = "";

  errs() << "-------QASM Generation Pass:\n";

  for (scc_iterator<CallGraphNode*> sccIb = scc_begin(rootNode),
         E = scc_end(rootNode); sccIb != E; ++sccIb)
    {
      const std::vector<CallGraphNode*> &nextSCC = *sccIb;

      if(debugDynGenQASMLoops)
	errs() << "\nSCC #" << ++sccNum << " : ";      

      for (std::vector<CallGraphNode*>::const_iterator nsccI = nextSCC.begin(),
	     E = nextSCC.end(); nsccI != E; ++nsccI)
	{
	  Function *F=(*nsccI)->getFunction();	  
	  
	  bool isQtmImage = false;
	  if(F && F->getName().find("_qtm")!=string::npos) isQtmImage = true;

	  if(F && !F->isDeclaration() && isQtmImage){
	    if(debugDynGenQASMLoops)
	      errs() << "=====Processing Function:" << F->getName() <<" \n ";

	    //initialize map structures for this function
	    vector<qGateArg> myQIFVec, myQIFVec1, myQIFVec2;
	    qbitsInFunc[F] = myQIFVec;
	    qbitsInitInFunc[F] = myQIFVec1;
	    funcArgList[F] = myQIFVec2;

	    std::vector<FnCall> myFuncMapVec;
	    mapFunction[F] = myFuncMapVec;

	    getFunctionArguments(F);

	    //get first instruction after alloc 
	    inst_iterator firstInstIter = inst_begin(F);
	    while(isa<AllocaInst>(*firstInstIter)) ++firstInstIter;
	    FirstInst = &(*firstInstIter);
	    
	    //insert alloca insts here
	    ArrayType *strTy = ArrayType::get(Type::getInt8Ty(FirstInst->getContext()), MAX_FUNCTION_NAME);
	    AllocaInst *strAlloc = new AllocaInst(strTy,"",FirstInst);
	    
	    //insert call to print qasm header
	    //insertCallToHeader(F->getName().str(),FirstInst);	    
	    insertCallToHeader(F,FirstInst, strAlloc);	    

	    
	    //visit Alloc Insts in func and find if function is quantum or classical function
	    for(inst_iterator instIb = inst_begin(F),instIe=inst_end(F); instIb!=instIe;++instIb){

	      Instruction *pInst = &*instIb; // Grab pointer to instruction reference	      

	      if(debugDynGenQASMLoops)
		errs() << "\n Processing Inst: "<<*pInst << "\n";

	      analyzeAllocInst(F,pInst, strAlloc);
	      analyzeStoreCbitInst(F,pInst);
	    }

	    map<Function*, vector<qGateArg> >::iterator mpItr = qbitsInFunc.find(F);
	    if((*mpItr).second.size()>0){ //Is Quantum Function
	      //printFuncHeader(F);
	    
	      for(inst_iterator instIb = inst_begin(F),instIe=inst_end(F); instIb!=instIe;++instIb){

		Instruction *pInst = &*instIb; // Grab pointer to instruction reference	      
		allDepQbit.clear();
		
		if(debugDynGenQASMLoops)
		  errs() << "\n Processing Inst: "<<*pInst << "\n";
		
		analyzeInst(F,pInst,strAlloc); //spatil: need a bool return type?
	      }
	      
	      //genQASM(F);
	      
	      }
	    
	  }
	  if(F && !F->isDeclaration() && !isQtmImage){

	    //assign qubit allocations
	    //get first instruction after alloc 
	    inst_iterator firstInstIter = inst_begin(F);
	    while(isa<AllocaInst>(*firstInstIter)) ++firstInstIter;
	    
	    FirstInst = &(*firstInstIter);

	    //insert alloca insts here
	    ArrayType *strTy = ArrayType::get(Type::getInt8Ty(FirstInst->getContext()), MAX_FUNCTION_NAME);
	    AllocaInst *strAlloc = new AllocaInst(strTy,"",FirstInst);

	    for(inst_iterator instIb = inst_begin(F),instIe=inst_end(F); instIb!=instIe;++instIb){

	      Instruction *pInst = &*instIb; // Grab pointer to instruction reference	      

	      if(debugDynGenQASMLoops)
		errs() << "\n Processing Inst: "<<*pInst << "\n";

	      analyzeAllocInstExec(F,pInst, strAlloc);

	      //if inst is intrinsic quantum function, remove it.
	      removeIntrinsicQtmExec(F,pInst);

	    }


	    //remove store-cbits from the function
	    cleanup_store_cbits(F);

	    //errs() << "Removing instructions:\n";
	    for(vector<Instruction*>::iterator iterInst = vRemoveInst.begin(); iterInst != vRemoveInst.end(); ++iterInst){
	      //errs() << *(*iterInst) << "\n";
	      (*iterInst)->eraseFromParent();
	    }     
	    vRemoveInst.clear();

	  }
	  else{	    
	    if(debugDynGenQASMLoops)
	      errs() << "WARNING: Ignoring external node or dummy function.";
	  }
	  
	}
      if (nextSCC.size() == 1 && sccIb.hasLoop())
	errs() << " (Has self-loop).";
    }
  errs()<<"\n--------End of QASM generation";
  errs() << "\n";


  
  return false;
}

