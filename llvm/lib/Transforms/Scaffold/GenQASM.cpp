//===- GenQASM.cpp - Generate qasm output -------------------===//
//
//                     The LLVM Scaffold Compiler Infrastructure
//
// This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#include <sstream>
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

bool debugGenQASM = false;

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

  struct GenQASM : public ModulePass {
    static char ID;  // Pass identification, replacement for typeid
    std::vector<Value*> vectQbit;

    std::vector<qGateArg> tmpDepQbit;
    std::vector<qGateArg> allDepQbit;

    vector<qGateArg> qbitsInFunc; //qbits in function
    vector<qGateArg> qbitsInitInFunc; //new qbits declared in function
    vector<qGateArg> funcArgList; //function arguments
    vector<FnCall> mapFunction; //trace sequence of qgate calls
    map<Value*, qGateArg> mapInstRtn;    //traces return cbits for Meas Inst

    int btCount; //backtrace count


    GenQASM() : ModulePass(ID) {  }

    bool getQbitArrDim(Type* instType, qGateArg* qa);
    bool backtraceOperand(Value* opd, int opOrIndex);
    void analyzeAllocInst(Function* F,Instruction* pinst);
    void analyzeCallInst(Function* F,Instruction* pinst);
    void analyzeInst(Function* F,Instruction* pinst);

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

char GenQASM::ID = 0;
static RegisterPass<GenQASM>
X("gen-qasm", "Generate QASM output code"); //spatil: should be Z or X??

bool GenQASM::backtraceOperand(Value* opd, int opOrIndex)
{

  if(opOrIndex == 0) //backtrace for operand
    {
      //search for opd in qbit/cbit vector
      std::vector<Value*>::iterator vIter=std::find(vectQbit.begin(),vectQbit.end(),opd);
      if(vIter != vectQbit.end()){
	if(debugGenQASM)
	  errs()<<"Found qubit associated: "<< opd->getName() << "\n";
	
	tmpDepQbit[0].argPtr = opd;
	
	return true;
      }
      
      if(btCount>MAX_BT_COUNT)
	return false;
      
      if(GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(opd))
	{
	  if(debugGenQASM)
	  {
	      errs() << "Get Elem Ptr Inst Found: " << *GEPI <<"\n";
	      errs() << GEPI->getPointerOperand()->getName();
	      errs() << " has index = " << GEPI->hasIndices();
	      errs() << " has all constant index = " << GEPI->hasAllConstantIndices() << "\n";
	  }

	  if(GEPI->hasAllConstantIndices()){
	    Instruction* pInst = dyn_cast<Instruction>(opd);
	    unsigned numOps = pInst->getNumOperands();
	    if(debugGenQASM)
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
		if(debugGenQASM)
		  errs()<<" Found constant index = "<<CI->getValue()<<"\n";
	      }
	    }

	    //NOTE: getelemptr instruction can have multiple indices. Currently considering last operand as desired index for qubit. Check this reasoning. 
	    ConstantInt *CI = dyn_cast<ConstantInt>(pInst->getOperand(numOps-1));
	    if(tmpDepQbit.size()==1){
	      tmpDepQbit[0].valOrIndex = CI->getZExtValue();
	      if(debugGenQASM)
		errs()<<" Found constant index = "<<CI->getValue()<<"\n";
	    }
	    return foundOne;
	  }
	  
	  else if(GEPI->hasIndices()){ //NOTE: Edit this function for multiple indices, some of which are constant, others are not.
	  
	    errs() << "Oh no! I don't know how to handle this case..ABORT ABORT..\n";
	    Instruction* pInst = dyn_cast<Instruction>(opd);
	    unsigned numOps = pInst->getNumOperands();
	    if(debugGenQASM)
	      errs() << " Has non-constant index. Num Operands: " << numOps << ": ";		
	    bool foundOne = backtraceOperand(pInst->getOperand(0),0);

	    if(tmpDepQbit[0].isQbit && !(tmpDepQbit[0].isPtr)){     
	      //NOTE: getelemptr instruction can have multiple indices. consider last operand as desired index for qubit. Check if this is true for all.
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
	  if(debugGenQASM)
	    errs()<<" Added default dim to qbit & not ptr variable.\n";
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
	if(debugGenQASM)
	  errs() << "Ending Recursion\n";
	return false;
      }
    }
  else if(opOrIndex == 0){ //opOrIndex == 1; i.e. Backtracing for Index    
    if(btCount>MAX_BT_COUNT) //prevent infinite backtracing
      return true;

    if(ConstantInt *CI = dyn_cast<ConstantInt>(opd)){
      tmpDepQbit[0].valOrIndex = CI->getZExtValue();
      if(debugGenQASM)
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
    if(debugGenQASM)
      errs()<<"backtracing for call inst: "<<*opd<<"\n";
    if(CallInst *endCI = dyn_cast<CallInst>(opd)){
      if(endCI->getCalledFunction()->getName().find("llvm.Meas") != string::npos){
	tmpDepQbit[0].argPtr = opd;

	if(debugGenQASM)
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

bool GenQASM::getQbitArrDim(Type *instType, qGateArg* qa)
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
    else if (elementType->isArrayTy()){
      myRet |= getQbitArrDim(elementType,qa);
    }
    else myRet = false;
  }

  return myRet;

}


void GenQASM::analyzeAllocInst(Function* F, Instruction* pInst){
  if (AllocaInst *AI = dyn_cast<AllocaInst>(pInst)) {
    Type *allocatedType = AI->getAllocatedType();
    
    if(ArrayType *arrayType = dyn_cast<ArrayType>(allocatedType)) {      
      qGateArg tmpQArg;

      Type *elementType = arrayType->getElementType();
      uint64_t arraySize = arrayType->getNumElements();
      if (elementType->isIntegerTy(16)){
	if(debugGenQASM)
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
	if(debugGenQASM)
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

	    if(debugGenQASM)
	      print_qgateArg(tmpQArg);
	  }	  
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
      if(debugGenQASM)
	errs() << "\tIs a Pointer of Type: " << *elementType << "\n";
      
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
    }
    return;
  }

}

void GenQASM::analyzeCallInst(Function* F, Instruction* pInst){
  if(CallInst *CI = dyn_cast<CallInst>(pInst))
    {
      if(debugGenQASM)      
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
	
	if(debugGenQASM)
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
	}
	else if(argType->isIntegerTy(16)){
	  tmpQGateArg.isQbit = true;
	  tmpQGateArg.valOrIndex = 0;	 
	}	  	
	else if(argType->isIntegerTy(1)){
	  tmpQGateArg.isCbit = true;
	  tmpQGateArg.valOrIndex = 0;	 
	}	  	
	
	//check if argument is constant int	
	if(ConstantInt *CInt = dyn_cast<ConstantInt>(CI->getArgOperand(iop))){
	  tmpQGateArg.valOrIndex = CInt->getZExtValue();
	  if(debugGenQASM){
	    errs()<<" Found constant argument = "<<CInt->getValue()<<"\n";
	  }
	}
	

	//check if argument is constant float	
	if(ConstantFP *CFP = dyn_cast<ConstantFP>(CI->getArgOperand(iop))){
	  tmpQGateArg.val = CFP->getValueAPF().convertToDouble();
	  tmpQGateArg.isDouble = true;
	  if(debugGenQASM){
	    errs()<<" Call Inst = "<<*CI<<"\n";
	    errs()<<" Found constant double argument = "<<tmpQGateArg.val<<"\n";
	  }
	}


	tmpDepQbit.push_back(tmpQGateArg);
	
	tracked_all_operands &= backtraceOperand(CI->getArgOperand(iop),0);
	
	if(tmpDepQbit.size()>0){
	  if(debugGenQASM)
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
	if(debugGenQASM)
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


void GenQASM::analyzeInst(Function* F, Instruction* pInst){
  if(debugGenQASM)
    errs() << "--Processing Inst: "<<*pInst << '\n';

  //analyzeAllocInst(F,pInst);
  analyzeCallInst(F,pInst);
    
  if(debugGenQASM)
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

void GenQASM::printFuncHeader(Function* F)
{

  //map<Function*, vector<qGateArg> >::iterator mpItr;
  //map<Function*, vector<qGateArg> >::iterator mpItr2;
  //map<Function*, vector<qGateArg> >::iterator mvpItr;

  //mpItr = qbitsInFunc.find(F);

  //print name of function
  errs()<<"\nmodule "<<F->getName();
  
  //print arguments of function
  //mpItr2=funcArgList.find(F);    
  errs()<<" ( ";    
  unsigned tmp_num_elem = funcArgList.size();
  
  if(tmp_num_elem > 0){
    for(unsigned tmp_i=0;tmp_i<tmp_num_elem - 1;tmp_i++){
      
      qGateArg tmpQA = funcArgList[tmp_i];
      
      if(debugGenQASM)
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
    
    if(debugGenQASM)
      print_qgateArg(funcArgList[tmp_num_elem-1]);
    
    
    if((funcArgList[tmp_num_elem-1]).isQbit)
      errs()<<"qbit";
    else if((funcArgList[tmp_num_elem-1]).isCbit)
      errs()<<"cbit";
    else{
      Type* argTy = (funcArgList[tmp_num_elem-1]).argPtr->getType();
      if(argTy->isDoubleTy()) errs() << "double";
      else if(argTy->isFloatTy()) errs() << "float";
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

      //if only single-dimensional qbit arrays expected
      //errs()<<"["<<(*vvit).valOrIndex<<"];\n ";

      //if n-dimensional qbit arrays expected 
      for(int ndim = 0; ndim < (*vvit).numDim; ndim++)
	errs()<<"["<<(*vvit).dimSize[ndim]<<"]";
      errs() << ";\n";
    }
  //errs() << "//--//-- Fn: " << F->getName() << " --//--//\n";
}
  
void GenQASM::genQASM(Function* F)
{
  //map<Function*, vector<qGateArg> >::iterator mpItr;
  //map<Function*, vector<qGateArg> >::iterator mpItr2;
  //map<Function*, vector<qGateArg> >::iterator mvpItr;
  
  //mpItr = qbitsInFunc.find(F);
  if(qbitsInFunc.size()>0){
    
    
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


	errs()<<fToPrint<<" ( ";

	//print all but last argument
	for(vector<qGateArg>::iterator vpIt=mapFunction[mIndex].qArgs.begin(), vpItE=mapFunction[mIndex].qArgs.end();vpIt!=vpItE-1;++vpIt)
	  {
	    if((*vpIt).isUndef)
	      errs() << " UNDEF ";
	    else{
	      if((*vpIt).isQbit || (*vpIt).isCbit){
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
	  if(tmpQA.isQbit || tmpQA.isCbit){
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
  }
}


void GenQASM::getFunctionArguments(Function* F)
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
	if(debugGenQASM)
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
      }
      else if (argType->isIntegerTy(16)){
	tmpQArg.isQbit = true;
	vectQbit.push_back(ait);
	qbitsInFunc.push_back(tmpQArg);
	funcArgList.push_back(tmpQArg);
      }
      else if (argType->isIntegerTy(1)){
	tmpQArg.isCbit = true;
	vectQbit.push_back(ait);
	qbitsInFunc.push_back(tmpQArg);
	funcArgList.push_back(tmpQArg);
      }
      else if(argType->isDoubleTy())     
	funcArgList.push_back(tmpQArg);

      if(debugGenQASM)
	print_qgateArg(tmpQArg);
    }
}

// run - Find datapaths for qubits
bool GenQASM::runOnModule(Module &M) {
  CallGraphNode* rootNode = getAnalysis<CallGraph>().getRoot();
  unsigned sccNum = 0;

  errs() << "-------QASM Generation Pass:\n";

  for (scc_iterator<CallGraphNode*> sccIb = scc_begin(rootNode),
         E = scc_end(rootNode); sccIb != E; ++sccIb)
    {
      const std::vector<CallGraphNode*> &nextSCC = *sccIb;

      if(debugGenQASM)
	errs() << "\nSCC #" << ++sccNum << " : ";      

      for (std::vector<CallGraphNode*>::const_iterator nsccI = nextSCC.begin(),
	     E = nextSCC.end(); nsccI != E; ++nsccI)
	{
	  Function *F=(*nsccI)->getFunction();	  
	  
	  if(F && !F->isDeclaration()){
	    if(debugGenQASM)
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

	      if(debugGenQASM)
		errs() << "\n Processing Inst: "<<*pInst << "\n";

	      analyzeAllocInst(F,pInst);
	    }

	    //map<Function*, vector<qGateArg> >::iterator mpItr = qbitsInFunc.find(F);
	    if(qbitsInFunc.size()>0){ //Is Quantum Function
	      printFuncHeader(F);
	    
	      for(inst_iterator instIb = inst_begin(F),instIe=inst_end(F); instIb!=instIe;++instIb){

		Instruction *pInst = &*instIb; // Grab pointer to instruction reference	      
		allDepQbit.clear();
		
		if(debugGenQASM)
		  errs() << "\n Processing Inst: "<<*pInst << "\n";
		
		analyzeInst(F,pInst); //spatil: need a bool return type?
	      }
	      

	      genQASM(F);	      

	    }
	    
	  }
	  else{
	    if(debugGenQASM)
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

