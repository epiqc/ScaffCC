//===----------------- CriticalResourceCount.cpp ----------------------===//
// This file implements the Scaffold Pass of counting the number 
//  of critical timesteps and gate parallelism in program
//  in callgraph post-order.
//
//        This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "CriticalResourceCount"
#include <vector>
#include <limits>
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


using namespace llvm;
using namespace std;

#define MAX_GATE_ARGS 15
#define MAX_BT_COUNT 15 //max backtrace allowed - to avoid infinite recursive loops
#define NUM_QGATES 17
#define _CNOT 0
#define _H 1
#define _S 2
#define _T 3
#define _X 4
#define _Y 5
#define _Z 6
#define _MeasX 7
#define _MeasZ 8
#define _PrepX 9
#define _PrepZ 10
#define _Tdag 11
#define _Sdag 12
#define _Rz 13
#define _Toffoli 14
#define _Fredkin 15
#define _All 16


bool debugCritDataPath = false;

namespace {
  
  struct qGateArg{ //arguments to qgate calls
    Value* argPtr;
    int argNum;
    bool isQbit;
    bool isCbit;
    bool isUndef;
    bool isPtr;
    int valOrIndex; //Value if not Qbit, Index if Qbit & not a Ptr

    qGateArg(): argPtr(NULL), argNum(-1), isQbit(false), isCbit(false), isUndef(false), isPtr(false), valOrIndex(-1){ }
  };
  
struct qArgInfo{
  string name;
  int index;

  qArgInfo(): name("none"), index(-1){ }
};

struct qGate{
  Function* qFunc;
  int numArgs;
  qArgInfo args[MAX_GATE_ARGS];
  qGate():qFunc(NULL), numArgs(0) { }
};

  struct TSParGateInfo{
    unsigned long long int parallel_gates[NUM_QGATES];
  };

  struct TSInfo{
    unsigned long long int timesteps;
    vector<TSParGateInfo> gates;
    TSInfo(): timesteps(0){ }
  };  

  struct MaxTSInfo{ //TimeStepInfo
    unsigned long long int timesteps;
    unsigned long long int parallel_gates[NUM_QGATES];
    MaxTSInfo(): timesteps(0){ }
  };

  struct CriticalResourceCount : public ModulePass {
    static char ID; // Pass identification
    
    string gate_name[NUM_QGATES];
    vector<qGateArg> tmpDepQbit;
    vector<Value*> vectQbit;
    
    int btCount; //backtrace count

    vector<qArgInfo> currTimeStep; //contains set of arguments operated on currently
    vector<string> currParallelFunc;
    MaxTSInfo maxParallelFactor; //overall max parallel factor
    map<string, int> gate_index;    
    vector<unsigned long long int> curr_parallel_ts; //vector of current timesteps that are parallel; used for comparing functions
    map<string, TSInfo > funcParallelFactor; //string is function name
    map<string, MaxTSInfo> funcMaxParallelFactor;

        
    CriticalResourceCount() : ModulePass(ID) {}
    
    bool backtraceOperand(Value* opd, int opOrIndex);
    void analyzeAllocInst(Function* F,Instruction* pinst);
    void analyzeCallInst(Function* F,Instruction* pinst);
    void getFunctionArguments(Function *F);
    
    
    void init_gate_names(){
        gate_name[_CNOT] = "CNOT";
        gate_name[_H] = "H";
        gate_name[_S] = "S";
        gate_name[_T] = "T";
        gate_name[_Toffoli] = "Toffoli";
        gate_name[_X] = "X";
        gate_name[_Y] = "Y";
        gate_name[_Z] = "Z";
        gate_name[_MeasX] = "MeasX";
        gate_name[_MeasZ] = "MeasZ";
        gate_name[_PrepX] = "PrepX";
        gate_name[_PrepZ] = "PrepZ";
        gate_name[_Sdag] = "Sdag";
        gate_name[_Tdag] = "Tdag";
        gate_name[_Fredkin] = "Fredkin";
        gate_name[_Rz] = "Rz";
        gate_name[_All] = "All";                    
        
        gate_index["CNOT"] = _CNOT;        
        gate_index["H"] = _H;
        gate_index["S"] = _S;
        gate_index["T"] = _T;
        gate_index["Toffoli"] = _Toffoli;
        gate_index["X"] = _X;
        gate_index["Y"] = _Y;
        gate_index["Z"] = _Z;
        gate_index["Sdag"] = _Sdag;
        gate_index["Tdag"] = _Tdag;
        gate_index["MeasX"] = _MeasX;
        gate_index["MeasZ"] = _MeasZ;
        gate_index["PrepX"] = _PrepX;
        gate_index["PrepZ"] = _PrepZ;
        gate_index["Fredkin"] = _Fredkin;
        gate_index["Rz"] = _Rz;
        gate_index["All"] = _All;                    
        
        
        }
        

