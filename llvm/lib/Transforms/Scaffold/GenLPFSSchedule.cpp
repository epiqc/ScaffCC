//===----------------- GenLPFSSched.cpp ----------------------===//
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

#include <vector>
#include <iostream> 
#include <limits>
#include <map>
#include <string>
#include <sstream>
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
//#include "llvm/ScheduleDAG.h"

//#define _DEBUG_LPFS // Optional: debug flag

using namespace llvm;
using namespace std;

static cl::opt<unsigned>
RES_CONSTRAINT("simd-kconstraint-lpfs", cl::init(8), cl::Hidden,
    cl::desc("k in SIMD-k Resource Constrained Scheduling"));

static cl::opt<unsigned>
DATA_CONSTRAINT("simd-dconstraint-lpfs", cl::init(1024), cl::Hidden,
    cl::desc("d in SIMD-d Resource Constrained Scheduling"));

static cl::opt<unsigned>
SIMD_L("simd_l", cl::init(1), cl::Hidden,
    cl::desc("l value for longest path first (lpfs)"));

static cl::opt<unsigned>
REFILL("refill", cl::init(0), cl::Hidden,
    cl::desc("refill value, whether or not to use refill in lpfs")); 

static cl::opt<unsigned>
OPP_SIMD("opp_simd", cl::init(1), cl::Hidden,
    cl::desc("opportunistic scheduling with lpfs")); 

static cl::opt<unsigned>
LOCAL_MEM("local_mem", cl::init(0), cl::Hidden,
    cl::desc("local memory scheduling with lpfs"));

static cl::opt<unsigned>
LOCAL_Q("local_Q", cl::init(INT_MAX), cl::Hidden,
    cl::desc("Q parameter for local memory depth"));

static cl::opt<unsigned>
LOCAL_WINDOW("local_W", cl::init(10), cl::Hidden,
    cl::desc("Look-ahead window parameter for local mem"));

static cl::opt<unsigned>
METRICS("metrics", cl::init(0), cl::Hidden,
    cl::desc("Print Metrics"));

static cl::opt<unsigned>
FULL_SCHED("full_sched", cl::init(0), cl::Hidden,
    cl::desc("Print Full Schedules"));

static cl::opt<unsigned>
MOVES_SCHED("moves_sched", cl::init(0), cl::Hidden,
    cl::desc("Print Schedule of Move Instructions"));

static cl::opt<unsigned>
LOCAL_MOVES_SCHED("local_moves_sched", cl::init(0), cl::Hidden,
    cl::desc("Print Schedule of Local Move Instructions"));



#define MAX_RES_CONSTRAINT 2000 
#define SSCHED_THRESH 10000000

#define MAX_GATE_ARGS 30
#define MAX_BT_COUNT 15 //max backtrace allowed - to avoid infinite recursive loops
#define NUM_QGATES 19
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
#define _Rx 13
#define _Ry 14
#define _Rz 15
#define _Toffoli 16
#define _Fredkin 17
#define _All 18

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
    int id;
    int simd;
    int loc;
    int nextTS;
    Instruction* last_inst;
    qArgInfo(): name("none"), index(-1), id(-1), simd(0), loc(0), nextTS(-1), last_inst(NULL) { }

    bool operator == (const qArgInfo& a) const{
      return (name == a.name && index == a.index);
    }
  };

  struct qGate{
    Function* qFunc;
    int numArgs;
    qArgInfo args[MAX_GATE_ARGS];
    double angle;
    qGate():qFunc(NULL), numArgs(0), angle(0.0) { }
  };

  struct op{
    qGate name;
    int id;
    int ts;
    int dist;
    bool followed;
    int simd;
    int tag;
    int path;
    Instruction* label;
    vector<Instruction*> in_edges;
    vector<Instruction*> out_edges;
    op(): name(),id(-1),ts(-1),dist(-1),followed(0),simd(-1),tag(0),path(0),label(NULL),in_edges(),out_edges() { }
  };

  struct qubit{
    string name;
    int index;
    int size;
    int last_op;
    vector<op> ops;
    int simd;
    int id;
    qubit():name("none"),index(0),size(1),last_op(-1),ops(),simd(-1),id(-1) { }
  };

  struct move{
    int ts;
    int src;
    int dest;
    qArgInfo arg;
    move():ts(-1),src(-1),dest(-1),arg() { }
  };

  struct ArrParGates{
    int typeOfGate[MAX_RES_CONSTRAINT];
    uint64_t numGates[MAX_RES_CONSTRAINT];
  };

  struct GenLPFSSched : public ModulePass {
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

    vector<op> readyQueue; //ready queue for use with LPFS scheduling
    vector<pair<Instruction*, op> > funcList; //list of the operations of a function


    vector<ArrParGates> currArrParGates;

    map<Instruction*, qGate> mapInstSet;
    vector<InstPri> priorityVector;

    map<Instruction*, op> mapCalls; //map between the instruction label and the operation attributes of each inst
    vector<Instruction*> longPath;      //longest path to be returned by find_lp
    vector<Instruction*> callList;  
    map<int, multimap<int, op> > schedule; //all the instructions in a given simd region
    int ots; //operating time steps
    int simds;
    int tgates_cnt; //tgates
    multimap<int, move> move_schedule; //all the instructions in a given simd region
    multimap<int, move> local_move_schedule; //all the instructions in a given simd region
    int mts; //move time steps
    map<int, vector<Instruction*> > longestPathList; //all the instructions in a given simd region

    vector<qArgInfo> active_qubits;
    map<string, qArgInfo> qubitMap;
    map<int, int> localMemSizeMap;
    map<int, int> regionSizeMap;

    map<Function*, modularInfo> funcInfo;
    vector<Function*> isLeaf;
    bool hasPrimitivesOnly;

    bool isFirstMeas;

    GenLPFSSched() : ModulePass(ID) {}

    // Get arguments from operation
    bool backtraceOperand(Value* opd, int opOrIndex);
    bool analyzeIntrinsicCallInst(Function* F, Instruction* pinst);
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

    void find_lp(Function* F, int pathNum);
    void lpfs(Function* F, int ts, int simd_l, int refill, int opp_simd);
    void take_path(Instruction* CI, int path);
    void sched_op(Instruction* currentOp, int timeStep, int simd);
    bool depsMet(Instruction* currentOp, int currentTime);
    void update_moves(int moves, int ts );



    void print_ready_queue(); 
    void print_funcList(); 
    void print_vectQbit(); 
    void print_mapCalls();
    void print_mapCallsEdges();
    void print_priorityVector();
    void print_longPath();
    void print_schedule(Function* F, int op_count);
    void print_moves_schedule(Function* F, int op_count);
    void print_local_moves_schedule(Function* F, int op_count);
    void print_schedule_metrics(Function* F, int op_count);

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
      gate_name[_Rx] = "Rx";
      gate_name[_Ry] = "Ry";
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
      gate_index["Rx"] = _Rx;
      gate_index["Ry"] = _Ry;
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

    bool DetermineLeafFunction (Function *F);

    void CountCriticalFunctionResources (Function *F);

    bool runOnModule (Module &M);    

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();  
      AU.addRequired<CallGraph>();
    }

  }; // End of struct GenLPFSSched
} // End of anonymous namespace



