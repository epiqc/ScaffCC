//===-------------------------- FunctionClone.cpp ------------------------===//
// This file implements the Scaffold pass of cloning functions with constant 
// integer arguments.
//
//        This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "ResourceCount"
#include <sstream>
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instruction.h"
#include "llvm/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Support/CFG.h"
#include "llvm/ADT/SCCIterator.h"

// DEBUG switch
bool debugCloning = false;

using namespace llvm;

// An anonymous namespace for the pass. Things declared inside it are
// only visible to the current file.
namespace {

  // Derived from ModulePass to work on callgraph
  struct FunctionClone : public ModulePass {
    static char ID; // Pass identification
    FunctionClone() : ModulePass(ID) {}

    Function *CloneFunctionInfo(const Function *F, ValueMap<const Value*, WeakVH> &VMap, Module *M);
    
    void insertNewCallSite(CallInst *CI, std::string specializedName, Module *M);

    bool runOnModule (Module &M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.setPreservesAll();  
        AU.addRequired<CallGraph>();
    }

  }; // End of struct FunctionClone
} // End of anonymous namespace


char FunctionClone::ID = 0;
static RegisterPass<FunctionClone> X("FunctionClone", "Function Cloning Pass", false, false);


Function *FunctionClone::CloneFunctionInfo(const Function *F, ValueMap<const Value*, WeakVH> &VMap, Module *M) {
  std::vector<Type*> ArgTypes;
  // the user might be deleting arguments to the function by specifying them in the VMap.
  // If so, we need to not add the arguments to the ArgTypes vector

  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end(); I!=E; I++)
    if (VMap.count(I) == 0) // haven't mapped the argument to anything yet?
      ArgTypes.push_back(I->getType());

  // create a new funcion type...
  FunctionType *FTy = FunctionType::get(
      F->getFunctionType()->getReturnType(), ArgTypes, F->getFunctionType()->isVarArg());

  // Create the new function
  Function *NewF = Function::Create(FTy, F->getLinkage(), F->getName(), M);

  // Loop over the arguments, copying the names of the mapped arguments over...
  Function::arg_iterator DestI = NewF->arg_begin();
  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end(); I!=E; ++I)
    if (VMap.count(I) == 0) {     // is this argument preserved?
      DestI->setName(I->getName());   // copy the name over..
      WeakVH wval(DestI++);
      VMap[I] = wval;          // add mapping to VMap
    }
  return NewF;
}

void FunctionClone::insertNewCallSite(CallInst *CI, std::string specializedName, Module *M) {
  CallSite CS = CallSite(CI);
  std::vector<Value*> Args;
  Args.reserve(CS.arg_size());
  CallSite::arg_iterator AI = CS.arg_begin();
  for (unsigned i = 0, e = CI->getCalledFunction()->getFunctionType()->getNumParams();
      i!=e; ++i, ++AI) //copy arguments FIXME: delete int args
    Args.push_back(*AI); 

  ArrayRef<Value*> ArgsRef(Args);
  
  CallInst* newCall = CallInst::Create(M->getFunction(specializedName), ArgsRef, "", (Instruction*)CI);
  newCall -> setCallingConv (CS.getCallingConv());
  if (CI -> isTailCall())
    newCall -> setTailCall();
}

