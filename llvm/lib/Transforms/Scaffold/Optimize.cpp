//===- Optimize.cpp - Optimize flattened circuit and produce QASM file-------------------===//
//
//                     The LLVM Scaffold Compiler Infrastructure
//								
//								Update: June 17 2018
//										-- Rx Ry have been added in detection stage
//											Need to handle these cases [unimplemented]
// This file was created by Scaffold Compiler Working Group
//===----------------------------------------------------------------------===//

#include <sstream>
#include <algorithm>
#include <string>
#include <climits>
#include <cstring>
#include <cstdlib>
#include "llvm/IR/Argument.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/ilist.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/IntrinsicInst.h"


using namespace llvm;
using namespace std;

#define MAX_BT_COUNT 15 //max backtrace allowed - to avoid infinite recursive loops
#define MAX_QBIT_ARR_DIM 5 //max dimensions allowed for qbit arrays
#define OPT_THRESHOLD 10
bool debugOptimize = false;

namespace {

  struct qGateArg{ //arguments to qgate calls
    Value* argPtr;
    int argNum;
    bool isQbit;
    bool isAbit;
    bool isCbit;
    bool isParam;
    bool isUndef;
    bool isPtr;
    bool isDouble;
    int numDim; //number of dimensions of qbit array
    int dimSize[MAX_QBIT_ARR_DIM]; //sizes of dimensions of array for qbit declarations OR indices of specific qbit for gate arguments
    int valOrIndex; //Value if not Qbit, Index if Qbit & not a Ptr
    double val;
    //Note: valOrIndex is of type integer. Assumes that quantities will be int in the program.
    qGateArg(): argPtr(NULL), argNum(-1), isQbit(false), isAbit(false), isCbit(false), isParam(false), isUndef(false), isPtr(false), isDouble(false), numDim(0), valOrIndex(-1), val(0.0){ }
  };
  
  struct FnCall{ //datapath sequence
    Function* func;
    Value* instPtr;
    std::vector<qGateArg> qArgs;
  };    

  struct OpGate{
    char gateTy;
    int target;
    int control1;
    int control2;
  };

  struct Optimize : public ModulePass {
    static char ID;  // Pass identification, replacement for typeid
    std::vector<Value*> vectQbit;

    std::vector<qGateArg> tmpDepQbit;
    std::vector<qGateArg> allDepQbit;

    map<Function*, vector<qGateArg> > mapQbitsInit;
    map<Function*, vector<qGateArg> > mapFuncArgs;
    map<Function*, vector<FnCall> > mapMapFunc;

    vector<qGateArg> qbitsInFunc; //qbits in function
    vector<qGateArg> qbitsInFuncShort; //qbits in function
    vector<qGateArg> qbitsInitInFunc; //new qbits declared in function
    vector<qGateArg> funcArgList; //function arguments
    vector<FnCall> mapFunction; //trace sequence of qgate calls
    vector<OpGate> gateList; //for optimizing gates
    vector<string>  bitMap;
    vector<unsigned> indMap;
    map<Value*, qGateArg> mapInstRtn;    //traces return cbits for Meas Inst

    int btCount; //backtrace count

    Optimize() : ModulePass(ID) {  }

    bool getQbitArrDim(Type* instType, qGateArg* qa);
    bool backtraceOperand(Value* opd, int opOrIndex);
    unsigned canSwap(vector<OpGate>& G, unsigned iInd, unsigned fInd);
    bool ifCommute(OpGate G1, OpGate G2);
    void optimal(vector<OpGate>& G, vector<unsigned>& M);
    unsigned countGate(vector<OpGate>& G);
    void erase_OpGate(OpGate& G);
    bool shareBit(OpGate G1, OpGate G2);
    void optimal_initial(Function* F, vector<string>& B, vector<OpGate>& G);
    void analyzeAllocInst(Function* F,Instruction* pinst);
    void analyzeAllocInstShort(Function* F,Instruction* pinst);
    void analyzeCallInst(Function* F,Instruction* pinst);
    void analyzeInst(Function* F,Instruction* pinst);

    // run - Print out SCCs in the call graph for the specified module.
    bool runOnModule(Module &M);

    void printFuncHeader(Function* F, bool lastFunc);

	string to_string(int var);

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
	     << "  isAbit: "<<qg.isAbit
	     << "  isCbit: "<<qg.isCbit
	     << "  isPtr: "<<qg.isPtr << "\n"
	     << "  Value or Index: "<<qg.valOrIndex<<"\n"
	     << "  Num of Dim: "<<qg.numDim<<"\n";
      for(int i = 0; i<qg.numDim; i++)
	errs() << "     dimSize ["<<i<<"] = "<<qg.dimSize[i] << "\n";
    }

    void genGateArray(Function* F);
    void getFunctionArguments(Function* F);
    bool DetermineQFunc(Function* F);
    
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
      AU.addRequired<CallGraphWrapperPass>();
    }
  };
}

char Optimize::ID = 0;
static RegisterPass<Optimize>
X("Optimize", "Optimize circuits"); //spatil: should be Z or X??