char GenLPFSSched::ID = 0;
static RegisterPass<GenLPFSSched> X("GenLPFSSchedule", "Generate LPFS Schedule");

//LPFS: Longest Path First Scheduling

vector<Instruction*> longestPathList;

void GenLPFSSched::lpfs(Function* F, int ts, int simd_l, int refill_simd, int opp_simd){
  //----------------Build Dependency Tree--------------------//
  int op_count = priorityVector.size();
  int sched_ops = 0;
  int moves = 0;
  int id_to_apply = 0;
  int qbit_id = 0;

  //Building Dependency Graph  
  int counter = 0;
  for(vector<Instruction*>::iterator fp1 = callList.begin(); fp1 != callList.end(); ++fp1){
    counter++;
#ifdef _DEBUG_LPFS
    if (counter % 1000 == 0)
      errs() << "Got up to instruction " << counter << "\n";
#endif
    bool found_one = false;
    map<Instruction*, op>::iterator mp1 = mapCalls.find(*fp1);
    (*mp1).second.id = id_to_apply++;
    for(int i=0; i < (*mp1).second.name.numArgs; ++i) {
      vector<Instruction*>::iterator fp2 = fp1;
      if((*mp1).second.name.args[i].id == -1) {
        (*mp1).second.name.args[i].id = qbit_id++;
        qArgInfo arg = (*mp1).second.name.args[i];
        stringstream ss;
        ss << arg.index;
        string name = arg.name + ss.str();
        qubitMap.insert(make_pair(name, arg));
      }
      if(fp1 != callList.end()) ++fp2;
      int counter2 = 0;
      while(fp2 != callList.end()){
        counter2++;  
        map<Instruction*, op>::iterator mp2 = mapCalls.find(*fp2);
        found_one = false;
        for(int j = 0; j < (*mp2).second.name.numArgs; ++j){
          if((*mp1).second.name.args[i].id == -1) {
            (*mp1).second.name.args[i].id = qbit_id++;
            qArgInfo arg = (*mp1).second.name.args[i];
            stringstream ss;
            ss << arg.index;
            string name = arg.name + ss.str();
            qubitMap.insert(make_pair(name, arg));
          }
          if((*mp1).second.name.args[i] == (*mp2).second.name.args[j]){
            string zz = (*mp1).second.name.args[i].name;             
            (*mp1).second.out_edges.push_back((*mp2).first);
            (*mp2).second.in_edges.push_back((*mp1).first);
            found_one = true;
          }
        }
        if(found_one){ 
          break;
        }
        ++fp2;
      }
    }
  }
#ifdef _DEBUG_LPFS
  errs() << "Finished Building Dependency Graph" << "\n";
#endif

  //-----Find the longest paths required for the simd_l constraint-----//
  longestPathList.clear();
  for(int i = 1; i <= simd_l; i++){
    find_lp(F,i);
    longestPathList[i] = longPath;
    longPath.clear();
  } 
#ifdef _DEBUG_LPFS
  errs() << "Finished Finding Longest Path(s)" << "\n";    
#endif

  //-------Assign the longest paths-------//
  for(map<int, vector<Instruction*> >::iterator pathNumber = longestPathList.begin(); pathNumber != longestPathList.end(); pathNumber++){
    for(vector<Instruction*>::reverse_iterator inst = (*pathNumber).second.rbegin(); inst != (*pathNumber).second.rend(); inst++){
      Instruction* myInst = (*inst);
      if(((*mapCalls.find(myInst)).second.simd) == -1)
        sched_op(myInst, ts++, (*pathNumber).first);
      sched_ops++;
    }   
  }
  ts = 0;

  while(sched_ops < op_count){ 
#ifdef _DEBUG_LPFS    
    errs() << "sched op = " << sched_ops << " op count = " << op_count << "\n";
#endif
    for(vector<InstPri>::reverse_iterator vit = priorityVector.rbegin(); vit!=priorityVector.rend(); ++vit){
      Instruction* myInst = (*vit).first;
      op myOp = (*mapCalls.find(myInst)).second;
      bool scheduled = false;
      //#ifdef _DEBUG_LPFS      
      //      errs() << "scheduling op " << myOp.name.qFunc->getName() << " containing " << myOp.name.numArgs << " args\n";
      //      errs() << "ts = " << ts << " Checking op: " << myOp.id << " ";
      //      print_qgate(myOp.name);      
      //#endif
      while((*mapCalls.find(myInst)).second.simd == -1) {
        int simdToSched = 1;
        if(depsMet(myInst, ts)) {
          if(opp_simd == 1) {
            while(simdToSched <= (int) RES_CONSTRAINT) { 
              map<int, multimap<int, op> >::iterator it = schedule.find(simdToSched);
              if(!(it == schedule.end())) {
                multimap<int, op> map = (*it).second;
                multimap<int, op>::iterator mit = map.find(ts);
                if(mit != map.end()) {
                  op curOp = (*mit).second;
                  /*------Add Data Constraint---*/        
                  if(myOp.name.qFunc == curOp.name.qFunc){
                    sched_op(myInst, ts, simdToSched);
                    scheduled = true;
                    sched_ops++;
                    break;
                  }
                }
              }
              simdToSched++;
            }
            if(!scheduled){
              int lowTS = std::numeric_limits<int>::max();
              int lowSD = 0;
              for(simdToSched = (int) RES_CONSTRAINT; simdToSched > 0; simdToSched--){
                int tempTS = ts;
                while(schedule[simdToSched].count(tempTS)) {
                  tempTS++;
                }
                if(tempTS <= lowTS){
                  lowTS = tempTS;
                  lowSD = simdToSched;
                }
              }
              sched_op(myInst, lowTS, lowSD);
              scheduled = true;
              sched_ops++;  
            }
          }
          else{
            //while(!schedule[simdToSched].count(ts)) ts++; // FIXME: bug, causes infinite loop.
            sched_op(myInst, ts, simdToSched);
            scheduled = true;
            sched_ops++;
            break;
          }
        }
        ts++;
      }
      ts = 0;
    }
  }
  while(schedule[1].count(ts)){
    update_moves(moves, ts++);   
  }
  ots = ts;  
}

