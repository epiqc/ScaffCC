//===----------------- GenSIMDSched.cpp ----------------------===//
// This file implements the Scaffold Pass of counting the number 
//  of critical timesteps and gate parallelism in program
//  in callgraph post-order.
//
//        This file was created by Scaffold Compiler Working Group
// Fine-grained list scheduling for leaf modules
// Coarse-grained scheduling for non-leaf modules
// Get T gate proportion within schedule length
// Cleaned up the code
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "GenSIMDSched"
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
#include "llvm/Support/CommandLine.h"


using namespace llvm;
using namespace std;


static cl::opt<unsigned>
RES_CONSTRAINT("simd-kconstraint", cl::init(10), cl::Hidden,
  cl::desc("k in SIMD-k Resource Constrained Scheduling"));

static cl::opt<unsigned>
DATA_CONSTRAINT("simd-dconstraint", cl::init(1024), cl::Hidden,
  cl::desc("k in SIMD-k Resource Constrained Scheduling"));

#define MAX_RES_CONSTRAINT 2000 
#define SSCHED_THRESH 10000000

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

bool debugGenSIMDSched = false;

namespace {

  typedef pair<Instruction*, uint64_t> InstPri; //instpriority
  
  struct CompareInstPriByValue {
    bool operator() (const InstPri& a, const InstPri& b) const {
      return a.second < b.second;
    };
  };

  struct modularInfo{
    uint64_t width;
    uint64_t length;
    //uint64_t ancilla;
    uint64_t tgates;
    uint64_t tgates_ub;
    uint64_t tgates_par;
    uint64_t tgates_par_ub;
    modularInfo(): width(0), length(0), tgates(0), tgates_ub(0), tgates_par(0), tgates_par_ub(0) {}
  };


  struct qGateArg{ //arguments to qgate calls
    Value* argPtr;
    int argNum;
    bool isQbit;
    bool isCbit;
    bool isUndef;
    bool isPtr;
    int valOrIndex; //Value if not Qbit, Index if Qbit & not a Ptr
    double angle;
    qGateArg(): argPtr(NULL), argNum(-1), isQbit(false), isCbit(false), isUndef(false), isPtr(false), valOrIndex(-1), angle(0.0){ }
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
  double angle;
  qGate():qFunc(NULL), numArgs(0), angle(0.0) { }
};

  struct ArrParGates{
    int typeOfGate[MAX_RES_CONSTRAINT];
    uint64_t numGates[MAX_RES_CONSTRAINT];
  };

  struct GenSIMDSched : public ModulePass {
    static char ID; // Pass identification
    
    string gate_name[NUM_QGATES];
    vector<qGateArg> tmpDepQbit;
    vector<Value*> vectQbit;
    
    int btCount; //backtrace count

    modularInfo totalSched;
    modularInfo currSched;

    map<string, int> gate_index;    

    map<string, map<int,uint64_t> > funcQbits; //qbits in current function
    map<Function*, map<unsigned int, map<int,uint64_t> > > tableFuncQbits;
    map<string, unsigned int> funcArgs;

    vector<ArrParGates> currArrParGates;

    map<Instruction*, qGate> mapInstSet;
    vector<InstPri> priorityVector;

    vector<Instruction*> vectCalls;

    map<Function*, modularInfo> funcInfo;
    vector<Function*> isLeaf;
    bool hasPrimitivesOnly;

    bool isFirstMeas;

    GenSIMDSched() : ModulePass(ID) {}
    
    // Get arguments from operation
    bool backtraceOperand(Value* opd, int opOrIndex);
    // 
    void analyzeAllocInst(Function* F,Instruction* pinst);
    void analyzeCallInst(Function* F,Instruction* pinst);
    void getFunctionArguments(Function *F);
    
    void saveTableFuncQbits(Function* F);
    void print_tableFuncQbits();
    void print_parallelism(Function* F);
    void print_ArrParGates();
    void cleanupCurrArrParGates();
    bool checkIfIntrinsic(Function* CF);

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
    void calc_critical_time(Function* F, qGate qg, bool isLeafFunc);        
    void print_funcQbits();
    void print_qgate(qGate qg);
    void print_critical_info(); 

    void print_scheduled_gate(qGate qg, uint64_t ts);

    uint64_t find_max_funcQbits();
    void memset_funcQbits(uint64_t val);
    uint64_t get_ts_to_schedule(Function* F, uint64_t ts, Function* funcToSched, uint64_t& first_step);
    uint64_t get_ts_to_schedule_leaf(Function* F, uint64_t ts, Function* funcToSched, uint64_t& first_step);

