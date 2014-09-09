//===----------------- QtmInst.cpp ----------------------===//
// This file implements the Scaffold Quantum Instruction Class
//
//
//
//        This file was created by Scaffold Compiler Working Group
//
//
//
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "QtmInst"
#include <vector>
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instruction.h"
#include "llvm/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Support/CFG.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/Argument.h"
#include "llvm/ADT/ilist.h"
#include "llvm/Constants.h"
#include "llvm/IntrinsicInst.h"
#include "scaffold/QtmGateArg.h"


using namespace llvm;
using namespace std;

#define MAX_GATE_ARGS 30
#define MAX_BT_COUNT 15 //max backtrace allowed - to avoid infinite recursive loops

bool debugQtmInst = false;

namespace {
  
  struct qArgInfo{
    string name;
    int index;
    qArgInfo(): name("none"), index(-1){ }
  };
  
  struct qGate{
    Function* qFunc;
    int numArgs;
    qArgInfo args[MAX_GATE_ARGS];
    double angle;
    qGate():qFunc(NULL), numArgs(0), angle(0.0) { }
  };

  struct QtmInst : public ModulePass {
    static char ID; // Pass identification
    
    vector<QtmGateArg> tmpDepQbit;
    
    int btCount; //backtrace count

    QtmInst() : ModulePass(ID) {}
    
    bool checkIfQbitOrCbit(Type* varType);

    bool backtraceOperand(Value* opd, int opOrIndex);
    void analyzeCallInst(Function* F,Instruction* pinst);

    void print_qgate(qGate qg);

    
    bool runOnModule (Module &M);    
    
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();  
      AU.addRequired<CallGraph>();    
    }
    
  }; // End of struct QtmInst
} // End of anonymous namespace

char QtmInst::ID = 0;
static RegisterPass<QtmInst> X("QtmInst", "Get gate and operands for a Qtm Inst");


bool QtmInst::checkIfQbitOrCbit(Type* varType)
{
  if(varType->isPointerTy()){
    Type *elementType = varType->getPointerElementType();
    if (elementType->isIntegerTy(16) || elementType->isIntegerTy(1)){ //qbit* or cbit*
      return true;	      
    }
  }
  else if (varType->isIntegerTy(16) || varType->isIntegerTy(1)){ //qbit or cbit
    return true;
  }
  return false;
}