void GenLPFSSched::update_moves(int moves, int ts ){
  vector<qArgInfo> current;
  vector<qArgInfo> next; 
  map<int, int> simd_active;
  bool added_move = false;

  //----Get Current Qubits-----//
  for(int simd = 1; simd <= (int) RES_CONSTRAINT; simd++){
    map<int, multimap<int, op> >::iterator it = schedule.find(simd);
    simd_active[simd] = 0;
    if(it != schedule.end()){
      multimap<int, op>::iterator mit = schedule[simd].find(ts);
      if(mit != (*it).second.end()) {
        simd_active[simd] = 1;
        simds = max(simds, simd); 
      }
      while(mit != (*it).second.end() && (*mit).first == ts){
        op myOp = (*mit).second;
        for(int i = 0; i < myOp.name.numArgs; i++){
          stringstream ss;
          ss << myOp.name.args[i].index;
          string name = myOp.name.args[i].name + ss.str();
          qArgInfo arg = (*qubitMap.find(name)).second; 
          arg.simd = myOp.simd;
          arg.last_inst = myOp.label;
          vector<qArgInfo>::iterator vit = current.begin();
          while(vit != current.end()) { 
            if((*vit) == arg){
              break;
            }
            vit++;
          }
          if(vit == current.end()) {
            current.push_back(arg);
            (*qubitMap.find(name)).second.simd = arg.simd;
            (*qubitMap.find(name)).second.last_inst = arg.last_inst;
          }
        }  
        mit++;
      }
    }
  }

#ifdef _DEBUG_LPFS
  errs() << "# AT TIMESTEP: " << ts << "\n";
#endif

  for(vector<qArgInfo>::iterator mapit = active_qubits.begin(); mapit != active_qubits.end(); mapit++){
#ifdef _DEBUG_LPFS
    errs() << "Currently examining: " << (*mapit).name << (*mapit).index << "\n";
#endif
    stringstream ss;
    ss << (*mapit).index;
    string name = (*mapit).name + ss.str();
    qArgInfo thisQbit = (*qubitMap.find(name)).second;
    int src = thisQbit.loc;         
    int dest = 0; 
    vector<qArgInfo>::iterator qit = current.begin();
    while(qit != current.end()){
      stringstream ss;
      ss << (*qit).index;
      string name2 = (*qit).name + ss.str();
      qArgInfo currQbit = (*qubitMap.find(name)).second;
      if((name == name2)&& (thisQbit.loc % 10 != 0)){       //If qubit is in current and active and not in memory
        dest = (*qit).simd;
        (*qubitMap.find(name)).second.simd = dest;
        next.push_back((*qubitMap.find(name)).second);
        current.erase(qit);
        break;
      }
      qit++;
    }

    if(!(simd_active[src] ) && !(dest)) { //Qbit doesn't need to move
      next.push_back((*qubitMap.find(name)).second);
    }
    else if(!LOCAL_MEM){
      if(((dest) && (dest != src)) || ((dest == 0) && (simd_active[src]))){
        //Moved into new location
        move newMove;
        newMove.src = src;
        newMove.dest = dest;
        newMove.arg = (*qubitMap.find(name)).second;
        move_schedule.insert(make_pair(ts,newMove));
        (*qubitMap.find(name)).second.loc = dest;
        //regionSizeMap[src]--;
        //regionSizeMap[dest]++;
        added_move = true;
      }
    }
    else if(LOCAL_MEM){
      if((dest) && (dest != src)){
        move newMove;
        newMove.src = src;
        newMove.dest = dest;
        newMove.arg = (*qubitMap.find(name)).second;
        move_schedule.insert(make_pair(ts,newMove));
        //regionSizeMap[src]--;
        //regionSizeMap[dest]++;
        (*qubitMap.find(name)).second.loc = dest; 
        added_move = true;
      }
      if(!(dest) && (simd_active[src])){
        int lowNextTS = std::numeric_limits<int>::max();
        int nextOpLoc = -1;
        op myOp = (*mapCalls.find(thisQbit.last_inst)).second;
        if(!myOp.out_edges.empty()){
          for(int i = 0; i < (int) myOp.out_edges.size(); i++){
            op nextOp = (*mapCalls.find(myOp.out_edges[i])).second;
            for(int j = 0; j < nextOp.name.numArgs; j++){
              qArgInfo arg = nextOp.name.args[j];
              if(arg == thisQbit){
                lowNextTS = nextOp.ts;
                nextOpLoc = nextOp.simd;
              }
            }
          }
        }
        (*qubitMap.find(name)).second.nextTS = lowNextTS;
        if((lowNextTS <= ts + (int) LOCAL_WINDOW) && (myOp.ts != ts) && (nextOpLoc == myOp.simd)  && ((*qubitMap.find(name)).second.loc % 10 != 0)){
          if( (src) && (localMemSizeMap[src*10] >= (int) LOCAL_Q)){
            int maxTS = 0;
            string maxName;
            qArgInfo victim;
            for(map<string,qArgInfo>::iterator mit = qubitMap.begin(); mit != qubitMap.end(); mit++){
              if((*mit).second.loc == src*10){
                if(maxTS <= (*mit).second.nextTS){
                  maxTS = (*mit).second.nextTS;
                  maxName = (*mit).first;
                  victim = (*mit).second;
                }
              }
            }
            move newTMove;
            newTMove.src = src*10;
            newTMove.dest = 0;
            newTMove.arg = victim;
            stringstream ss;
            ss << victim.index;
            maxName = victim.name + ss.str();
            (*qubitMap.find(maxName)).second.loc = 0;
            move_schedule.insert(make_pair(ts, newTMove));
            added_move = true;
            localMemSizeMap[src*10]--;
          }
          move newMove;
          newMove.src = src;
          newMove.dest = src * 10;
          newMove.arg = (*qubitMap.find(name)).second;
          (*qubitMap.find(name)).second.loc = newMove.dest;
          next.push_back((*qubitMap.find(name)).second);
          local_move_schedule.insert(make_pair(ts,newMove));
          // regionSizeMap[src]--;
          localMemSizeMap[newMove.dest]++;
          //errs() << "TS: " << ts << " Added local mem: " << name <<" : " << (*qubitMap.find(name)).second.loc << "\n";
        }
        else if(myOp.ts != ts) {
          move newMove;
          newMove.src = src;
          newMove.dest = dest;
          newMove.arg = (*qubitMap.find(name)).second;
          (*qubitMap.find(name)).second.loc = newMove.dest;
          move_schedule.insert(make_pair(ts,newMove));
          //regionSizeMap[src]--;
          added_move = true;
        }
      }
    }
  }

  /*  
      errs() << "Current qubits after deletion: " << current.size() << "TIME: " << ts <<  "\n";
      for(vector<qArgInfo>::iterator mit = current.begin(); mit != current.end(); mit++){
      errs() << (*mit).name << (*mit).index << " DEST: " << (*mit).simd << " ID: " << (*mit).id <<  "\n";
      }
   */


  for(vector<qArgInfo>::iterator mapit = current.begin(); mapit != current.end(); mapit++){
    stringstream ss;
    ss << (*mapit).index;
    string name = (*mapit).name + ss.str();

    qArgInfo curQbit = (*qubitMap.find(name)).second; 
    int dest = curQbit.simd;    //Region where qubit is needed for operation at current timestep
    next.push_back(curQbit);

    if(curQbit.loc == 0 || !LOCAL_MEM){   //Memory location qubit is stored in
      move newMove;
      newMove.src = 0;
      newMove.dest = dest;
      newMove.arg = curQbit;
      (*qubitMap.find(name)).second.loc = dest;
      move_schedule.insert(make_pair(ts,newMove));
      added_move = true;
    }
    else{
      move newMove;
      newMove.src = curQbit.loc;
      newMove.dest = dest;
      newMove.arg = curQbit;
      (*qubitMap.find(name)).second.loc = dest;
      local_move_schedule.insert(make_pair(ts,newMove));
      localMemSizeMap[curQbit.loc]--;
      //errs() << "TS: " << ts << " Grabbed from local: " << name << " : " << curQbit.loc << "\n";
    }
  }
  active_qubits = next;
}