bool Optimize::backtraceOperand(Value* opd, int opOrIndex)
{

  if(opOrIndex == 0) //backtrace for operand
  {
    //search for opd in qbit/cbit vector
    std::vector<Value*>::iterator vIter=std::find(vectQbit.begin(),vectQbit.end(),opd);
    if(vIter != vectQbit.end()){
      if(debugOptimize)
        errs()<<"Found qubit associated: "<< opd->getName() << "\n";

      tmpDepQbit[0].argPtr = opd;

      return true;
    }

    if(btCount>MAX_BT_COUNT)
      return false;

    if(GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(opd))
	{
	  if(debugOptimize)
	  {
	      errs() << "Get Elem Ptr Inst Found: " << *GEPI <<"\n";
	      errs() << GEPI->getPointerOperand()->getName();
	      errs() << " has index = " << GEPI->hasIndices();
	      errs() << " has all constant index = " << GEPI->hasAllConstantIndices() << "\n";
	  }

	  if(GEPI->hasAllConstantIndices()){
	    Instruction* pInst = dyn_cast<Instruction>(opd);
	    unsigned numOps = pInst->getNumOperands();
	    if(debugOptimize)
	      errs() << " Has constant index. Num Operands: " << numOps << ": ";

	    
	    bool foundOne = backtraceOperand(pInst->getOperand(0),0);

	    if(numOps>2){ //set the dimensionality of the qbit
	      tmpDepQbit[0].numDim = numOps-2;
	    
	    for(unsigned arrIter=2; arrIter < numOps; arrIter++)
	      {
		ConstantInt *CI = dyn_cast<ConstantInt>(pInst->getOperand(arrIter));
		//errs() << "Arr[ "<<arrIter<<" ] = "<<CI->getZExtValue()<<"\n";
		if(tmpDepQbit.size()==1){
		  tmpDepQbit[0].dimSize[arrIter-2] = CI->getZExtValue();  
		}
	      }
	    }
	    else if(numOps==2){
	      tmpDepQbit[0].numDim = 1;
	      ConstantInt *CI = dyn_cast<ConstantInt>(pInst->getOperand(numOps-1));
	      if(tmpDepQbit.size()==1){
		tmpDepQbit[0].dimSize[0] = CI->getZExtValue();
		if(debugOptimize)
		  errs()<<" Found constant index = "<<CI->getValue()<<"\n";
	      }
	    }

	    //NOTE: getelemptr instruction can have multiple indices. Currently considering last operand as desired index for qubit. Check this reasoning. 
	    ConstantInt *CI = dyn_cast<ConstantInt>(pInst->getOperand(numOps-1));
	    if(tmpDepQbit.size()==1){
	      tmpDepQbit[0].valOrIndex = CI->getZExtValue();
	      if(debugOptimize)
		errs()<<" Found constant index = "<<CI->getValue()<<"\n";
	    }
	    return foundOne;
	  }
	  
	  else if(GEPI->hasIndices()){ //NOTE: Edit this function for multiple indices, some of which are constant, others are not.
	  
	    errs() << "Oh no! I don't know how to handle this case..ABORT ABORT..\n";
	    Instruction* pInst = dyn_cast<Instruction>(opd);
	    unsigned numOps = pInst->getNumOperands();
	    if(debugOptimize)
	      errs() << " Has non-constant index. Num Operands: " << numOps << ": ";		
	    bool foundOne = backtraceOperand(pInst->getOperand(0),0);

	    if(tmpDepQbit[0].isQbit && !(tmpDepQbit[0].isPtr)){     
	      //NOTE: getelemptr instruction can have multiple indices. consider last operand as desired index for qubit. Check if this is true for all.
	      backtraceOperand(pInst->getOperand(numOps-1),1);
	      
	    }
		else if(tmpDepQbit[0].isAbit && !(tmpDepQbit[0].isPtr)){
			backtraceOperand(pInst->getOperand(numOps-1),1);
		}
	    return foundOne;
	  }	  
	  else{	    
	    Instruction* pInst = dyn_cast<Instruction>(opd);
	    unsigned numOps = pInst->getNumOperands();
	    bool foundOne = false;
	    for(unsigned iop=0;(iop<numOps && !foundOne);iop++){
	      foundOne = foundOne || backtraceOperand(pInst->getOperand(iop),0);
	    }
	    return foundOne;
	  }
	}
      
      if(isa<LoadInst>(opd)){
	if(tmpDepQbit[0].isQbit && !tmpDepQbit[0].isPtr){
	  tmpDepQbit[0].numDim = 1;
	  tmpDepQbit[0].dimSize[0] = 0;
	  if(debugOptimize)
	    errs()<<" Added default dim to qbit & not ptr variable.\n";
	}
	else if(tmpDepQbit[0].isAbit && !tmpDepQbit[0].isPtr){
		tmpDepQbit[0].numDim = 1;
		tmpDepQbit[0].dimSize[0] = 0;
	}
      }
      if(Instruction* pInst = dyn_cast<Instruction>(opd)){
	unsigned numOps = pInst->getNumOperands();
	bool foundOne = false;
	for(unsigned iop=0;(iop<numOps && !foundOne);iop++){
	  btCount++;
	  foundOne = foundOne || backtraceOperand(pInst->getOperand(iop),0);
	  btCount--;
	}
	return foundOne;
      }
      else{
	if(debugOptimize)
	  errs() << "Ending Recursion\n";
	return false;
      }
    }
  else if(opOrIndex == 0){ //opOrIndex == 1; i.e. Backtracing for Index    
    if(btCount>MAX_BT_COUNT) //prevent infinite backtracing
      return true;

    if(ConstantInt *CI = dyn_cast<ConstantInt>(opd)){
      tmpDepQbit[0].valOrIndex = CI->getZExtValue();
      if(debugOptimize)
	errs()<<" Found constant index = "<<CI->getValue()<<"\n";

      return true;
    }      

    if(Instruction* pInst = dyn_cast<Instruction>(opd)){
      unsigned numOps = pInst->getNumOperands();
      bool foundOne = false;
      for(unsigned iop=0;(iop<numOps && !foundOne);iop++){
	btCount++;
	foundOne = foundOne || backtraceOperand(pInst->getOperand(iop),1);
	btCount--;
      }
      return foundOne;
    }

  }

  else{ //opOrIndex == 2: backtracing to call inst MeasZ
    if(debugOptimize)
      errs()<<"backtracing for call inst: "<<*opd<<"\n";
    if(CallInst *endCI = dyn_cast<CallInst>(opd)){
      if(endCI->getCalledFunction()->getName().find("llvm.Meas") != string::npos){
	tmpDepQbit[0].argPtr = opd;

	if(debugOptimize)
	  errs()<<" Found call inst = "<<*endCI<<"\n";
	return true;
      }
      else{
	if(Instruction* pInst = dyn_cast<Instruction>(opd)){
	  unsigned numOps = pInst->getNumOperands();
	  bool foundOne=false;
	  for(unsigned iop=0;(iop<numOps && !foundOne);iop++){
	    btCount++;
	    foundOne = foundOne || backtraceOperand(pInst->getOperand(iop),2);
	    btCount--;
	  }
	  return foundOne;
	}
      }
    }
    else{
      if(Instruction* pInst = dyn_cast<Instruction>(opd)){
	unsigned numOps = pInst->getNumOperands();
	bool foundOne=false;
	for(unsigned iop=0;(iop<numOps && !foundOne);iop++){
	  btCount++;
	  foundOne = foundOne || backtraceOperand(pInst->getOperand(iop),2);
	  btCount--;
	}
	return foundOne;
      }
    }
  }
  return false;
}

bool Optimize::getQbitArrDim(Type *instType, qGateArg* qa)
{
  bool myRet = false;

  errs() << "In get_all_dimensions \n";

  if(ArrayType *arrayType = dyn_cast<ArrayType>(instType)) {
    Type *elementType = arrayType->getElementType();
    uint64_t arraySize = arrayType->getNumElements();
    errs() << "Array Size = "<<arraySize << "\n";
    qa->dimSize[qa->numDim] = arraySize;
    qa->numDim++;

    if (elementType->isIntegerTy(16)){
      myRet = true;
      qa->isQbit = true;
    }
    else if (elementType->isIntegerTy(1)){
      myRet = true;
      qa->isCbit = true;
    }
	else if(elementType->isIntegerTy(8)){
		myRet = true;
		qa->isAbit = true;
	}
    else if (elementType->isArrayTy()){
      myRet |= getQbitArrDim(elementType,qa);
    }
    else myRet = false;
  }

  return myRet;

}

void Optimize::analyzeAllocInstShort(Function* F, Instruction* pInst){

  if (AllocaInst *AI = dyn_cast<AllocaInst>(pInst)) {
    Type *allocatedType = AI->getAllocatedType();
    
    if(ArrayType *arrayType = dyn_cast<ArrayType>(allocatedType)) {      
      qGateArg tmpQArg;

      Type *elementType = arrayType->getElementType();
      // uint64_t arraySize = arrayType->getNumElements();
      if (elementType->isIntegerTy(16)){
	if(debugOptimize)
	  errs() << "New QBit Allocation Found: " << AI->getName() <<"\n";
	qbitsInFuncShort.push_back(tmpQArg);
      }
      
      else if (elementType->isIntegerTy(1)){
	if(debugOptimize)
	  errs() << "New CBit Allocation Found: " << AI->getName() <<"\n";
	qbitsInFuncShort.push_back(tmpQArg);
      }

	  else if (elementType->isIntegerTy(8)){
		qbitsInFuncShort.push_back(tmpQArg);
	  }

    }
    else if(allocatedType->isIntegerTy(16)){
        qGateArg tmpQArg;
        qbitsInFuncShort.push_back(tmpQArg);
    }
	else if(allocatedType->isIntegerTy(8)){
		qGateArg tmpQArg;
		qbitsInFuncShort.push_back(tmpQArg);
	}
	//find argName2 in funcArgList - avoid printing out qbit declaration twice
	//std::map<Function*, vector<qGateArg> >::iterator mIter = funcArgList.find(F);
	//if(mIter != funcArgList.end()){
    return;
  }

}