    void save_blackbox_info(Function* F);
    uint64_t calc_critical_time_unbounded(Function* F, qGate qg);        

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
    
    uint64_t getNumCritSteps(Function* F){
      map<Function*, modularInfo>::iterator mf = funcInfo.find(F);
      assert(mf!=funcInfo.end());
      return (mf->second.length);
    }

    void CountCriticalFunctionResources (Function *F);
    
    bool runOnModule (Module &M);    
    
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();  
      AU.addRequired<CallGraph>();    
    }
    
  }; // End of struct GenSIMDSched
} // End of anonymous namespace



char GenSIMDSched::ID = 0;
static RegisterPass<GenSIMDSched> X("GenSIMDSchedule", "Generate SIMD Schedule");

void GenSIMDSched::getFunctionArguments(Function* F)
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
          funcArgs[argName] = argNum;
      }
      else if (argType->isIntegerTy(1)){ //cbit
        tmpQArg.isCbit = true;
        vectQbit.push_back(ait);
          funcArgs[argName] = argNum;
      }
      
    }
}

bool GenSIMDSched::backtraceOperand(Value* opd, int opOrIndex)
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


void GenSIMDSched::analyzeAllocInst(Function* F, Instruction* pInst){
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
        tmpMap[-1] = 0; //entry for entire array ops
        tmpMap[-2] = 0; //entry for max
        funcQbits[AI->getName()]=tmpMap;

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


void GenSIMDSched::init_critical_path_algo(Function* F){

  currSched.width = 0;
  currSched.length = 0;
  currSched.tgates = 0;
  currSched.tgates_ub = 0;
  currSched.tgates_par = 0;
  currSched.tgates_par_ub = 0;

  totalSched.width = 0;
  totalSched.length = 0;
  totalSched.tgates = 0;
  totalSched.tgates_ub = 0;
  totalSched.tgates_par = 0;
  totalSched.tgates_par_ub = 0;

  isFirstMeas = true;

  hasPrimitivesOnly = true;
}

void GenSIMDSched::print_funcQbits(){
  for(map<string, map<int,uint64_t> >::iterator mIter = funcQbits.begin(); mIter!=funcQbits.end(); ++mIter){
    errs() << "Var "<< (*mIter).first << " ---> ";
    for(map<int,uint64_t>::iterator indexIter  = (*mIter).second.begin(); indexIter!=(*mIter).second.end(); ++indexIter){
      errs() << (*indexIter).first << ":"<<(*indexIter).second<< "  ";
    }
    errs() << "\n";
  }
}

void GenSIMDSched::print_ArrParGates(){
  errs() << "Printing ArrParGate Vector \n";
    int j = 0;
    for(vector<ArrParGates>::iterator vit = currArrParGates.begin(); vit!=currArrParGates.end(); ++vit, j++){
      errs() << j << " -- ";
      for(unsigned int i=0;i<RES_CONSTRAINT;i++)
        errs() << (*vit).typeOfGate[i] << " : " << (*vit).numGates[i] << " ; ";
      errs() << "\n";
    }
  
}

void GenSIMDSched::print_qgate(qGate qg){
  errs() << qg.qFunc->getName() << " : ";
  for(int i=0;i<qg.numArgs;i++){
    errs() << qg.args[i].name << qg.args[i].index << ", "  ;
  }
  errs() << "\n";
}

uint64_t GenSIMDSched::get_ts_to_schedule(Function* F, uint64_t ts, Function* funcToSched, uint64_t& first_step){
  //F is non-leaf. Treat all incoming function as blackboxes

  //errs() << "\n funcTOSched = " << funcToSched->getName() << "\n";

  //print_funcQbits();
  //print_ArrParGates();

  uint64_t Win = 0;
  uint64_t Lin = 0;
  uint64_t Tin = 0;
  uint64_t TinUB = 0;
  uint64_t TinPar = 0;
  uint64_t TinParUB = 0;

  if(checkIfIntrinsic(funcToSched)){
    Win = 1;
    Lin = 1;

    if(funcToSched->getIntrinsicID() == Intrinsic::T
       || funcToSched->getIntrinsicID() == Intrinsic::Tdag){
      //errs() << "Found T or Tdag \n";
      Lin = 100; //cost of T gate
      Tin = 1;
      TinUB = 1;
      TinPar = 1;
      TinParUB = 1;
    }
    else{
      Tin = 0;
      TinUB = 0;
      TinPar = 0;
      TinParUB = 0;
    }
  }
  
  else{
    map<Function*, modularInfo>::iterator fin = funcInfo.find(funcToSched);
    assert(fin!=funcInfo.end() && "Func not found in funcInfo");
        
    Win = (*fin).second.width; //width for incoming func
    Lin = (*fin).second.length; //length for incoming func 
    Tin = (*fin).second.tgates; //tgates for incoming func  
    TinUB = (*fin).second.tgates_ub; //tgates for incoming func  
    TinPar = (*fin).second.tgates_par; //tgates for incoming func  
    TinParUB = (*fin).second.tgates_par_ub;
  }

  //errs() << "Curr Width = " << currSched.width << " Curr Length = " << currSched.length << " CurrTgates = " << currSched.tgates << "\n";
  //errs() << "Func Width = " << Win << " Func Length = " << Lin <<  " Func Tgates = " << Tin << "\n";
  //errs() << "Total Width = " << currSched.width << " Total Length = " << totalSched.length << "TS = " << ts  << " TotalTgates = " << totalSched.tgates <<"\n";


  if(ts < totalSched.length+currSched.length){ //might be able to parallelize  
    if((Win + currSched.width) <= RES_CONSTRAINT) //Hooray, can be parallelized
      {
        //first_step = totalSched.length; //where the func got scheduled
        first_step = max(ts,totalSched.length); //where the func got scheduled
        currSched.width += Win;
        //currSched.length = max(Lin, currSched.length);
        currSched.length = max(first_step - totalSched.length + Lin, currSched.length);
        currSched.tgates = max(Tin, currSched.tgates);
        currSched.tgates_ub = min(TinUB+currSched.tgates_ub, currSched.length);
        currSched.tgates_par = max(TinPar, currSched.tgates_par);
        currSched.tgates_par_ub = min(TinParUB+currSched.tgates_par_ub, currSched.width);
        //errs() << "Parallel. New Width = " << currSched.width << " New Length = " << currSched.length << " New Tgates = " << currSched.tgates << " Tpar= " << currSched.tgates_par << " TparUB=" << currSched.tgates_par_ub << "\n";
         }
    else // must be serialized due to SIMD-k constraint
      {

        first_step = totalSched.length+currSched.length; //where the func got scheduled

        totalSched.width = max(totalSched.width,currSched.width);
        totalSched.length += currSched.length;
        totalSched.tgates += currSched.tgates;
        totalSched.tgates_ub += currSched.tgates_ub;
        totalSched.tgates_par = max(totalSched.tgates_par,currSched.tgates_par);
        totalSched.tgates_par_ub = max(totalSched.tgates_par_ub,currSched.tgates_par_ub);
        

        currSched.length = Lin; //create new
        currSched.width = Win; //create new W
        currSched.tgates = Tin; //create new W
        currSched.tgates_ub = TinUB; //create new W
        currSched.tgates_par = TinPar; //create new W
        currSched.tgates_par_ub = TinParUB; //create new W
        //errs() << "Serial(SIMD-k). New Width = " << currSched.width << " New Length = " << currSched.length << " New Tgates= " << currSched.tgates<< " New TgatesUB= " << currSched.tgates_ub<< " New TgatesPar= " << currSched.tgates_par<< " TparUB=" << currSched.tgates_par_ub << "\n";
      }
  }

  else //cannot parallelize due to data dependency
    {
      //first_step = currSched.length; //where the func got scheduled
      first_step = totalSched.length+currSched.length; //where the func got scheduled
      totalSched.width = max(totalSched.width,currSched.width);
      totalSched.length += currSched.length;
      totalSched.tgates += currSched.tgates;
      totalSched.tgates_ub += currSched.tgates_ub;
      totalSched.tgates_par = max(totalSched.tgates_par,currSched.tgates_par);
      totalSched.tgates_par_ub = max(totalSched.tgates_par_ub,currSched.tgates_par_ub);

      currSched.length = Lin; //create new L
      currSched.width = Win; //create new W
      currSched.tgates = Tin; //create new T
      currSched.tgates_ub = TinUB; //create new W
      currSched.tgates_par = TinPar; //create new W
      currSched.tgates_par_ub = TinParUB; //create new W

      //errs() << "Serial(Dependency). New Width = " << currSched.width << " New Length = " << currSched.length << " TotalW=" << totalSched.width << " TotalL=" << totalSched.length << " TotalT=" << totalSched.tgates << " TotalT_UB=" << totalSched.tgates_ub << " TotalTPar=" << totalSched.tgates_par << " TotalTParUB=" << totalSched.tgates_par_ub << "\n";
    }

  return (totalSched.length+currSched.length)-1; //where the last dependency should be recorded
  //return (currSched.length)-1; //where the last dependency should be recorded
}


uint64_t GenSIMDSched::get_ts_to_schedule_leaf(Function* F, uint64_t ts, Function* funcToSched, uint64_t& first_step){

  //errs() << " funcTOSched = " << funcToSched->getName() << "\n";
  //errs() << " Size of currSched = " << currArrParGates.size() << "\n";

  //F is leaf. Treat all incoming functions with respect  
  int funcIndex = -1;

  assert(funcToSched->getName().str().find("llvm.")!=string::npos &&  "Non-Intrinsic Func Found in Leaf Function"); 
  
  map<string, int>::iterator gidx = gate_index.find(funcToSched->getName().str().substr(5));
  assert(gidx!=gate_index.end() && "No Gate Index Found for this Intrinsic Function");
  funcIndex = (*gidx).second;

  //schedule intrinsic function

  uint64_t retVal = 0;
  bool FirstEntrySched = false;

  int costOfGate = 1;

  /*if(funcIndex == _T || funcIndex == _Tdag)
    costOfGate = 10;*/

  for(int c = 0; c<costOfGate; c++){
    bool foundEntry = false;
  
    int searchFuncIndex = funcIndex+c*20;
  
    for(uint64_t i = ts; (i<currArrParGates.size() && !foundEntry); i++){
      for(unsigned int j = 0; (j<RES_CONSTRAINT && !foundEntry); j++){
        //if((currArrParGates[i].typeOfGate[j] == searchFuncIndex) && (currArrParGates[i].numGates[j] < DATA_CONSTRAINT)){
        if((currArrParGates[i].typeOfGate[j] == searchFuncIndex) && ((searchFuncIndex == _CNOT && 2*currArrParGates[i].numGates[j] < DATA_CONSTRAINT)
                                || (searchFuncIndex != _CNOT && currArrParGates[i].numGates[j]< DATA_CONSTRAINT))){
          currArrParGates[i].numGates[j] += 1;
          if(!FirstEntrySched){
            first_step = i;
            FirstEntrySched = true;
          }
          
          if((c==0) && (funcIndex == _T || funcIndex == _Tdag)){
            if(currArrParGates[i].numGates[j] > currSched.tgates_par){
              currSched.tgates_par = currArrParGates[i].numGates[j];
              currSched.tgates_par_ub = currArrParGates[i].numGates[j];
              //errs() << "Incr tgate_par \n";
            }
          }

          //errs() << "GateType Parallelism. TS = " << i << " K-factor=" << j << " Tpar=" << currSched.tgates_par << " TparUB=" << currSched.tgates_par_ub <<"\n";
          foundEntry = true;
          retVal = i;
          ts = i+1; //update value of ts to start with in next iteration
          //return i;
        }
        else if(currArrParGates[i].typeOfGate[j] == -1){
          currArrParGates[i].typeOfGate[j] = searchFuncIndex;
          currArrParGates[i].numGates[j] = 1;
          if(!FirstEntrySched){
            first_step = i;
            FirstEntrySched = true;
          }
          if(j >= currSched.width) //update currSched.width
            currSched.width = j+1;
          
          //Add to T gate count if T or Tdag gate
          if((c==0) && (funcIndex == _T || funcIndex == _Tdag)){
            //errs() << "Found T or Tdag \n";
            
            bool prevTgateFound = false;
            for(unsigned int jcheck=0; jcheck<RES_CONSTRAINT; jcheck++){
              if(currArrParGates[i].typeOfGate[jcheck] == _T
                 || currArrParGates[i].typeOfGate[jcheck] == _Tdag)
                prevTgateFound = true;
            }
            if(!prevTgateFound){
              currSched.tgates++;
              currSched.tgates_ub++;
              if(currSched.tgates_par == 0){
                currSched.tgates_par = 1;
                currSched.tgates_par_ub = 1;
                }
              //errs() << "--Incr tgate \n";
            }
          }
          
          //errs() << "Unscheduled. New Width = " << currSched.width << " New Length = " << currSched.length << " Tgates=" << currSched.tgates << " TgatesUB=" << currSched.tgates_ub << " TgatesPar=" << currSched.tgates_par << " TgatesParUB=" << currSched.tgates_par_ub <<" K-factor=" << j <<"\n";
          foundEntry = true;
          retVal = i;
          ts = i+1;
          //return i;
        }
      }
    }
  
    if(!foundEntry){
      //suitable ts not found, create a new ts for this type of gate
      //add entry to vectArrParGates
      
      ArrParGates tmpArrPar; //initialize
      for(unsigned int k=0;k<RES_CONSTRAINT; k++){
        tmpArrPar.typeOfGate[k] = -1;
        tmpArrPar.numGates[k] = 0;
      }
  
      tmpArrPar.typeOfGate[0] = searchFuncIndex;
      tmpArrPar.numGates[0] = 1;
      currArrParGates.push_back(tmpArrPar);
      if(!FirstEntrySched){
        first_step = currArrParGates.size()-1;
        FirstEntrySched = true;
      }

      if((c==0) && (funcIndex == _T || funcIndex == _Tdag)){
        currSched.tgates++;
        currSched.tgates_ub++;
        //errs() << "Incr tgates \n";
        if(currSched.tgates_par == 0){
          currSched.tgates_par = 1;
          currSched.tgates_par_ub = 1;
          }
      }
      
      if(currSched.width == 0) //update currSched.width
        currSched.width = 1;
      currSched.length++;
      
      //errs() << "NEW TS. New Width = " << currSched.width << " New Length = " << currSched.length << " T gates = " << currSched.tgates << " TgatesUB=" << currSched.tgates_ub << " TgatesPar=" << currSched.tgates_par << " TgatesParUB=" << currSched.tgates_par_ub<< "\n";
      
      retVal = currArrParGates.size()-1;
      ts = currArrParGates.size();

      //return currArrParGates.size()-1;
    } 
  } // cost Of Gate
  
  //print_ArrParGates();
  return retVal;
  
}

void GenSIMDSched::cleanupCurrArrParGates(){
    currArrParGates.clear();    
}

/*bool GenSIMDSched::checkTgatePar(Function* F, uint64_t par){
  //is function leaf?
  vector<Function*>::iterator vit = find(isLeaf.begin(), isLeaf.end(), F);
  if(vit==isLeaf.end()) //not a leaf
    return true;

  //is Leaf
  //iterate over currArrParGates and check for 

  }*/

void GenSIMDSched::save_blackbox_info(Function* F){
  //save black box info
  modularInfo tmpMod;

  tmpMod.width = max(totalSched.width,currSched.width);
  tmpMod.length = totalSched.length + currSched.length;
  tmpMod.tgates = totalSched.tgates + currSched.tgates;
  tmpMod.tgates_ub = totalSched.tgates_ub + currSched.tgates_ub;

  //check if leaf function
  //if leaf go thru currArrParGates vector and find T parallelism

  //if not leaf function
  tmpMod.tgates_par = max(totalSched.tgates_par,currSched.tgates_par);
  tmpMod.tgates_par_ub = max(totalSched.tgates_par_ub,currSched.tgates_par_ub);

  //bool checkTgatePar = checkTgatePar(F,tmpMod.tgates_par);

  funcInfo[F] = tmpMod;

  bool funcIsLeaf = true;
  vector<Function*>::iterator vit = find(isLeaf.begin(), isLeaf.end(), F);
  if(vit==isLeaf.end()) //not a leaf
    funcIsLeaf=false;
  
  //errs() << "SIMD k="<<RES_CONSTRAINT<<" d=" << DATA_CONSTRAINT << " " << F->getName() << " " << tmpMod.width << " " << tmpMod.length << " " <<tmpMod.tgates << " " << tmpMod.tgates_ub << " " << tmpMod.tgates_par<< " " << tmpMod.tgates_par_ub << " leaf=" << funcIsLeaf << "\n";

}


void GenSIMDSched::print_critical_info(){
    errs() << "Timesteps = " << currArrParGates.size() << "\n";
    for(unsigned int i = 0; i<currArrParGates.size(); i++){
        errs() << i << " :";
        for(unsigned int k=0;k<RES_CONSTRAINT;k++){      
          errs() << currArrParGates[i].typeOfGate[k] << " : " << currArrParGates[i].numGates[k] << " / ";
        }
        errs() << "\n";
    }
}

void GenSIMDSched::print_parallelism(Function* F){
  uint64_t maxGates[NUM_QGATES];
  for(int k = 0; k<NUM_QGATES; k++)
    maxGates[k] = 0;

  for(vector<ArrParGates>::iterator vit = currArrParGates.begin(); vit!=currArrParGates.end(); ++vit){
    for(unsigned int i = 0; i<RES_CONSTRAINT; i++)
      if((*vit).numGates[i] > maxGates[(*vit).typeOfGate[i]])
        maxGates[(*vit).typeOfGate[i]] = (*vit).numGates[i];
  }

  errs() << "\nMax Parallelism Factors: \n";
  for(int k = 0; k<NUM_QGATES-1; k++){ //do not print 'All'
    errs() << gate_name[k] << " : " << maxGates[k] << "\n";
  }  
}

uint64_t GenSIMDSched::find_max_funcQbits(){
  uint64_t max_timesteps = 0;
  for(map<string, map<int,uint64_t> >::iterator mIter = funcQbits.begin(); mIter!=funcQbits.end(); ++mIter){
    map<int,uint64_t>::iterator arrIter = (*mIter).second.find(-2); //max ts is in -2 entry
    if((*arrIter).second > max_timesteps)
      max_timesteps = (*arrIter).second;
  }

  //print_funcQbits();
  //errs() << "Max timestep = " << max_timesteps << "\n";
  return max_timesteps;

}

void GenSIMDSched::memset_funcQbits(uint64_t val){
  for(map<string, map<int,uint64_t> >::iterator mIter = funcQbits.begin(); mIter!=funcQbits.end(); ++mIter){
    for(map<int,uint64_t>::iterator arrIter = (*mIter).second.begin(); arrIter!=(*mIter).second.end();++arrIter)
      (*arrIter).second = val;
  }
}

void GenSIMDSched::print_scheduled_gate(qGate qg, uint64_t ts){
  string tmpGateName = qg.qFunc->getName();
  if(tmpGateName.find("llvm.")!=string::npos)
    tmpGateName = tmpGateName.substr(5);
  errs() << ts << " " << tmpGateName;
  for(int i = 0; i<qg.numArgs; i++){
    errs() << " " << qg.args[i].name;
    if(qg.args[i].index != -1)
      errs() << qg.args[i].index;
  }

  /*
  if(tmpGateName == "PrepX" || tmpGateName == "PrepZ"){
    if(qg.angle > 0)
      errs() << " 1";
    else
      errs() << " 0";
  }
  else if(tmpGateName == "Rz" || tmpGateName == "Ry" || tmpGateName == "Rx")
    errs() << " "<<qg.angle;
  */

  errs() << "\n";
}

void GenSIMDSched::print_tableFuncQbits(){
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


void GenSIMDSched::calc_critical_time(Function* F, qGate qg, bool isLeafFunc){
  string fname = qg.qFunc->getName();

  //print_qgate(qg);
  uint64_t max_ts_of_all_args = 0;

  uint64_t first_step = 0;

  if(isFirstMeas && (fname == "llvm.MeasX" || fname == "llvm.MeasZ")){
    uint64_t maxFQ = find_max_funcQbits();
    uint64_t max_ts_sched; 

    //errs() << "First Meas && Before Scheduled \n";
    //print_funcQbits();

    if(isLeafFunc)
      max_ts_sched = get_ts_to_schedule_leaf(F,maxFQ, qg.qFunc, first_step);
    else
      max_ts_sched = get_ts_to_schedule(F,maxFQ, qg.qFunc, first_step);

    memset_funcQbits(max_ts_sched);

    //set this Meas in this max_ts_sched+1
    map<string, map<int,uint64_t> >::iterator mIter = funcQbits.find(qg.args[0].name);
    assert(mIter != funcQbits.end() && "Meas Gate Var not found in funcQbits");

    int argIndex = qg.args[0].index; //must have only one argument
    assert(argIndex != -1 && "Meas gate has array argument");

    //update the timestep number for that argument
    map<int,uint64_t>::iterator indexIter = (*mIter).second.find(argIndex);
    (*indexIter).second =  max_ts_sched + 1;
    
    //update -2 entry for the array, i.e. max ts over all indices
    indexIter = (*mIter).second.find(-2);
    (*indexIter).second = max_ts_sched + 1;
    
    //errs() << "Scheduled in "<< max_ts_sched+1 << "\n";
    //print_funcQbits();

    isFirstMeas = false;
    print_scheduled_gate(qg,max_ts_sched+1);
    return;    
  }
  else{
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
            //((*mIter).second)[argIndex] = 0;
            if((*fullArrayIndexIter).second > max_ts_of_all_args)
              max_ts_of_all_args = (*fullArrayIndexIter).second;
          }
        }
    }
    
    if(debugGenSIMDSched){
      errs() << "Before Scheduling: \n";
      print_funcQbits();
    }
    
    //errs() << "Max timestep for all args = " << max_ts_of_all_args << "\n";
    
    //find timestep from max_ts_of_all_args where type of gate is same as this gate or no gate has been scheduled.
    uint64_t ts_sched;

      if(isLeafFunc)
        ts_sched = get_ts_to_schedule_leaf(F,max_ts_of_all_args, qg.qFunc, first_step);
      else
        ts_sched = get_ts_to_schedule(F,max_ts_of_all_args, qg.qFunc, first_step);

      //errs() << "ts_sched = " << ts_sched << " FirstStep = " << first_step << "\n";
      
      //schedule gate in max_ts_of_all_args + 1th timestep = ts_sched+1
      print_scheduled_gate(qg,first_step+1);
      

      //if(currArrParGates.size() != 0){
      
      //update last timestep for all arguments of qgate
      for(int i=0;i<qg.numArgs; i++){
        map<string, map<int,uint64_t> >::iterator mIter = funcQbits.find(qg.args[i].name);
        
        int argIndex = qg.args[i].index;
        
        if(argIndex == -1){
          for(map<int,uint64_t>::iterator entryIter = (*mIter).second.begin(); entryIter!=(*mIter).second.end();++entryIter){
            (*entryIter).second = ts_sched + 1;
          }
          
        }
        else{
          //update the timestep number for that argument
          map<int,uint64_t>::iterator indexIter = (*mIter).second.find(argIndex);
          (*indexIter).second =  ts_sched + 1;
          
          //update -2 entry for the array, i.e. max ts over all indices
          indexIter = (*mIter).second.find(-2);
          if((*indexIter).second < ts_sched + 1)
            (*indexIter).second = ts_sched + 1;
        }  
      }
      //}
      } // not first MeasX gate
  
  if(debugGenSIMDSched){   
    errs() << "\nAfter Scheduling: \n";
    print_funcQbits();
    print_critical_info();
    errs() << "\n";
  }
  
}