bool GenLPFSSched::depsMet(Instruction* currentOp, int currentTime){
  op myOp = (*mapCalls.find(currentOp)).second;
  bool answer = true;
  if(!(myOp.in_edges.empty())){
    for(vector<Instruction*>::iterator parent = myOp.in_edges.begin(); parent != myOp.in_edges.end(); parent++){
      if(((*mapCalls.find(*parent))).second.simd == -1) answer = false;
      if(((*mapCalls.find(*parent))).second.ts >= currentTime) answer = false;
      //            errs() << "Current timestep: " << currentTime << " Parent's timestep: " << ((*mapCalls.find(*parent))).second.ts << "\n"; 
    }
  }
  return answer;
}

// Schedule at given timestep and simd region
void GenLPFSSched::sched_op(Instruction* currentOp, int timeStep, int simd){
  if((*mapCalls.find(currentOp)).second.simd == -1){
    (*mapCalls.find(currentOp)).second.ts = timeStep;
    (*mapCalls.find(currentOp)).second.simd = simd;
    (*mapCalls.find(currentOp)).second.followed = 1;
    (*mapCalls.find(currentOp)).second.label = currentOp;
    schedule[simd].insert(make_pair(timeStep, ((*mapCalls.find(currentOp)).second)));
    regionSizeMap[simd]++;
    if(((*mapCalls.find(currentOp)).second.name.qFunc->getName().find("llvm.T") != std::string::npos)||((*mapCalls.find(currentOp)).second.name.qFunc->getName().find("llvm.Tdag") != std::string::npos )) { 
      tgates_cnt++;
    }
  }
}

//Helper Functions

