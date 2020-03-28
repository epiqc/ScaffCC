//===- GenQASM.cpp - Generate qasm output -------------------===//
//
//                     The LLVM Scaffold Compiler Infrastructure
//
// This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#include <sstream>
#include <algorithm>
#include <string>
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
#include "llvm/IR//InstIterator.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/ilist.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/IntrinsicInst.h"

using namespace llvm;
using namespace std;

#define MAX_BACKTRACE_COUNT 15 //max backtrace allowed - to avoid infinite recursive loops
#define MAX_QBIT_ARR_DIM 5 //max dimensions allowed for qbit arrays

bool debugGenQASM = false;

namespace {

  enum argType{
    /* Quantum Type. */
    qbit,
    abit,
    cbit,
    /* Classical Type. */
    intVal,
    doubleVal,
    /* Not defined. */
    undef
  };

  /* Non-general case for backtraceOperand_. */
  enum backtraceExp{
    cmpConstant,
    ptrToArray,
    nonExp
  };

  enum setupExp{
    funcArg,
    alloca
  };

  struct dataRepresentation{
    Value * instPtr;
    argType type;
    bool isPtr;
    vector<int> index; // in the reverse order
    vector<int> dimSize; // sizes of dimensions vector

    /* Classical Inst. */
    int intValue;
    double doubValue;
    bool isClassical();
    string val();

    /* Quantum Inst. */
    bool isqbit();
    bool iscbit();
    void printQRegisterName();
    string registerType();
    /* Used in conditional statement, since OpenQASM only support array-wise measurement. */
    string cbitArrayString();

    dataRepresentation() : instPtr(NULL), type(undef), isPtr(false), index(std::vector<int>()), dimSize(vector<int>()){}

    string getName(){
      string name = instPtr->getName();
      replace(name.begin(), name.end(), '.', '_');
      return name; }
    void printDebugMode();
  };

  string dataRepresentation::registerType(){
    stringstream ss;
    if(isClassical()){
      if(instPtr->getType()->isFloatTy()){
        ss << "float ";
      }else if(instPtr->getType()->isDoubleTy()){
        ss << "double ";
      }else if(instPtr->getType()->isIntegerTy(32)){
        ss << "int ";
      }else{
        ss << "UNRECOGNIZED " << instPtr->getType() << " ";
      }
    }else{
      if(isqbit()){
        ss << "qbit ";
      }else if(iscbit()){
        ss << "cbit ";
      }else{
        ss << "UNRECOGNIZED " << instPtr->getType() << " ";
      }
    }
    if(isPtr) ss << "*";
    return ss.str();
  }

  bool dataRepresentation::isClassical(){
    if(type == intVal || type == doubleVal) return true; else return false;
  }

  string dataRepresentation::val(){
    stringstream ss;
    if(type == intVal) ss << " " << intValue << " ";
    if(type == doubleVal) ss << " " << doubValue << " ";
    return ss.str();    
  }

  bool dataRepresentation::isqbit(){
    if(type == qbit || type == abit) return true; else return false;
  }

  bool dataRepresentation::iscbit(){
    if(type == cbit) return true; else return false;
  }

  void dataRepresentation::printQRegisterName(){
    switch(type){
      case undef:
        errs() << " UNDEF ";
        break;
      case qbit:
      case abit:
      case cbit:
        errs() << getName();
        break;
      default:
        errs() << " error ";
        break;
    }
  }