void Optimize::analyzeAllocInst(Function* F, Instruction* pInst){
  if (AllocaInst *AI = dyn_cast<AllocaInst>(pInst)) {
    Type *allocatedType = AI->getAllocatedType();
    
    if(ArrayType *arrayType = dyn_cast<ArrayType>(allocatedType)) {      
      qGateArg tmpQArg;
	  
	  Type *elementType = arrayType->getElementType();
      uint64_t arraySize = arrayType->getNumElements();
      if (elementType->isIntegerTy(16)){
	if(debugOptimize)
	  errs() << "New QBit Allocation Found: " << AI->getName() <<"\n";
	vectQbit.push_back(AI);
	tmpQArg.isQbit = true;
	tmpQArg.argPtr = AI;
	tmpQArg.numDim = 1;
	tmpQArg.dimSize[0] = arraySize;
	tmpQArg.valOrIndex = arraySize;
	//(qbitsInFunc.find(F))->second.push_back(tmpQArg);
	qbitsInFunc.push_back(tmpQArg);
	//(qbitsInitInFunc.find(F))->second.push_back(tmpQArg);	
	qbitsInitInFunc.push_back(tmpQArg);	
      }
      
      else if (elementType->isIntegerTy(1)){
	if(debugOptimize)
	  errs() << "New CBit Allocation Found: " << AI->getName() <<"\n";
	vectQbit.push_back(AI); //Cbit added here
	tmpQArg.isCbit = true;
	tmpQArg.argPtr = AI;
	tmpQArg.numDim = 1;
	tmpQArg.dimSize[0] = arraySize;
	tmpQArg.valOrIndex = arraySize;
	//(qbitsInFunc.find(F))->second.push_back(tmpQArg);
	qbitsInFunc.push_back(tmpQArg);
	//(qbitsInitInFunc.find(F))->second.push_back(tmpQArg);	
	qbitsInitInFunc.push_back(tmpQArg);	
      }

	  else if (elementType->isIntegerTy(8)){
		vectQbit.push_back(AI); //Cbit added here
		tmpQArg.isCbit = false;
		tmpQArg.isAbit = true;
		tmpQArg.argPtr = AI;
		tmpQArg.numDim = 1;
		tmpQArg.dimSize[0] = arraySize;
		tmpQArg.valOrIndex = arraySize;
		//(qbitsInFunc.find(F))->second.push_back(tmpQArg);
		qbitsInFunc.push_back(tmpQArg);
		//(qbitsInitInFunc.find(F))->second.push_back(tmpQArg);	
		qbitsInitInFunc.push_back(tmpQArg);	
	  }

      else if(elementType->isArrayTy()){
	  errs() << "Multidimensional array\n";

	  tmpQArg.dimSize[0] = arraySize;
	  tmpQArg.numDim++;
	  tmpQArg.valOrIndex = arraySize;

	  //recurse on multi-dimensional array
	  bool isQAlloc = getQbitArrDim(elementType,&tmpQArg);

	  if(isQAlloc){
	    vectQbit.push_back(AI);
	    tmpQArg.argPtr = AI;
	    //(qbitsInFunc.find(F))->second.push_back(tmpQArg);
	    qbitsInFunc.push_back(tmpQArg);
	    //(qbitsInitInFunc.find(F))->second.push_back(tmpQArg);
	    qbitsInitInFunc.push_back(tmpQArg);

	    if(debugOptimize)
	      print_qgateArg(tmpQArg);
	  }	  
      }

    }
    else if(allocatedType->isIntegerTy(16)){
        qGateArg tmpQArg;
        if(debugOptimize) errs() << "Found New Qbit Allocation \n";
        vectQbit.push_back(AI);
        tmpQArg.isQbit = true;
        tmpQArg.argPtr = AI;
        tmpQArg.numDim = 1;
//        tmpQArg.dimSize[0] = 1;
        tmpQArg.dimSize[0] = cast<ConstantInt>(AI->getArraySize())->getSExtValue();
        tmpQArg.valOrIndex = 1;
        qbitsInFunc.push_back(tmpQArg);
        qbitsInitInFunc.push_back(tmpQArg);
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
      if (elementType->isIntegerTy(16)){
	vectQbit.push_back(AI);
	
	qGateArg tmpQArg;
	tmpQArg.isPtr = true;
	tmpQArg.isQbit = true;
	tmpQArg.argPtr = AI;
	
	//(qbitsInFunc.find(F))->second.push_back(tmpQArg);
	qbitsInFunc.push_back(tmpQArg);
	
	std::string argName = AI->getName();
	unsigned pos = argName.find(".addr");
	std::string argName2 = argName.substr(0,pos);

	//find argName2 in funcArgList - avoid printing out qbit declaration twice
	//std::map<Function*, vector<qGateArg> >::iterator mIter = funcArgList.find(F);
	//if(mIter != funcArgList.end()){
	  bool foundit = false;
	  for(vector<qGateArg>::iterator vParamIter = funcArgList.begin();(vParamIter!=funcArgList.end() && !foundit);++vParamIter){
	    if((*vParamIter).argPtr->getName() == argName2){ 
	      foundit = true;
	    }
	  }
	  if(!foundit) //do not add duplicate declaration	    
	    qbitsInitInFunc.push_back(tmpQArg);
	  //}
      }
	  else if (elementType->isIntegerTy(8)){
		vectQbit.push_back(AI);
		qGateArg tmpQArg;
		tmpQArg.isPtr = true;
		tmpQArg.isAbit = true;
		tmpQArg.argPtr = AI;

		qbitsInFunc.push_back(tmpQArg);

		std::string argName = AI->getName();
		unsigned pos = argName.find(".addr");
		std::string argName2 = argName.substr(0,pos);
		bool foundit = false;
		for(vector<qGateArg>::iterator vParamIter = funcArgList.begin();(vParamIter!=funcArgList.end() && !foundit);++vParamIter){
	    	if((*vParamIter).argPtr->getName() == argName2) foundit = true;
	  	}
		if(!foundit) qbitsInitInFunc.push_back(tmpQArg);
	  }
	}
    return;
  }

}