bool FunctionClone::runOnModule (Module &M) {

  // iterate over all functions, and over all instructions in those functions
  // find call sites that have constant integer or double values. In Post-Order.
  CallGraphNode* rootNode = getAnalysis<CallGraph>().getRoot();
  
  std::vector<Function*> vectPostOrder;
  
  for (scc_iterator<CallGraphNode*> sccIb = scc_begin(rootNode), E = scc_end(rootNode); sccIb != E; ++sccIb) {
    const std::vector<CallGraphNode*> &nextSCC = *sccIb;
    for (std::vector<CallGraphNode*>::const_iterator nsccI = nextSCC.begin(), E = nextSCC.end(); nsccI != E; ++nsccI) {
      Function *f = (*nsccI)->getFunction();	  

      if(f && !f->isDeclaration())
        vectPostOrder.push_back(f);
    }
  }

  unsigned int initial_vector_size = (unsigned int)(vectPostOrder.size());

  
  //reverse the vector
  std::reverse(vectPostOrder.begin(),vectPostOrder.end());


  //--- start traversing in reverse order for a pre-order

    
  // keep track of which functions are cloned
  // and need not be processed when we reach them
  std::vector <Function*> funcErase;
  
  for(std::vector<Function*>::iterator vit = vectPostOrder.begin(), vitE = vectPostOrder.end();
      vit!=vitE; ++vit) { 
    Function *f = *vit;      
    std::string fname = f->getName();

    if (debugCloning) {
      errs() << "---------------------------------------------" << "\n";
      errs() << "Caller: " << fname << "\n";
    }

    // what instructions (call sites) need to be erased after this function has been processed
    std::vector <Instruction*> instErase;
    
    // in vectPostOrder traversal, when reaching a function that has been marked for deletion...
    // skip and do not inspect it anymore
    if (std::find(funcErase.begin(), funcErase.end(), (*vit)) != funcErase.end()) {
      if (debugCloning)
        errs() << "Skipping...: " << (*vit)->getName() << "\n";  
      continue;
    }      


    for (inst_iterator I = inst_begin(*f), E = inst_end(*f); I != E; ++I) {
      Instruction *pInst = &*I;          
      if(CallInst *CI = dyn_cast<CallInst>(pInst)) {
        if (!CI->getCalledFunction()->isIntrinsic() && !CI->getCalledFunction()->isDeclaration()) {
          std::map<unsigned, int> valueOfInt; // map argument index to const int value
          std::map<unsigned, double> valueOfDouble; // map argument index to const double value          
          // scan for constant int or double arguments and save them
          for(unsigned iop=0;iop < CI->getNumArgOperands(); iop++) {
            if(ConstantInt *CInt = dyn_cast<ConstantInt>(CI->getArgOperand(iop)))
              valueOfInt[iop] = CInt->getZExtValue();
            if(ConstantFP *CDouble = dyn_cast<ConstantFP>(CI->getArgOperand(iop)))
              valueOfDouble[iop] = CDouble->getValueAPF().convertToDouble();              
          }

          if (!valueOfInt.empty() || !valueOfDouble.empty()) {
            // make copy and start cloning/pruning
            Function *F = CI->getCalledFunction();


            std::stringstream ss; 
            for (std::map<unsigned, int>::iterator i = valueOfInt.begin(), e = valueOfInt.end(); i!=e; ++i)
              ss << "_" << i->second;
            for (std::map<unsigned, double>::iterator i = valueOfDouble.begin(), e = valueOfDouble.end(); i!=e; ++i)
              ss << "_" << i->second;             
            std::string specializedName = F->getName().str() + ss.str();

            // process specializedName string to convert dots into underscores (for flat qasm generation purposes)
            for (unsigned long i = 0; i < specializedName.length(); ++i)
              if (specializedName[i] == '.' || specializedName[i] == '-')
                specializedName[i] = '_';

            // don't clone if it has been before
            if (M.getFunction(specializedName)) {
              insertNewCallSite(CI, specializedName, &M);
              instErase.push_back((Instruction*)CI);
              continue;
            }

            ValueMap<const Value*, WeakVH> VMap;
            Function *specializedFunction = CloneFunctionInfo(F, VMap, &M); 
            specializedFunction->setName(specializedName);

            if (debugCloning)
              errs() << "\tCloned Function: " << specializedFunction->getName() << "\n";

            // Iterate over function arguments to apply constants to VMap
            for (Function::arg_iterator i = F->arg_begin(), ie = F->arg_end(); i!=ie; ++i) {
              Argument *arg = i;
              unsigned argNo = arg->getArgNo();
              if (valueOfInt.count(argNo) == 1) { 
                // Replace int arg with Const expression                     
                Value *val;                    
                val = ConstantInt::get(Type::getInt32Ty(M.getContext()), valueOfInt[argNo]);
                WeakVH wval(val);
                VMap[i] = wval;
              }
              else if (valueOfDouble.count(argNo) == 1) {
                // Replace double arg with Const expression
                Value *val;
                val = ConstantFP::get(Type::getDoubleTy(M.getContext()), valueOfDouble[argNo]);
                WeakVH wval(val);
                VMap[i] = wval;
              }
            }

            SmallVector<ReturnInst*,1> Returns; // FIXME: what is the length of this vector?
            ClonedCodeInfo SpecializedFunctionInfo;

            CloneAndPruneFunctionInto (specializedFunction,   // NewFunc
                                        F,                    // OldFunc
                                        VMap,                 // ValueMap
                                        0,                    // ModuleLevelChanges
                                        Returns,              // Returns
                                        ".",                  // NameSuffix
                                        &SpecializedFunctionInfo,  // CodeInfo
                                        0);                   // TD            

            // replace CI to call the new cloned function
            insertNewCallSite(CI, specializedName, &M);
            instErase.push_back((Instruction*)CI); // queue for erasing

            // once a Function is cloned, it is a candidate for removal from vector
            // mark for deletion
            if (std::find(funcErase.begin(), funcErase.end(), F) == funcErase.end())
              funcErase.push_back(F);
            

            //insert this new cloned function into the vector
            std::vector<Function*>::iterator it = std::find(vectPostOrder.begin(), vectPostOrder.end(), F);
            vectPostOrder.insert(it, specializedFunction);

            // the insertion will invalidate vit - reassign the vector to be safe
            vit = std::find(vectPostOrder.begin(), vectPostOrder.end(), f);
      
            
          }  
        }
      }
    }
    
    // remove instructions (call sites) that called the original (before cloning) function
    for (std::vector<Instruction*>::iterator i = instErase.begin(), e = instErase.end(); i!=e; ++i)
      (*i)->eraseFromParent();


    // recompute vitE in case of change
    vitE = vectPostOrder.end();

  }

  unsigned int final_vector_size = (unsigned int)(vectPostOrder.size());

  errs() << "Functions Cloned: " << final_vector_size - initial_vector_size << "\n";
  // Erase functions that were marked for deletion - FIXME: Gives error. Not necessary now.
  //for (std::vector<Function*>::iterator i = funcErase.begin(), e = funcErase.end(); i != e; ++i)
  //  (*i)->eraseFromParent();

  return true;

} // End runOnModule