  void dataRepresentation::printDebugMode(){
    if(isClassical()){
      /* Printing Classical Value. */
      errs() << "\t\tPrinting Constant: ";
      if(type == intVal) errs() << intValue << "\n";
      if(type == doubleVal) errs() << doubValue << "\n";
    }else{
      errs() << "\t\tPrinting Quantum Register:\n";
      errs() << "\t\tName: ";
      printQRegisterName();
      errs() << "\n";
      errs() << "\t\tType: ";
      switch(type){
        case undef:
          errs() << "UNDEF\n";
          break;
        case abit:
        case qbit:
          errs() << "qubit\n";
          break;
        case cbit:
          errs() << "cbit\n";
          break;
        default:
          errs() << "error\n";
          break;
      }
      errs() << "\t\tSize: ";
      for(unsigned i = 0; i < dimSize.size(); i++){
        errs() << "[" << dimSize[i] << "]";
      }
      errs() << "\n";
      errs() << "\t\tIndex: ";
      if(index.size() == 0) errs() << "Not applied.\n";
      for(unsigned i = 0; i < index.size(); i++){
        errs() << "[" << index[i] << "]";
      }
      errs() << "\n";
    }
  }

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
  std::vector<dataRepresentation> qArgs_;
    std::vector<qGateArg> qArgs;
  };

  bool isAllocQuantumType(Type * allocatedType){
    if(allocatedType->isIntegerTy(16)){
      return true;
    }else if(allocatedType->isIntegerTy(8)){
      return true;
    }else if(allocatedType->isIntegerTy(1)){
      return true;
    }else if(allocatedType->isPointerTy()){
      return isAllocQuantumType(allocatedType->getPointerElementType());
    }else if(ArrayType * arrayType = dyn_cast<ArrayType>(allocatedType)){
      return isAllocQuantumType(arrayType->getElementType());
    }else{
      return false;
    }
  }

  argType quantumRegisterSetupHelper(dataRepresentation * qRegister, Type * type){
    if(type->isIntegerTy(16)){
      return qbit;
    }else if(type->isIntegerTy(8)){
      return abit;
    }else if(type->isIntegerTy(1)){
      return cbit;
    }else if(type->isArrayTy()){
      ArrayType * arrayType = dyn_cast<ArrayType>(type);
      qRegister->dimSize.push_back(arrayType->getNumElements());
      return quantumRegisterSetupHelper(qRegister, arrayType->getElementType());
    }else if(type->isPointerTy()){
      qRegister->isPtr = true;
      return quantumRegisterSetupHelper(qRegister, type->getPointerElementType());
    }else{
      return undef;
    }
  }

  void quantumRegisterSetup(dataRepresentation * qRegister){
    Type * allocatedType;
    if(AllocaInst * AI = dyn_cast<AllocaInst>(qRegister->instPtr)){
      allocatedType = AI->getAllocatedType();
      allocatedType = qRegister->instPtr->getType();
    }else{
      allocatedType = qRegister->instPtr->getType();
    }

    qRegister->type = quantumRegisterSetupHelper(qRegister, allocatedType);
  }

  void classicalRegisterSetup(dataRepresentation * cRegister){
    if(cRegister->instPtr == NULL){
      return;
    }else if(ConstantInt * CInt = dyn_cast<ConstantInt>(cRegister->instPtr)){
      cRegister->type = intVal;
      cRegister->intValue = CInt->getSExtValue();
      return;
    }else if(ConstantFP * CFP = dyn_cast<ConstantFP>(cRegister->instPtr)){
      cRegister->type = doubleVal;
      cRegister->doubValue = CFP->getValueAPF().convertToDouble();
      return;
    }else{
      Type *type = cRegister->instPtr->getType();
      if(type->isIntegerTy(32)){
        cRegister->type = intVal;
        //cRegister->name = cRegister->instPtr->getName();
        return;
      }
      else if(type->isDoubleTy()){
        cRegister->type = doubleVal;
        //cRegister->name = cRegister->instPtr->getName();
        return;
      }
      errs() << "Unhandled Case!\n";
      return;
    };
  }

  struct GenQASM : public ModulePass {
    static char ID;  // Pass identification, replacement for typeid
    GenQASM() : ModulePass(ID){}
    std::vector<Value*> vectQbit;

    std::vector<qGateArg> tmpDepQbit;
    std::vector<dataRepresentation> tmpDepQbit_;
    std::vector<qGateArg> allDepQbit;
    std::vector<dataRepresentation> allDepQbit_;


    map<Function*, vector<qGateArg> > mapQbitsInit;
    map<Function *, vector<dataRepresentation> > mapQbitsInit_;
    map<Function*, vector<qGateArg> > mapFuncArgs;
    map<Function *, vector<dataRepresentation> > mapFuncArgs_;
    map<Function*, vector<FnCall> > mapMapFunc;
    map<BasicBlock *, vector<FnCall> > fnCallTable; 


    vector<qGateArg> qbitsInFunc; //qbits in function
    vector<dataRepresentation> qbitsInCurrentFunc;
    vector<qGateArg> qbitsInitInFunc; //new qbits declared in function
    vector<dataRepresentation> qbitsInitInCurrentFunc;
    vector<qGateArg> funcArgList; //function arguments
    vector<dataRepresentation> funcArgList_;
    vector<FnCall> mapFunction; //trace sequence of qgate calls
    vector<FnCall> fnCall;
    map<Value*, qGateArg> mapInstRtn;    //traces return cbits for Meas Inst
    map<Value*, dataRepresentation> mapInstRtn_;

    /* If the block branches to more than one successors. */
    map<BasicBlock *, vector<dataRepresentation> > basicBlockCondTable;

    int btCount; //backtrace count
    int backtraceCount;

    dataRepresentation backtraceOperand(Value * operand, backtraceExp exp);
    void backtraceOperand_helper(dataRepresentation * datRepPtr, Value * operand, int gettingIndex, backtraceExp exp);

    bool getQbitArrDim(Type* instType, qGateArg* qa);
    bool backtraceOperand_(Value* opd, int opOrIndex);
    void analyzeAllocInst(Function* F,Instruction* pinst);
    void analyzeCallInst(Function* F,Instruction* pinst);
    void analyzeInst(Function* F,Instruction* pinst);
    void analyzeInst_block(BasicBlock * basicBlock, Instruction * pInst);

    void processStoreCbitInst(CallInst * pInst);
    void processCallInst(CallInst * pInst);
    void processConditionInst(BasicBlock * basicBlock, BranchInst * branchInst);

    // run - Print out SCCs in the call graph for the specified module.
    bool runOnModule(Module &M);

    void printFuncHeader(Function* F, bool mainFunc);

    string to_string(int var);

    string printVarName(StringRef s)
    {
      std::string sName = s.str();
      std::replace(sName.begin(), sName.end(), '.', '_');

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

    void genQASM(Function* F);
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

char GenQASM::ID = 0;
static RegisterPass<GenQASM>
X("gen-qasm", "Generate QASM output code"); //spatil: should be Z or X??

void GenQASM::backtraceOperand_helper(dataRepresentation * datRepPtr, Value * operand, int gettingIndex, backtraceExp exp){

  if(backtraceCount > MAX_BACKTRACE_COUNT){
    errs() << "Exceed max backtrace count...";
    return;
  }else{
    backtraceCount++;
  }

  if(AllocaInst * AI = dyn_cast<AllocaInst>(operand)){
    if(debugGenQASM)
      errs() << "\n\t\tAlloca inst Found: " << *AI << "\n";
    datRepPtr->instPtr = AI;
    if(datRepPtr->isPtr){
      for(vector<dataRepresentation>::iterator vvit = qbitsInitInCurrentFunc.begin(),
        vvitE = qbitsInitInCurrentFunc.end(); vvit != vvitE; ++vvit){
        if((*vvit).iscbit()){
          (*vvit).isPtr = true;
        }
      } 
    }
    return;

  }else if(GetElementPtrInst * GEPI = dyn_cast<GetElementPtrInst>(operand)){
    if(debugGenQASM)
      errs() << "\n\t\tGetElemPtr inst Found: " << *GEPI << "\n";
    /* Get index. */
    if(GEPI->getNumOperands() == 3){
      if(exp == ptrToArray){
        ConstantInt * CInt = dyn_cast<ConstantInt>(GEPI->getOperand(2));
        int index = CInt->getSExtValue();
        if(debugGenQASM) errs() << "\t\t[" << gettingIndex << "] Index: " << index << "\n";
        backtraceOperand_helper(datRepPtr, GEPI->getOperand(0), gettingIndex, exp);
      }else if(GEPI->getOperand(2)->getType()->isIntegerTy(32)){
        /* Merely a pointer. */
        datRepPtr->isPtr = true;
        backtraceOperand_helper(datRepPtr, GEPI->getOperand(0), gettingIndex, exp);
      }else{
        ConstantInt * CInt = dyn_cast<ConstantInt>(GEPI->getOperand(2));
        int index = CInt->getSExtValue();
        if(debugGenQASM) errs() << "\t\t[" << gettingIndex << "] Index: " << index << "\n";
        datRepPtr->index.push_back(index);
        backtraceOperand_helper(datRepPtr, GEPI->getOperand(0), gettingIndex, exp);
      }
    }else if(GEPI->getNumOperands() == 2){
      ConstantInt * CInt = dyn_cast<ConstantInt>(GEPI->getOperand(1));
      int index = CInt->getSExtValue();
      if(debugGenQASM) errs() << "\t\t[" << gettingIndex << "] Index: " << index << "\n";
      datRepPtr->index.push_back(index);
      backtraceOperand_helper(datRepPtr, GEPI->getOperand(0), gettingIndex, ptrToArray);
    }else{
      errs() << "UNHANDLED GEPI CASE, BOOOOOOOOOO!\n";
    }
    return;

  }else if(CallInst * CI = dyn_cast<CallInst>(operand)){
    if(debugGenQASM)
      errs() << "\n\t\tCall inst Found: " << *CI << "\n";
    if(CI->getCalledFunction()->getName().find("llvm.Meas") != string::npos){
      if(debugGenQASM) errs() << "\n\t\tBACKTRACE OPERAND QUBIT: \n";
      backtraceOperand_helper(datRepPtr, CI->getOperand(0), gettingIndex, exp);
    }
    return;
  }else if(PtrToIntInst * PTTI = dyn_cast<PtrToIntInst>(operand)){
    if(debugGenQASM)
      errs() << "\n\t\tPtrToInt inst Found: " << *PTTI << "\n";
    backtraceOperand_helper(datRepPtr, PTTI->getOperand(0), 0, exp);
    return;
  }else if(ConstantInt * CInt = dyn_cast<ConstantInt>(operand)){
    datRepPtr->instPtr = CInt;
    if(debugGenQASM) errs() << "\n\t\tInt Constant Found: " << CInt->getSExtValue() << "\n";
    return;
  }else if(ConstantFP * CFP = dyn_cast<ConstantFP>(operand)){
    datRepPtr->instPtr = CFP;
    if(debugGenQASM) errs() << "\n\t\tDouble Constant Found: " << CFP->getValueAPF().convertToDouble() << "\n";
    return;
  }else if(LoadInst * LI = dyn_cast<LoadInst>(operand)){
    if(debugGenQASM) errs() << "\n\t\tLoad inst Found: " << *LI << "\n";
    if(exp == cmpConstant){
      if(debugGenQASM) errs() << "\n\t\tCmp constant is 1.\n";
      datRepPtr->type = intVal;
      datRepPtr->intValue = 1;
      return;
    }else{
      backtraceOperand_helper(datRepPtr, LI->getOperand(0), 0, exp);
      return;
    }
  }else if(CmpInst * CMPI = dyn_cast<CmpInst>(operand)){
    if(debugGenQASM) errs() << "\n\t\tCompare inst Found: " << *CMPI << "\n";
    if(exp == cmpConstant){
      backtraceOperand_helper(datRepPtr, CMPI->getOperand(1), 0, exp);
    }else{
      backtraceOperand_helper(datRepPtr, CMPI->getOperand(0), 0, exp);
    }
    return;
  }else if(ZExtInst * ZEI = dyn_cast<ZExtInst>(operand)){
    if(debugGenQASM) errs() << "\n\t\tZExt inst Found: " << *ZEI << "\n";
    backtraceOperand_helper(datRepPtr, ZEI->getOperand(0), 0, exp);
    return;
  }else if(isa<UndefValue>(operand)){
    datRepPtr->type = undef;
    if(debugGenQASM) errs() << "Undef Inst: " << *operand << "\n";
    return;
  }else if(Argument * arg = dyn_cast<Argument>(operand)){
    if(debugGenQASM)
      errs() << "\n\t\tArgument Found: " << *arg << "\n";
      datRepPtr->instPtr = arg;
    return;
  }else{
    if(debugGenQASM) errs() << "UNHANDLED CASE, BOOOOOOOOOO! Inst: " << *operand << "\n";
    return;
  }
}

dataRepresentation GenQASM::backtraceOperand(Value * operand, backtraceExp exp){

  backtraceCount = 0;

  dataRepresentation returnDR;
  int gettingIndex = 0;

  backtraceOperand_helper(&returnDR, operand, gettingIndex, exp);
  
  return returnDR;
}

bool GenQASM::backtraceOperand_(Value* opd, int opOrIndex)
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

    if(btCount>MAX_BACKTRACE_COUNT)
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

      
      bool foundOne = backtraceOperand_(pInst->getOperand(0),0);

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
      bool foundOne = backtraceOperand_(pInst->getOperand(0),0);

      if(tmpDepQbit[0].isQbit && !(tmpDepQbit[0].isPtr)){     
        //NOTE: getelemptr instruction can have multiple indices. consider last operand as desired index for qubit. Check if this is true for all.
        backtraceOperand_(pInst->getOperand(numOps-1),1);
        
      }
    else if(tmpDepQbit[0].isAbit && !(tmpDepQbit[0].isPtr)){
      backtraceOperand_(pInst->getOperand(numOps-1),1);
    }
      return foundOne;
    }   
    else{     
      Instruction* pInst = dyn_cast<Instruction>(opd);
      unsigned numOps = pInst->getNumOperands();
      bool foundOne = false;
      for(unsigned iop=0;(iop<numOps && !foundOne);iop++){
        foundOne = foundOne || backtraceOperand_(pInst->getOperand(iop),0);
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
    foundOne = foundOne || backtraceOperand_(pInst->getOperand(iop),0);
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
    if(btCount>MAX_BACKTRACE_COUNT) //prevent infinite backtracing
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
  foundOne = foundOne || backtraceOperand_(pInst->getOperand(iop),1);
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
      foundOne = foundOne || backtraceOperand_(pInst->getOperand(iop),2);
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
    foundOne = foundOne || backtraceOperand_(pInst->getOperand(iop),2);
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

void GenQASM::analyzeAllocInst(Function* F, Instruction* pInst){
  if (AllocaInst *AI = dyn_cast<AllocaInst>(pInst)){
    Type *allocatedType = AI->getAllocatedType();
    if(isAllocQuantumType(allocatedType)){

      if(debugGenQASM) errs() << "\tNew qubit allocation: " << AI->getName() << "\n";

      dataRepresentation qRegister;
      qRegister.instPtr = AI;
      quantumRegisterSetup(&qRegister);
      
      if(debugGenQASM) qRegister.printDebugMode();

      if(qRegister.instPtr->getName().find(".addr")!= string::npos){
      
        /* Note: Necessary if -mem2reg is not run on IR before.
          Eg, without -mem2reg
            module(i8 * %q){
              %q.addr = alloca i8*, align 8
              ...
            }
            qbit q.addr must be mapped to argement q. */

        if(qRegister.type == qbit || qRegister.type == abit){
          bool qbitExisting = false;
          string qbitname = qRegister.instPtr->getName().str();
          unsigned pos = qbitname.find(".addr");
          qbitname = qbitname.substr(0, pos);
          for(vector<dataRepresentation>::iterator vvit = funcArgList_.begin(),
            vvitE = funcArgList_.end(); ((vvit != vvitE) && !qbitExisting); ++vvit){
            if((*vvit).instPtr->getName() == qbitname) qbitExisting = true;
          }
          
          if(!qbitExisting) qbitsInCurrentFunc.push_back(qRegister);
        }else{
          //qubits/new qubits in function
          qbitsInCurrentFunc.push_back(qRegister);
          qbitsInitInCurrentFunc.push_back(qRegister);
        }
      }else{
        //qubits/new qubits in function
        qbitsInCurrentFunc.push_back(qRegister);
        qbitsInitInCurrentFunc.push_back(qRegister);
      }
    }
/* =========================================================================================== */
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

  if(debugGenQASM)
  print_qgateArg(tmpQArg);
  }   
  }

  }
  else if(allocatedType->isIntegerTy(16)){
  qGateArg tmpQArg;
  if(debugGenQASM) errs() << "Found New Qbit Allocation \n";
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
/* =========================================================================================== */
    return;
  }
}

void GenQASM::analyzeCallInst(Function* F, Instruction* pInst){
  if(CallInst *CI = dyn_cast<CallInst>(pInst))
    {
      if(debugGenQASM)      
  errs() << "Call inst: " << CI->getCalledFunction()->getName() << "\n";

      if(CI->getCalledFunction()->getName() == "store_cbit"){ //trace return values
  qGateArg tmpQGateArg1;
  tmpQGateArg1.isCbit = true;
  tmpDepQbit.push_back(tmpQGateArg1);
  backtraceOperand_(CI->getArgOperand(0),2); //value Operand
  Value* rtnVal = tmpDepQbit[0].argPtr;
  tmpDepQbit.clear();

  qGateArg tmpQGateArg2;
  tmpQGateArg2.isCbit = true;
  tmpQGateArg2.isPtr = true;
  tmpDepQbit.push_back(tmpQGateArg2); 
  backtraceOperand_(CI->getArgOperand(1),0); //pointer Operand

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
  
  tracked_all_operands &= backtraceOperand_(CI->getArgOperand(iop),0);
  
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

void GenQASM::processStoreCbitInst(CallInst * CI){
  Value * rtnVal_ = CI->getArgOperand(0);
  tmpDepQbit_.clear();

  if(debugGenQASM) errs() << "\n\t\tBACKTRACE OPERAND CBIT: \n";
  dataRepresentation cbit = backtraceOperand(CI->getArgOperand(1), nonExp);
  quantumRegisterSetup(&cbit);
  if(debugGenQASM) errs() << "\t\tBacktrace End, Cbit Found: " << "\n";
  if(debugGenQASM) cbit.printDebugMode();
  tmpDepQbit_.push_back(cbit);
  mapInstRtn_[rtnVal_] = tmpDepQbit_[0];
  tmpDepQbit_.clear();

  return;
}
void GenQASM::processCallInst(CallInst * callInst){
  /* Traverse all argument operand in call inst. */
  for(unsigned iOperand = 0; iOperand < callInst->getNumArgOperands(); iOperand++){

    tmpDepQbit_.clear();

    if(debugGenQASM) errs() << "\tANALYZE CALL INST OPERAND NUM: " << iOperand << "\n";
    dataRepresentation argument = backtraceOperand(callInst->getArgOperand(iOperand), nonExp);
    Type* argType = callInst->getArgOperand(iOperand)->getType();

    if(isAllocQuantumType(argType)){
      quantumRegisterSetup(&argument);
      tmpDepQbit_.push_back(argument);
      if(debugGenQASM) argument.printDebugMode();
    }else{
      classicalRegisterSetup(&argument);
      tmpDepQbit_.push_back(argument);
      if(debugGenQASM) argument.printDebugMode();
    }
      
    allDepQbit_.push_back(tmpDepQbit_[0]);
    tmpDepQbit_.clear();
  }
        
  //form info packet
  FnCall fnCallInfoPack;
  fnCallInfoPack.func = callInst->getCalledFunction();
  fnCallInfoPack.instPtr = callInst;

  if(allDepQbit_.size() > 0){
    for(unsigned iArg = 0; iArg < allDepQbit_.size(); iArg++)
      fnCallInfoPack.qArgs_.push_back(allDepQbit_[iArg]);
  }
  fnCall.push_back(fnCallInfoPack);
  return;
}
void GenQASM::processConditionInst(BasicBlock * basicBlock, BranchInst * branchInst){
  if(branchInst->getNumOperands() == 3){
    if(debugGenQASM) errs() << "\n\t\tBACKTRACE OPERAND CONDITION: " << "\n";
    dataRepresentation measure = backtraceOperand(branchInst->getOperand(0), cmpConstant);
    classicalRegisterSetup(&measure);
    if(debugGenQASM) errs() << "\t\tBacktrace End, Value Found: " << "\n";
    if(debugGenQASM) measure.printDebugMode();
    dataRepresentation cbit = backtraceOperand(branchInst->getOperand(0), nonExp);
    quantumRegisterSetup(&cbit);
    if(debugGenQASM) errs() << "\t\tBacktrace End, Cbit Found: " << "\n";
    if(debugGenQASM) cbit.printDebugMode();
    vector<dataRepresentation> cond;
    cond.push_back(measure);
    cond.push_back(cbit);
    basicBlockCondTable[basicBlock] = cond;

    if(debugGenQASM)
      errs() << "\n\tIf then block: " << branchInst->getOperand(2)->getName() << "\n";
    if(debugGenQASM)
      errs() << "\n\tIf end block: " << branchInst->getOperand(1)->getName() << "\n";
  }
  return;
}

void GenQASM::analyzeInst_block(BasicBlock * basicBlock, Instruction * pInst){

  unsigned numOps = pInst->getNumOperands();

  if(AllocaInst * AI = dyn_cast<AllocaInst>(pInst)){
    if(debugGenQASM){
      errs() << "\n\tAllocation Instruction: " << *AI << "\n";
      errs() << "\tNum Operands: " << numOps << ";\n";
    }
  }else if(GetElementPtrInst * GEPI = dyn_cast<GetElementPtrInst>(pInst)){
    if(debugGenQASM){
      errs() << "\n\tGetElementPointer Instruction: " << *GEPI << "\n";
      errs() << "\tNum Operands: " << numOps << ";\n";
    }
  }else if(CallInst * CI = dyn_cast<CallInst>(pInst)){
    if(debugGenQASM){
      errs() << "\n\tCall Instruction: " << *CI << "\n";
      errs() << "\tNum Operands: " << numOps << ";\n";
    }
    if(CI->getCalledFunction()->getName() == "store_cbit"){
      processStoreCbitInst(CI);
    }else{
      processCallInst(CI);
    }
  }else if(BranchInst *BI = dyn_cast<BranchInst>(pInst)){
    if(debugGenQASM){
      errs() << "\n\tBranch Instruction: " << *BI << "\n";
      errs() << "\tNum Operands: " << numOps << ";\n";
    }
    processConditionInst(basicBlock, BI);
  }else if(LoadInst *LI = dyn_cast<LoadInst>(pInst)){
    if(debugGenQASM){
      errs() << "\n\tLoad Instruction: " << *LI << "\n";
      errs() << "\tNum Operands: " << numOps << ";\n";
    }
  }else if(ReturnInst * RI = dyn_cast<ReturnInst>(pInst)){
    if(debugGenQASM){
      errs() << "\n\tReturn Instruction: " << *RI << "\n";
      errs() << "\tNum Operands: " << numOps << ";\n";
    }
  }else{
    if(debugGenQASM){
      errs() << "\n\tUnhandled Instruction: " << *pInst << "\n";
      errs() << "\tNum Operands: " << numOps << ";\n";
    } 
  }
  return;
}

void GenQASM::analyzeInst(Function* F, Instruction* pInst){
  if(debugGenQASM)
    errs() << "--Processing Inst: "<<*pInst << '\n';
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

std::string GenQASM::to_string(int var){
  stringstream ss;
  ss << var;
  return ss.str();
}

void GenQASM::printFuncHeader(Function* F, bool mainFunc){

  /* Print Function Name. */
  std::string funcName = F->getName();
  std::replace(funcName.begin(), funcName.end(), '.','_');
  std::replace(funcName.begin(), funcName.end(), '-','_');

  if(mainFunc) errs() << "\nmodule main";
  else errs()<<"\nmodule "<< funcName;

  /* Print the Function Argument. */
  errs()<<"(";    

  funcArgList = mapFuncArgs.find(F)->second;
  funcArgList_ = mapFuncArgs_.find(F)->second;

  unsigned num = funcArgList.size();

  if(num > 0){
    for(unsigned i = 0; i < num - 1; i++){

      dataRepresentation argument = funcArgList_[i];
      if(debugGenQASM) argument.printDebugMode();     
      errs() << argument.registerType();
      errs() << argument.getName() << ", ";

      //  else if(argTy->isIntegerTy(32)){
      //  string var = to_string(tmpQA.valOrIndex);
      //  errs() << var;

    }
    if(debugGenQASM) funcArgList_[num-1].printDebugMode();
    errs() << funcArgList_[num-1].registerType();
    errs() << funcArgList_[num-1].getName();
  }

  errs()<<"){\n ";   

  /* Print qbits declared in Function. */
  qbitsInitInFunc = mapQbitsInit.find(F)->second;
  qbitsInitInCurrentFunc = mapQbitsInit_.find(F)->second;

  for(vector<qGateArg>::iterator vvit=qbitsInitInFunc.begin(),vvitE=qbitsInitInFunc.end();vvit!=vvitE;++vvit){
    std::string qName = (*vvit).argPtr->getName();
    std::replace(qName.begin(), qName.end(), '.','_');
    if((*vvit).isQbit)
    errs()<<"\tqbit "<<printVarName(qName);
    if((*vvit).isCbit) errs()<<"\tcbit "<<printVarName(qName);
    if((*vvit).isAbit) errs() << "\tqbit "<<printVarName(qName);

    //if only single-dimensional qbit arrays expected
    //errs()<<"["<<(*vvit).valOrIndex<<"];\n ";

    //if n-dimensional qbit arrays expected 
    for(int ndim = 0; ndim < (*vvit).numDim; ndim++)
      errs()<<"["<<(*vvit).dimSize[ndim]<<"]";
    errs() << ";\n";
  }
  //errs() << "//--//-- Fn: " << F->getName() << " --//--//\n";
}
  
void GenQASM::genQASM(Function* F) {
    
  mapFunction = mapMapFunc.find(F)->second; 
  //print gates in function
  //map<Function*, vector<FnCall> >::iterator mfvIt = mapFunction.find(F);
  for(unsigned mIndex=0;mIndex<mapFunction.size();mIndex++) {

    if(mapFunction[mIndex].qArgs.size()>0) {

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
      else if(fToPrint.find("Y.") != string::npos) fToPrint = "Y";
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

  errs()<<"}\n";
}

void GenQASM::getFunctionArguments(Function* F){

  for(Function::arg_iterator ait=F->arg_begin(); ait!=F->arg_end(); ++ait){
    dataRepresentation arg;
    arg.instPtr = ait;
    Type * argType = ait->getType();
    if(isAllocQuantumType(argType)){
      quantumRegisterSetup(&arg);
      qbitsInCurrentFunc.push_back(arg);

    }else{
      classicalRegisterSetup(&arg);
    }
    funcArgList_.push_back(arg);
    if(debugGenQASM) arg.printDebugMode();
  }

/* =================================================================== */
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

      if(debugGenQASM)
  print_qgateArg(tmpQArg);
    }
/* =================================================================== */
}

/* Run - Find Datapaths for qubits. */
bool GenQASM::runOnModule(Module &M){
  /* Functions with quantum registers and operations. */
  vector<Function*> quantumFuncs;

  const char *debug_val = getenv("DEBUG_GENQASM");
  if(debug_val){
    if(!strncmp(debug_val, "1", 1)) debugGenQASM = true;
    else debugGenQASM = false;
  }

  debug_val = getenv("DEBUG_SCAFFOLD");
  if(debug_val && !debugGenQASM){
    if(!strncmp(debug_val, "1", 1)) debugGenQASM = true;
    else debugGenQASM = false;
  }

  unsigned sccNum = 0;

  errs() << "-------QASM Generation Pass:\n";
  /* Iterate over all functions, and over all instructions in it. */
  CallGraph CG = CallGraph(M);

  CallGraphNode *rootNode = nullptr;

  for(auto it = CG.begin();it != CG.end();it++){
    if(!(it->second->getFunction())) continue;
    if(it->second->getFunction()->getName() == "main"){
      rootNode = &(*it->second);
      break;
    }
  }

  for(scc_iterator<CallGraphNode*> sccIb = scc_begin(rootNode),
      E = scc_end(rootNode); sccIb != E; ++sccIb){
    const std::vector<CallGraphNode*> &nextSCC = *sccIb;
    if(debugGenQASM)
      errs() << "\nSCC #" << ++sccNum << " : ";
      for (std::vector<CallGraphNode*>::const_iterator nsccI = nextSCC.begin(),
          E = nextSCC.end(); nsccI != E; ++nsccI){
        Function *F=(*nsccI)->getFunction();    
        if(F && !F->isDeclaration()){
          if(debugGenQASM)
            errs() << "Processing Function:" << F->getName() <<" \n ";
          /* Initialize map structures for this function. */
          qbitsInFunc.clear();
          qbitsInCurrentFunc.clear();
          qbitsInitInFunc.clear();
          qbitsInitInCurrentFunc.clear();
          funcArgList.clear();
          funcArgList_.clear();
          mapFunction.clear();
          getFunctionArguments(F);

          /* Visit Alloc Insts in func and find if function is quantum or classical function. */
          for(inst_iterator instIb = inst_begin(F), instIe=inst_end(F); instIb!=instIe; ++instIb){
            /* Grab pointer to instruction reference. */
            Instruction *pInst = &*instIb;
            if(debugGenQASM)
              errs() << "\n Processing Inst: "<<*pInst << "\n";
            analyzeAllocInst(F, pInst);
          }

          /* Process Quantum Function. */
          if(qbitsInFunc.size()>0){
            mapQbitsInit.insert(make_pair(F, qbitsInitInFunc));
            mapQbitsInit_.insert(make_pair(F, qbitsInitInCurrentFunc));
            mapFuncArgs.insert(make_pair(F, funcArgList));
            mapFuncArgs_.insert(make_pair(F, funcArgList_));
            quantumFuncs.push_back(F);

            for(Function::iterator BB = F->begin(), BBend = F->end(); BB != BBend; ++BB){
              if(debugGenQASM)
                errs() << "\nBasicBlock: " << BB->getName() << "\n";

              fnCall.clear();

              for(BasicBlock::iterator instIb = BB->begin(), instIe = BB->end(); instIb != instIe; ++instIb){
                Instruction * pInst = &*instIb;  
                allDepQbit_.clear();
                analyzeInst_block(&(*BB), pInst); 
              }
              fnCallTable.insert(make_pair(&(*BB), fnCall));              
            }
/* ========================================================================================================= */
            for(inst_iterator instIb = inst_begin(F),instIe=inst_end(F); instIb!=instIe;++instIb){

              Instruction *pInst = &*instIb; // Grab pointer to instruction reference       
              allDepQbit.clear();

              if(debugGenQASM)
                errs() << "\n Processing Inst: "<<*pInst << "\n";

              analyzeInst(F,pInst); //spatil: need a bool return type?
            }
/* ========================================================================================================= */           
            mapMapFunc.insert(make_pair(F, mapFunction));
          }
        }else{
            if(debugGenQASM){
              errs() << "WARNING: Ignoring external node or dummy function: ";
              if(F) errs() << F->getName();
            }
        }
      }
    if (nextSCC.size() == 1 && sccIb.hasLoop())
      errs() << " (Has self-loop).";
    }
    bool hasMain = false;
    for( vector<Function*>::iterator it = quantumFuncs.begin(); it!=quantumFuncs.end(); it++){
      if ((*it)->getName() == "main") hasMain = true;
    }

    vector<Function*>::iterator lastItPos;
    if(!hasMain){
        lastItPos = quantumFuncs.end();
        lastItPos--;
    }

    for( vector<Function*>::iterator it = quantumFuncs.begin(); it != quantumFuncs.end(); it++){
    std::string newName = (*it)->getName();
    if(newName.find("CNOT") != string::npos) newName = "CNOT";
    else if(newName.find("NOT.") != string::npos) newName = "X";
    else if(newName.find("Toffoli.") != string::npos) newName = "Toffoli";
    else if(newName.find("MeasX") != string::npos) newName = "MeasX";
    else if(newName.find("MeasZ") != string::npos) newName = "MeasZ";
    else if(newName.find("H.i") != string::npos) newName = "H";
    else if(newName.find("Fredkin") != string::npos) newName = "Fredkin";
    else if(newName.find("PrepX") != string::npos) newName = "PrepX";
    else if(newName.find("PrepZ") != string::npos) newName = "PrepZ";
    else if(newName.substr(0,2) == "Rx") newName = "Rx";
    else if(newName.substr(0,2) == "Ry") newName = "Ry";
    else if(newName.substr(0,2) == "Rz") newName = "Rz";
    else if(newName.find("S.") != string::npos) newName = "S";
    else if(newName.find("T.") != string::npos) newName = "T";
    else if(newName.find("Sdag") != string::npos) newName = "Sdag";
    else if(newName.find("Tdag") != string::npos) newName = "Tdag";
    else if(newName.find("X.") != string::npos) newName = "X";
    else if(newName.find("Y.") != string::npos) newName = "Y";
    else if(newName.find("Z.") != string::npos) newName = "Z";

    std::replace(newName.begin(), newName.end(), '.', '_');

    (*it)->setName(newName);
        if(it == lastItPos) printFuncHeader((*it), true);
        else printFuncHeader((*it), false);
        genQASM((*it));
    }

  errs()<<"\n--------End of QASM generation";
  errs() << "\n";
  
  return false;
}