void Optimize::analyzeCallInst(Function* F, Instruction* pInst){
  if(CallInst *CI = dyn_cast<CallInst>(pInst))
    {
      if(debugOptimize)      
	errs() << "Call inst: " << CI->getCalledFunction()->getName() << "\n";

      if(CI->getCalledFunction()->getName() == "store_cbit"){	//trace return values
	qGateArg tmpQGateArg1;
	tmpQGateArg1.isCbit = true;
	tmpDepQbit.push_back(tmpQGateArg1);
	backtraceOperand(CI->getArgOperand(0),2); //value Operand
	Value* rtnVal = tmpDepQbit[0].argPtr;
	tmpDepQbit.clear();

	qGateArg tmpQGateArg2;
	tmpQGateArg2.isCbit = true;
	tmpQGateArg2.isPtr = true;
	tmpDepQbit.push_back(tmpQGateArg2);	
	backtraceOperand(CI->getArgOperand(1),0); //pointer Operand

	//insert info in map here
	mapInstRtn[rtnVal] = tmpDepQbit[0];

	tmpDepQbit.clear();
	return;
      }
      
      bool tracked_all_operands = true;
      
      for(unsigned iop=0;iop<CI->getNumArgOperands();iop++){
	tmpDepQbit.clear();
	
	qGateArg tmpQGateArg;
	btCount=0;
	
	if(debugOptimize)
	  errs() << "Call inst operand num: " << iop << "\n";
	
	tmpQGateArg.argNum = iop;
	
	
	if(isa<UndefValue>(CI->getArgOperand(iop))){
	  //errs() << "WARNING: LLVM IR code has UNDEF values. \n";
	  tmpQGateArg.isUndef = true;	
	  //exit(1);
	  //assert(0 && "LLVM IR code has UNDEF values. Aborting...");
	}
	
	Type* argType = CI->getArgOperand(iop)->getType();
	if(argType->isPointerTy()){
	  tmpQGateArg.isPtr = true;
	  Type *argElemType = argType->getPointerElementType();
	  if(argElemType->isIntegerTy(16))
	    tmpQGateArg.isQbit = true;
	  if(argElemType->isIntegerTy(1))
	    tmpQGateArg.isCbit = true;
	  if(argElemType->isIntegerTy(8))
		tmpQGateArg.isAbit = true;
	}
	else if(argType->isIntegerTy(16)){
	  tmpQGateArg.isQbit = true;
	  tmpQGateArg.valOrIndex = 0;	 
	}	  	
	else if(argType->isIntegerTy(32)){
	  if(ConstantInt *CInt = dyn_cast<ConstantInt>(CI->getArgOperand(iop))){
	  	tmpQGateArg.isParam = true;
	  	tmpQGateArg.valOrIndex = CInt->getZExtValue();
	  }
	}
	else if(argType->isIntegerTy(8)){
		tmpQGateArg.isAbit = true;
		tmpQGateArg.valOrIndex = 0;
	}
	else if(argType->isIntegerTy(1)){
	  tmpQGateArg.isCbit = true;
	  tmpQGateArg.valOrIndex = 0;	 
	}	  	
	
	//check if argument is constant int	
	if(ConstantInt *CInt = dyn_cast<ConstantInt>(CI->getArgOperand(iop))){
	  tmpQGateArg.valOrIndex = CInt->getZExtValue();
	  if(debugOptimize){
	    errs()<<" Found constant argument = "<<CInt->getValue()<<"\n";
	  }
	}
	
	//check if argument is constant float	
	if(ConstantFP *CFP = dyn_cast<ConstantFP>(CI->getArgOperand(iop))){
	  tmpQGateArg.val = CFP->getValueAPF().convertToDouble();
	  tmpQGateArg.isDouble = true;
	  if(debugOptimize){
	    errs()<<" Call Inst = "<<*CI<<"\n";
	    errs()<<" Found constant double argument = "<<tmpQGateArg.val<<"\n";
	  }
	}

	tmpDepQbit.push_back(tmpQGateArg);
	
	tracked_all_operands &= backtraceOperand(CI->getArgOperand(iop),0);
	
	if(tmpDepQbit.size()>0){
	  if(debugOptimize)
	    print_qgateArg(tmpDepQbit[0]);
	  
	  allDepQbit.push_back(tmpDepQbit[0]);
	  assert(tmpDepQbit.size() == 1 && "tmpDepQbit SIZE GT 1");
	  tmpDepQbit.clear();
	}
	
      }
                  
      //form info packet
      FnCall qInfo;
      qInfo.func = CI->getCalledFunction();
      qInfo.instPtr = CI;
      
      if(allDepQbit.size() > 0){
	if(debugOptimize)
	  {
	    errs() << "\nCall inst: " << CI->getCalledFunction()->getName();	    
	    errs() << ": Found all arguments: ";       
	    for(unsigned int vb=0; vb<allDepQbit.size(); vb++){
	      if(allDepQbit[vb].argPtr)
		errs() << allDepQbit[vb].argPtr->getName() <<" ";
	      else
		errs() << allDepQbit[vb].valOrIndex <<" ";
	    }
	    errs()<<"\n";
	  }
	
	//populate vector of passed qubit arguments
	for(unsigned int vb=0; vb<allDepQbit.size(); vb++)
	  qInfo.qArgs.push_back(allDepQbit[vb]);
	
      }
      
      //map<Function*, vector<FnCall> >::iterator mvdpit = mapFunction.find(F);	
      //(*mvdpit).second.push_back(qInfo);      
      mapFunction.push_back(qInfo);

      return;      
    }
}

