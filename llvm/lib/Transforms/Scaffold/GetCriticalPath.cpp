//===----------------- GetCriticalPath.cpp ----------------------===//
// This file implements the Scaffold Pass of counting the number 
//  of critical timesteps and gate parallelism in program
//  in callgraph post-order.
//
//        This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "GetCriticalPath"
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

#define MAX_GATE_ARGS 30
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

bool debugGetCriticalPath = false;

namespace {

  struct qGateArg{ //arguments to qgate calls
    Value* argPtr;
    int argNum;
    bool isQbit;
    bool isCbit;
    bool isUndef;
    bool isPtr;
    int valOrIndex; //Value if not Qbit, Index if Qbit & not a Ptr
    qGateArg(): argPtr(NULL), argNum(-1), isQbit(false), isCbit(false), isUndef(false), isPtr(false), valOrIndex(-1) { }
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
    uint64_t asap_num;
    uint64_t alap_num;
    qGate():qFunc(NULL), numArgs(0), asap_num(0), alap_num(0) { }
  };

  struct ArrParGates{
    uint64_t parallel_gates[NUM_QGATES];
  };

  struct allTSParallelism{
    uint64_t timesteps;
    vector<ArrParGates> gates;
    allTSParallelism(): timesteps(0){ }
  };  

  struct MaxInfo{ //TimeStepInfo
    uint64_t timesteps;
    uint64_t parallel_gates[NUM_QGATES];
    MaxInfo(): timesteps(0){ }
  };

  struct GetCriticalPath : public ModulePass {
    static char ID; // Pass identification

    string gate_name[NUM_QGATES];
    vector<qGateArg> tmpDepQbit;
    vector<Value*> vectQbit;

    int btCount; //backtrace count

    vector<qArgInfo> currTimeStep; //contains set of arguments operated on currently
    vector<string> currParallelFunc;
    MaxInfo maxParallelFactor; //overall max parallel factor
    map<string, int> gate_index;    
    vector<uint64_t> curr_parallel_ts; //vector of current timesteps that are parallel; used for comparing functions
    map<string, allTSParallelism > funcParallelFactor; //string is function name
    map<string, MaxInfo> funcMaxParallelFactor;

    map<string, map<int,uint64_t> > funcQbits; //qbits in current function
    map<string, map<int,uint64_t> > funcQbitsStart; //start ts of qbits in current function
    map<string, map<int,uint64_t> > funcQbitsHalf; //qbits in current function
    map<Function*, map<unsigned int, map<int,uint64_t> > > tableFuncQbits;
    map<Function*, map<unsigned int, map<int,uint64_t> > > tableFuncQbitsStart;
    map<string, unsigned int> funcArgs;

    map<uint64_t, vector<qGate> > tsGates;
    map<Function*, uint64_t> crit_path_f; 

    allTSParallelism currTS;

    bool isFirstMeas;
    bool isLeaf;

    uint64_t highestDelay;

    GetCriticalPath() : ModulePass(ID) {}

    bool backtraceOperand(Value* opd, int opOrIndex);
    void analyzeAllocInst(Function* F,Instruction* pinst);
    void analyzeCallInst(Function* F,Instruction* pinst);
    void getFunctionArguments(Function *F);

    void saveTableFuncQbits(Function* F);
    void saveTableFuncQbitsStart(Function* F);
    void print_tableFuncQbits();
    void print_tableFuncQbitsStart();
    void print_tsGates();

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
    void calc_critical_time(Function* F, qGate qg);        
    void print_funcQbits();
    void print_funcQbitsHalf();
    void print_qgate(qGate qg);
    void print_critical_info(string func);
    void calc_max_parallelism_statistic();
    uint64_t compute_max_ts_of_all_args(qGate qg);
    uint64_t compute_least_slack(Function* F, qGate qg, uint64_t tmax);

    void update_critical_info(string currFunc, uint64_t ts, string fname);

    void print_scheduled_gate(qGate qg, uint64_t ts);
    void addToTSGates(qGate qg, uint64_t ts);

    void init_funcQbitsHalf(uint64_t i);
    void gen_half_funcQbits(uint64_t ct, uint64_t hct);
    void schedule_alap_insts(uint64_t ct, uint64_t hct);
    void process_nonLeafALAP();

    uint64_t find_max_funcQbits();
    void memset_funcQbits(uint64_t val);
    void memset_funcQbitsHalf(uint64_t val);

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

  }; // End of struct GetCriticalPath
} // End of anonymous namespace



char GetCriticalPath::ID = 0;
static RegisterPass<GetCriticalPath> X("GetCriticalPath", "Get Critical Path");