bool QtmInst::backtraceOperand(Value* opd, int opOrIndex)
{
  if(opOrIndex == 0) //backtrace for operand
    {

      if(Argument *fArg = dyn_cast<Argument>(opd))
	{
	  Type* argType = fArg->getType();
	  bool isQtm = checkIfQbitOrCbit(argType);
	  if(isQtm){
	    tmpDepQbit[0].argPtr = opd;
	    return true;
	  } 
	}

      if (AllocaInst *AI = dyn_cast<AllocaInst>(opd)) {
	Type *allocatedType = AI->getAllocatedType();
    
	if(ArrayType *arrayType = dyn_cast<ArrayType>(allocatedType)) {      
	  Type *elementType = arrayType->getElementType();
	  bool isQtm = checkIfQbitOrCbit(elementType);
	  if(isQtm){
	    tmpDepQbit[0].argPtr = opd;
	    return true;
	  }
	}
      }

      if(btCount>MAX_BT_COUNT)
	return false;
      
      if(isa<GetElementPtrInst>(opd))
	{
	  Instruction* pInst = dyn_cast<Instruction>(opd);
	  backtraceOperand(pInst->getOperand(0),0);
	  unsigned numOps = pInst->getNumOperands();

	  //NOTE: getelemptr instruction can have multiple indices. Currently considering last operand as desired index for qubit. This may not be true for multi-dimensional qubit arrays 
	  if(ConstantInt *CI = dyn_cast<ConstantInt>(pInst->getOperand(numOps-1))){
	    if(tmpDepQbit.size()==1){
	      tmpDepQbit[0].valOrIndex = CI->getZExtValue();
	    }
	  }
	  else{

	    if(tmpDepQbit[0].isQbit && !(tmpDepQbit[0].isPtr)){
     	      //NOTE: getelemptr instruction can have multiple indices. consider last operand as desired index for qubit. This may not be true for multi-dimensional qubit arrays
	      backtraceOperand(pInst->getOperand(numOps-1),1);
	      
	    }

	  }	 
	  /*
	    //For multi-dimensional qubit array, we may need to backtrace multiple pointer values
	    for(unsigned iop=1;iop<numOps;iop++){ //first opd was already backtraced
	      backtraceOperand(pInst->getOperand(iop),0);

	    }*/
	  return true;
	}
      
      if(Instruction* pInst = dyn_cast<Instruction>(opd)){
	unsigned numOps = pInst->getNumOperands();
	for(unsigned iop=0;iop<numOps;iop++){
	  btCount++;
	  backtraceOperand(pInst->getOperand(iop),0);
	  btCount--;
	}
	return true;
      }
      else{
	return true;
      }
    }
  else if(opOrIndex == 0){ //opOrIndex == 1; i.e. Backtracing for Index    
    if(btCount>MAX_BT_COUNT) //prevent infinite backtracing
      return true;

    if(ConstantInt *CI = dyn_cast<ConstantInt>(opd)){
      tmpDepQbit[0].valOrIndex = CI->getZExtValue();
      return true;
    }      

    if(Instruction* pInst = dyn_cast<Instruction>(opd)){
      unsigned numOps = pInst->getNumOperands();
      for(unsigned iop=0;iop<numOps;iop++){
	btCount++;
	backtraceOperand(pInst->getOperand(iop),1);
	btCount--;
      }
    }

  }
  else{ //opOrIndex == 2: backtracing to call inst MeasZ
    if(CallInst *endCI = dyn_cast<CallInst>(opd)){
      if(endCI->getCalledFunction()->getName().find("llvm.Meas") != string::npos){
	tmpDepQbit[0].argPtr = opd;

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


void QtmInst::print_qgate(qGate qg){
  errs() << qg.qFunc->getName() << " : ";
  for(int i=0;i<qg.numArgs;i++){
    errs() << qg.args[i].name << "[" << qg.args[i].index << "], "  ;
  }
  errs() << "\n";
}

void QtmInst::analyzeCallInst(Function* F, Instruction* pInst){
  if(CallInst *CI = dyn_cast<CallInst>(pInst))
    {      
      if(debugQtmInst)
	errs() << "Call inst: " << CI->getCalledFunction()->getName() << "\n";

      if(CI->getCalledFunction()->getName() == "store_cbit"){	//trace return values
	return;
      }      

      vector<QtmGateArg> allDepQbit;                                  
      
      bool tracked_operand = false;

      int myPrepState = -1;
      double myRotationAngle = 0.0;
      
      for(unsigned iop=0;iop<CI->getNumArgOperands();iop++){
	tmpDepQbit.clear();
	
	QtmGateArg tmpQGateArg;
	btCount=0;
	
	tmpQGateArg.argNum = iop;
		
	if(isa<UndefValue>(CI->getArgOperand(iop))){
	  errs() << "WARNING: LLVM IR code has UNDEF values. \n";
	  tmpQGateArg.isUndef = true;	
	  //exit(1);
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
	  myPrepState = CInt->getZExtValue();	  
	}
	
	//check if argument is constant float
	if(ConstantFP *CFP = dyn_cast<ConstantFP>(CI->getArgOperand(iop))){
	  myRotationAngle = CFP->getValueAPF().convertToDouble();
	}	  	
	
        //if(tmpQGateArg.isQbit || tmpQGateArg.isCbit){
	if(tmpQGateArg.isQbit){
            tmpDepQbit.push_back(tmpQGateArg);	
            tracked_operand = backtraceOperand(CI->getArgOperand(iop),0);
	}

        if(tmpDepQbit.size()>0){	  
	  allDepQbit.push_back(tmpDepQbit[0]);
	  assert(tmpDepQbit.size() == 1 && "tmpDepQbit SIZE GT 1");
	  tmpDepQbit.clear();
	}
	
      }
      
      if(allDepQbit.size() > 0){
	if(debugQtmInst)
	{
	    errs() << "\nCall inst: " << CI->getCalledFunction()->getName();	    
	    errs() << ": Found all arguments: ";       
	    for(unsigned int vb=0; vb<allDepQbit.size(); vb++){
	      if(allDepQbit[vb].argPtr)
		errs() << allDepQbit[vb].argPtr->getName() <<" Index: ";
                                
	      //else
		errs() << allDepQbit[vb].valOrIndex <<" ";
	    }
	    errs()<<"\n";
	    
	}

          
       string fname =  CI->getCalledFunction()->getName();  
       qGate thisGate;
       thisGate.qFunc =  CI->getCalledFunction();

       if(myPrepState!=-1) thisGate.angle = (float)myPrepState;
       if(myRotationAngle!=0.0) thisGate.angle = myRotationAngle;

       for(unsigned int vb=0; vb<allDepQbit.size(); vb++){
            if(allDepQbit[vb].argPtr){
	      //errs() << allDepQbit[vb].argPtr->getName() <<" Index: ";
	      //errs() << allDepQbit[vb].valOrIndex <<"\n";
	      QtmGateArg param =  allDepQbit[vb];       
                thisGate.args[thisGate.numArgs].name = param.argPtr->getName();
		if(!param.isPtr)
		  thisGate.args[thisGate.numArgs].index = param.valOrIndex;
                thisGate.numArgs++;
	    }
       }

       print_qgate(thisGate);

      }    
      allDepQbit.erase(allDepQbit.begin(),allDepQbit.end());
    }
}

bool QtmInst::runOnModule (Module &M) {
  
  // iterate over all functions, and over all instructions in those functions
  CallGraphNode* rootNode = getAnalysis<CallGraph>().getRoot();
  
  //Post-order
  for (scc_iterator<CallGraphNode*> sccIb = scc_begin(rootNode), E = scc_end(rootNode); sccIb != E; ++sccIb) {
    const std::vector<CallGraphNode*> &nextSCC = *sccIb;
    for (std::vector<CallGraphNode*>::const_iterator nsccI = nextSCC.begin(), E = nextSCC.end(); nsccI != E; ++nsccI) {
      Function *F = (*nsccI)->getFunction();	  
            
      if(F && !F->isDeclaration()){
	errs() << "\n#Function " << F->getName() << "\n";      
       	
	for (inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E; ++I) {
	  Instruction *Inst = &*I;                            // Grab pointer to instruction reference
	  if(isa<CallInst>(Inst))
	    analyzeCallInst(F,Inst);	

	}
	
      }
      else{
	    if(debugQtmInst)
	      errs() << "WARNING: Ignoring external node or dummy function.\n";
	  }
    }
  }

  return false;
} // End runOnModule