void Optimize::analyzeInst(Function* F, Instruction* pInst){
  if(debugOptimize)
    errs() << "--Processing Inst: "<<*pInst << '\n';

  //analyzeAllocInst(F,pInst);
  analyzeCallInst(F,pInst);
    
  if(debugOptimize)
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

std::string Optimize::to_string(int var){
	stringstream ss;
	ss << var;
	return ss.str();
}

void Optimize::printFuncHeader(Function* F, bool lastFunc)
{

  //map<Function*, vector<qGateArg> >::iterator mpItr;
  //map<Function*, vector<qGateArg> >::iterator mpItr2;
  //map<Function*, vector<qGateArg> >::iterator mvpItr;

  //mpItr = qbitsInFunc.find(F);

  //print name of function

  funcArgList = mapFuncArgs.find(F)->second;
  qbitsInitInFunc = mapQbitsInit.find(F)->second;

  std::string newName = F->getName();
  std::replace(newName.begin(), newName.end(), '.','_');
  std::replace(newName.begin(), newName.end(), '-','_');

  if(lastFunc) errs() << "\nmodule main";
  else errs()<<"\nmodule "<< newName;
  
  //print arguments of function
  //mpItr2=funcArgList.find(F);    
  errs()<<" ( ";    
  unsigned tmp_num_elem = funcArgList.size();
  
  if(tmp_num_elem > 0){
    for(unsigned tmp_i=0;tmp_i<tmp_num_elem - 1;tmp_i++){
      
      qGateArg tmpQA = funcArgList[tmp_i];
      
      if(debugOptimize)
	print_qgateArg(tmpQA);
      
      if(tmpQA.isQbit)
	errs()<<"qbit";
      else if(tmpQA.isCbit)
	errs()<<"cbit";
	  else if(tmpQA.isAbit)
		errs() << "qbit";
      else{
	Type* argTy = tmpQA.argPtr->getType();
	if(argTy->isDoubleTy()) errs() << "double";
	else if(argTy->isFloatTy()) errs() << "float";
	else if(argTy->isIntegerTy(32)){
		string var = to_string(tmpQA.valOrIndex);
		errs() << var;
	}
	else
	  errs()<<"UNRECOGNIZED "<<argTy<<" ";
      }
      
      if(tmpQA.isPtr)
	errs()<<"*";	  
      
      errs()<<" "<<printVarName(tmpQA.argPtr->getName())<<" , ";
    }
    
    if(debugOptimize)
      print_qgateArg(funcArgList[tmp_num_elem-1]);
    
    if((funcArgList[tmp_num_elem-1]).isQbit)
      errs()<<"qbit";
    else if((funcArgList[tmp_num_elem-1]).isCbit)
      errs()<<"cbit";
	else if((funcArgList[tmp_num_elem-1]).isAbit)
	  errs() << "qbit";
    else{
      Type* argTy = (funcArgList[tmp_num_elem-1]).argPtr->getType();
      if(argTy->isDoubleTy()) errs() << "double";
      else if(argTy->isFloatTy()) errs() << "float";
	  else if(argTy->isIntegerTy(32)){
			string var = to_string(funcArgList[tmp_num_elem-1].valOrIndex); 
//			errs() << var; 
			errs() << "int size"; 
	  }
	else
	errs()<<"UNRECOGNIZED "<<argTy<<" ";
    }
    
    if((funcArgList[tmp_num_elem-1]).isPtr)
      errs()<<"*";
    
    errs() <<" "<<printVarName((funcArgList[tmp_num_elem-1]).argPtr->getName());
  }
  
  errs()<<" ) {\n ";   
  
  //print qbits declared in function
  //mvpItr=qbitsInitInFunc.find(F);	    
  for(vector<qGateArg>::iterator vvit=qbitsInitInFunc.begin(),vvitE=qbitsInitInFunc.end();vvit!=vvitE;++vvit)
    {	
      if((*vvit).isQbit)
	errs()<<"\tqbit "<<printVarName((*vvit).argPtr->getName());
      if((*vvit).isCbit)
	errs()<<"\tcbit "<<printVarName((*vvit).argPtr->getName());
	  if((*vvit).isAbit)
	errs() << "\tqbit "<<printVarName((*vvit).argPtr->getName());

      //if only single-dimensional qbit arrays expected
      //errs()<<"["<<(*vvit).valOrIndex<<"];\n ";

      //if n-dimensional qbit arrays expected 
      for(int ndim = 0; ndim < (*vvit).numDim; ndim++)
        errs()<<"["<<(*vvit).dimSize[ndim]<<"]";
        errs() << ";\n";
    }
  //errs() << "//--//-- Fn: " << F->getName() << " --//--//\n";
}
  
void Optimize::genGateArray(Function* F)
{
  //map<Function*, vector<qGateArg> >::iterator mpItr;
  //map<Function*, vector<qGateArg> >::iterator mpItr2;
  //map<Function*, vector<qGateArg> >::iterator mvpItr;
  
  //mpItr = qbitsInFunc.find(F);
//  if(qbitsInFunc.size()>0){
    
    mapFunction = mapMapFunc.find(F)->second; 
    //print gates in function
    //map<Function*, vector<FnCall> >::iterator mfvIt = mapFunction.find(F);
    for(unsigned mIndex=0;mIndex<mapFunction.size();mIndex++){
      if(mapFunction[mIndex].qArgs.size()>0)
      {

	string fToPrint = mapFunction[mIndex].func->getName();
	if(fToPrint.find("llvm.") != string::npos)
	  fToPrint = fToPrint.substr(5);
	errs()<<"\t";

	//print return operand before printing MeasZ
	if(fToPrint.find("Meas") != string::npos){
	  //get inst ptr
	  Value* thisInstPtr = mapFunction[mIndex].instPtr;
	  //find inst in mapInstRtn
	  map<Value*, qGateArg>::iterator mvq = mapInstRtn.find(thisInstPtr);
	  if(mvq!=mapInstRtn.end()){
	    errs()<<printVarName(((*mvq).second).argPtr->getName());
	    if(((*mvq).second).isPtr)
	      errs()<<"["<<((*mvq).second).valOrIndex<<"]";
	    errs()<<" = ";
	  }	  
	}
	// Intrinsic Conversion
	if(fToPrint.find("CNOT") != string::npos) fToPrint = "CNOT";
	else if(fToPrint.find("Toffoli.") != string::npos) fToPrint = "Toffoli";
	else if(fToPrint.find("MeasX") != string::npos) fToPrint = "MeasX";
	else if(fToPrint.find("MeasZ") != string::npos) fToPrint = "MeasZ";
	else if(fToPrint.find("H.i") != string::npos) fToPrint = "H";
	else if(fToPrint.find("Fredkin") != string::npos) fToPrint = "Fredkin";
	else if(fToPrint.find("PrepX") != string::npos) fToPrint = "PrepX";
	else if(fToPrint.find("PrepZ") != string::npos) fToPrint = "PrepZ";
	else if(fToPrint.substr(0,2) == "Rx") fToPrint = "Rx";
	else if(fToPrint.substr(0,2) == "Ry") fToPrint = "Ry";
	else if(fToPrint.substr(0,2) == "Rz") fToPrint = "Rz";
	else if(fToPrint.find("S.") != string::npos) fToPrint = "S";
	else if(fToPrint.find("T.") != string::npos) fToPrint = "T";
	else if(fToPrint.find("Sdag") != string::npos) fToPrint = "Sdag";
	else if(fToPrint.find("Tdag") != string::npos) fToPrint = "Tdag";
	else if(fToPrint.find("X.") != string::npos) fToPrint = "X";
	else if(fToPrint.find("Z.") != string::npos) fToPrint = "Z";

	std::replace(fToPrint.begin(), fToPrint.end(), '.', '_');
	std::replace(fToPrint.begin(), fToPrint.end(), '-', '_');
	errs()<<fToPrint<<" ( ";

	//print all but last argument
	for(vector<qGateArg>::iterator vpIt=mapFunction[mIndex].qArgs.begin(), vpItE=mapFunction[mIndex].qArgs.end();vpIt!=vpItE-1;++vpIt)
	  {
	    if((*vpIt).isUndef)
	      errs() << " UNDEF ";
	    else{
	      if((*vpIt).isQbit || (*vpIt).isCbit || (*vpIt).isAbit ){
		errs()<<printVarName((*vpIt).argPtr->getName());
		if(!((*vpIt).isPtr)){		  
		  //if only single-dimensional qbit arrays expected
		  //--if((*vpIt).numDim == 0)
		  //errs()<<"["<<(*vpIt).valOrIndex<<"]";
		  //--else
		    //if n-dimensional qbit arrays expected 
		    for(int ndim = 0; ndim < (*vpIt).numDim; ndim++)
		      errs()<<"["<<(*vpIt).dimSize[ndim]<<"]";
		}
	      }
		  else if((*vpIt).isParam){
			errs() << (*vpIt).valOrIndex;
		  }
	      else{
		//assert(!(*vpIt).isPtr); 
		if((*vpIt).isPtr) //NOTE: not expecting non-quantum pointer variables as arguments to quantum functions. If they exist, then print out name of variable
		  errs() << " UNRECOGNIZED ";
		else if((*vpIt).isDouble)
		  errs() << (*vpIt).val;
		else
		  errs()<<(*vpIt).valOrIndex;	      
	      }
	    }	    	    
	    errs()<<" , ";
	  }

	//print last element	
	qGateArg tmpQA = mapFunction[mIndex].qArgs.back();

	if(tmpQA.isUndef)
	  errs() << " UNDEF ";
	else{
	  if(tmpQA.isQbit || tmpQA.isCbit || tmpQA.isAbit){
	    errs()<<printVarName(tmpQA.argPtr->getName());
	    if(!(tmpQA.isPtr)){
	      //if only single-dimensional qbit arrays expected
	      //--if(tmpQA.numDim == 0)
	      //errs()<<"["<<tmpQA.valOrIndex<<"]";
	      //--else
		//if n-dimensional qbit arrays expected 
		for(int ndim = 0; ndim < tmpQA.numDim; ndim++)
		  errs()<<"["<<tmpQA.dimSize[ndim]<<"]";	      	      	      
	    }
	  }
	  else{
	    //assert(!tmpQA.isPtr); //NOTE: not expecting non-quantum pointer variables as arguments to quantum functions. If they exist, then print out name of variable
	    if(tmpQA.isPtr)
	      errs() << " UNRECOGNIZED ";
	    else if(tmpQA.isDouble) 
	      errs() << tmpQA.val;
	    else
	      errs()<<tmpQA.valOrIndex;	    
	  }
	  
	}
	errs()<<" );\n ";	      
      }
    }

    //errs() << "//--//-- End Fn: " << F->getName() << " --//--// \n";
    errs()<<"}\n";
//  }
}


bool Optimize::DetermineQFunc(Function* F)
{
    for(inst_iterator instIb = inst_begin(F),instIe=inst_end(F); instIb!=instIe;++instIb){
        Instruction *pInst = &*instIb; // Grab pointer to instruction reference	      
	    analyzeAllocInstShort(F,pInst);
	}
    if(qbitsInFuncShort.size() > 0) return true;
    qbitsInFuncShort.clear();
    return false;
}

unsigned Optimize::canSwap(vector<OpGate>& G, unsigned iInd, unsigned fInd)
{
    if(iInd > fInd) {
        errs()<<"initial index larger than final index.\n";
        return UINT_MAX-1;
    }
    
    unsigned iPos = iInd, fPos = fInd;
    for(unsigned i = iInd + 1; i <= fInd; i++){
        if(ifCommute(G[iInd], G[i])){ iPos = i; }
        else { break; }
    }
    for(unsigned i = fInd - 1; i >= iInd; i--){
        if(ifCommute(G[fInd], G[i])){fPos = i;}
        else{break;}
    } 
    if(iPos >= fPos - 1 ){
        return max(fPos - 1, iInd); 
    }
    else{
        return UINT_MAX; 
    }
}

bool Optimize::ifCommute(OpGate G1, OpGate G2){
    bool G1st = false, G2st = false;
    if(G1.gateTy == 't' || G1.gateTy == 's' || G1.gateTy =='S' || G1.gateTy == 'T') {G1st =true;}
    if(G2.gateTy == 't' || G2.gateTy == 's' || G2.gateTy =='S' || G2.gateTy == 'T') {G2st =true;}

    if(G1.gateTy =='\0' || G2.gateTy == '\0') {return true;}

    if(G1.gateTy != 'c' && G1.gateTy != 'o' && G2.gateTy != 'c' && G2.gateTy != 'o'){
        if(G1.target != G2.target) {return true;}
        else if(G1st && G2st) {return true;}
        else {return false;}
    }

    else if((G1.gateTy != 'c' && G1.gateTy != 'o')){
        if(G1.target != G2.target && G1.target != G2.control1 && G1.target != G2.control2){return true;}
        if(G1st && G1.target != G2.target){return true;}
        return false;
    }

    else if((G2.gateTy != 'c' && G2.gateTy != 'o')){
        if(G2.target != G1.target && G2.target != G1.control1 && G2.target != G1.control2){return true;}
        if(G2st && G1.target != G2.target){return true;}
        return false;
    }

    else if(G1.target != G2.control1 && G1.target != G2.control2 && G2.target != G1.control1 && G2.target != G1.control2){
        return true; 
    }
    return false;
}

bool Optimize::shareBit(OpGate G1, OpGate G2){
    if((G1.target == G2.target && G1.target != 0) || (G1.target == G2.control1 && G1.target != 0) || (G1.target == G2.control2 && G1.target != 0) || (G1.control1 == G2.control2 && G1.control1 != 0) || (G1.control1 == G2.control1 && G1.control1 != 0) || (G1.control1 == G2.target && G1.control1 != 0) || (G1.control2 == G2.target && G1.control2 != 0) || (G1.control2 == G2.control1 && G1.control2 != 0) || (G1.control2 == G2.control2 && G1.control2 != 0)){ return true;}
    else {return false;}
}
void Optimize::getFunctionArguments(Function* F)
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
	if(debugOptimize)
	  errs()<<"Argument Type: " << *argType <<"\n";

	tmpQArg.isPtr = true;

	Type *elementType = argType->getPointerElementType();
	if (elementType->isIntegerTy(16)){
	  tmpQArg.isQbit = true;
	  vectQbit.push_back(ait);
	  qbitsInFunc.push_back(tmpQArg);
	  funcArgList.push_back(tmpQArg);
	}
	else if (elementType->isIntegerTy(1)){
	  tmpQArg.isCbit = true;
	  vectQbit.push_back(ait);
	  qbitsInFunc.push_back(tmpQArg);
	  funcArgList.push_back(tmpQArg);
	}
	else if(elementType->isIntegerTy(8)){
	  tmpQArg.isAbit = true;
	  vectQbit.push_back(ait);
	  qbitsInFunc.push_back(tmpQArg);
	  funcArgList.push_back(tmpQArg);
	}
      }
      else if (argType->isIntegerTy(16)){
	tmpQArg.isQbit = true;
	vectQbit.push_back(ait);
	qbitsInFunc.push_back(tmpQArg);
	funcArgList.push_back(tmpQArg);
      }
      else if (argType->isIntegerTy(32)){
	tmpQArg.isParam = true;
	vectQbit.push_back(ait);
	funcArgList.push_back(tmpQArg);
      }
      else if (argType->isIntegerTy(1)){
	tmpQArg.isCbit = true;
	vectQbit.push_back(ait);
	qbitsInFunc.push_back(tmpQArg);
	funcArgList.push_back(tmpQArg);
      }
	  else if(argType->isIntegerTy(8)){
		tmpQArg.isAbit = true;
		vectQbit.push_back(ait);
		qbitsInFunc.push_back(tmpQArg);
		funcArgList.push_back(tmpQArg);
	  }
      else if(argType->isDoubleTy())     
	funcArgList.push_back(tmpQArg);

      if(debugOptimize)
	print_qgateArg(tmpQArg);
    }
}