void GetCriticalPath::getFunctionArguments(Function* F)
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

        map<int,uint64_t> tmpMap;
        tmpMap[-1] = 0; //add entry for entire array
        tmpMap[-2] = 0; //add entry for max
        funcQbits[argName]=tmpMap;

        map<int,uint64_t> tmpMapHalf;
        funcQbitsHalf[argName]=tmpMapHalf;

        funcArgs[argName] = argNum;

      }
      else if (elementType->isIntegerTy(1)){ //cbit*
        tmpQArg.isCbit = true;
        vectQbit.push_back(ait);
        funcArgs[argName] = argNum;
      }
    }
    else if (argType->isIntegerTy(16)){ //qbit
      tmpQArg.isQbit = true;
      vectQbit.push_back(ait);

      map<int,uint64_t> tmpMap;
      tmpMap[-1] = 0; //add entry for entire array
      tmpMap[-2] = 0; //add entry for max
      funcQbits[argName]=tmpMap;

      map<int,uint64_t> tmpMapHalf;
      funcQbitsHalf[argName]=tmpMapHalf;

      funcArgs[argName] = argNum;
    }
    else if (argType->isIntegerTy(1)){ //cbit
      tmpQArg.isCbit = true;
      vectQbit.push_back(ait);
      funcArgs[argName] = argNum;
    }

  }
}

bool GetCriticalPath::backtraceOperand(Value* opd, int opOrIndex)
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


