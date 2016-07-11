//===- GenRKQC.cpp - Generate qasm output -------------------===//
//
//                     The LLVM Scaffold Compiler Infrastructure
//
// This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

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
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace std;

cl::opt<string> FILE_NAME("filename", cl::init(""), cl::Hidden,
  cl::desc("Name of the operand file, for post processing"));

#define MAX_BT_COUNT 15 //max backtrace allowed - to avoid infinite recursive loops
#define MAX_QBIT_ARR_DIM 5 //max dimensions allowed for qbit arrays

bool debugGenrkqc = false;

namespace {

  struct qGateArg{ //arguments to qgate calls
    Value* argPtr;
    int argNum;
    bool isQbit;
    bool isCbit;
    bool isUndef;
    bool isPtr;
    bool isDouble;
    bool isInt;
    int numDim; //number of dimensions of qbit array
    int dimSize[MAX_QBIT_ARR_DIM]; //sizes of dimensions of array for qbit declarations OR indices of specific qbit for gate arguments
    int valOrIndex; //Value if not Qbit, Index if Qbit & not a Ptr
    double val;
    //Note: valOrIndex is of type integer. Assumes that quantities will be int in the program.
    qGateArg(): argPtr(NULL), argNum(-1), isQbit(false), isCbit(false), isUndef(false), isPtr(false), isDouble(false), isInt(false), numDim(0), valOrIndex(-1), val(0.0){ }
  };
  
  struct FnCall{ //datapath sequence
    Function* func;
    Value* instPtr;
    std::vector<qGateArg> qArgs;
  };    

  struct GenRKQC : public ModulePass {
    static char ID;  // Pass identification, replacement for typeid
    std::vector<Value*> vectQbit;

    bool thisFuncIsIntrinsic;
    bool circuitHasRKQC;

    string lastFuncName;

    vector<Function*> isRKQC;
    vector<string> isRKQCNames;
    vector<Instruction*> vectCalls;
    vector<string> funcArgs;

    map<Instruction*, vector<qGateArg> > mapCallsArgs;
    map<Function* , vector<qGateArg> > funcCallsArgs;
    map<pair<Function*, vector<string> >, vector<Instruction*> > RkqcFunctions;
    map<Function* , vector<string> > qbitDecls;
    map<Function*, vector<Instruction*> > mapFuncsInsts; 
    map<Function*, vector<qGateArg> > mapQbitsInitInFunc;

    std::vector<qGateArg> tmpDepQbit;
    std::vector<qGateArg> allDepQbit;

    vector<qGateArg> qbitsInFunc; //qbits in function
    vector<qGateArg> qbitsInitInFunc; //new qbits declared in function
    vector<qGateArg> funcArgList; //function arguments
    vector<FnCall> mapFunction; //trace sequence of qgate calls
    map<Value*, qGateArg> mapInstRtn;    //traces return cbits for Meas Inst

    int btCount; //backtrace count


    GenRKQC() : ModulePass(ID) {  }

    bool getQbitArrDim(Type* instType, qGateArg* qa);
    bool backtraceOperand(Value* opd, int opOrIndex);
    void analyzeAllocInst(Function* F,Instruction* pinst);
    void analyzeCallInst(Function* F,Instruction* pinst);
    void analyzeInst(Function* F,Instruction* pinst);


    bool analyzeRKQC(Function* F);
    bool analyzeIntrinsicCallInstRKQC(Instruction* F);
    bool checkIfIntrinsicRKQC(Function* F);
    bool checkRKQCCalls(Instruction* F);


    // run - Print out SCCs in the call graph for the specified module.
    bool runOnModule(Module &M);

    void printFuncHeader(Function* F, bool funcIsRKQC, ofstream& main_out);
    void printFuncHeaderToFile(Function* F, ofstream& ss, bool lastFunc);

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