unsigned Optimize::countGate(vector<OpGate>& G){
    unsigned rtvl = 0;
    for(unsigned i = 0; i < G.size(); i++){
        if(G[i].gateTy != '\0') rtvl++; 
    }
    return rtvl;
}
//Convert llvm internal data structure to OpGate for cleaner qubit argument(of gate) presentation
//and easier qubit map later
void Optimize::optimal_initial(Function * F, vector<string> &B, vector<OpGate> &G){
             
    mapFunction = mapMapFunc.find(F)->second;
    
    B.push_back(" ");

    unsigned i = 1;
    for(vector<qGateArg>::iterator vvit=qbitsInitInFunc.begin(),vvitE=qbitsInitInFunc.end();vvit!=vvitE;++vvit){
        
        string qName = printVarName((*vvit).argPtr->getName());
        unsigned dMul = 1;
        //flatten multi-array 
        for(unsigned ndim = 0; ndim < (*vvit).numDim; ndim++){
            dMul *= dMul * (*vvit).dimSize[ndim];
         }
        for(unsigned dimIter = 0; dimIter < dMul; dimIter++){
            string newQ = qName + '_' + to_string(dimIter);
            B.push_back(newQ);
            i++;
        }
    }

    for(unsigned mIndex=0;mIndex<mapFunction.size();mIndex++){
        OpGate nxtGate = {'\0', 0, 0, 0};

	    string fName = mapFunction[mIndex].func->getName();

        if(fName.find("H.") !=string::npos){ nxtGate.gateTy = 'h';}
        else if(fName.find("CNOT") !=string::npos){ nxtGate.gateTy = 'c';}
        else if(fName.find("Tof") !=string::npos){ nxtGate.gateTy = 'o';}
        else if(fName.find("MeasX") !=string::npos){ nxtGate.gateTy = 'X';}
        else if(fName.find("MeasZ") !=string::npos){ nxtGate.gateTy = 'Z';}
        else if(fName.find("PrepX") !=string::npos){ nxtGate.gateTy = 'n';}
        else if(fName.find("PrepZ") !=string::npos){ nxtGate.gateTy = 'm';}
        else if(fName.find("Sdag") !=string::npos){ nxtGate.gateTy = 'S';}
        else if(fName.find("S.") !=string::npos){ nxtGate.gateTy = 's';}
        else if(fName.find("T.") !=string::npos){ nxtGate.gateTy = 't';}
        else if(fName.find("Tdag") !=string::npos){ nxtGate.gateTy = 'T';}
		// Need a new variable representation for rotations in different axes?
//      else if(fName.substr(0,2) == "Rx"){ nxtGate.gateTy = 'r';}
//      else if(fName.substr(0,2) == "Ry"){ nxtGate.gateTy = 'r';}
        else if(fName.substr(0,2) == "Rz"){ nxtGate.gateTy = 'r';}
        else if(fName.find("X.") !=string::npos){ nxtGate.gateTy = 'x';}
        else if(fName.find("Z.") !=string::npos){ nxtGate.gateTy = 'z';}
       
        unsigned ToC = 0; //target or control
        for(vector<qGateArg>::iterator vpIt=mapFunction[mIndex].qArgs.begin(), vpItE=mapFunction[mIndex].qArgs.end();vpIt!=vpItE;++vpIt){
           string qName = printVarName((*vpIt).argPtr->getName()) ;
           unsigned qInd = 0;
           for(vector<qGateArg>::iterator vvit=qbitsInitInFunc.begin(),vvitE=qbitsInitInFunc.end();vvit!=vvitE;++vvit){
                   if((*vvit).argPtr == (*vpIt).argPtr){
                       for(unsigned ndim = 0; ndim < (*vpIt).numDim-1; ndim++){
                            qInd += (*vpIt).dimSize[ndim]*(*vvit).dimSize[ndim+1];
                       }
                       qInd += (*vpIt).dimSize[(*vpIt).numDim-1];
                   }
           }
           qName = qName + '_' + to_string(qInd);
           bool foundGate = false;
           for(unsigned i = 1; i < B.size(); i++){

               if(qName.compare(B[i]) == 0){
                   if(nxtGate.gateTy == 'c'){
                      if(ToC == 0) 
                        {nxtGate.control1 = i;}
                      if(ToC == 1)
                        {nxtGate.target = i;}
                   }
                   else if(nxtGate.gateTy == 'o'){
                      if(ToC == 0) 
                        {nxtGate.control1 = i;}
                      if(ToC == 1)
                        {nxtGate.control2 = i;}
                      if(ToC == 2)
                        {nxtGate.target = i;}
                   }
                   else{
                      if(ToC == 0)
                        {nxtGate.target = i;}
                   }
                   foundGate = true;
                   break;
               } 
           }
           ToC++;
           if(!foundGate){errs()<<"can't find the gate.\n";}
       }
       G.push_back(nxtGate);
   }
    //errs()<<"Printing OpGate list"<<"\n";
    //for(vector<OpGate>::iterator Git = G.begin(), GitE = G.end(); Git != GitE; ++Git){
    //   errs()<< Git->gateTy<<" "<<Git->control1<<" "<<Git->control2<< " " << Git->target<<"\n";
    // } 
    for(unsigned i = 0; i < B.size()+1; i++){
        indMap.push_back(i);
    }

} 
void Optimize::erase_OpGate(OpGate& g){
    g.gateTy = '\0';
    g.target = 0;
    g.control1 = 0;
    g.control2 = 0;
    return;
}
void Optimize::optimal(vector<OpGate>& G, vector<unsigned>& M){

    if(G.size() == 0) return;

    //templates
    for(unsigned gInd = 0; gInd < G.size(); gInd++){
        temp_begin:
        if(G[gInd].gateTy == '\0') continue;
        if(G[gInd].gateTy == 'c'){
            unsigned gOff = 1;
            unsigned gCount = 0; 
            while(((gOff + gInd) < G.size()) && gCount < OPT_THRESHOLD){
               if(G[gInd+gOff].gateTy == 'c' && G[gInd+gOff].target == G[gInd].target && G[gInd+gOff].control1 == G[gInd].control1){
                  unsigned newPos =  canSwap(G, gInd, gInd + gOff);
                  if(newPos != UINT_MAX) {
                      erase_OpGate(G[gInd]);
                      erase_OpGate(G[gInd+gOff]);
                      break;
                  }
               }       
               if(G[gInd+gOff].gateTy == 'c' && G[gInd+gOff].target == G[gInd].control1 && G[gInd+gOff].control1 == G[gInd].target){

                  unsigned newPos =  canSwap(G, gInd, gInd + gOff);
                  if(newPos == UINT_MAX) {break;}
                  if(newPos != gInd){
                      OpGate temp = G[gInd];
                      for(unsigned i =  gInd; i < newPos; i++){
                          G[i] = G[i+1];
                      }
                      G[newPos] = temp;
                      goto temp_begin;
                  }
                  else{
                      erase_OpGate(G[gInd]);
                      unsigned tempUn = G[gInd + gOff].target;
                      G[gInd + gOff].target = G[gInd + gOff].control1;
                      G[gInd + gOff].control1 = tempUn;
                      tempUn = M[G[gInd].target];
                      M[G[gInd].target] = M[G[gInd].control1];
                      M[G[gInd].control1] = tempUn;
                      break;
                  }
                  
               }
               gOff++;
               if(shareBit(G[gInd], G[gInd + gOff])) {gCount++;}
            }
        }
        if(G[gInd].gateTy != '\0'){
            G[gInd].control1 = M[G[gInd].control1];
            G[gInd].target = M[G[gInd].target];
        }
        
    }

}

