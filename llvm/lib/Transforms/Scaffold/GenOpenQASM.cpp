//===- GenOpenQASM.cpp - Generate OpenQASM output -----------===//
//
//         The LLVM Scaffold Compiler Infrastructure
//
// This file was created by Scaffold Compiler Working Group
//
//===--------------------------------------------------------===//

#include <sstream>
#include <algorithm>
#include <numeric>
#include <string>
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

#define MAX_BACKTRACE_COUNT 150 //max number of backtrace allowed (avoid infinite recursive backtrace)

/* Set true if debug mode. */
bool debugGenOpenQASM = false;

namespace{

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

  /* Non-general case for backtraceoperand. */
  enum backtraceExp{
    cmpConstant,
    ptrToArray,
    nonExp
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
    string qbitVarString();
    string cbitVarString();
    /* Used in conditional statement, since OpenQASM only support array-wise measurement. */
    string cbitArrayString();

    dataRepresentation() : instPtr(NULL), type(undef), isPtr(false), index(vector<int>()), dimSize(vector<int>()){}

    string getName(){ return instPtr->getName(); }
    void printDebugMode();
  };
  
  /* Datapath Sequence. */
  struct FnCall{
    Function* func;
    Value* instPtr;
    std::vector<dataRepresentation> qArgs;
  };

  struct CondCall{
    Value* instPtr;
    std::vector<dataRepresentation> qArgs;
  };

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

  string dataRepresentation::qbitVarString(){
    stringstream ss;
    if(!instPtr) return ss.str();
    ss << getName();
    if(!isPtr){
      if(index.size() == 0) index.push_back(0);
      if(index.size() > 1){
        for(unsigned i = index.size()-1; i > 0; i--)
          ss << "x" << index[i];
      }
      ss << "[" << to_string(index[0]) << "]";
    }

    string retStr = ss.str();
    std::replace(retStr.begin(), retStr.end(), '.', '_');
    //std::replace(retStr.begin(), retStr.end(), '_', 'x');
    return retStr;  
  }

  string dataRepresentation::cbitVarString(){
    stringstream ss;
    ss << getName();
    if(!isPtr){
      for(unsigned i = 0; i < index.size(); i++){
        ss << "x" << index[i];
      }
      ss << "[0]";
    }
    string retStr = ss.str();
    std::replace(retStr.begin(), retStr.end(), '.', '_');
    //std::replace(retStr.begin(), retStr.end(), '_', 'x');
    return retStr;  
  }

  string dataRepresentation::cbitArrayString(){
    stringstream ss;
    ss << getName();

    if(!isPtr){
      for(unsigned i = 0; i < index.size(); i++){
        ss << "x" << index[i];
      }
    }

    return ss.str();
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
    AllocaInst * AI = dyn_cast<AllocaInst>(qRegister->instPtr);
    Type * allocatedType = AI->getAllocatedType();
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

      errs() << "Unhandled Case!\n";
      return;
    };
  }

  bool isAllocQuantumType(Type * allocatedType){
    if(allocatedType->isIntegerTy(16)){
      return true;
    }else if(allocatedType->isIntegerTy(8)){
      return true;
    }else if(allocatedType->isIntegerTy(1)){
      return true;
    }else if(ArrayType * arrayType = dyn_cast<ArrayType>(allocatedType)){
      return isAllocQuantumType(arrayType->getElementType());
    }else{
      return false;
    }
  }

  struct GenQASM : public ModulePass {
    /* Pass Identification, Replacement for typeid. */
    static char ID;
    GenQASM() : ModulePass(ID) {}

    std::vector<dataRepresentation> tmpDepQbit_;
    std::vector<dataRepresentation> allDepQbit_;
    
    map<BasicBlock *, vector<FnCall> > fnCallTable; 
    map<Function *, vector<dataRepresentation> > mapQbitsInit;
    map<Function *, vector<dataRepresentation> > mapFuncArgs;

    vector<dataRepresentation> qbitsInCurrentFunc;
    vector<dataRepresentation> qbitsInitInCurrentFunc;
    vector<dataRepresentation> funcArgList;

    vector<FnCall> fnCall;

    map<Value*, dataRepresentation> mapInstRtn;

    /* If the block branches to more than one successors. */
    map<BasicBlock *, vector<dataRepresentation> > basicBlockCondTable;

    int backtraceCount;

    dataRepresentation backtraceOperand(Value * operand, backtraceExp exp);
    void backtraceOperand_helper(dataRepresentation * datRepPtr, Value * operand, int gettingIndex, backtraceExp exp);
    
    void analyzeAllocInst(Function* F,Instruction* pinst);
    void analyzeInst_block(BasicBlock* basicBlock, Instruction* pInst);

    void processStoreCbitInst(CallInst * pInst);
    void processCallInst(CallInst * pInst);
    void processConditionInst(BasicBlock * basicBlock, BranchInst * branchInst);

    void getFunctionArguments(Function* F);

    /* Run - Print out SCCs in the call graph for the module. */
    bool runOnModule(Module &M);

    void genQASM_REG(Function* F);
    void genQASM_block(BasicBlock * blockBlock);

    /* getAnalysisUsage - Requires the CallGraph. */
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequired<CallGraphWrapperPass>();
    }
  };
}