uint64_t GenSIMDSched::calc_critical_time_unbounded(Function* F, qGate qg){
  string fname = qg.qFunc->getName();

  if(debugGenSIMDSched){   
    print_qgate(qg);    
    print_tableFuncQbits();
  }

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
        }
      }
    }
    
    if(debugGenSIMDSched){
      errs() << "Before Scheduling: \n";
      print_funcQbits();
      }
    
    //errs() << "Max timestep for all args = " << max_ts_of_all_args << "\n";
      
      //schedule gate in max_ts_of_all_args + 1th timestep
      //--print_scheduled_gate(qg,max_ts_of_all_args+1);
      
      //update last timestep for all arguments of qgate
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

  
  if(debugGenSIMDSched)
  {   
    errs() << "\nAfter Scheduling: \n";
    print_funcQbits();
    errs() << "\n";
  }

  return max_ts_of_all_args+1;
  
} //calc_critical_time_unbounded


bool GenSIMDSched::checkIfIntrinsic(Function* CF){
  if(CF->isIntrinsic()){
    if((CF->getIntrinsicID() == Intrinsic::CNOT)
       || (CF->getIntrinsicID() == Intrinsic::Fredkin)
       || (CF->getIntrinsicID() == Intrinsic::H)
       || (CF->getIntrinsicID() == Intrinsic::MeasX)
       || (CF->getIntrinsicID() == Intrinsic::MeasZ)
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
      return true;
    }
  }
  return false;
}