    void init_gates_as_functions();    
    void init_critical_path_algo(Function* F);
    void reset_current_ts_info(Function* F);
    void print_current_ts_info();
    void print_vector_TSGates(Function* F);
    void print_max_critical_info(MaxTSInfo ts);
    void update_max_critical_info(Function* F);
    void calc_critical_time(Function* F, qGate qg);        
    void update_currParallelFunc(TSInfo ts);                
    void print_currParallelFunc();
    void print_TSInfo(TSInfo ts);
    void copy_max_parallel_factor(Function *F);

    void print_qgateArg(qGateArg qg)
    {
      errs()<< "Printing QGate Argument:\n";
      if(qg.argPtr) errs() << "  Name: "<<qg.argPtr->getName()<<"\n";
      errs() << "  Arg Num: "<<qg.argNum<<"\n"
	     << "  isUndef: "<<qg.isUndef
	     << "  isQbit: "<<qg.isQbit
	     << "  isCbit: "<<qg.isCbit
	     << "  isPtr: "<<qg.isPtr << "\n"
	     << "  Value or Index: "<<qg.valOrIndex<<"\n";
    }                    
    
    void CountCriticalFunctionResources (Function *F);
    
    bool runOnModule (Module &M);
    
    
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();  
      AU.addRequired<CallGraph>();    
    }
    
  }; // End of struct CriticalResourceCount
} // End of anonymous namespace



char CriticalResourceCount::ID = 0;
static RegisterPass<CriticalResourceCount> X("CriticalResourceCount", "Critical Resource Counter Pass");

void CriticalResourceCount::getFunctionArguments(Function* F)
{
  for(Function::arg_iterator ait=F->arg_begin();ait!=F->arg_end();++ait)
    {    
      //if(ait) errs() << "Argument: "<<ait->getName()<< " ";

      string argName = (ait->getName()).str();
      Type* argType = ait->getType();
      unsigned int argNum=ait->getArgNo();         

      qGateArg tmpQArg;
      tmpQArg.argPtr = ait;
      tmpQArg.argNum = argNum;

      if(argType->isPointerTy()){
	tmpQArg.isPtr = true;

	Type *elementType = argType->getPointerElementType();
	if (elementType->isIntegerTy(16)){ //qbit*
	  tmpQArg.isQbit = true;
	  vectQbit.push_back(ait);
	}
	else if (elementType->isIntegerTy(1)){ //cbit*
	  tmpQArg.isCbit = true;
	  vectQbit.push_back(ait);
	}
      }
      else if (argType->isIntegerTy(16)){ //qbit
	tmpQArg.isQbit = true;
	vectQbit.push_back(ait);
      }
      else if (argType->isIntegerTy(1)){ //cbit
	tmpQArg.isCbit = true;
	vectQbit.push_back(ait);
      }
      
    }
}