char GenQASM::ID = 0;
static RegisterPass<GenQASM>
X("gen-openqasm", "Generate OpenQASM output code"); //spatil: should be Z or X??

void GenQASM::backtraceOperand_helper(dataRepresentation * datRepPtr, Value * operand, int gettingIndex, backtraceExp exp){

  if(backtraceCount > MAX_BACKTRACE_COUNT){
    errs() << "Exceed max backtrace count...";
    return;
  }else{
    backtraceCount++;
  }

  if(AllocaInst * AI = dyn_cast<AllocaInst>(operand)){
    if(debugGenOpenQASM)
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
    if(debugGenOpenQASM)
      errs() << "\n\t\tGetElemPtr inst Found: " << *GEPI << "\n";
    /* Get index. */
    if(GEPI->getNumOperands() == 3){
      if(exp == ptrToArray){
        ConstantInt * CInt = dyn_cast<ConstantInt>(GEPI->getOperand(2));
        int index = CInt->getSExtValue();
        if(debugGenOpenQASM) errs() << "\t\t[" << gettingIndex << "] Index: " << index << "\n";
        backtraceOperand_helper(datRepPtr, GEPI->getOperand(0), gettingIndex, exp);
      }else if(GEPI->getOperand(2)->getType()->isIntegerTy(32)){
        /* Merely a pointer. */
        datRepPtr->isPtr = true;
        backtraceOperand_helper(datRepPtr, GEPI->getOperand(0), gettingIndex, exp);
      }else{
        ConstantInt * CInt = dyn_cast<ConstantInt>(GEPI->getOperand(2));
        int index = CInt->getSExtValue();
        if(debugGenOpenQASM) errs() << "\t\t[" << gettingIndex << "] Index: " << index << "\n";
        datRepPtr->index.push_back(index);
        backtraceOperand_helper(datRepPtr, GEPI->getOperand(0), gettingIndex, exp);
      }
    }else if(GEPI->getNumOperands() == 2){
      ConstantInt * CInt = dyn_cast<ConstantInt>(GEPI->getOperand(1));
      int index = CInt->getSExtValue();
      if(debugGenOpenQASM) errs() << "\t\t[" << gettingIndex << "] Index: " << index << "\n";
      datRepPtr->index.push_back(index);
      backtraceOperand_helper(datRepPtr, GEPI->getOperand(0), gettingIndex, ptrToArray);
      errs().flush();
    }else{
      errs() << "UNHANDLED GEPI CASE\n";
    }
    return;

  }else if(CallInst * CI = dyn_cast<CallInst>(operand)){
    if(debugGenOpenQASM)
      errs() << "\n\t\tCall inst Found: " << *CI << "\n";
    if(CI->getCalledFunction()->getName().find("llvm.Meas") != string::npos){
      if(debugGenOpenQASM){
         errs() << "\n\t\tBACKTRACE OPERAND QUBIT: \n";
         errs().flush();
      }
      backtraceOperand_helper(datRepPtr, CI->getOperand(0), gettingIndex, exp);
    }
    return;
  }else if(PtrToIntInst * PTTI = dyn_cast<PtrToIntInst>(operand)){
    if(debugGenOpenQASM)
      errs() << "\n\t\tPtrToInt inst Found: " << *PTTI << "\n";
    backtraceOperand_helper(datRepPtr, PTTI->getOperand(0), 0, exp);
    return;
  }else if(ConstantInt * CInt = dyn_cast<ConstantInt>(operand)){
    datRepPtr->instPtr = CInt;
    if(debugGenOpenQASM) errs() << "\n\t\tInt Constant Found: " << CInt->getSExtValue() << "\n";
    return;
  }else if(ConstantFP * CFP = dyn_cast<ConstantFP>(operand)){
    datRepPtr->instPtr = CFP;
    if(debugGenOpenQASM) errs() << "\n\t\tDouble Constant Found: " << CFP->getValueAPF().convertToDouble() << "\n";
    return;
  }else if(LoadInst * LI = dyn_cast<LoadInst>(operand)){
    if(debugGenOpenQASM) errs() << "\n\t\tLoad inst Found: " << *LI << "\n";
    if(exp == cmpConstant){
      if(debugGenOpenQASM) errs() << "\n\t\tCmp constant is 1.\n";
      datRepPtr->type = intVal;
      datRepPtr->intValue = 1;
      return;
    }else{
      backtraceOperand_helper(datRepPtr, LI->getOperand(0), 0, exp);
      return;
    }
  }else if(CmpInst * CMPI = dyn_cast<CmpInst>(operand)){
    if(debugGenOpenQASM) errs() << "\n\t\tCompare inst Found: " << *CMPI << "\n";
    if(exp == cmpConstant){
      backtraceOperand_helper(datRepPtr, CMPI->getOperand(1), 0, exp);
    }else{
      backtraceOperand_helper(datRepPtr, CMPI->getOperand(0), 0, exp);
    }
    return;
  }else if(ZExtInst * ZEI = dyn_cast<ZExtInst>(operand)){
    if(debugGenOpenQASM) errs() << "\n\t\tZExt inst Found: " << *ZEI << "\n";
    backtraceOperand_helper(datRepPtr, ZEI->getOperand(0), 0, exp);
    return;
  }else if(isa<UndefValue>(operand)){
    datRepPtr->type = undef;
    if(debugGenOpenQASM) errs() << "Undef Inst: " << *operand << "\n";
    return;
  }else{
    if(debugGenOpenQASM) errs() << "UNHANDLED CASE, Inst: " << *operand << "\n";
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

void GenQASM::analyzeAllocInst(Function * F, Instruction * pInst){
  if (AllocaInst * AI = dyn_cast<AllocaInst>(pInst)){
    Type * allocatedType_ = AI->getAllocatedType();

    if(isAllocQuantumType(allocatedType_)){

      if(debugGenOpenQASM) errs() << "\tNew qubit allocation: " << AI->getName() << "\n";

      dataRepresentation qRegister;
      qRegister.instPtr = AI;
      quantumRegisterSetup(&qRegister);
      
      if(debugGenOpenQASM) qRegister.printDebugMode();

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
          for(vector<dataRepresentation>::iterator vvit = funcArgList.begin(),
            vvitE = funcArgList.end(); ((vvit != vvitE) && !qbitExisting); ++vvit){
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
    return;
    }
}

void GenQASM::processStoreCbitInst(CallInst * CI){

  Value * rtnVal_ = CI->getArgOperand(0);
  tmpDepQbit_.clear();

  if(debugGenOpenQASM) errs() << "\n\t\tBACKTRACE OPERAND CBIT: \n";
  dataRepresentation cbit = backtraceOperand(CI->getArgOperand(1), nonExp);
  quantumRegisterSetup(&cbit);
  if(debugGenOpenQASM) errs() << "\t\tBacktrace End, Cbit Found: " << "\n";
  if(debugGenOpenQASM) cbit.printDebugMode();
  tmpDepQbit_.push_back(cbit);
  mapInstRtn[rtnVal_] = tmpDepQbit_[0];
  tmpDepQbit_.clear();

  return;
}

void GenQASM::processCallInst(CallInst * callInst){
  /* Traverse all argument operand in call inst. */
  for(unsigned iOperand = 0; iOperand < callInst->getNumArgOperands(); iOperand++){

    tmpDepQbit_.clear();

    if(debugGenOpenQASM) errs() << "\tANALYZE CALL INST OPERAND NUM: " << iOperand << "\n";
    dataRepresentation argument = backtraceOperand(callInst->getArgOperand(iOperand), nonExp);
    Type* argType = callInst->getArgOperand(iOperand)->getType();

    if(isAllocQuantumType(argType)){
      quantumRegisterSetup(&argument);
      tmpDepQbit_.push_back(argument);
      if(debugGenOpenQASM) argument.printDebugMode();
    }else{
      classicalRegisterSetup(&argument);
      tmpDepQbit_.push_back(argument);
      if(debugGenOpenQASM) argument.printDebugMode();
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
      fnCallInfoPack.qArgs.push_back(allDepQbit_[iArg]);
  }
  fnCall.push_back(fnCallInfoPack);
  return;
}


void GenQASM::processConditionInst(BasicBlock * basicBlock, BranchInst* branchInst){
  
  if(branchInst->getNumOperands() == 3){
    if(debugGenOpenQASM) errs() << "\n\t\tBACKTRACE OPERAND CONDITION: " << "\n";
    dataRepresentation measure = backtraceOperand(branchInst->getOperand(0), cmpConstant);
    classicalRegisterSetup(&measure);
    if(debugGenOpenQASM) errs() << "\t\tBacktrace End, Value Found: " << "\n";
    if(debugGenOpenQASM) measure.printDebugMode();
    dataRepresentation cbit = backtraceOperand(branchInst->getOperand(0), nonExp);
    quantumRegisterSetup(&cbit);
    if(debugGenOpenQASM) errs() << "\t\tBacktrace End, Cbit Found: " << "\n";
    if(debugGenOpenQASM) cbit.printDebugMode();
    vector<dataRepresentation> cond;
    cond.push_back(measure);
    cond.push_back(cbit);
    basicBlockCondTable[basicBlock] = cond;

    if(debugGenOpenQASM)
      errs() << "\n\tIf then block: " << branchInst->getOperand(2)->getName() << "\n";
    if(debugGenOpenQASM)
      errs() << "\n\tIf end block: " << branchInst->getOperand(1)->getName() << "\n";
  }
  return;
}

void GenQASM::analyzeInst_block(BasicBlock * basicBlock, Instruction * pInst){

  unsigned numOps = pInst->getNumOperands();

  if(AllocaInst * AI = dyn_cast<AllocaInst>(pInst)){
    if(debugGenOpenQASM){
      errs() << "\n\tAllocation Instruction: " << *AI << "\n";
      errs() << "\tNum Operands: " << numOps << ";\n";
    }
  }else if(GetElementPtrInst * GEPI = dyn_cast<GetElementPtrInst>(pInst)){
    if(debugGenOpenQASM){
      errs() << "\n\tGetElementPointer Instruction: " << *GEPI << "\n";
      errs() << "\tNum Operands: " << numOps << ";\n";
    }
  }else if(CallInst * CI = dyn_cast<CallInst>(pInst)){
    if(debugGenOpenQASM){
      errs() << "\n\tCall Instruction: " << *CI << "\n";
      errs() << "\tNum Operands: " << numOps << ";\n";
    }
    if(CI->getCalledFunction()->getName() == "store_cbit"){
      processStoreCbitInst(CI);
    }else{
      processCallInst(CI);
    }
  }else if(BranchInst *BI = dyn_cast<BranchInst>(pInst)){
    if(debugGenOpenQASM){
      errs() << "\n\tBranch Instruction: " << *BI << "\n";
      errs() << "\tNum Operands: " << numOps << ";\n";
    }
    processConditionInst(basicBlock, BI);
  }else if(LoadInst *LI = dyn_cast<LoadInst>(pInst)){
    if(debugGenOpenQASM){
      errs() << "\n\tLoad Instruction: " << *LI << "\n";
      errs() << "\tNum Operands: " << numOps << ";\n";
    }
  }else{
    if(debugGenOpenQASM){
      errs() << "\n\tUnhandled Instruction: " << *pInst << "\n";
      errs() << "\tNum Operands: " << numOps << ";\n";
    } 
  }
  return;
}

void GenQASM::genQASM_REG(Function* F){
  /* Print qbits declared in function. */
  for(vector<dataRepresentation>::iterator vvit = qbitsInitInCurrentFunc.begin(),
    vvitE = qbitsInitInCurrentFunc.end(); vvit != vvitE; ++vvit){
    if((*vvit).isqbit()){
      int num = accumulate((*vvit).dimSize.begin(), (*vvit).dimSize.end(), 1, multiplies<int>());
      int numDim = (*vvit).dimSize.size();
      int j = numDim-2;
      //vector<int> count(numDim-1,0);
      vector<int> count(numDim,0);
      for(int n = 0; n < num/(*vvit).dimSize[numDim-1]; n++){
        string ss = (*vvit).getName();
        std::replace(ss.begin(), ss.end(), '.', '_');
        //std::replace(ss.begin(), ss.end(), '_', 'x');
        errs() << "qreg " << ss;
        /* dimension more than one. */
        if(j >= 0){
          for(int i = 0; i < numDim-1; i++) errs() << "x" << count[i];
          if(count[j] < (*vvit).dimSize[j]-1) count[j]++; else j--;
        }
        errs() << "[" << (*vvit).dimSize[numDim-1] << "];\n";
      }
    }

    if((*vvit).iscbit()){
      int num = accumulate((*vvit).dimSize.begin(), (*vvit).dimSize.end(), 1, multiplies<int>());
      int numDim = (*vvit).dimSize.size();
      int j = numDim-1;
      vector<int> count(numDim,0);
      if((*vvit).isPtr){
        string ss = (*vvit).getName();
        std::replace(ss.begin(), ss.end(), '.', '_');
        //std::replace(ss.begin(), ss.end(), '_', 'x');
        errs() << "creg " << ss << "[" << num << "];\n";
      }else{
        for(int n = 0; n < num; n++){
          string ss = (*vvit).getName();
          std::replace(ss.begin(), ss.end(), '.', '_');
          //std::replace(ss.begin(), ss.end(), '_', 'x');
          errs() << "creg " << ss;
          for(int i = 0; i < numDim; i++){
            errs() << "x" << count[i];
          }
          if(count[j] < (*vvit).dimSize[j]) count[j]++; else j--;
          errs() << "[1];\n";
        }
      }
    }
  }
}

void GenQASM::genQASM_block(BasicBlock * blockBlock){
  vector<FnCall> fnCallList = fnCallTable.find(blockBlock)->second;

  for(unsigned mIndex = 0; mIndex < fnCallList.size(); mIndex++){
    /* If the FuncCall is related to quantum operation. */
    if(fnCallList[mIndex].qArgs.size()>0){
      string fToPrint = fnCallList[mIndex].func->getName();
      if(fToPrint.find("llvm.") != string::npos) fToPrint = fToPrint.substr(5);

      //MeasX, PrepZ/X, and Fredkin require some special works, so we handle them separately
      //NB(pranav): a better way to handle these would be to decompose MeasX, PrepZ/X,
      //and Fredkin in terms of the other gates during an earlier LLVM pass

      if(fToPrint.find("MeasX") != string::npos){
        errs()<<"h " << fnCallList[mIndex].qArgs.front().qbitVarString() << ";\n";
        fToPrint = "MeasZ";
      }

      if(fToPrint.find("MeasZ") != string::npos){
        errs()<<"measure " << fnCallList[mIndex].qArgs.front().qbitVarString() << " -> ";

        //get inst ptr
        Value* thisInstPtr = fnCallList[mIndex].instPtr;
        map<Value*, dataRepresentation>::iterator hit = mapInstRtn.find(thisInstPtr);
        if(hit != mapInstRtn.end()){
          errs() << hit->second.cbitVarString() << ";\n";
        }
        continue;
      }

      if(fToPrint.find("Prep") != string::npos) {

        if(fnCallList[mIndex].qArgs.front().isPtr) fnCallList[mIndex].qArgs.front().isPtr = false;

        errs()<<"reset " << fnCallList[mIndex].qArgs.front().qbitVarString() << ";\n";

        /* For preparation to | 1 > state, apply a bit flip after reset to get 0->1. */
        if(fnCallList[mIndex].qArgs.back().intValue == 1)
          errs() << "x " << fnCallList[mIndex].qArgs.front().qbitVarString() << ";\n";

        /* For preparation in X basis, change basis from Z basis with H gate. */
        if(fToPrint.find("PrepX") != string::npos)
          errs() << "h " << fnCallList[mIndex].qArgs.front().qbitVarString() << ";\n";

        continue;
      }

      if(fToPrint.find("Fredkin") != string::npos) {
        /* Fredkin Gate Decomposition. */
        // ers() << "//Decompose Fredkin(q0, q1, q2) = (I ⊗ CNOT(q1, q2)) * Toffoli(q0, q2, q1) * (I ⊗ CNOT(q1, q2))\n";
        
        if(fnCallList[mIndex].qArgs[0].isPtr) fnCallList[mIndex].qArgs[0].isPtr = false;
        if(fnCallList[mIndex].qArgs[1].isPtr) fnCallList[mIndex].qArgs[1].isPtr = false;
        if(fnCallList[mIndex].qArgs[2].isPtr) fnCallList[mIndex].qArgs[2].isPtr = false;

        //Step 1, CNOT(second input, third input)
        errs() << "cx " << fnCallList[mIndex].qArgs[1].qbitVarString() << ", ";
        errs() << fnCallList[mIndex].qArgs[2].qbitVarString() << ";\n";

        //Step 2, Toffoli(first input, third input, second input)
        errs() << "ccx " << fnCallList[mIndex].qArgs[0].qbitVarString() << ", ";
        errs() << fnCallList[mIndex].qArgs[1].qbitVarString() << ", ";
        errs() << fnCallList[mIndex].qArgs[2].qbitVarString() << ";\n";

        //Step 3, CNOT(second input, third input)
        errs() << "cx " << fnCallList[mIndex].qArgs[1].qbitVarString() << ", ";
        errs() << fnCallList[mIndex].qArgs[2].qbitVarString() << ";\n";

        continue;
      }

      if(fToPrint.substr(0,2) == "Rx") fToPrint = "rx";
      else if(fToPrint.substr(0,2) == "Ry") fToPrint = "ry";
      else if(fToPrint.substr(0,2) == "Rz") fToPrint = "rz";

      if(fToPrint.find("rx") != string::npos || fToPrint.find("ry") != string::npos || fToPrint.find("rz") != string::npos){
        if(fnCallList[mIndex].qArgs.front().isPtr) fnCallList[mIndex].qArgs.front().isPtr = false;
        errs() << fToPrint << "(" << fnCallList[mIndex].qArgs.back().val() << ") "
          << fnCallList[mIndex].qArgs.front().qbitVarString() << ";\n";
        continue;
      }

      if(fToPrint.find("CNOT") != string::npos) fToPrint = "cx";
      else if(fToPrint.find("Toffoli.") != string::npos) fToPrint = "ccx";
      else if(fToPrint.find("H.") != string::npos) fToPrint = "h";
      else if(fToPrint.find("S.") != string::npos) fToPrint = "s";
      else if(fToPrint.find("T.") != string::npos) fToPrint = "t";
      else if(fToPrint.find("Sdag") != string::npos) fToPrint = "sdg";
      else if(fToPrint.find("Tdag") != string::npos) fToPrint = "tdg";
      else if(fToPrint.find("X.") != string::npos) fToPrint = "x";
      else if(fToPrint.find("Y.") != string::npos) fToPrint = "y";
      else if(fToPrint.find("Z.") != string::npos) fToPrint = "z";

      std::replace(fToPrint.begin(), fToPrint.end(), '.', '_');
      std::replace(fToPrint.begin(), fToPrint.end(), '-', '_');

      errs() << fToPrint << " ";
      for(vector<dataRepresentation>::iterator vpIt=fnCallList[mIndex].qArgs.begin(), 
        vpItE=fnCallList[mIndex].qArgs.end(); vpIt != vpItE-1; ++vpIt){
          if(vpIt->isPtr) vpIt->isPtr = false;
          errs() << vpIt->qbitVarString() << ", ";
      }
      if(fnCallList[mIndex].qArgs.back().isPtr) fnCallList[mIndex].qArgs.back().isPtr = false;
      errs()<< fnCallList[mIndex].qArgs.back().qbitVarString() << ";\n";
      }
    }

  /* Print conditional statement. */
  map<BasicBlock *, vector<dataRepresentation> >::iterator hit = basicBlockCondTable.find(blockBlock);
  if(basicBlockCondTable.end()!= hit){
    errs() << "if(" << hit->second.back().cbitArrayString() << " ==" << 
      hit->second.front().val() << ") ";
  }
  return ;
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
    funcArgList.push_back(arg);
    if(debugGenOpenQASM) arg.printDebugMode();
  }
}

/* Run - Find Datapaths for qubits. */
bool GenQASM::runOnModule(Module &M){
  /* Functions with quantum registers and operations. */
  vector<Function*> quantumFuncs;

  const char *debug_val = getenv("DEBUG_GEN_OPENQASM");
  if(debug_val){
    if(!strncmp(debug_val, "1", 1)) debugGenOpenQASM = true;
    else debugGenOpenQASM = false;
  }

  debug_val = getenv("DEBUG_SCAFFOLD");
  if(debug_val && !debugGenOpenQASM){
    if(!strncmp(debug_val, "1", 1)) debugGenOpenQASM = true;
    else debugGenOpenQASM = false;
  }

  unsigned sccNum = 0;

  errs() << "OPENQASM 2.0;\n";
  errs() << "include \"qelib1.inc\";\n";

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

  for (scc_iterator<CallGraphNode*> sccIb = scc_begin(rootNode),
      E = scc_end(rootNode); sccIb != E; ++sccIb){

    const std::vector<CallGraphNode*> &nextSCC = *sccIb;
    if(debugGenOpenQASM) 
      errs() << "\nSCC #" << ++sccNum << " : ";      

    for (std::vector<CallGraphNode*>::const_iterator nsccI = nextSCC.begin(),
        E = nextSCC.end(); nsccI != E; ++nsccI){

      Function *F = (*nsccI)->getFunction();    
    
      if(F && !F->isDeclaration()){
        if(debugGenOpenQASM)
          errs() << "Processing Function:" << F->getName() <<" \n ";

        /* Initialize map structures for this function. */
        qbitsInCurrentFunc.clear();
        qbitsInitInCurrentFunc.clear();
        funcArgList.clear();        

        getFunctionArguments(F);
        
        /* Iterate through all the blocks in the function. */
        for(Function::iterator BB = F->begin(), BBend = F->end(); BB != BBend; ++BB){
          
          /* Iterate through all the instructions in the block and find all the quantum alloc inst. */
          if(debugGenOpenQASM) errs() << "\n-----ANALYZE ALLOCATION INST IN BLOCK: " << BB->getName() << "-----\n";
          for(BasicBlock::iterator instIb = BB->begin(), instIe = BB->end(); instIb != instIe; ++instIb){
            Instruction * pInst = &*instIb;
            if(debugGenOpenQASM)
              errs() << "\n\tProcessing Inst: " << *pInst << "\n";
            analyzeAllocInst(F,pInst);
          }
        }

        /* Process Quantum Function. */
        if(qbitsInCurrentFunc.size()>0){
          mapQbitsInit.insert(make_pair(F, qbitsInitInCurrentFunc));
          mapFuncArgs.insert(make_pair(F, funcArgList));
          quantumFuncs.push_back(F);
          if(debugGenOpenQASM){
            errs() << "\n-----END" << "\n";
            errs() << "\n\n-----ANALYZE QUANTUM FUNC CALL IN FUNCTION : " << F->getName() << "----\n";
          }

          for(Function::iterator BB = F->begin(), BBend = F->end(); BB != BBend; ++BB){
            if(debugGenOpenQASM)
              errs() << "\nBasicBlock: " << BB->getName() << "\n";

            fnCall.clear();

            for(BasicBlock::iterator instIb = BB->begin(), instIe = BB->end(); instIb != instIe; ++instIb){
              Instruction *pInst = &*instIb;  
              allDepQbit_.clear();

              analyzeInst_block(&(*BB), pInst);
              
            }
            fnCallTable.insert(make_pair(&(*BB), fnCall));
          }
          if(debugGenOpenQASM) errs() << "\n";
        }
      }else{
        if(debugGenOpenQASM){
          errs() << "WARNING: Ignoring external node or dummy function: ";
          if(F) errs() << F->getName();
        }
      }
    }
    if(nextSCC.size() == 1 && sccIb.hasLoop())
      errs() << " (Has self-loop).";
  }

  bool hasMain = false;
  for(vector<Function*>::iterator it = quantumFuncs.begin(); it!=quantumFuncs.end(); it++){
    if ((*it)->getName() == "main") hasMain = true;
  }

  vector<Function*>::iterator lastItPos;
  if(!hasMain){
    lastItPos = quantumFuncs.end();
    lastItPos--;
  }

  for(vector<Function*>::iterator it = quantumFuncs.begin(); it != quantumFuncs.end(); it++){
    std::string newName = (*it)->getName();
    if(newName.find("CNOT") != string::npos) newName = "CNOT";
    else if(newName.find("NOT.") != string::npos) newName = "X";
    else if(newName.find("Toffoli.") != string::npos) newName = "Toffoli";
    else if(newName.find("MeasX") != string::npos) newName = "MeasX";
    else if(newName.find("MeasZ") != string::npos) newName = "MeasZ";
    else if(newName.find("H.i") != string::npos) newName = "H";
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
    else if(newName.find("Z.") != string::npos) newName = "Z";
    else if(newName.find("Y.") != string::npos) newName = "Y";

    std::replace(newName.begin(), newName.end(), '.', '_');

    (*it)->setName(newName);
    genQASM_REG((*it));

    for(Function::iterator BB = (*it)->begin(), BBend = (*it)->end(); BB != BBend; ++BB){
      genQASM_block(&(*BB));
    }
  }
  return false;
}