void GetCriticalPath::analyzeAllocInst(Function* F, Instruction* pInst){
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

        map<int,uint64_t> tmpMap; //add qbit to funcQbits
        tmpMap[-1] = 0; //entry for entire array
        tmpMap[-2] = 0; //entry for max
        funcQbits[AI->getName()]=tmpMap;

        map<int,uint64_t> tmpMapHalf;
        funcQbitsHalf[AI->getName()]=tmpMapHalf;

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


void GetCriticalPath::init_critical_path_algo(Function* F){

  MaxInfo structMaxInfo;
  allTSParallelism vectGateInfo; //initialize an entry in the function map
  //ArrParGates tmpParGates;

  isFirstMeas = true;
  isLeaf = true;
  highestDelay = 0;

  //clear tsGates
  for(map<uint64_t, vector<qGate> >::iterator mit=tsGates.begin(); mit!=tsGates.end(); ++mit){
    (*mit).second.clear();
  }
  tsGates.clear();

  currTimeStep.clear(); //initialize critical time steps   

  curr_parallel_ts.clear();

  maxParallelFactor.timesteps = 0;   //initialize critical time steps

  for(int i=0; i< NUM_QGATES ; i++){
    maxParallelFactor.parallel_gates[i] = 0;
    structMaxInfo.parallel_gates[i] = 0;
    //tmpParGates.parallel_gates[i] = 0;
  }
  //vectGateInfo.gates.push_back(tmpParGates); //dummy first entry
  funcParallelFactor[F->getName().str()] = vectGateInfo;
  funcMaxParallelFactor[F->getName().str()] = structMaxInfo;
  currParallelFunc.clear();
}

void GetCriticalPath::print_funcQbits(){
  for(map<string, map<int,uint64_t> >::iterator mIter = funcQbits.begin(); mIter!=funcQbits.end(); ++mIter){
    errs() << "Var "<< (*mIter).first << " ---> ";
    for(map<int,uint64_t>::iterator indexIter  = (*mIter).second.begin(); indexIter!=(*mIter).second.end(); ++indexIter){
      errs() << (*indexIter).first << ":"<<(*indexIter).second<< "  ";
    }
    errs() << "\n";
  }
}

void GetCriticalPath::print_funcQbitsHalf(){
  errs() << "Printing funcQbitsHalf ---- \n";
  for(map<string, map<int,uint64_t> >::iterator mIter = funcQbitsHalf.begin(); mIter!=funcQbitsHalf.end(); ++mIter){
    errs() << "Var "<< (*mIter).first << " ---> ";
    for(map<int,uint64_t>::iterator indexIter  = (*mIter).second.begin(); indexIter!=(*mIter).second.end(); ++indexIter){
      errs() << (*indexIter).first << ":"<<(*indexIter).second<< "  ";
    }
    errs() << "\n";
  }
}

void GetCriticalPath::print_qgate(qGate qg){
  errs() << "--Gate: " << qg.qFunc->getName() << " : ";
  for(int i=0;i<qg.numArgs;i++){
    errs() << qg.args[i].name << " idx=" << qg.args[i].index 
      << ", "  ;
  }
  errs() << "ASAP=" << qg.asap_num << " ALAP=" << qg.alap_num;
  errs() << "\n";
}


void GetCriticalPath::print_critical_info(string func){
  map<string, allTSParallelism>::iterator fitr = funcParallelFactor.find(func);
  assert(fitr!=funcParallelFactor.end() && "Func not found in funcParFac");
  errs() << "Timesteps = " << (*fitr).second.timesteps << "\n";
  for(unsigned int i = 0; i<(*fitr).second.gates.size(); i++){
    errs() << i << " :";
    for(int k=0;k<NUM_QGATES;k++)
      errs() << " " << (*fitr).second.gates[i].parallel_gates[k];
    errs() << "\n";
  }
}

void GetCriticalPath::update_critical_info(string currFunc, uint64_t ts, string fname){
  map<string, allTSParallelism>::iterator fitr = funcParallelFactor.find(fname);
  map<string, allTSParallelism>::iterator citr = funcParallelFactor.find(currFunc);
  assert(fitr!=funcParallelFactor.end() && "parallel func not found in funcParallelFactor");

  assert(citr!=funcParallelFactor.end() && "curr func not found in funcParallelFactor");

  unsigned currTSsize = (*citr).second.gates.size();
  unsigned newTSsize = (*fitr).second.gates.size();

  //errs() << "ts = " << ts << " currsize = " << currTSsize << " newsize = "<<newTSsize <<"\n";

  if(ts+newTSsize > currTSsize)
    (*citr).second.timesteps = ts+newTSsize;

  if(ts + newTSsize <= currTSsize){
    for(unsigned i = 0; i<newTSsize; i++){
      for(int k=0; k<NUM_QGATES;k++){
        (*citr).second.gates[ts+i].parallel_gates[k] += (*fitr).second.gates[i].parallel_gates[k];            
      }
    }
  }
  else{
    for(unsigned i = 0; i<currTSsize-ts; i++){
      //errs() << "i = " << i << " ts+i=" << ts+i << "\n";
      for(int k=0; k<NUM_QGATES;k++)
        (*citr).second.gates[ts+i].parallel_gates[k] += (*fitr).second.gates[i].parallel_gates[k];            
    }

    for(unsigned i = currTSsize-ts; i<newTSsize; i++){
      //errs() << "i = " << i << "\n";
      ArrParGates tmpParGates;
      for(int k=0; k<NUM_QGATES;k++){
        tmpParGates.parallel_gates[k] = (*fitr).second.gates[i].parallel_gates[k];                              
      }
      (*citr).second.gates.push_back(tmpParGates); 
    }

  }

  //print_critical_info(currFunc);
}


uint64_t GetCriticalPath::find_max_funcQbits(){
  uint64_t max_timesteps = 0;
  for(map<string, map<int,uint64_t> >::iterator mIter = funcQbits.begin(); mIter!=funcQbits.end(); ++mIter){
    map<int,uint64_t>::iterator arrIter = (*mIter).second.find(-2);
    if((*arrIter).second > max_timesteps)
      max_timesteps = (*arrIter).second;
  }

  return max_timesteps;

}

void GetCriticalPath::memset_funcQbits(uint64_t val){
  for(map<string, map<int,uint64_t> >::iterator mIter = funcQbits.begin(); mIter!=funcQbits.end(); ++mIter){
    for(map<int,uint64_t>::iterator arrIter = (*mIter).second.begin(); arrIter!=(*mIter).second.end();++arrIter)
      (*arrIter).second = val;
  }
}

void GetCriticalPath::memset_funcQbitsHalf(uint64_t val){
  for(map<string, map<int,uint64_t> >::iterator mIter = funcQbitsHalf.begin(); mIter!=funcQbitsHalf.end(); ++mIter){
    for(map<int,uint64_t>::iterator arrIter = (*mIter).second.begin(); arrIter!=(*mIter).second.end();++arrIter)
      (*arrIter).second = val;
  }
}

void GetCriticalPath::print_scheduled_gate(qGate qg, uint64_t ts){
  string tmpGateName = qg.qFunc->getName();
  if(tmpGateName.find("llvm.")!=string::npos)
    tmpGateName = tmpGateName.substr(5);
  errs() << ts << " : " << tmpGateName;
  for(int i = 0; i<qg.numArgs; i++){
    //if(qg.args[i].index != -1)
    errs() << " " << qg.args[i].name << qg.args[i].index;
  }

  errs() << "\n";
}

void GetCriticalPath::print_tableFuncQbits(){
  for(map<Function*, map<unsigned int, map<int, uint64_t> > >::iterator m1 = tableFuncQbits.begin(); m1!=tableFuncQbits.end(); ++m1){
    errs() << "Function " << (*m1).first->getName() << " \n  ";
    for(map<unsigned int, map<int, uint64_t> >::iterator m2 = (*m1).second.begin(); m2!=(*m1).second.end(); ++m2){
      errs() << "\tArg# "<< (*m2).first << " -- ";
      for(map<int, uint64_t>::iterator m3 = (*m2).second.begin(); m3!=(*m2).second.end(); ++m3){
        errs() << " ; " << (*m3).first << " : " << (*m3).second;
      }
      errs() << "\n";
    }
  }
}

void GetCriticalPath::print_tableFuncQbitsStart(){
  errs() << "Printing tableFuncQbitsStart\n";
  for(map<Function*, map<unsigned int, map<int, uint64_t> > >::iterator m1 = tableFuncQbitsStart.begin(); m1!=tableFuncQbitsStart.end(); ++m1){
    errs() << "Function " << (*m1).first->getName() << " \n  ";
    for(map<unsigned int, map<int, uint64_t> >::iterator m2 = (*m1).second.begin(); m2!=(*m1).second.end(); ++m2){
      errs() << "\tArg# "<< (*m2).first << " -- ";
      for(map<int, uint64_t>::iterator m3 = (*m2).second.begin(); m3!=(*m2).second.end(); ++m3){
        errs() << " ; " << (*m3).first << " : " << (*m3).second;
      }
      errs() << "\n";
    }
  }
}

void GetCriticalPath::calc_max_parallelism_statistic()
{
  map<string, allTSParallelism>::iterator fitr = funcParallelFactor.find("main");
  assert(fitr!=funcParallelFactor.end() && "Func not found in funcParFac");
  errs() << "Timesteps = " << (*fitr).second.timesteps << "\n";

  uint64_t max_par = 0;
  uint64_t ts_par = 0;

  for(unsigned int i = 0; i<(*fitr).second.gates.size(); i++){

    uint64_t nonZero = 0;

    for(int k=0;k<NUM_QGATES-1;k++){ //skip the 'All' entry
      //count non-zero entries
      if((*fitr).second.gates[i].parallel_gates[k] > 0)
        nonZero++;
    }

    if(nonZero>max_par){
      max_par = nonZero;
      ts_par = i;
    }

  }

  errs() << "Max parallelism in types of gates = " << max_par << " in TS: " << ts_par << "\n";

}

void GetCriticalPath::print_tsGates()
{
  for(map<uint64_t, vector<qGate> >::iterator mit = tsGates.begin(); mit!=tsGates.end(); ++mit){
    errs() << "TS#"<<(*mit).first << " --> ";
    for(vector<qGate>::iterator vit = (*mit).second.begin(); vit!=(*mit).second.end();++vit)
      print_qgate(*vit);
  }

}

void GetCriticalPath::addToTSGates(qGate qg, uint64_t ts)
{
  if(isLeaf){
    //add to tsGates
    map<uint64_t, vector<qGate> >::iterator git = tsGates.find(ts);
    if(git!=tsGates.end()){
      (*git).second.push_back(qg); //add to vector
    }
    else{
      vector<qGate> tmpVect;
      tmpVect.push_back(qg);
      tsGates[ts] = tmpVect; //add to map	
    } 
  } //isLeaf
}

uint64_t GetCriticalPath::compute_max_ts_of_all_args(qGate qg)
{

  uint64_t max_ts_of_all_args = 0;

  //find last timestep for all arguments of qgate
  for(int i=0;i<qg.numArgs; i++){
    map<string, map<int,uint64_t> >::iterator mIter = funcQbits.find(qg.args[i].name);
    assert(mIter!=funcQbits.end()); //should already have an entry for the name of the qbit

    int argIndex = qg.args[i].index;

    //find the index of argument in the map<int,int>
    if(argIndex == -1) //operation on entire array
    {
      //find max for the array
      map<int,uint64_t>::iterator indexIter = (*mIter).second.find(-2);
      if((*indexIter).second > max_ts_of_all_args)
        max_ts_of_all_args = (*indexIter).second;	  
    }
    else
    {
      map<int,uint64_t>::iterator indexIter = (*mIter).second.find(argIndex);
      if(indexIter!=(*mIter).second.end()){
        if((*indexIter).second > max_ts_of_all_args)
          max_ts_of_all_args = (*indexIter).second;
      }
      else{
        //find the value for entire array
        map<int,uint64_t>::iterator fullArrayIndexIter = (*mIter).second.find(-1);

        ((*mIter).second)[argIndex] = (*fullArrayIndexIter).second;
        if((*fullArrayIndexIter).second > max_ts_of_all_args)
          max_ts_of_all_args = (*fullArrayIndexIter).second;
        //((*mIter).second)[argIndex] = 0;
      }
    }
  }

  if(debugGetCriticalPath){
    errs() << "Before Scheduling: \n";
    print_funcQbits();
  }

  //errs() << "Max timestep for all args = " << max_ts_of_all_args << "\n";

  return max_ts_of_all_args;

}

uint64_t GetCriticalPath::compute_least_slack(Function* F, qGate qg, uint64_t tmax){

  //errs() << "In compute least \n";

  //print_funcQbits();
  //print_qgate(qg);

  vector<uint64_t> endsAt; //currently ends at
  vector<uint64_t> startsAt; //supposed to start at

  for(int i=0;i<qg.numArgs; i++){
    startsAt.push_back(0);
    endsAt.push_back(0);
  }

  //compute startsAt
  //print_tableFuncQbitsStart();

  map<Function*, map<unsigned int, map<int, uint64_t> > >::iterator tableIt = tableFuncQbitsStart.find(qg.qFunc);
  assert(tableIt!=tableFuncQbitsStart.end() && "No previous entry for this function");

  for(int i=0;i<qg.numArgs; i++){    
    map<unsigned int, map<int, uint64_t> >::iterator entryIt = (*tableIt).second.find(i);
    if(entryIt!=(*tableIt).second.end()){

      //differentiate for qbit and qbit*

      if(qg.args[i].index == -1){ //qbit*

        //errs() << "Array\n";

        map<int, uint64_t>::iterator lookUpIt = (*entryIt).second.find(-2);
        startsAt[i] = tmax + (*lookUpIt).second;			
      }
      else{ //qbit was passed
        //errs() << "i = " << i << " Qbit\n";
        //errs() << "index = " << qg.args[i].index << " Qbit\n";
        //take the 0th entry and add that to the index entry
        map<int, uint64_t>::iterator lookUpQbitIt = (*entryIt).second.find(0);
        assert(lookUpQbitIt!=(*entryIt).second.end() && "arg index not found in tablefuncqbitshalf"); //there exists entry for reqd index in the func table of called func
        startsAt[i] = tmax + (*lookUpQbitIt).second;

      }
    }
    else{
      assert(false && "entry not found in tableFuncStart");
    }
  }

  //compute endsAt
  //find last timestep for all arguments of qgate
  for(int i=0;i<qg.numArgs; i++){
    map<string, map<int,uint64_t> >::iterator mIter = funcQbits.find(qg.args[i].name);
    assert(mIter!=funcQbits.end()); //should already have an entry for the name of the qbit

    int argIndex = qg.args[i].index;

    //find the index of argument in the map<int,int>
    if(argIndex == -1) //operation on entire array
    {
      //find max for the array
      map<int,uint64_t>::iterator indexIter = (*mIter).second.find(-2);
      endsAt[i] = (*indexIter).second;	  
    }
    else
    {
      map<int,uint64_t>::iterator indexIter = (*mIter).second.find(argIndex);
      if(indexIter!=(*mIter).second.end()){
        endsAt[i] = (*indexIter).second;
      }
      else{
        //find the value for entire array
        map<int,uint64_t>::iterator fullArrayIndexIter = (*mIter).second.find(-1);

        ((*mIter).second)[argIndex] = (*fullArrayIndexIter).second;
        endsAt[i] = (*fullArrayIndexIter).second;
      }
    }
  }

  //print_tableFuncQbits();
  //print_tableFuncQbitsStart();

  //for(int i=0;i<qg.numArgs; i++){
  //errs() << "Arg#"<<i<<" End: " << endsAt[i] << " St: " << startsAt[i] << "\n";
  //}

  //compute least slack
  uint64_t leastslack = 0;
  if(qg.numArgs > 0){
    leastslack = startsAt[0] - endsAt[0];
    for(int i=1;i<qg.numArgs; i++){
      if(startsAt[i]-endsAt[i] < leastslack)
        leastslack = startsAt[i]-endsAt[i];
    }
  }

  //errs() << "LS = " << leastslack << "\n";
  if(leastslack > tmax) leastslack = 1;

  return leastslack;  
}

void GetCriticalPath::calc_critical_time(Function* F, qGate qg){
  string fname = qg.qFunc->getName();

  //print_qgate(qg);

  if(isFirstMeas && (fname == "llvm.MeasX" || fname == "llvm.MeasZ")){
    uint64_t maxFQ = find_max_funcQbits();
    memset_funcQbits(maxFQ);

    //--print_scheduled_gate(qg,maxFQ+1);
    addToTSGates(qg,maxFQ+1);

    map<string, map<int,uint64_t> >::iterator mIter = funcQbits.find(qg.args[0].name);

    int argIndex = qg.args[0].index;

    //update the timestep number for that argument
    map<int,uint64_t>::iterator indexIter = (*mIter).second.find(argIndex);
    (*indexIter).second =  maxFQ + 1;

    //update -2 entry for the array, i.e. max ts over all indices
    indexIter = (*mIter).second.find(-2);
    (*indexIter).second = maxFQ + 1;

    //update_critical_info(F->getName().str(), maxFQ, qg.qFunc->getName(), qg.angle);   
    isFirstMeas = false;
  }
  else{ //not a Meas gate

    uint64_t max_ts_of_all_args = compute_max_ts_of_all_args(qg);;

    //errs() << "Max timestep for all args = " << max_ts_of_all_args << "\n";


    if(fname.find("llvm.")!=string::npos){ //is intrinsic

      //schedule gate in max_ts_of_all_args + 1th timestep
      //--print_scheduled_gate(qg,max_ts_of_all_args+1);
      addToTSGates(qg,max_ts_of_all_args+1);
      qg.asap_num = max_ts_of_all_args+1;

      //find last timestep for all arguments of qgate
      for(int i=0;i<qg.numArgs; i++){
        map<string, map<int,uint64_t> >::iterator mIter = funcQbits.find(qg.args[i].name);

        int argIndex = qg.args[i].index;

        if(argIndex == -1){
          for(map<int,uint64_t>::iterator entryIter = (*mIter).second.begin(); entryIter!=(*mIter).second.end();++entryIter){
            (*entryIter).second = max_ts_of_all_args + 1;
          }

        }
        else{
          //update the timestep number for that argument
          map<int,uint64_t>::iterator indexIter = (*mIter).second.find(argIndex);
          (*indexIter).second =  max_ts_of_all_args + 1;

          //update -2 entry for the array, i.e. max ts over all indices
          indexIter = (*mIter).second.find(-2);
          if((*indexIter).second < max_ts_of_all_args + 1)
            (*indexIter).second = max_ts_of_all_args + 1;
        }  
      }
    } //intrinsic func

    else{ //not an intrinsic function

      //not an intrinsic function
      //errs() << "Non intrinsic \n";
      isLeaf = false;

      //errs() << "Max ts = " << max_ts_of_all_args << "\n";

      //for all operands of newly called function, compute slack
      uint64_t least_slack = compute_least_slack(F, qg, max_ts_of_all_args);

      //errs() << "Least slack = " << least_slack << "\n";

      //start scheduling from max_ts_of_all_args - least_slack + 1

      //small code to ensure that any ancilla processing that does not fall within this function's critical path get a cycle in the schedule.
      map<Function*, uint64_t>::iterator delayIt = crit_path_f.find(qg.qFunc);
      assert(delayIt != crit_path_f.end() && "Func not found in critpathf");
      uint64_t tmpDelay = max_ts_of_all_args + (*delayIt).second - least_slack + 1; 
      if(tmpDelay > highestDelay) highestDelay = tmpDelay;

      //check tableFuncQbits for values to update with
      map<Function*, map<unsigned int, map<int, uint64_t> > >::iterator tableIt = tableFuncQbits.find(qg.qFunc);
      assert(tableIt!=tableFuncQbits.end() && "No previous entry for this function");

      for(int i=0;i<qg.numArgs; i++){
        map<string, map<int,uint64_t> >::iterator mIter = funcQbits.find(qg.args[i].name);
        assert(mIter!=funcQbits.end()); //should already have an entry for the name of the qbit

        map<unsigned int, map<int, uint64_t> >::iterator entryIt = (*tableIt).second.find(i);
        if(entryIt!=(*tableIt).second.end()){

          //differentiate for qbit and qbit*

          if(qg.args[i].index == -1){ //qbit*
            for(map<int, uint64_t>::iterator indexIt = (*entryIt).second.begin(); indexIt!=(*entryIt).second.end(); ++indexIt){
              map<int,uint64_t>::iterator currEntryIt = (*mIter).second.find((*indexIt).first);
              if(currEntryIt!=(*mIter).second.end())
                (*currEntryIt).second = max_ts_of_all_args + (*indexIt).second - least_slack + 1;
              else
                (*mIter).second[(*indexIt).first] = max_ts_of_all_args + (*indexIt).second - least_slack + 1;
            }
          }
          else{ //qbit was passed
            //take the 0th entry and add that to the index entry
            map<int, uint64_t>::iterator lookUpQbitIt = (*entryIt).second.find(0);
            assert(lookUpQbitIt!=(*entryIt).second.end()); //there exists entry for 0 in the func table of called func
            map<int, uint64_t>::iterator currEntryIt = (*mIter).second.find(qg.args[i].index);
            if(currEntryIt!=(*mIter).second.end())
              (*currEntryIt).second = max_ts_of_all_args + (*lookUpQbitIt).second - least_slack + 1;
            else
              (*mIter).second[qg.args[i].index] = max_ts_of_all_args + (*lookUpQbitIt).second - least_slack + 1;

            //update -2 entry for the array, i.e. max ts over all indices
            currEntryIt = (*mIter).second.find(-2);
            if((*currEntryIt).second < max_ts_of_all_args + (*lookUpQbitIt).second)
              (*currEntryIt).second = max_ts_of_all_args + (*lookUpQbitIt).second- least_slack + 1;	    
          }
        } 
      }
    } //not intrinsic Gate
    //update_critical_info(F->getName().str(), max_ts_of_all_args, qg.qFunc->getName(), qg.angle);   
  } // not a MeasX gate

  if(debugGetCriticalPath)
  {   
    errs() << "\nAfter Scheduling: \n";
    print_funcQbits();
    print_qgate(qg);
    errs() << "\n";
  }

}

void GetCriticalPath::analyzeCallInst(Function* F, Instruction* pInst){
  if(CallInst *CI = dyn_cast<CallInst>(pInst))
  {      
    if(debugGetCriticalPath)
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


      //if(tmpQGateArg.isQbit || tmpQGateArg.isCbit){
      if(tmpQGateArg.isQbit){
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
      if(debugGetCriticalPath)
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


  void GetCriticalPath::saveTableFuncQbits(Function* F){
    map<unsigned int, map<int, uint64_t> > tmpFuncQbitsMap;

    for(map<string, map<int, uint64_t> >::iterator mapIt = funcQbits.begin(); mapIt!=funcQbits.end(); ++mapIt){
      map<string, unsigned int>::iterator argIt = funcArgs.find((*mapIt).first);
      if(argIt!=funcArgs.end()){
        unsigned int argNum = (*argIt).second;
        tmpFuncQbitsMap[argNum] = (*mapIt).second;
      }
    }
    tableFuncQbits[F] = tmpFuncQbitsMap;
  }


  void GetCriticalPath::saveTableFuncQbitsStart(Function* F){
    map<unsigned int, map<int, uint64_t> > tmpFuncQbitsMap;

    for(map<string, map<int, uint64_t> >::iterator mapIt = funcQbitsHalf.begin(); mapIt!=funcQbitsHalf.end(); ++mapIt){
      map<string, unsigned int>::iterator argIt = funcArgs.find((*mapIt).first);
      if(argIt!=funcArgs.end()){
        unsigned int argNum = (*argIt).second;
        tmpFuncQbitsMap[argNum] = (*mapIt).second;
      }
    }
    tableFuncQbitsStart[F] = tmpFuncQbitsMap;
  }


  void GetCriticalPath::init_funcQbitsHalf(uint64_t i){
    //copy all entries of funcQbit
    //print_funcQbitsHalf();

    for(map<string, map<int,uint64_t> >::iterator inItr = funcQbits.begin(); inItr!=funcQbits.end(); ++inItr){
      //find string in funcQbitsHalf
      //errs() << "Looking for string " << (*inItr).first << "\n";
      map<string, map<int,uint64_t> >::iterator outItr = funcQbitsHalf.find((*inItr).first);
      assert(outItr != funcQbitsHalf.end() && "funcQbitsHalf does not have entry");

      for(map<int,uint64_t>::iterator in2Itr = (*inItr).second.begin(); in2Itr!=(*inItr).second.end(); ++in2Itr){
        (*outItr).second[(*in2Itr).first] = i;
      }
    }

  }

  void GetCriticalPath::gen_half_funcQbits(uint64_t ct, uint64_t hct){
    init_funcQbitsHalf(ct);

    for(uint64_t i=hct+1; i<ct; i++){
      map<uint64_t, vector<qGate> >::iterator mit=tsGates.find(i);
      for(vector<qGate>::iterator vit = (*mit).second.begin(); vit!=(*mit).second.end(); ++vit){
        //iterate over the args
        for(int j=0; j<(*vit).numArgs; j++){
          string argName = (*vit).args[j].name;
          int argIndex = (*vit).args[j].index;
          map<string, map<int,uint64_t> >::iterator fit = funcQbitsHalf.find(argName);
          assert(fit!=funcQbitsHalf.end() && "arg not found in funQbitsHalf");

          assert(argIndex != -1 && "argindex is -1");

          map<int,uint64_t>::iterator mfit = (*fit).second.find(argIndex);
          assert(mfit != (*fit).second.end() && "arg index not found in funcQbitsHalf");

          if(i < (*mfit).second){ //gate scheduled in TS=i
            (*mfit).second = i;	  
          }

        }

      }

    }
    //print_funcQbitsHalf();

  }

  void GetCriticalPath::schedule_alap_insts(uint64_t ct, uint64_t hct){
    //errs() << "Scheduling insts ALAP, starting from ts: " << hct << "\n";

    assert(hct!=0 && "ZERO hct");

    for(uint64_t i=hct; i>=1; i--){
      map<uint64_t, vector<qGate> >::iterator mit=tsGates.find(i);
      for(vector<qGate>::iterator vit = (*mit).second.begin(); vit!=(*mit).second.end(); ++vit){
        //iterate over the args and get ALAP num
        uint64_t min_ts_of_all_args = ct;

        for(int j=0; j<(*vit).numArgs; j++){
          string argName = (*vit).args[j].name;
          int argIndex = (*vit).args[j].index;

          map<string, map<int,uint64_t> >::iterator fit = funcQbitsHalf.find(argName);
          assert(fit!=funcQbitsHalf.end() && "arg not found in funQbitsHalf");
          assert(argIndex != -1 && "argIndex = -1 in sched_alap");

          //if(argIndex == -1) //operation on entire array
          //{
          //find min for the array
          //map<int,uint64_t>::iterator indexIter = (*fit).second.find(-2);
          //  if((*indexIter).second < min_ts_of_all_args)
          //    min_ts_of_all_args = (*indexIter).second;	  
          //}
          //else
          //{
          map<int,uint64_t>::iterator mfit = (*fit).second.find(argIndex);
          assert(mfit != (*fit).second.end() && "arg index not found in funcQbitsHalf");

          if((*mfit).second < min_ts_of_all_args)
            min_ts_of_all_args = (*mfit).second;
          //}
        }
        //print_qgate((*vit));
        //errs() << "min_ts_of_all_args = " << min_ts_of_all_args << "\n";
        (*vit).alap_num = min_ts_of_all_args-1;

        //schedule gate in min_ts_of_all_args - 1; update funcQbitsHalf
        //--print_scheduled_gate((*vit),min_ts_of_all_args-1);

        //find last timestep for all arguments of qgate
        for(int j=0;j<(*vit).numArgs; j++){
          map<string, map<int,uint64_t> >::iterator mIter = funcQbitsHalf.find((*vit).args[j].name);

          int argIndex = (*vit).args[j].index;

          if(argIndex == -1){
            for(map<int,uint64_t>::iterator entryIter = (*mIter).second.begin(); entryIter!=(*mIter).second.end();++entryIter){
              (*entryIter).second = min_ts_of_all_args - 1;
            }

          }
          else{
            //update the timestep number for that argument
            map<int,uint64_t>::iterator indexIter = (*mIter).second.find(argIndex);
            (*indexIter).second =  min_ts_of_all_args - 1;

            //update -2 entry for the array, i.e. min ts over all indices
            indexIter = (*mIter).second.find(-2);
            if((*indexIter).second > min_ts_of_all_args - 1)
              (*indexIter).second = min_ts_of_all_args - 1;
          }  
        }
      }
    }
    //print_funcQbitsHalf();
  }

  void GetCriticalPath::process_nonLeafALAP(){
    //copy entries from funcQbits and set values to 1 => zero slack
    init_funcQbitsHalf(1);

  }

  void GetCriticalPath::CountCriticalFunctionResources (Function *F) {
    // Traverse instruction by instruction
    init_critical_path_algo(F);


    for (inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E; ++I) {
      Instruction *Inst = &*I;                            // Grab pointer to instruction reference
      analyzeAllocInst(F,Inst);          
      analyzeCallInst(F,Inst);	
    }

    if(isLeaf){
      //print_tsGates();

      //do ALAP processing
      uint64_t critTimeF = max(find_max_funcQbits(), highestDelay);
      uint64_t halfCritTime = critTimeF/2;

      //errs() << "from FuncQbits = " << find_max_funcQbits();
      //errs() << " highestDelay = " << highestDelay << "\n";
      //errs() << "crittimeF=" << critTimeF << "\n";

      if(critTimeF > 3){
        //generate_half_funcQbits_table
        gen_half_funcQbits(critTimeF,halfCritTime);

        //schedule instrs in first half ALAP
        schedule_alap_insts(critTimeF,halfCritTime);
      }
      else
        init_funcQbitsHalf(1);


      //print_funcQbits();
      //print_funcQbitsHalf();

    }
    else{
      //dummy info for ALAP
      process_nonLeafALAP();

    }


    saveTableFuncQbits(F);
    saveTableFuncQbitsStart(F);
    //print_tableFuncQbits();
    //print_tableFuncQbitsStart();

  }


  void GetCriticalPath::init_gates_as_functions(){
    for(int  i =0; i< NUM_QGATES ; i++){
      string gName = gate_name[i];
      string fName = "llvm.";
      fName.append(gName);

      allTSParallelism tmp_info;
      MaxInfo tmp_max_info;

      tmp_info.timesteps = 1;
      tmp_max_info.timesteps = 1;

      ArrParGates tmp_gate_info;
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


  bool GetCriticalPath::runOnModule (Module &M) {
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
          //errs() << "\nFunction: " << F->getName() << "\n";      

          funcQbits.clear();
          funcQbitsHalf.clear();
          funcArgs.clear();

          getFunctionArguments(F);

          // count the critical resources for this function
          CountCriticalFunctionResources(F);

          crit_path_f[F] = max(find_max_funcQbits(), highestDelay);
          //if(F->getName() == "main")
          //errs() << F->getName() << ": " << "Critical Path Length : " << find_max_funcQbits() << "\n";
          errs() << F->getName() << " " << max(find_max_funcQbits(),highestDelay) << " isLeaf= " << isLeaf <<"\n";	
        }
        else{
          if(debugGetCriticalPath)
            errs() << "WARNING: Ignoring external node or dummy function.\n";
        }
      }
    }
    //print_tableFuncQbits();
    //print_critical_info("main");

    //calc_max_parallelism_statistic();

    return false;
  } // End runOnModule