    void genRKQC(Function* F, ofstream& main_out);
    void mergeQASM();
    void mergeLLVMIR();
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

char GenRKQC::ID = 0;
static RegisterPass<GenRKQC>
X("GenRKQC", "Generate RKQC output code"); 

bool GenRKQC::backtraceOperand(Value* opd, int opOrIndex)
{

  if(opOrIndex == 0) //backtrace for operand
    {
      //search for opd in qbit/cbit vector
      std::vector<Value*>::iterator vIter=std::find(vectQbit.begin(),vectQbit.end(),opd);
      if(vIter != vectQbit.end()){
	if(debugGenrkqc)
	  errs()<<"Found qubit associated: "<< opd->getName() << "\n";
	
	tmpDepQbit[0].argPtr = opd;
	
	return true;
      }
      
      if(btCount>MAX_BT_COUNT)
	return false;
      
      if(GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(opd))
	{
	  if(debugGenrkqc)
	  {
	      errs() << "Get Elem Ptr Inst Found: " << *GEPI <<"\n";
	      errs() << GEPI->getPointerOperand()->getName();
	      errs() << " has index = " << GEPI->hasIndices();
	      errs() << " has all constant index = " << GEPI->hasAllConstantIndices() << "\n";
	  }

	  if(GEPI->hasAllConstantIndices()){
	    Instruction* pInst = dyn_cast<Instruction>(opd);
	    unsigned numOps = pInst->getNumOperands();
	    if(debugGenrkqc)
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
		if(debugGenrkqc)
		  errs()<<" Found constant index = "<<CI->getValue()<<"\n";
	      }
	    }

	    //NOTE: getelemptr instruction can have multiple indices. Currently considering last operand as desired index for qubit. Check this reasoning. 
	    ConstantInt *CI = dyn_cast<ConstantInt>(pInst->getOperand(numOps-1));
	    if(tmpDepQbit.size()==1){
	      tmpDepQbit[0].valOrIndex = CI->getZExtValue();
	      if(debugGenrkqc)
		errs()<<" Found constant index = "<<CI->getValue()<<"\n";
	    }
	    return foundOne;
	  }
	  
	  else if(GEPI->hasIndices()){ //NOTE: Edit this function for multiple indices, some of which are constant, others are not.
	  
	    errs() << "Oh no! I don't know how to handle this case..ABORT ABORT..\n";
	    Instruction* pInst = dyn_cast<Instruction>(opd);
	    unsigned numOps = pInst->getNumOperands();
	    if(debugGenrkqc)
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
	  if(debugGenrkqc)
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
	if(debugGenrkqc)
	  errs() << "Ending Recursion\n";
	return false;
      }
    }
  else if(opOrIndex == 0){ //opOrIndex == 1; i.e. Backtracing for Index    
    if(btCount>MAX_BT_COUNT) //prevent infinite backtracing
      return true;

    if(ConstantInt *CI = dyn_cast<ConstantInt>(opd)){
      tmpDepQbit[0].valOrIndex = CI->getZExtValue();
      if(debugGenrkqc)
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
    if(debugGenrkqc)
      errs()<<"backtracing for call inst: "<<*opd<<"\n";
    if(CallInst *endCI = dyn_cast<CallInst>(opd)){
      if(endCI->getCalledFunction()->getName().find("llvm.Meas") != string::npos){
	tmpDepQbit[0].argPtr = opd;

	if(debugGenrkqc)
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

bool GenRKQC::getQbitArrDim(Type *instType, qGateArg* qa)
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


void GenRKQC::analyzeAllocInst(Function* F, Instruction* pInst){
    if (AllocaInst *AI = dyn_cast<AllocaInst>(pInst)) {
        Type *allocatedType = AI->getAllocatedType();
        if (allocatedType->isIntegerTy(32)){
            string intName = AI->getName();
            if (debugGenrkqc) errs() << "New Int Allocation Found: " << intName << "\n";
            if (intName.find(".addr") == string::npos){
                qbitDecls.find(F)->second.push_back( "int " + intName );
            }
        }
        else if(ArrayType *arrayType = dyn_cast<ArrayType>(allocatedType)) {      
            qGateArg tmpQArg;

            Type *elementType = arrayType->getElementType();
            uint64_t arraySize = arrayType->getNumElements();
            if (elementType->isIntegerTy(16)){
                if(debugGenrkqc) errs() << "New QBit Allocation Found: " << AI->getName() <<"\n";
                string qbitName = AI->getName();
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
                stringstream ss;
                ss << arraySize;
                qbitDecls.find(F)->second.push_back( "qbit " + qbitName + "(" + ss.str() + ")");
            }
              
            else if (elementType->isIntegerTy(1)){
                if(debugGenrkqc) errs() << "New CBit Allocation Found: " << AI->getName() <<"\n";
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
                if(debugGenrkqc) errs() << "Multidimensional array\n";

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
                  if(debugGenrkqc) print_qgateArg(tmpQArg);
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
            if(debugGenrkqc) errs() << "\tIs a Pointer of Type: " << *elementType << "\n";
            
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
              }
        }
        return;
    }
    mapQbitsInitInFunc.insert( make_pair ( F, qbitsInitInFunc ) );
    qbitsInitInFunc.clear();
}

void GenRKQC::analyzeCallInst(Function* F, Instruction* pInst){
  if(CallInst *CI = dyn_cast<CallInst>(pInst))
    {
      if(debugGenrkqc)      
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
	
	if(debugGenrkqc)
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
      tmpQGateArg.isInt = true;
	  if(debugGenrkqc){
	    errs()<<" Found constant argument = "<<CInt->getValue()<<"\n";
	  }
	}
	

	//check if argument is constant float	
	if(ConstantFP *CFP = dyn_cast<ConstantFP>(CI->getArgOperand(iop))){
	  tmpQGateArg.val = CFP->getValueAPF().convertToDouble();
	  tmpQGateArg.isDouble = true;
	  if(debugGenrkqc){
	    errs()<<" Call Inst = "<<*CI<<"\n";
	    errs()<<" Found constant double argument = "<<tmpQGateArg.val<<"\n";
	  }
	}


	tmpDepQbit.push_back(tmpQGateArg);
	
	tracked_all_operands &= backtraceOperand(CI->getArgOperand(iop),0);
	
	if(tmpDepQbit.size()>0){
	  if(debugGenrkqc)
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
        vector<qGateArg> instArgs;
        for(unsigned int it = 0; it < allDepQbit.size(); it++){
            instArgs.push_back(allDepQbit[it]);
//            if(allDepQbit[it].argPtr) instArgs.push_back(allDepQbit[it].argPtr->getName()) ;
//            else{
//                stringstream ss;
//                ss << allDepQbit[it].valOrIndex;
//                instArgs.push_back(ss.str());
//            }
        }

        mapCallsArgs.insert( make_pair( pInst, instArgs ) );

	    if(debugGenrkqc)
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


void GenRKQC::analyzeInst(Function* F, Instruction* pInst){
  if(debugGenrkqc)
    errs() << "--Processing Inst: "<<*pInst << '\n';

  //analyzeAllocInst(F,pInst);
  analyzeCallInst(F,pInst);
    
  if(debugGenrkqc)
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
void GenRKQC::printFuncHeaderToFile(Function* F, ofstream& ss, bool lastFunc)
{
  //map<Function*, vector<qGateArg> >::iterator mpItr;
  //map<Function*, vector<qGateArg> >::iterator mpItr2;
  //map<Function*, vector<qGateArg> >::iterator mvpItr;

  //mpItr = qbitsInFunc.find(F);

  //print name of function

  string funcName = F->getName();
  if (lastFunc) {
    lastFuncName = funcName;
    ss << "\nint main () {\n"; 
    ss << "  set_LLVM(true);\n";
  }

  else {
    ss <<"\nvoid "<< funcName;
  
    //print arguments of function
    //mpItr2=funcArgList.find(F);    
    ss <<" ( ";    
    funcArgList = funcCallsArgs.find(F)->second;
    unsigned tmp_num_elem = funcArgList.size();
    
    if(tmp_num_elem > 0){
      for(unsigned tmp_i=0;tmp_i<tmp_num_elem - 1;tmp_i++){
        
        qGateArg tmpQA = funcArgList[tmp_i];
        
        if(debugGenrkqc)
      print_qgateArg(tmpQA);
        
        if(tmpQA.isQbit)
      ss <<"qint";
        else if(tmpQA.isCbit)
      ss <<"cbit";
        else{
      Type* argTy = tmpQA.argPtr->getType();
      if(argTy->isDoubleTy()) ss << "double";
      else if(argTy->isFloatTy()) ss << "float";
      else
        ss <<"UNRECOGNIZED "<<argTy<<" ";
        }
        
//        if(tmpQA.isPtr)
//      	  ss <<"*";	  
        
        ss <<" "<<printVarName(tmpQA.argPtr->getName())<<" , ";
      }
      
      if(debugGenrkqc)
        print_qgateArg(funcArgList[tmp_num_elem-1]);
      
      
      if((funcArgList[tmp_num_elem-1]).isQbit)
        ss <<"qint";
      else if((funcArgList[tmp_num_elem-1]).isCbit)
        ss <<"cbit";
      else{
        Type* argTy = (funcArgList[tmp_num_elem-1]).argPtr->getType();
        if(argTy->isDoubleTy()) ss  << "double";
        else if(argTy->isFloatTy()) ss  << "float";
        else
      ss <<"UNRECOGNIZED "<<argTy<<" ";
      }
      
//      if((funcArgList[tmp_num_elem-1]).isPtr)
//          ss <<"*";
      
      ss  <<" "<<printVarName((funcArgList[tmp_num_elem-1]).argPtr->getName());
      
    }
    
    ss <<" ) {\n ";   
  }
  // -- Below Required to Transform Function Parameters to Declarations -- //
  /*else {
    lastFuncName = funcName;
    ss << "\nint main () {\n"; 
    ss << "  set_LLVM(true);\n";

    funcArgList = funcCallsArgs.find(F)->second;
    unsigned tmp_num_elem = funcArgList.size();
    
    if(tmp_num_elem > 0){
      for(unsigned tmp_i=0;tmp_i<tmp_num_elem - 1;tmp_i++){
        
        qGateArg tmpQA = funcArgList[tmp_i];
        
        if(debugGenrkqc)
      print_qgateArg(tmpQA);
        
        if(tmpQA.isQbit)
      ss <<"  qbit";
        else if(tmpQA.isCbit)
      ss <<"  cbit";
        else{
      Type* argTy = tmpQA.argPtr->getType();
      if(argTy->isDoubleTy()) ss << "double";
      else if(argTy->isFloatTy()) ss << "float";
      else
        ss <<"UNRECOGNIZED "<<argTy<<" ";
        }
        
//        if(tmpQA.isPtr)
//      	  ss <<"*";	  
        
        ss <<" "<<printVarName(tmpQA.argPtr->getName())<<" ;\n";
      }
      
      if(debugGenrkqc)
        print_qgateArg(funcArgList[tmp_num_elem-1]);
      
      
      if((funcArgList[tmp_num_elem-1]).isQbit)
        ss <<"  qbit";
      else if((funcArgList[tmp_num_elem-1]).isCbit)
        ss <<"  cbit";
      else{
        Type* argTy = (funcArgList[tmp_num_elem-1]).argPtr->getType();
        if(argTy->isDoubleTy()) ss  << "double";
        else if(argTy->isFloatTy()) ss  << "float";
        else
      ss <<"UNRECOGNIZED "<<argTy<<" ";
      }
      
//      if((funcArgList[tmp_num_elem-1]).isPtr)
//          ss <<"*";
      
      ss  <<" "<<printVarName((funcArgList[tmp_num_elem-1]).argPtr->getName()) << " ;\n";
      
    }
  
  
  }*/

    //print qbits declared in function
    //mvpItr=qbitsInitInFunc.find(F);	    
    vector<qGateArg> qbitsInit = mapQbitsInitInFunc.find(F)->second;
    for(vector<qGateArg>::iterator vvit=qbitsInit.begin(),vvitE=qbitsInit.end();vvit!=vvitE;++vvit)
      {	
        if((*vvit).isQbit)
      ss <<"  qbit "<<printVarName((*vvit).argPtr->getName());
        if((*vvit).isCbit)
      ss <<"  cbit "<<printVarName((*vvit).argPtr->getName());

        //if only single-dimensional qbit arrays expected
        //ss <<"["<<(*vvit).valOrIndex<<"];\n ";

        //if n-dimensional qbit arrays expected 
        for(int ndim = 0; ndim < (*vvit).numDim; ndim++)
      ss <<"("<<(*vvit).dimSize[ndim]<<")";
        ss  << ";\n";
      }
   
    //ss  << "//--//-- Fn: " << F->getName() << " --//--//\n";
}
void GenRKQC::printFuncHeader(Function* F, bool funcIsRKQC, ofstream& main_out)
{

  //map<Function*, vector<qGateArg> >::iterator mpItr;
  //map<Function*, vector<qGateArg> >::iterator mpItr2;
  //map<Function*, vector<qGateArg> >::iterator mvpItr;

  //mpItr = qbitsInFunc.find(F);
  if (funcIsRKQC) {
      main_out << "\nextern void " << F->getName().str() << " () {};\n";
  }
  else {
  //print name of function
      string funcName = F->getName();
      main_out <<"\nvoid "<< funcName;
      
      //print arguments of function
      //mpItr2=funcArgList.find(F);    
      main_out <<" ( ";    
      funcArgList = funcCallsArgs.find(F)->second;
      unsigned tmp_num_elem = funcArgList.size();
      
      if(tmp_num_elem > 0){
        for(unsigned tmp_i=0;tmp_i<tmp_num_elem - 1;tmp_i++){
          
          qGateArg tmpQA = funcArgList[tmp_i];
          
          if(debugGenrkqc)
    	print_qgateArg(tmpQA);
          
          if(tmpQA.isQbit)
    	main_out <<"qbit";
          else if(tmpQA.isCbit)
    	main_out <<"cbit";
          else{
    	Type* argTy = tmpQA.argPtr->getType();
    	if(argTy->isDoubleTy()) main_out << "double";
    	else if(argTy->isFloatTy()) main_out << "float";
    	else
    	  main_out <<"UNRECOGNIZED "<<argTy<<" ";
          }
          
          if(tmpQA.isPtr)
    	main_out <<"*";	  
          
          main_out <<" "<<printVarName(tmpQA.argPtr->getName())<<" , ";
        }
        
        if(debugGenrkqc)
          print_qgateArg(funcArgList[tmp_num_elem-1]);
        
        
        if((funcArgList[tmp_num_elem-1]).isQbit)
          main_out <<"qbit";
        else if((funcArgList[tmp_num_elem-1]).isCbit)
          main_out <<"cbit";
        else{
          Type* argTy = (funcArgList[tmp_num_elem-1]).argPtr->getType();
          if(argTy->isDoubleTy()) main_out  << "double";
          else if(argTy->isFloatTy()) main_out  << "float";
          else
    	main_out <<"UNRECOGNIZED "<<argTy<<" ";
        }
        
        if((funcArgList[tmp_num_elem-1]).isPtr)
          main_out <<"*";
        
        main_out  <<" "<<printVarName((funcArgList[tmp_num_elem-1]).argPtr->getName());
      }
      
      main_out <<" ) {\n ";   
      
      //print qbits declared in function
      //mvpItr=qbitsInitInFunc.find(F);	    
      for(vector<qGateArg>::iterator vvit=qbitsInitInFunc.begin(),vvitE=qbitsInitInFunc.end();vvit!=vvitE;++vvit)
        {	
          if((*vvit).isQbit)
    	main_out <<"  qbit "<<printVarName((*vvit).argPtr->getName());
          if((*vvit).isCbit)
    	main_out <<"  cbit "<<printVarName((*vvit).argPtr->getName());
    
          //if only single-dimensional qbit arrays expected
          //errs() <<"["<<(*vvit).valOrIndex<<"];\n ";
    
          //if n-dimensional qbit arrays expected 
          for(int ndim = 0; ndim < (*vvit).numDim; ndim++)
    	main_out <<"["<<(*vvit).dimSize[ndim]<<"]";
          main_out  << ";\n";
        }
      //errs()  << "//--//-- Fn: " << F->getName() << " --//--//\n";
    }
}
  
void GenRKQC::genRKQC(Function* F, ofstream& main_out)
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
	main_out<<"\t";

	//print return operand before printing MeasZ
	if(fToPrint.find("Meas") != string::npos){
	  //get inst ptr
	  Value* thisInstPtr = mapFunction[mIndex].instPtr;
	  //find inst in mapInstRtn
	  map<Value*, qGateArg>::iterator mvq = mapInstRtn.find(thisInstPtr);
	  if(mvq!=mapInstRtn.end()){
	    main_out<<printVarName(((*mvq).second).argPtr->getName());
	    if(((*mvq).second).isPtr)
	      main_out<<"["<<((*mvq).second).valOrIndex<<"]";
	    main_out <<" = ";
	  }	  
	}


	main_out<<fToPrint<<" ( ";

	//print all but last argument
	for(vector<qGateArg>::iterator vpIt=mapFunction[mIndex].qArgs.begin(), vpItE=mapFunction[mIndex].qArgs.end();vpIt!=vpItE-1;++vpIt)
	  {
	    if((*vpIt).isUndef)
	      main_out << " UNDEF ";
	    else{
	      if((*vpIt).isQbit || (*vpIt).isCbit){
		main_out<<printVarName((*vpIt).argPtr->getName());
		if(!((*vpIt).isPtr)){		  
		  //if only single-dimensional qbit arrays expected
		  //--if((*vpIt).numDim == 0)
		  //errs()<<"["<<(*vpIt).valOrIndex<<"]";
		  //--else
		    //if n-dimensional qbit arrays expected 
		    for(int ndim = 0; ndim < (*vpIt).numDim; ndim++)
		      main_out<<"["<<(*vpIt).dimSize[ndim]<<"]";
		}
	      }
	      else{
		//assert(!(*vpIt).isPtr); 
		if((*vpIt).isPtr) //NOTE: not expecting non-quantum pointer variables as arguments to quantum functions. If they exist, then print out name of variable
		  main_out << " UNRECOGNIZED ";
		else if((*vpIt).isDouble)
		  main_out << (*vpIt).val;
		else
		  main_out <<(*vpIt).valOrIndex;	      
	      }
	    }	    	    
	    main_out<<" , ";
	  }

	//print last element	
	qGateArg tmpQA = mapFunction[mIndex].qArgs.back();

	if(tmpQA.isUndef)
	  main_out << " UNDEF ";
	else{
	  if(tmpQA.isQbit || tmpQA.isCbit){
	    main_out<<printVarName(tmpQA.argPtr->getName());
	    if(!(tmpQA.isPtr)){
	      //if only single-dimensional qbit arrays expected
	      //--if(tmpQA.numDim == 0)
	      //main_out<<"["<<tmpQA.valOrIndex<<"]";
	      //--else
		//if n-dimensional qbit arrays expected 
		for(int ndim = 0; ndim < tmpQA.numDim; ndim++)
		  main_out<<"["<<tmpQA.dimSize[ndim]<<"]";	      	      	      
	    }
	  }
	  else{
	    //assert(!tmpQA.isPtr); //NOTE: not expecting non-quantum pointer variables as arguments to quantum functions. If they exist, then print out name of variable
	    if(tmpQA.isPtr)
	      main_out << " UNRECOGNIZED ";
	    else if(tmpQA.isDouble) 
	      main_out << tmpQA.val;
	    else
	      main_out<<tmpQA.valOrIndex;	    
	  }
	  
	}
	main_out<<" );\n ";	      
      }
    }

    //main_out << "//--//-- End Fn: " << F->getName() << " --//--// \n";
    main_out<<"}\n";
  }
}

void GenRKQC::mergeQASM()
{
    fstream rkqc_file;
    fstream main_file;
    fstream new_main_file;
    
    rkqc_file.open( "rkqc.qasm", fstream::in | fstream::out | fstream::app );
    if( !(rkqc_file.is_open() ) ){
        errs() << "Problem With RKQC File\n";
        exit(1);
    }
    main_file.open( "main.qasm", fstream::in | fstream::out );
    if( !(main_file.is_open() ) ){
        errs() << "Problem With Main QASM File\n";
        exit(1);
    }
    new_main_file.open( "new_main.qasm", fstream::out | fstream::in | fstream::trunc );
    if( !(new_main_file.is_open() ) ){
        errs() << "Problem With New Main QASM File\n";
        exit(1);
    }
    string line;
    string currentLine;
    int i;
    size_t position;
    int lineNo = 0;
    int toReplaceNo = 0;
    string toReplace = "extern void";
    string replacer = "module";
    size_t len = toReplace.length();
    while( getline(main_file, currentLine) ){
        lineNo++;
        position = currentLine.find("extern void");
        if (position != string::npos){
            toReplaceNo = lineNo;
            line = currentLine;
            if(debugGenrkqc) errs() << "Found New RKQC Function\n";
        }
        new_main_file << currentLine << endl;
    }
    line.replace( position, len, replacer ); 
    size_t cleanPosition = line.find( "}" );
    line.erase( cleanPosition, 2*sizeof(char) );
    new_main_file.seekp(ios::beg);
    for( i=0; i<toReplaceNo-1; i++ ) new_main_file.ignore(numeric_limits<streamsize>::max(), '\n');
    new_main_file << line << endl;
    while( getline( rkqc_file, currentLine ) ){
        new_main_file << currentLine << ";" << endl;
    }
    new_main_file << "}\n";

    rkqc_file.close();
    main_file.close();
    new_main_file.close();

}

void GenRKQC::mergeLLVMIR()
{
    fstream new_ll_file;
    fstream old_ll_file;
    fstream final_ll_file;
    
    new_ll_file.open( "rkqc.qasm", fstream::in | fstream::out | fstream::app );
    if( !(new_ll_file.is_open() ) ){
        errs() << "Problem With RKQC File\n";
        exit(1);
    }
    string filename = FILE_NAME;
    old_ll_file.open( filename.c_str(), fstream::in | fstream::out );
    if( !(old_ll_file.is_open() ) ){
        errs() << "Problem With Main LL File\n";
        exit(1);
    }
    string new_filename = FILE_NAME + "_final.ll";
    final_ll_file.open( new_filename.c_str(), fstream::out | fstream::in | fstream::trunc );
    if( !(final_ll_file.is_open() ) ){
        errs() << "Problem With New Main QASM File\n";
        exit(1);
    }
    string line;
    string currentLine;
    size_t position;
    size_t lastFuncPosition;
    string funcName;

    //---- Get Header of LL File ----- //
    while( getline( old_ll_file, currentLine ) ){
        if( currentLine.find("define") != string::npos ) break;
        else final_ll_file << currentLine << endl;
    }

    final_ll_file << currentLine << endl;
    lastFuncPosition = currentLine.find(lastFuncName);
    size_t pos1 = currentLine.find("@");
    size_t pos2 = currentLine.find("(");
    size_t len = pos2-pos1;
    funcName = currentLine.substr(currentLine.find("@")+1, len-1 ); 
    if (lastFuncPosition != string::npos) {
        while( getline( new_ll_file, line )){
            final_ll_file << line << endl;
        }
        final_ll_file << "  ret void" << endl << "}" << endl;
    }
    else if (find(isRKQCNames.begin(), isRKQCNames.end(), funcName) != isRKQCNames.end() ) {
        final_ll_file << "  ret void" << endl << "}" << endl;
    }
    else {
        while( getline( old_ll_file, currentLine ) ){
            if (currentLine.find("}") != string::npos){
                final_ll_file << currentLine << endl;
                break;
            }
            else final_ll_file << currentLine << endl;
        }
    }


    //---- Get Remainder of LL File -----//

    while( getline(old_ll_file, currentLine) ){
        position = currentLine.find("define");
        if (position != string::npos){
            final_ll_file << currentLine << endl;
            lastFuncPosition = currentLine.find(lastFuncName);
            size_t pos1 = currentLine.find("@");
            size_t pos2 = currentLine.find("(");
            size_t len = pos2-pos1;
            funcName = currentLine.substr(currentLine.find("@")+1, len-1 ); 
            if (lastFuncPosition != string::npos) {
                while( getline( new_ll_file, line )){
                    final_ll_file << line << endl;
                }
                if (funcName == "main" ) final_ll_file << "  ret i32 0" << endl << "}" << endl; 
                else final_ll_file << "  ret void" << endl << "}" << endl;
            }
            else if (find(isRKQCNames.begin(), isRKQCNames.end(), funcName) != isRKQCNames.end() ) {
                final_ll_file << "  ret void" << endl << "}" << endl;
            }
            else {
                while( getline( old_ll_file, currentLine ) ){
                    if (currentLine.find("}") != string::npos){
                        final_ll_file << currentLine << endl;
                        break;
                    }
                    else final_ll_file << currentLine << endl;
                }
            }

            if (lastFuncPosition != string::npos){
                while( getline( new_ll_file, line ) ){
                    final_ll_file << line << endl;
                }
            }
        }
    }
    final_ll_file << "declare void @llvm.CNOT(i16, i16) nounwind" << endl;
    final_ll_file << "declare void @llvm.X(i16) nounwind" << endl;
    final_ll_file << "declare void @llvm.Toffoli(i16, i16, i16) nounwind" << endl;
    final_ll_file << "declare void @store_cbit(i1, i1*)" << endl;

    new_ll_file.close();
    old_ll_file.close();
    final_ll_file.close();
}




void GenRKQC::getFunctionArguments(Function* F)
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
	if(debugGenrkqc)
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

      if(debugGenrkqc)
	    print_qgateArg(tmpQArg);
    }
    funcCallsArgs.insert( make_pair (F, funcArgList ) );
    funcArgList.clear();
}

bool GenRKQC::checkIfIntrinsicRKQC(Function* CF){
    if(CF->isIntrinsic()){
        if((CF->getIntrinsicID() == Intrinsic::cnot)
            || (CF->getIntrinsicID() == Intrinsic::toffoli)
            || (CF->getIntrinsicID() == Intrinsic::NOT)
            || (CF->getIntrinsicID() == Intrinsic::assign_value_of_b_to_a)
            || (CF->getIntrinsicID() == Intrinsic::assign_value_of_int_to_a)
            || (CF->getIntrinsicID() == Intrinsic::a_swap_b)
            || (CF->getIntrinsicID() == Intrinsic::a_eq_a_plus_b)
            || (CF->getIntrinsicID() == Intrinsic::a_eq_a_minus_b)
            || (CF->getIntrinsicID() == Intrinsic::a_eq_a_plus_b_times_c)){
            return true;
         }
    }
    return false;
}

bool GenRKQC::analyzeIntrinsicCallInstRKQC(Instruction* F){
    if(CallInst *CI = dyn_cast<CallInst>(F)){
        if(!checkIfIntrinsicRKQC(CI->getCalledFunction())) return false;
    }
    return true;
}

bool GenRKQC::checkRKQCCalls(Instruction* F){
    if(CallInst *CI = dyn_cast<CallInst>(F)){
        if(find(isRKQC.begin(), isRKQC.end(), CI->getCalledFunction()) != isRKQC.end() ) return true;
    }
    return false;
}

bool GenRKQC::analyzeRKQC(Function* F){
    funcArgs.clear();
    for (inst_iterator I = inst_begin(*F), E = inst_end(*F); I!=E; ++I) {
        Instruction *Inst = &*I;
        if(isa<CallInst>(Inst)){
            vectCalls.push_back(Inst);
        }
    }
    mapFuncsInsts.insert( make_pair ( F, vectCalls ) );
    for (vector<Instruction*>::iterator rit = vectCalls.begin(); rit != vectCalls.end(); ++rit){
        if (!analyzeIntrinsicCallInstRKQC(*rit)){
            if (!checkRKQCCalls(*rit) ) return false;
        }
    }
    if (F->getName() == "main") return false;
    return true;
}



// run - Find datapaths for qubits
bool GenRKQC::runOnModule(Module &M) {
    errs() << "FILENAME: " << FILE_NAME << "\n";
    ofstream main_output;
    main_output.open( "main.qasm" );

    CallGraphNode* rootNode = getAnalysis<CallGraph>().getRoot();
    unsigned sccNum = 0;

    for (scc_iterator<CallGraphNode*> sccIb = scc_begin(rootNode), E = scc_end(rootNode); sccIb != E; ++sccIb){
        const std::vector<CallGraphNode*> &nextSCC = *sccIb;
        if(debugGenrkqc) errs() << "\nSCC #" << ++sccNum << " : ";      
        for (std::vector<CallGraphNode*>::const_iterator nsccI = nextSCC.begin(),E = nextSCC.end(); nsccI != E; ++nsccI){
            Function *F=(*nsccI)->getFunction();	  
            if(F && !F->isDeclaration()){
                if(debugGenrkqc) errs() << "Processing Function:" << F->getName() <<" \n ";

              //initialize structures for this function
                vectCalls.clear();
                funcArgs.clear();
                qbitsInFunc.clear();
                qbitsInitInFunc.clear();
                funcArgList.clear();
                mapFunction.clear();
                
                getFunctionArguments(F);


                if(analyzeRKQC(F)) {
                    circuitHasRKQC = true;
                    isRKQC.push_back(F);
                    isRKQCNames.push_back(F->getName());
                    if (debugGenrkqc) errs() << "Added to RKQC List: " << F->getName() << "\n";
                    vector<string> thisFuncQbits;
                    qbitDecls.insert( make_pair( F, thisFuncQbits ) );
                    RkqcFunctions.insert( make_pair( make_pair( F, funcArgs), vectCalls) );
                

                //visit Alloc Insts in func and find if function is quantum or classical function

                    for(inst_iterator instIb = inst_begin(F),instIe=inst_end(F); instIb!=instIe;++instIb){
                        Instruction *pInst = &*instIb; // Grab pointer to instruction reference	      
                        analyzeAllocInst(F,pInst);
                    }
                }

                int size = qbitsInFunc.size();
                //map<Function*, vector<qGateArg> >::iterator mpItr = qbitsInFunc.find(F);
                if(qbitsInFunc.size()>0){ //Is Quantum Function
                  bool funcRKQC = false;
                  if( find( isRKQC.begin(), isRKQC.end(), F) != isRKQC.end() ) funcRKQC = true;
                  printFuncHeader(F, funcRKQC, main_output);
                  for(inst_iterator instIb = inst_begin(F),instIe=inst_end(F); instIb!=instIe;++instIb){
      	            Instruction *pInst = &*instIb; // Grab pointer to instruction reference	      
      	            allDepQbit.clear();
      	            if(debugGenrkqc) errs() << "\n Processing Inst: "<<*pInst << "\n";
       	            analyzeInst(F,pInst); 
                  }
                  if( find(isRKQC.begin(), isRKQC.end(), F) == isRKQC.end() ) genRKQC(F, main_output);	      
                }
            }
            else{
              if(debugGenrkqc)
                errs() << "WARNING: Ignoring external node or dummy function.";
            }
        }
        if (nextSCC.size() == 1 && sccIb.hasLoop())
             errs() << " (Has self-loop).";
      }
      errs() << "\n";

    // Post Order Processing Of Each RKQC Function //
    if( circuitHasRKQC ){
        ofstream rkqc_file;
        rkqc_file.open("rkqc.cpp");
        rkqc_file << "#include <iostream> \n";
        rkqc_file << "#include <core/circuit.hpp>\n";
        rkqc_file << "#include <core/functions/add_gates.hpp>\n";
        rkqc_file << "#include <boost/lexical_cast.hpp>\n\n";
        rkqc_file << "using namespace revkit;\n\n";
        int i;

        vector<Function*>::iterator it;
        if (isRKQC.size() != 0 ){
            vector<Function*>::iterator endit = isRKQC.end();
            endit--;
            bool lastFunc = false;

//------------ Print Each RKQC Function Name and Arguments----------------//

            for (it = isRKQC.begin(); it != isRKQC.end(); ++it){
                if( it == endit ) lastFunc = true;
                printFuncHeaderToFile(*it, rkqc_file, lastFunc);
                //Print Each Instruction Name and Arguments
                vector<Instruction*> instNames = mapFuncsInsts.find(*it)->second; 
                for (i = 0; i < instNames.size(); i++){
                    CallInst *CI = dyn_cast<CallInst>(instNames[i]);
                    string instName = CI->getCalledFunction()->getName();
                    if( instName.find("rkqc.") != string::npos)
                        rkqc_file << "  " << instName.substr(instName.find("rkqc.") + 5) << " (";
                    else rkqc_file << "  " << instName << " (";
                    //Print out each argument
                    vector<qGateArg> instArgs = (mapCallsArgs.find(instNames[i]))->second;    
                    vector<qGateArg>::iterator endIt = instArgs.end();
                    endIt--;
                    for (vector<qGateArg>::iterator vpIt = instArgs.begin(); vpIt != instArgs.end();++vpIt){
                        if((*vpIt).isQbit || (*vpIt).isCbit){
                            if((*vpIt).argPtr) rkqc_file << printVarName((*vpIt).argPtr->getName());
                            if(!((*vpIt).isPtr)){
                                for(int ndim = 0; ndim < (*vpIt).numDim; ndim++) {
                                    rkqc_file << "[" << (*vpIt).dimSize[ndim] << "]";
                                }
                            }
                        }
                        else if((*vpIt).isInt){ 
                            stringstream ss;
                            ss << (*vpIt).valOrIndex;
                            rkqc_file << ss.str() ;
                        }
                        if ( ((vpIt) != endIt ) ) rkqc_file << ",";
                    }
                    rkqc_file << ");\n";
                }
                rkqc_file << "}\n\n";
            }
        }
        rkqc_file.close();
        main_output.close();
        system("rkqc rkqc");
    }
//--------------- Merge rkqc.qasm with ${NAME}.qasm -------------------------//
//    mergeQASM();
    mergeLLVMIR();
    return false;
}