bool CriticalResourceCount::backtraceOperand(Value* opd, int opOrIndex)
{
  if(opOrIndex == 0) //backtrace for operand
    {
      //search for opd in qbit/cbit vector
      vector<Value*>::iterator vIter=find(vectQbit.begin(),vectQbit.end(),opd);
      if(vIter != vectQbit.end()){
	tmpDepQbit[0].argPtr = opd;
	
	return true;
      }
      
      if(btCount>MAX_BT_COUNT)
	return false;
      
      if(GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(opd))
	{

	  if(GEPI->hasAllConstantIndices()){
	    Instruction* pInst = dyn_cast<Instruction>(opd);
	    unsigned numOps = pInst->getNumOperands();

	    backtraceOperand(pInst->getOperand(0),0);
	    
	    //NOTE: getelemptr instruction can have multiple indices. Currently considering last operand as desired index for qubit. Check this reasoning. 
	    if(ConstantInt *CI = dyn_cast<ConstantInt>(pInst->getOperand(numOps-1))){
	      if(tmpDepQbit.size()==1){
		tmpDepQbit[0].valOrIndex = CI->getZExtValue();
	      }
	    }
	  }
	  
	  else if(GEPI->hasIndices()){
	    
	    Instruction* pInst = dyn_cast<Instruction>(opd);
	    unsigned numOps = pInst->getNumOperands();
	    backtraceOperand(pInst->getOperand(0),0);

	    if(tmpDepQbit[0].isQbit && !(tmpDepQbit[0].isPtr)){     
	      //NOTE: getelemptr instruction can have multiple indices. consider last operand as desired index for qubit. Check if this is true for all.
	      backtraceOperand(pInst->getOperand(numOps-1),1);
	      
	    }
	  }
	  else{	    
	    Instruction* pInst = dyn_cast<Instruction>(opd);
	    unsigned numOps = pInst->getNumOperands();
	    for(unsigned iop=0;iop<numOps;iop++){
	      backtraceOperand(pInst->getOperand(iop),0);
	    }
	  }
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


void CriticalResourceCount::analyzeAllocInst(Function* F, Instruction* pInst){
  if (AllocaInst *AI = dyn_cast<AllocaInst>(pInst)) {
    Type *allocatedType = AI->getAllocatedType();
    
    if(ArrayType *arrayType = dyn_cast<ArrayType>(allocatedType)) {      
      qGateArg tmpQArg;
      
      Type *elementType = arrayType->getElementType();
      uint64_t arraySize = arrayType->getNumElements();
      if (elementType->isIntegerTy(16)){
	vectQbit.push_back(AI);
	tmpQArg.isQbit = true;
	tmpQArg.argPtr = AI;
	tmpQArg.valOrIndex = arraySize;
      }
      
      if (elementType->isIntegerTy(1)){
	vectQbit.push_back(AI); //Cbit added here
	tmpQArg.isCbit = true;
	tmpQArg.argPtr = AI;
	tmpQArg.valOrIndex = arraySize;
      }
    }
  }
}


void CriticalResourceCount::init_critical_path_algo(Function* F){

  MaxTSInfo structMaxInfo;
  TSInfo vectGateInfo; //initialize an entry in the function map
  funcParallelFactor[F->getName().str()] = vectGateInfo;

  currTimeStep.clear(); //initialize critical time steps   

  curr_parallel_ts.clear();

  maxParallelFactor.timesteps = 0;   //initialize critical time steps

  for(int i=0; i< NUM_QGATES ; i++){
    maxParallelFactor.parallel_gates[i] = 0;
    structMaxInfo.parallel_gates[i] = 0;
  }
  funcMaxParallelFactor[F->getName().str()] = structMaxInfo;
  currParallelFunc.clear();
}


void CriticalResourceCount::print_vector_TSGates(Function* F){
  map<string, TSInfo>::iterator fp = funcParallelFactor.find(F->getName().str());
  errs() << "Printing function map \n";
  for(vector<TSParGateInfo>::iterator vit = (*fp).second.gates.begin(); vit!=(*fp).second.gates.end(); ++vit){
    for(int i=0; i< NUM_QGATES ; i++){
      errs() << (*vit).parallel_gates[i] << " ";
    }
    errs() << "\n";
  }
}

void CriticalResourceCount::print_currParallelFunc(){
  errs() << "currParallelFunc: ";
  for(vector<string>::iterator vit = currParallelFunc.begin(); vit!=currParallelFunc.end(); ++vit){
      errs() << (*vit) << " ";
    }
    errs() << "\n";
  }

void CriticalResourceCount::reset_current_ts_info(Function* F){ //current timestep info

  //clear arguments being operated upon
  currTimeStep.clear(); //initialize critical time steps

  curr_parallel_ts.clear(); //reset current timestep info

  if(debugCritDataPath){
    print_currParallelFunc();
    errs() << " --------------- \n";
  }

  currParallelFunc.clear();

  if(debugCritDataPath)
    print_vector_TSGates(F);
  
}

void CriticalResourceCount::print_current_ts_info(){ //current timestep info
  errs() << "\n Current Critical Time Steps = "<<maxParallelFactor.timesteps << " ";
  for(vector<string>::iterator vit = currParallelFunc.begin(); vit!=currParallelFunc.end(); ++vit)
      errs() << (*vit) << " ";
    errs() << "\n" ;
  
}

void CriticalResourceCount::print_max_critical_info(MaxTSInfo ts){ //max timestep info
    errs() << "MAX Critical Time Steps = "<<ts.timesteps << "\n";
    errs() << "MAX Parallelism Factors: \n";
    for(int i=0; i<NUM_QGATES; i++)
      errs() << "\t" << gate_name[i] << ": " << ts.parallel_gates[i] << "\n";
    errs() << "\n" ;
}

void CriticalResourceCount::print_TSInfo(TSInfo ts){
    for(vector<TSParGateInfo>::iterator printIt = ts.gates.begin(); printIt!=ts.gates.end(); ++ printIt)
      {
	for(int print_var = 0; print_var < NUM_QGATES; print_var++)
	  errs() << (*printIt).parallel_gates[print_var] << " ";
	errs() << "\n";
      }
}

void CriticalResourceCount::update_max_critical_info(Function* F){ //max timestep info

  if(currParallelFunc.size()>1)
  {

    //add all parallel timesteps first
    TSInfo sum_info; 
    
    sum_info.timesteps = 0;
    sum_info.gates.clear();
    
    for(vector<string>::iterator vit=currParallelFunc.begin(); vit!=currParallelFunc.end(); ++vit){
      map<string,TSInfo>::iterator tsIter = funcParallelFactor.find(*vit);
      if(debugCritDataPath)
	errs() << "Timesteps of function: " << (*vit) << " are " << (*tsIter).second.timesteps << "\n";
      if((*tsIter).second.timesteps > sum_info.timesteps)
	sum_info.timesteps = (*tsIter).second.timesteps;
      
      unsigned int vsize = sum_info.gates.size();
      unsigned int fnsize = (*tsIter).second.gates.size();
      
      //errs() << "vsize = "<<vsize<< " fnSize = "<<fnsize <<"\n";

      if(vsize<fnsize){
	for(unsigned int j=0; j<vsize;j++){
	  for(int k=0;k<NUM_QGATES;k++)
	    sum_info.gates[j].parallel_gates[k] += (*tsIter).second.gates[j].parallel_gates[k];
	}

	for(unsigned int j=vsize;j<fnsize;j++){
	  sum_info.gates.push_back((*tsIter).second.gates[j]);
	}
      }
      else{
	for(unsigned int j=0; j<fnsize;j++){
	  for(int k=0;k<NUM_QGATES;k++)
	    sum_info.gates[j].parallel_gates[k] += (*tsIter).second.gates[j].parallel_gates[k];
	}	
      }
    }

    //save sum_info into the vector<TSParGateInfo>
    if(F->getName() != "main"){ //do not save for main
      map<string,TSInfo>::iterator fnIter = funcParallelFactor.find(F->getName().str());
      //append sum_info.gates to fnIter.second.gates
      (*fnIter).second.gates.insert((*fnIter).second.gates.end(),sum_info.gates.begin(), sum_info.gates.end());
    
    }
    
    //compare maxParallelFactor with the data in sum_info
    for(vector<TSParGateInfo>::iterator gateIter = sum_info.gates.begin(); gateIter!= sum_info.gates.end(); ++gateIter){
      for(int i=0; i<NUM_QGATES; i++){            
	if((*gateIter).parallel_gates[i] > maxParallelFactor.parallel_gates[i])
	  maxParallelFactor.parallel_gates[i] = (*gateIter).parallel_gates[i];        
      }        
    }
    
    //update critical timestep info
    maxParallelFactor.timesteps += sum_info.timesteps;
  }
  else{
    //directly compare maxParallelFactor with the timesteps in function in currParallelFunc

    map<string,TSInfo>::iterator tsIter;
    
    string fStr = currParallelFunc.front();
    tsIter = funcParallelFactor.find(fStr);

    if(debugCritDataPath)
      errs() << "Timesteps of function: " << fStr << " are " << (*tsIter).second.timesteps << "\n";

    //save TSInfo
    if(F->getName() != "main"){ //do not save for main
      map<string,TSInfo>::iterator fnIter = funcParallelFactor.find(F->getName().str());
      //append sum_info.gates to fnIter.second.gates
      (*fnIter).second.gates.insert((*fnIter).second.gates.end(),(*tsIter).second.gates.begin(), (*tsIter).second.gates.end());    
    }
    
    //compare maxParallelFactor with the data in sum_info
    map<string, MaxTSInfo>::iterator maxIter = funcMaxParallelFactor.find(fStr);

    for(int i=0; i<NUM_QGATES; i++){            
      if((*maxIter).second.parallel_gates[i] > maxParallelFactor.parallel_gates[i])
	maxParallelFactor.parallel_gates[i] = (*maxIter).second.parallel_gates[i]; 
    }        
    
    //update critical timestep info
    maxParallelFactor.timesteps += (*tsIter).second.timesteps;    
  }  
}

void CriticalResourceCount::calc_critical_time(Function* F, qGate qg){
  string fname = qg.qFunc->getName();
  
  //check each arg with args in currTimeStep
  bool is_dependency = false;
  
  for(vector<qArgInfo>::iterator vit = currTimeStep.begin(); (vit!=currTimeStep.end()) && (!is_dependency); ++vit)
    {
      for(int i = 0; i<qg.numArgs; i++){
	if((*vit).name == qg.args[i].name) //var name matches
	  {
	    if((*vit).index == -1 || qg.args[i].index == -1){ //argument is a ptr, so pessimistic checking
	      is_dependency = true; 	      
	      break;
	    }
	    else{
	      //check for index match
	      if((*vit).index == qg.args[i].index)
		{
		  is_dependency = true;
		  break;
		} //index matches
	    }
	  } //var name match
      } //for all arguments 
    } // for currTimeStep::iterator
  
  if(is_dependency){ //process this time step and advance

    update_max_critical_info(F);
    
    if(debugCritDataPath){
      print_max_critical_info(maxParallelFactor);
    }
        
    //clear info for current timestep
    reset_current_ts_info(F);
  }    
  
  //Add args to curr args
  for(int i = 0; i<qg.numArgs; i++){
    currTimeStep.push_back(qg.args[i]);
  }            
  
  //Add timestreps of function to list of timesteps
  //Info for this function must already be present in the func_parallelism map        
  map<string, TSInfo>::iterator fIter = funcParallelFactor.find(fname); 
  curr_parallel_ts.push_back((*fIter).second.timesteps);
  
  //Add function to currParallelFunc
  currParallelFunc.push_back(fname);
}



void CriticalResourceCount::analyzeCallInst(Function* F, Instruction* pInst){
  if(CallInst *CI = dyn_cast<CallInst>(pInst))
    {      
      if(debugCritDataPath)
	errs() << "Call inst: " << CI->getCalledFunction()->getName() << "\n";

      if(CI->getCalledFunction()->getName() == "store_cbit"){	//trace return values
	return;
      }

      vector<qGateArg> allDepQbit;                                  
      
      bool tracked_all_operands = true;
      
      for(unsigned iop=0;iop<CI->getNumArgOperands();iop++){
	tmpDepQbit.clear();
	
	qGateArg tmpQGateArg;
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
	
        if(tmpQGateArg.isQbit || tmpQGateArg.isCbit){
            tmpDepQbit.push_back(tmpQGateArg);	
            tracked_all_operands &= backtraceOperand(CI->getArgOperand(iop),0);
	}

        if(tmpDepQbit.size()>0){	  
	  allDepQbit.push_back(tmpDepQbit[0]);
	  assert(tmpDepQbit.size() == 1 && "tmpDepQbit SIZE GT 1");
	  tmpDepQbit.clear();
	}
	
      }
      
      if(allDepQbit.size() > 0){
	if(debugCritDataPath)
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
       for(unsigned int vb=0; vb<allDepQbit.size(); vb++){
            if(allDepQbit[vb].argPtr){
                qGateArg param =  allDepQbit[vb];       
                thisGate.args[thisGate.numArgs].name = param.argPtr->getName();
		if(!param.isPtr)
		  thisGate.args[thisGate.numArgs].index = param.valOrIndex;
                thisGate.numArgs++;
	    }
       }
       
       calc_critical_time(F,thisGate);       

      }    
      allDepQbit.erase(allDepQbit.begin(),allDepQbit.end());
    }
}

void CriticalResourceCount::copy_max_parallel_factor(Function *F){
  map<string, MaxTSInfo>::iterator fparIter = funcMaxParallelFactor.find(F->getName().str());
  (*fparIter).second.timesteps = maxParallelFactor.timesteps;
  for(int i=0;i<NUM_QGATES;i++)
    {
      (*fparIter).second.parallel_gates[i] = maxParallelFactor.parallel_gates[i];
    }
}

void CriticalResourceCount::CountCriticalFunctionResources (Function *F) {
      // Traverse instruction by instruction
  init_critical_path_algo(F);
  
  
  for (inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E; ++I) {
    Instruction *Inst = &*I;                            // Grab pointer to instruction reference
    analyzeAllocInst(F,Inst);          
    analyzeCallInst(F,Inst);	
  }
  
  update_max_critical_info(F);
  print_max_critical_info(maxParallelFactor);  

  //copy maxParallelFactor into funcMaxParallelFactor
  copy_max_parallel_factor(F);
  
  map<string, TSInfo>::iterator fparIter = funcParallelFactor.find(F->getName().str());
  (*fparIter).second.timesteps = maxParallelFactor.timesteps;

  reset_current_ts_info(F);
  
}


void CriticalResourceCount::init_gates_as_functions(){
  for(int  i =0; i< NUM_QGATES ; i++){
    string gName = gate_name[i];
    string fName = "llvm.";
    fName.append(gName);

    TSInfo tmp_info;
    MaxTSInfo tmp_max_info;

    tmp_info.timesteps = 1;
    tmp_max_info.timesteps = 1;

    TSParGateInfo tmp_gate_info;
    for(int  k=0; k< NUM_QGATES ; k++){
      tmp_gate_info.parallel_gates[k] = 0;
      tmp_max_info.parallel_gates[k] = 0;
    }
    tmp_gate_info.parallel_gates[i] = 1;
    tmp_gate_info.parallel_gates[_All] = 1;

    tmp_max_info.parallel_gates[i] = 1;
    tmp_max_info.parallel_gates[_All] = 1;
    
    tmp_info.gates.push_back(tmp_gate_info);

    funcParallelFactor[fName] = tmp_info;
    funcMaxParallelFactor[fName] = tmp_max_info;
    
  }
}


bool CriticalResourceCount::runOnModule (Module &M) {
  init_gate_names();
  init_gates_as_functions();
  
  // iterate over all functions, and over all instructions in those functions
  CallGraphNode* rootNode = getAnalysis<CallGraph>().getRoot();
  
  //Post-order
  for (scc_iterator<CallGraphNode*> sccIb = scc_begin(rootNode), E = scc_end(rootNode); sccIb != E; ++sccIb) {
    const std::vector<CallGraphNode*> &nextSCC = *sccIb;
    for (std::vector<CallGraphNode*>::const_iterator nsccI = nextSCC.begin(), E = nextSCC.end(); nsccI != E; ++nsccI) {
      Function *F = (*nsccI)->getFunction();	  
            
      if(F && !F->isDeclaration()){
      errs() << "\nFunction: " << F->getName() << "\n";      

      getFunctionArguments(F);
      
      // count the critical resources for this function
      CountCriticalFunctionResources(F);

    }
      else{
	    if(debugCritDataPath)
	      errs() << "WARNING: Ignoring external node or dummy function.\n";
	  }
    }
  }  
  return false;
} // End runOnModule