void GenLPFSSched::find_lp(Function* F, int pathNum){
  //----------------Find Longest Path----------------------------//
  longPath.push_back(*callList.begin());

  for(map<Instruction*, op>::iterator mp1 = mapCalls.begin(); mp1 != (--mapCalls.end()); ++mp1){
    if(!(*mp1).second.followed) (*mp1).second.dist = 1;
  }
  //	for(map<Instruction*, op>::iterator mp1 = mapCalls.begin(); mp1 != (--mapCalls.end()); ++mp1){
  for(vector<Instruction*>::iterator fp1 = callList.begin(); fp1 != (--callList.end()); ++fp1){
    map<Instruction*, op>:: iterator mp1 = mapCalls.find(*fp1);
    if((*mp1).second.followed){
      (*mp1).second.dist = 0;
    }
    else{ 
      op thisOp = (*mp1).second;

      if(!(thisOp.out_edges.empty())){
        for(vector<Instruction*>::iterator child = thisOp.out_edges.begin(); child != thisOp.out_edges.end(); child++){
          op childOp = (*mapCalls.find(*child)).second;
          childOp.dist = max(childOp.dist, thisOp.dist + 1);
          mapCalls[*child] = childOp; 
        }
      }
      if((!thisOp.followed) && (!(longPath.empty()))){
        if(thisOp.dist >= (*mapCalls.find(longPath[0])).second.dist){
          longPath[0] = (*mp1).first;
        }
      }
    }
    //        errs() << "INST: " << (*mp1).first << " Dist: " << (*mp1).second.dist << "\n";
  }

  if(!(longPath.empty())){ 
    //        for(int i = 0; i < (int)longPath.size(); i++){ 
    while((*mapCalls.find(longPath[longPath.size() - 1])).second.dist > 1){ 
      op botOp = (*mapCalls.find(longPath[longPath.size()-1])).second;
      int currDist = botOp.dist - 1;
      take_path(longPath[longPath.size() - 1], pathNum); 
      for(vector<Instruction*>::iterator parent = botOp.in_edges.begin(); parent != botOp.in_edges.end(); ++parent){
        if(((*mapCalls.find(*parent)).second.dist == currDist) && (!(*mapCalls.find(*parent)).second.followed)){
          longPath.push_back(*parent); //next operation is appended to the path, so path vector is in reverse
          break;
        } 
      }
    }
  }
  }                         

  void GenLPFSSched::take_path(Instruction* CI, int path){
    op currentOp = (*mapCalls.find(CI)).second;
    //    print_qgate(currentOp.name);
    currentOp.dist = 0;
    currentOp.path = path;
    //    currentOp.simd = path;
    currentOp.followed = 1;
    mapCalls[CI] = currentOp; 
  }


  void GenLPFSSched::getFunctionArguments(Function* F)
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

  bool GenLPFSSched::backtraceOperand(Value* opd, int opOrIndex)
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
        if(endCI->getCalledFunction()->getName().find("llvm.Meas") != std::string::npos){
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


  void GenLPFSSched::analyzeAllocInst(Function* F, Instruction* pInst){
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


  void GenLPFSSched::init_critical_path_algo(Function* F){

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

  void GenLPFSSched::print_funcQbits(){
    for(map<string, map<int,uint64_t> >::iterator mIter = funcQbits.begin(); mIter!=funcQbits.end(); ++mIter){
      errs() << "Var "<< (*mIter).first << " ---> ";
      for(map<int,uint64_t>::iterator indexIter  = (*mIter).second.begin(); indexIter!=(*mIter).second.end(); ++indexIter){
        errs() << (*indexIter).first << ":"<<(*indexIter).second<< "  ";
      }
      errs() << "\n";
    }
  }

  void GenLPFSSched::print_ArrParGates(){
    errs() << "Printing ArrParGate Vector \n";
    int j = 0;
    for(vector<ArrParGates>::iterator vit = currArrParGates.begin(); vit!=currArrParGates.end(); ++vit, j++){
      errs() << j << " -- ";
      for(unsigned int i=0;i<RES_CONSTRAINT;i++)
        errs() << (*vit).typeOfGate[i] << " : " << (*vit).numGates[i] << " ; ";
      errs() << "\n";
    }

  }

  void GenLPFSSched::print_qgate(qGate qg){
    errs() << qg.qFunc->getName() << " : ";
    for(int i=0;i<qg.numArgs;i++){
      errs() << qg.args[i].name << qg.args[i].index << ", "  ;
    }
    errs() << "\n";
  }

  uint64_t GenLPFSSched::get_ts_to_schedule(Function* F, uint64_t ts, Function* funcToSched, uint64_t& first_step){
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


  uint64_t GenLPFSSched::get_ts_to_schedule_leaf(Function* F, uint64_t ts, Function* funcToSched, uint64_t& first_step){

    //errs() << " funcTOSched = " << funcToSched->getName() << "\n";
    //errs() << " Size of currSched = " << currArrParGates.size() << "\n";

    //F is leaf. Treat all incoming functions with respect  
    int funcIndex = -1;

    assert(funcToSched->getName().str().find("llvm.")!=std::string::npos &&  "Non-Intrinsic Func Found in Leaf Function"); 

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

    void GenLPFSSched::cleanupCurrArrParGates(){
      currArrParGates.clear();    
    }

    /*bool GenLPFSSched::checkTgatePar(Function* F, uint64_t par){
    //is function leaf?
    vector<Function*>::iterator vit = find(isLeaf.begin(), isLeaf.end(), F);
    if(vit==isLeaf.end()) //not a leaf
    return true;
    //is Leaf
    //iterate over currArrParGates and check for 
    }*/

    void GenLPFSSched::save_blackbox_info(Function* F){
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


    void GenLPFSSched::print_critical_info(){
      errs() << "Timesteps = " << currArrParGates.size() << "\n";
      for(unsigned int i = 0; i<currArrParGates.size(); i++){
        errs() << i << " :";
        for(unsigned int k=0;k<RES_CONSTRAINT;k++){      
          errs() << currArrParGates[i].typeOfGate[k] << " : " << currArrParGates[i].numGates[k] << " / ";
        }
        errs() << "\n";
      }
    }

    void GenLPFSSched::print_parallelism(Function* F){
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

    uint64_t GenLPFSSched::find_max_funcQbits(){
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

    void GenLPFSSched::memset_funcQbits(uint64_t val){
      for(map<string, map<int,uint64_t> >::iterator mIter = funcQbits.begin(); mIter!=funcQbits.end(); ++mIter){
        for(map<int,uint64_t>::iterator arrIter = (*mIter).second.begin(); arrIter!=(*mIter).second.end();++arrIter)
          (*arrIter).second = val;
      }
    }

    void GenLPFSSched::print_scheduled_gate(qGate qg, uint64_t ts){
      string tmpGateName = qg.qFunc->getName();
      if(tmpGateName.find("llvm.")!=std::string::npos)
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

    void GenLPFSSched::print_tableFuncQbits(){
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

    /*
       void GenLPFSSched::print_ready_queue(){
       for(vector<op>::iterator r1 = readyQueue.begin(); r1!=readyQueue.end(); ++r1){
       errs() << "READY QUEUE ENTRY " << (*r1).name.qFunc->getName() << "\n";
       }
       }
     */
    void GenLPFSSched::print_funcList(){
      if(!(funcList.empty())){
        for(vector<pair<Instruction*, op > >::iterator f1 = funcList.begin(); f1 != funcList.end(); ++f1){
          errs() << "\n #Function List Entry: " << (*f1).first; 
          print_qgate((*f1).second.name); 
        }      
      }
    } 
    /*
       void GenLPFSSched::print_vectQbit(){
       for(vector<Value*>::iterator vq1 = vectQbit.begin(); vq1!=vectQbit.end(); ++vq1){
       errs() << "#Vector Qubit Entry: " << (*vq1) << "\n";
       }
       }  
     */
    void GenLPFSSched::print_mapCalls(){
      for(map<Instruction*, op>::iterator mp = mapCalls.begin(); mp!=mapCalls.end(); ++mp){
        errs() << "INSTRUCTION: " << (*mp).first << " timestep " << mapCalls[(*mp).first].ts << " Dist: " << mapCalls[(*mp).first].dist << " | Followed: " << mapCalls[(*mp).first].followed << " qGate: ";
        print_qgate(mapCalls[(*mp).first].name);
        //    errs() << " Followed? " << (*mp).second.followed << "\n"; 
      }
    }

    void GenLPFSSched::print_mapCallsEdges(){
      for(map<Instruction*, op>::iterator mp = mapCalls.begin(); mp!=mapCalls.end(); ++mp){
        errs() << "INST_LABEL: " << (*mp).first << "\n In_Edges: ";
        for(int i = 0; i < (int)(*mp).second.in_edges.size(); ++i)
          errs() <<  (*mp).second.in_edges[i] << " ";
        errs() << "\n Out_Edges: ";
        for(int i = 0; i < (int)(*mp).second.out_edges.size(); ++i)
          errs() << (*mp).second.out_edges[i] << " ";
        errs() << "\n";
      }
    }



    void GenLPFSSched::print_priorityVector(){
      for(vector<InstPri>::iterator pvit = priorityVector.begin(); pvit != priorityVector.end(); ++pvit) 
        //    errs() << "#PRIORITY VECTOR ENTRY: " << (*pvit).second << " " << (*pvit).second << "\n";
        errs() << "#PRIORITY VECTOR ENTRY: " << (*mapCalls.find((*pvit).first)).second.name.qFunc->getName() << "\n";
    }

    void GenLPFSSched::print_longPath(){
      errs() << "\n Longest Path: \n";
      int i = 1;
      for(vector<Instruction*>::reverse_iterator rlp = longPath.rbegin(); rlp != longPath.rend(); ++rlp){
        errs() << i++ << " - " << (*mapCalls.find(*rlp)).second.id << " "; 
        print_qgate((*mapCalls.find(*rlp)).second.name);

      }
    }

    void GenLPFSSched::print_schedule(Function* F, int op_count){
      int ts = 0;
      while(ts < op_count){
        multimap<int, move>::iterator moveOper = move_schedule.find(ts); 
        multimap<int, move>::iterator bmoveOper = local_move_schedule.find(ts); 
        while((moveOper != move_schedule.end()) && ((*moveOper).first == ts)){
          errs() << (*moveOper).first << ",0 TMOV " << (*moveOper).second.dest << " " << (*moveOper).second.src << " " <<  (*moveOper).second.arg.name << (*moveOper).second.arg.index << "\n";
          moveOper++;
        }
        while((bmoveOper != local_move_schedule.end()) && ((*bmoveOper).first == ts)){
          errs() << (*bmoveOper).first << ",0 BMOV " << (*bmoveOper).second.dest << " " << (*bmoveOper).second.src << " " <<  (*bmoveOper).second.arg.name << (*bmoveOper).second.arg.index << "\n";
          bmoveOper++;
        }
        for(map<int, multimap<int, op> >::iterator pit = schedule.begin(); pit != schedule.end(); pit++){
          if(!(*pit).second.empty()){
            multimap<int, op>::iterator oper = (*pit).second.find(ts);
            while(oper != (*pit).second.end() && (*oper).first == ts){
              errs() << (*oper).first << "," << (*oper).second.simd << " ";
              string tmpName = (*oper).second.name.qFunc->getName();
              if( tmpName.find("llvm.") != std::string::npos) {
                unsigned firstDotPos = tmpName.find('.');
                unsigned secondDotPos = tmpName.find('.', firstDotPos+1);
                if (firstDotPos == secondDotPos)
                  errs() << tmpName.substr(firstDotPos+1, std::string::npos);
                else
                  errs() << tmpName.substr(firstDotPos+1, secondDotPos-firstDotPos-1);
              }
              else 
                errs() << tmpName;
              //                    errs() << "Args of this function: " << (*oper).second.name.numArgs << "\n";
              for(int i = 0; i<(*oper).second.name.numArgs; i++){
                errs() << " " << (*oper).second.name.args[i].name;
                if((*oper).second.name.args[i].index != -1) errs() << (*oper).second.name.args[i].index;
              }
              //                    errs() << " : Path = " << (*oper).second.path << " : ID = " << (*oper).second.id;
              errs() << "\n";
              oper++; 

            }
          }
        }
        ts++;
      }
    }

    void GenLPFSSched::print_moves_schedule(Function* F, int op_count){
      int ts = 0;
      errs() << "MOVE LIST SIZE: " << move_schedule.size() << "\n";
      for(multimap<int, move>::iterator mit = move_schedule.begin(); mit != move_schedule.end(); mit++){
        while((*mit).first == ts){
          errs() << (*mit).first << ",0 TMOV " << (*mit).second.dest << " " << (*mit).second.src << " " << (*mit).second.arg.name << (*mit).second.arg.index << "\n";
          mit++;
        }
        ts++;
      }
    }


    void GenLPFSSched::print_local_moves_schedule(Function* F, int op_count){
      int ts = 0;
      multimap<int, move>::iterator mit;
      for(mit = local_move_schedule.begin(); mit != local_move_schedule.end(); mit++){
        while((*mit).first == ts){
          errs() << (*mit).first << ",0 BMOV " << (*mit).second.dest << " " << (*mit).second.src << " " << (*mit).second.arg.name << (*mit).second.arg.index << "\n";
          mit++;
        }
        ts++;
      }
    }

    void GenLPFSSched::print_schedule_metrics(Function* F, int op_count){
      int moves_count = move_schedule.size();
      int bmoves_count = local_move_schedule.size();
      for(multimap<int, move>::iterator mit = move_schedule.begin(); mit != move_schedule.end(); mit = move_schedule.upper_bound(mit->first)){
        mts++;
      }

      errs() << "ops = " << op_count << "\n";
      errs() << "tmoves = " << moves_count << "\n";
      errs() << "bmoves = " << bmoves_count << "\n";
      errs() << "ots = " << ots << "\n";
      errs() << "mts = " << mts << "\n";
      errs() << "ts = " << (ots - mts) + (mts * 5) << "\n";
      errs() << "SIMDs = " << simds << "\n";
      errs() << "tgates = " << tgates_cnt << "\n";

    }

    void GenLPFSSched::calc_critical_time(Function* F, qGate qg, bool isLeafFunc){
      string fname = qg.qFunc->getName();

      //print_qgate(qg);
      uint64_t max_ts_of_all_args = 0;
      uint64_t first_step = 0;
      if(isFirstMeas && (fname.find("llvm.MeasX") != std::string::npos || fname.find("llvm.MeasZ") != std::string::npos)){
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
        //    print_scheduled_gate(qg,max_ts_sched+1);
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

        //errs() << "Max timestep for all args = " << max_ts_of_all_args << "\n";

        //find timestep from max_ts_of_all_args where type of gate is same as this gate or no gate has been scheduled.
        uint64_t ts_sched;

        if(isLeafFunc)
          ts_sched = get_ts_to_schedule_leaf(F,max_ts_of_all_args, qg.qFunc, first_step);
        else
          ts_sched = get_ts_to_schedule(F,max_ts_of_all_args, qg.qFunc, first_step);

        //errs() << "ts_sched = " << ts_sched << " FirstStep = " << first_step << "\n";

        //schedule gate in max_ts_of_all_args + 1th timestep = ts_sched+1


        //      print_scheduled_gate(qg,first_step+1);


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
    }

    uint64_t GenLPFSSched::calc_critical_time_unbounded(Function* F, qGate qg){
      string fname = qg.qFunc->getName();

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

      return max_ts_of_all_args+1;

    } //calc_critical_time_unbounded


    bool GenLPFSSched::checkIfIntrinsic(Function* CF){
      if(CF->isIntrinsic()){
        if((CF->getIntrinsicID() == Intrinsic::CNOT)
            || (CF->getIntrinsicID() == Intrinsic::Fredkin)
            || (CF->getIntrinsicID() == Intrinsic::H)
            || (CF->getIntrinsicID() == Intrinsic::MeasX)
            || (CF->getIntrinsicID() == Intrinsic::MeasZ)
            || (CF->getIntrinsicID() == Intrinsic::PrepX)
            || (CF->getIntrinsicID() == Intrinsic::PrepZ)
            || (CF->getIntrinsicID() == Intrinsic::Rx)
            || (CF->getIntrinsicID() == Intrinsic::Ry)
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


    void GenLPFSSched::analyzeCallInst(Function* F, Instruction* pInst){
      if(CallInst *CI = dyn_cast<CallInst>(pInst))
      {      
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

          //        errs() << "Checking Inst Types \n";
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
          //check if Intrinsic
          bool thisFuncIsIntrinsic = checkIfIntrinsic(CI->getCalledFunction());
          if(!thisFuncIsIntrinsic) {
            hasPrimitivesOnly = false;
            //            string gname = CI->getCalledFunction()->getName();
            //            errs() << "Non-Instric Func is: " << gname << "\n";
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
              qGateArg param =  allDepQbit[vb];       
              thisGate.args[thisGate.numArgs].name = param.argPtr->getName();
              if(!param.isPtr)
                thisGate.args[thisGate.numArgs].index = param.valOrIndex;
              thisGate.numArgs++;
            }
          }

          //       errs() << "Calc Crit Times\n";
          uint64_t thisTS = calc_critical_time_unbounded(F,thisGate);       
          //update priorityVector
          priorityVector.push_back(make_pair(pInst,thisTS));


          //add to mapInstSet
          mapCalls[pInst].name = thisGate;

          op newOp;
          newOp.name = thisGate;
          //funcList.push_back(make_pair(pInst,newOp));


        }    
        allDepQbit.erase(allDepQbit.begin(),allDepQbit.end());
        }
      }


      void GenLPFSSched::saveTableFuncQbits(Function* F){
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


      bool GenLPFSSched::analyzeIntrinsicCallInst(Function *F, Instruction *Inst){
        if(CallInst *CI = dyn_cast<CallInst>(Inst)){
          bool thisFuncIsIntrinsic = checkIfIntrinsic(CI->getCalledFunction());
          if(!thisFuncIsIntrinsic) {
            return false;
          }
        }
        return true;
      }

      bool GenLPFSSched::DetermineLeafFunction (Function *F) {
        bool isALeaf = false;
        vector<Instruction*> InstList;
        for (inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E; ++I) {
          Instruction *Inst = &*I;
          if(CallInst *CI = dyn_cast<CallInst>(Inst)){
            op newOp;
            string called_func_name = CI->getCalledFunction()->getName();
            InstList.push_back(Inst);
          }
        } 
        for(vector<Instruction*>::reverse_iterator rit = InstList.rbegin(); rit!=InstList.rend(); ++rit){
          isALeaf = analyzeIntrinsicCallInst(F,(*rit));  
          if (!isALeaf) {
            if( dyn_cast<CallInst>((*rit))->getCalledFunction()->getName() != "store_cbit" )
              return false;
          }
        }
        if(isALeaf) {
          isLeaf.push_back(F);
          return true;
        }
        return false;
      }


      void GenLPFSSched::CountCriticalFunctionResources (Function *F) {
        // Traverse instruction by instruction
        init_critical_path_algo(F);

        //get qbits in function
        for (inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E; ++I) {
          Instruction *Inst = &*I;                            // Grab pointer to instruction reference
          analyzeAllocInst(F,Inst);          
          if(!isa<AllocaInst>(Inst))
            break;
        }

        //  errs() << "Finding priorities--- \n";
        //find priorities for instructions
        for (inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E; ++I) {
          Instruction *Inst = &*I;
          if(CallInst *CI = dyn_cast<CallInst>(Inst)){
            op newOp;
            string called_func_name = CI->getCalledFunction()->getName();
            mapCalls.insert(make_pair(Inst, newOp));
            callList.push_back(Inst);
            // errs() << "Added instruction: " << Inst << ": " << called_func_name << "\n";
            if(F->getName() == "measure") {
              // errs() << "Added Inst " << called_func_name << " : " << Inst << " to call list for measure \n";
            }
          }
        } 

        //traverse in reverse sequence
        //  errs() << "Beginning analysis" << "\n";
        for(vector<Instruction*>::reverse_iterator rit = callList.rbegin(); rit!=callList.rend(); ++rit){
          //        errs() << "Analyzing: " << mapCalls.find(*rit)->second.name.qFunc->getName() << "\n";
          //        errs() << "Analyzing: " << dyn_cast<CallInst>(*rit)->getCalledFunction()->getName() << "\n";
          analyzeCallInst(F,(*rit));  
        }
        //  errs() << "Finished Analyzing" << "\n";
        //is function leaf or not?
        //  if(hasPrimitivesOnly) isLeaf.push_back(F);

        //sort vector
        sort(priorityVector.begin(), priorityVector.end(), CompareInstPriByValue());

        //reset funcQbits vector in preparation for scheduling
        memset_funcQbits(0);

        //errs() << "Finding Schedule--- \n";

        for(vector<InstPri>::reverse_iterator vit = priorityVector.rbegin(); vit!=priorityVector.rend(); ++vit){
          //get qgate
          //    errs() << "priority scheduling..." << "\n";
          map<Instruction*, op>::iterator mit = mapCalls.find((*vit).first);
          assert(mit!=mapCalls.end() && "Instruction Not Found in MapInstSet.");
          qGate thisGate = (*mit).second.name;
          //    if(!(F->getName() == "main")) { 
          //        errs() << "CHECK: " << thisGate.qFunc->getName() << "\n";
          //      if(hasPrimitivesOnly)
          //        calc_critical_time(F,thisGate,true);
          //      else
          //        calc_critical_time(F,thisGate,false);
          //    }
        }
      }


      void GenLPFSSched::init_gates_as_functions(){
        //add blackbox entry for each of these ??
        for(int  i =0; i< NUM_QGATES ; i++){
          string gName = gate_name[i];
          string fName = "llvm.";
          fName.append(gName);
        }
      }


      bool GenLPFSSched::runOnModule (Module &M) {
        init_gate_names();
        init_gates_as_functions();
        int timeStep = 0; 
        errs() << "M: $::SIMD_K=" << RES_CONSTRAINT <<"; $::SIMD_D=" << DATA_CONSTRAINT << "; $::SIMD_L=" << SIMD_L << "\n"; 

        // iterate over all functions, and over all instructions in those functions
        CallGraphNode* rootNode = getAnalysis<CallGraph>().getRoot();
        //Post-order
        for (scc_iterator<CallGraphNode*> sccIb = scc_begin(rootNode), E = scc_end(rootNode); sccIb != E; ++sccIb) {
          const std::vector<CallGraphNode*> &nextSCC = *sccIb;
          for (std::vector<CallGraphNode*>::const_iterator nsccI = nextSCC.begin(), E = nextSCC.end(); nsccI != E; ++nsccI) {
            Function *F = (*nsccI)->getFunction();      
            if(F && !F->isDeclaration()){

              funcQbits.clear();
              funcArgs.clear();
              mapCalls.clear();
              funcList.clear();
              mapInstSet.clear();
              priorityVector.clear();
              longPath.clear();
              callList.clear();
              qubitMap.clear();
              localMemSizeMap.clear();
              regionSizeMap.clear();
              mts = 0;    
              ots = 0;    
              simds = 0; 
              tgates_cnt = 0;   

              getFunctionArguments(F);

              for(int k = 1; k <= (int) RES_CONSTRAINT; k++){
                localMemSizeMap.insert(make_pair(k*10, 0));
                regionSizeMap.insert(make_pair(k, 0));
              }

              // count the critical resources for this function
              if (DetermineLeafFunction(F)){
                CountCriticalFunctionResources(F);
              }
              vector<Function*>::iterator vit = find(isLeaf.begin(), isLeaf.end(), F);
              if(vit != isLeaf.end()){
                errs() << "\nLPFS:\n";
                errs() << "Function: " << F->getName() 
                  << " (sched: lpfs, k: " << RES_CONSTRAINT 
                  << ", d: " << DATA_CONSTRAINT 
                  << " l: " << SIMD_L 
                  << ", opp: " << OPP_SIMD 
                  << ", refill: " << REFILL 
                  << ") \n"; 
                errs() << "==================================================================\n";
                lpfs(F, timeStep, SIMD_L, REFILL, OPP_SIMD);
              }

              int op_count = callList.size();
              if(!(schedule.empty())) {
                if(METRICS)
                  print_schedule_metrics(F,op_count);
                if(FULL_SCHED)
                  print_schedule(F,op_count);
                if(MOVES_SCHED)
                  print_moves_schedule(F,op_count);
                if(LOCAL_MOVES_SCHED)
                  print_local_moves_schedule(F, op_count);
              }
              schedule.clear();
              move_schedule.clear();
              local_move_schedule.clear();
              active_qubits.clear();
              cleanupCurrArrParGates();
            }
          }
        }
        return false;
      } // End runOnModule