void GenSIMDSched::analyzeCallInst(Function* F, Instruction* pInst){
  if(CallInst *CI = dyn_cast<CallInst>(pInst))
    {      
      if(debugGenSIMDSched)
        errs() << "Call inst: " << CI->getCalledFunction()->getName() << "\n";

      if(CI->getCalledFunction()->getName() == "store_cbit"){   //trace return values
        return;
      }      

      vector<qGateArg> allDepQbit;                                  
      
      bool tracked_all_operands = true;

      int myPrepState = -1;
      double myRotationAngle = 0.0;
      
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
            tracked_all_operands &= backtraceOperand(CI->getArgOperand(iop),0);
        }

        if(tmpDepQbit.size()>0){          
          allDepQbit.push_back(tmpDepQbit[0]);
          assert(tmpDepQbit.size() == 1 && "tmpDepQbit SIZE GT 1");
          tmpDepQbit.clear();
        }
        
      }
      
      if(allDepQbit.size() > 0){
        if(debugGenSIMDSched)
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


        //check if Intrinsic
        bool thisFuncIsIntrinsic = checkIfIntrinsic(CI->getCalledFunction());
        if(!thisFuncIsIntrinsic) hasPrimitivesOnly = false;
          
       string fname =  CI->getCalledFunction()->getName();  
       qGate thisGate;
       thisGate.qFunc =  CI->getCalledFunction();

       if(myPrepState!=-1) thisGate.angle = (float)myPrepState;
       if(myRotationAngle!=0.0) thisGate.angle = myRotationAngle;

       for(unsigned int vb=0; vb<allDepQbit.size(); vb++){
            if(allDepQbit[vb].argPtr){
              //errs() << allDepQbit[vb].argPtr->getName() <<" Index: ";
              //errs() << allDepQbit[vb].valOrIndex <<"\n";
                qGateArg param =  allDepQbit[vb];       
                //errs() << "1\n";
                thisGate.args[thisGate.numArgs].name = param.argPtr->getName();
                //errs() << "2\n";
                if(!param.isPtr)
                  thisGate.args[thisGate.numArgs].index = param.valOrIndex;
                //errs() << "3\n";
                thisGate.numArgs++;
                //errs() << "4\n";
            }
       }
       //errs() << "5\n";


       uint64_t thisTS = calc_critical_time_unbounded(F,thisGate);       
       //update priorityVector
       priorityVector.push_back(make_pair(pInst,thisTS));

       //add to mapInstSet
       mapInstSet[pInst] = thisGate;

      }    
      allDepQbit.erase(allDepQbit.begin(),allDepQbit.end());
    }
}