// run - Find datapaths for qubits
bool Optimize::runOnModule(Module &M) {

  const char *debug_val = getenv("DEBUG_OPTIMIZE");
  if(debug_val){
    if(!strncmp(debug_val, "1", 1)) debugOptimize = true;
    else debugOptimize = false;
  }

  debug_val = getenv("DEBUG_SCAFFOLD");
  if(debug_val && !debugOptimize){
    if(!strncmp(debug_val, "1", 1)) debugOptimize = true;
    else debugOptimize = false;
  }

  vector<Function*> qFuncs;

  CallGraph cg = CallGraph(M);

  CallGraphNode *rootNode = nullptr;

  for(auto it = cg.begin();it != cg.end();it++){
    if(!(it->second->getFunction())) continue;
    if(it->second->getFunction()->getName() == "main"){
      rootNode = &(*it->second);
      break;
    }
  }
  unsigned sccNum = 0;

//  for (scc_iterator<CallGraphNode*> sccIb = scc_begin(rootNode),
//         E = scc_end(rootNode); sccIb != E; ++sccIb)
//    {
//      const std::vector<CallGraphNode*> &nextSCC = *sccIb;
//
//      if(debugOptimize)
//	errs() << "\nSCC #" << ++sccNum << " : ";      
//
//      for (std::vector<CallGraphNode*>::const_iterator nsccI = nextSCC.begin(),
//	     E = nextSCC.end(); nsccI != E; ++nsccI)
//	{
//	  Function *F=(*nsccI)->getFunction();	  
//	  
//	  if(F && !F->isDeclaration()){
//        if(DetermineQFunc(F)){
//            qFuncs.push_back(F);
//        }
//      }
//    }
//  }
//
//  bool hasMain = false;
//  for( vector<Function*>::iterator it = qFuncs.begin(); it!=qFuncs.end(); it++)
//  {
//    if ((*it)->getName() == "main") hasMain = true;
//  }
//  if(!hasMain){
//    vector<Function*>::iterator it = qFuncs.end();
//    it--;
//    (*it)->setName("main");
//  }
   
  CallGraphNode *rootNode1 = nullptr;

  for(auto it = cg.begin();it != cg.end();it++){
    if(!(it->second->getFunction())) continue;
    if(it->second->getFunction()->getName() == "main"){
      rootNode1 = &(*it->second);
      break;
    }
  }

  sccNum = 0;

  for (scc_iterator<CallGraphNode*> sccIb = scc_begin(rootNode1),
         E = scc_end(rootNode1); sccIb != E; ++sccIb)
    {
      const std::vector<CallGraphNode*> &nextSCC = *sccIb;

      if(debugOptimize)
	errs() << "\nSCC #" << ++sccNum << " : ";      
      
      for (std::vector<CallGraphNode*>::const_iterator nsccI = nextSCC.begin(),
	     E = nextSCC.end(); nsccI != E; ++nsccI)
	{
	  Function *F=(*nsccI)->getFunction();	  
	  
	  if(F && !F->isDeclaration()){
	    if(debugOptimize)
	    errs() << "Processing Function:" << F->getName() <<" \n ";

	    //initialize map structures for this function
	    //vector<qGateArg> myQIFVec, myQIFVec1, myQIFVec2;
	    qbitsInFunc.clear();
	    qbitsInitInFunc.clear();
	    funcArgList.clear();

	    //std::vector<FnCall> myFuncMapVec;
	    mapFunction.clear();

	    getFunctionArguments(F);

	    //visit Alloc Insts in func and find if function is quantum or classical function

	    for(inst_iterator instIb = inst_begin(F),instIe=inst_end(F); instIb!=instIe;++instIb){

	      Instruction *pInst = &*instIb; // Grab pointer to instruction reference	      
          
	      if(debugOptimize)
		errs() << "\n Processing Inst: "<<*pInst << "\n";

	      analyzeAllocInst(F,pInst);
	    }

	    //map<Function*, vector<qGateArg> >::iterator mpItr = qbitsInFunc.find(F);
	    if(qbitsInFunc.size()>0){ //Is Quantum Function
          mapQbitsInit.insert( make_pair( F, qbitsInitInFunc ) );
          mapFuncArgs.insert( make_pair( F, funcArgList ) );
          qFuncs.push_back(F);
	    
	      for(inst_iterator instIb = inst_begin(F),instIe=inst_end(F); instIb!=instIe;++instIb){

		Instruction *pInst = &*instIb; // Grab pointer to instruction reference	      
		allDepQbit.clear();
		
		if(debugOptimize)
		  errs() << "\n Processing Inst: "<<*pInst << "\n";
		
		analyzeInst(F,pInst); //spatil: need a bool return type?

	      }
          mapMapFunc.insert( make_pair( F, mapFunction ) );
	    }
	  }
	  else{
	    if(debugOptimize)
	      errs() << "WARNING: Ignoring external node or dummy function.";
	  }
	  
	}
      if (nextSCC.size() == 1 && sccIb.hasLoop())
	errs() << " (Has self-loop).";
    }

    bool hasMain = false;
    for( vector<Function*>::iterator it = qFuncs.begin(); it!=qFuncs.end(); it++)
    {
      if ((*it)->getName() == "main") hasMain = true;
    }

    vector<Function*>::iterator lastItPos;
    if(!hasMain){
        lastItPos = qFuncs.end();
        lastItPos--;
    }

    for(vector<Function*>::iterator it = qFuncs.begin(); it != qFuncs.end(); it++){
        if((*it)->getName() == "main")   //assuming after flattening, the modules left are Toffoli and main, only care about main.
        {
            mapFunction = mapMapFunc.find(*it)->second;
            //Convert llvm internal data structure to OpGate for cleaner qubit argument(of gate) presentation
            //and easier qubit map later
            optimal_initial((*it), bitMap, gateList);
            
            unsigned gNumBefore = 0;
            unsigned gNumAfter = gateList.size();
            while(gNumBefore != gNumAfter){
                gNumBefore = gNumAfter;
                optimal(gateList, indMap);
                gNumAfter = countGate(gateList);
                //errs()<<"Before:"<<gNumBefore<<" After:"<<gNumAfter<<"\n";
            }
            for(unsigned i = 1; i < bitMap.size();i++){
                errs()<<"qubit "<<bitMap[i]<<"\n"; 
            }
            /*
            for(unsigned i = 1; i < gateList.size();i++){
                if(gateList[i].gateTy != '\0')
                    errs()<<gateList[i].gateTy<<" "<<gateList[i].target<<"\n";
            }*/
            for(unsigned i = 0; i < gateList.size(); i++){
                if(gateList[i].gateTy == '\0'){continue;}
                if(gateList[i].gateTy == 'h'){errs()<<"H ";}
                if(gateList[i].gateTy == 'c'){errs()<<"CNOT ";}
                if(gateList[i].gateTy == 'o'){errs()<<"Tof ";}
                if(gateList[i].gateTy == 'X'){errs()<<"MeasX ";}
                if(gateList[i].gateTy == 'Z'){errs()<<"MeasZ ";}
                if(gateList[i].gateTy == 'n'){errs()<<"PrepX ";}
                if(gateList[i].gateTy == 'm'){errs()<<"PrepZ ";}
                if(gateList[i].gateTy == 'S'){errs()<<"Sdag ";}
                if(gateList[i].gateTy == 's'){errs()<<"S ";}
                if(gateList[i].gateTy == 't'){errs()<<"T ";}
                if(gateList[i].gateTy == 'T'){errs()<<"Tdag ";}
                if(gateList[i].gateTy == 'r'){errs()<<"Rz "; }
                if(gateList[i].gateTy == 'x'){ errs()<<"X ";}
                if(gateList[i].gateTy == 'z'){ errs()<<"Z ";}

                if(gateList[i].control2 > 0)
                    {errs()<<bitMap[gateList[i].control2]<<",";
                    }
                if(gateList[i].control1 > 0){
                    errs()<<bitMap[gateList[i].control1]<<",";
                }
                if(gateList[i].target > 0)
                    {errs()<<bitMap[gateList[i].target];} 

                errs()<<"\n";

            }
            /*
            for(unsigned mIndex=0;mIndex<mapFunction.size();mIndex++){
                errs()<<mapFunction[mIndex].func->getName()<<" ";  

                for(vector<qGateArg>::iterator vpIt=mapFunction[mIndex].qArgs.begin(), vpItE=mapFunction[mIndex].qArgs.end();vpIt!=vpItE;++vpIt){
                   qbitsInitInFunc = mapQbitsInit.find((*it))->second;
                   errs()<<(*vpIt).argPtr->getName();
                   for(int ndim = 0; ndim < (*vpIt).numDim; ndim++)
                       errs()<<"["<<(*vpIt).dimSize[ndim]<<"]\n";
                   errs()<<" ";
                }
                errs()<<"\n";
            } 
            */
        }
        /*
        if(it == lastItPos) printFuncHeader((*it), true);
        else printFuncHeader((*it), false);
        genGateArray((*it));
        mapFunction = mapMapFunc.find(*it)->second;
        for(unsigned mIndex=0;mIndex<mapFunction.size();mIndex++){
          if(mapFunction[mIndex].qArgs.size()>0){
            errs()<<"func name: "<<mapFunction[mIndex].func->getName()<<" ";
    
          for(vector<qGateArg>::iterator vpIt=mapFunction[mIndex].qArgs.begin(), vpItE=mapFunction[mIndex].qArgs.end();vpIt!=vpItE;++vpIt){
            errs()<<(*vpIt).argPtr->getName()<<" ";
          }
            errs()<<"\n";
          }
        }
        */
    
    }

  return false;
}