void GenSIMDSched::saveTableFuncQbits(Function* F){
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


void GenSIMDSched::CountCriticalFunctionResources (Function *F) {
      // Traverse instruction by instruction
  init_critical_path_algo(F);
  

  //get qbits in function
  for (inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E; ++I) {
    Instruction *Inst = &*I;                            // Grab pointer to instruction reference
    analyzeAllocInst(F,Inst);          
    if(!isa<AllocaInst>(Inst))
      break;
  }

  //errs() << "Finding priorities--- \n";
  //find priorities for instructions
  for (inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E; ++I) {
    Instruction *Inst = &*I;                            // Grab pointer to instruction reference
    if(isa<CallInst>(Inst))
      vectCalls.push_back(Inst);
  }

  //traverse in reverse sequence
  for(vector<Instruction*>::reverse_iterator rit = vectCalls.rbegin(); rit!=vectCalls.rend(); ++rit)
    analyzeCallInst(F,(*rit));  

  //is function leaf or not?
  if(hasPrimitivesOnly) isLeaf.push_back(F);

  //sort vector
  sort(priorityVector.begin(), priorityVector.end(), CompareInstPriByValue());

  //reset funcQbits vector in preparation for scheduling
  memset_funcQbits(0);

  //errs() << "Finding Schedule--- \n";
  for(vector<InstPri>::reverse_iterator vit = priorityVector.rbegin(); vit!=priorityVector.rend(); ++vit){
    //get qgate
    map<Instruction*, qGate>::iterator mit = mapInstSet.find((*vit).first);
    assert(mit!=mapInstSet.end() && "Instruction Not Found in MapInstSet.");

    qGate thisGate = (*mit).second;

//    errs() << (*vit).second << " | ";   
 
    if(hasPrimitivesOnly)
      calc_critical_time(F,thisGate,true);
    else
      calc_critical_time(F,thisGate,false);

  }


  saveTableFuncQbits(F);  
  save_blackbox_info(F);



  //print_ArrParGates();
  //print_tableFuncQbits();

}


void GenSIMDSched::init_gates_as_functions(){
    
    //add blackbox entry for each of these ??
    
  for(int  i =0; i< NUM_QGATES ; i++){
    string gName = gate_name[i];
    string fName = "llvm.";
    fName.append(gName);
    
  }

}


bool GenSIMDSched::runOnModule (Module &M) {
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
        //errs() << "SIMD_K " << RES_CONSTRAINT << ", SIMD_D " << DATA_CONSTRAINT << "\n";      
        errs() << "#Function " << F->getName() << "\n";      
        //errs() << "#Timestep GateName Operand1 Operand2 \n";
        
        funcQbits.clear();
        funcArgs.clear();
        vectCalls.clear();
        mapInstSet.clear();
        priorityVector.clear();
        
        getFunctionArguments(F);

        // count the critical resources for this function
        CountCriticalFunctionResources(F);

        if(F->getName() == "main"){
          //print_ArrParGates(F);
          //errs() << "\n#Num of critical time steps for function main : " << getNumCritSteps(F) << "\n";           
        }

        //print_critical_info();
        errs() << "#EndFunction\n";
        cleanupCurrArrParGates(); 
      }
      else{
            if(debugGenSIMDSched)
              errs() << "WARNING: Ignoring external node or dummy function.\n";
          }
    }
  }
  //print_tableFuncQbits();
  //print_parallelism();

  return false;
} // End runOnModule
