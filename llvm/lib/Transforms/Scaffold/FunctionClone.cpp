//===-------------------------- FunctionClone.cpp ------------------------===//
// This file implements the Scaffold pass of cloning functions with constant 
// integer or double arguments.
//
//        This file was created by Scaffold Compiler Working Group
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "FunctionClone"
#include <sstream>
#include <algorithm>
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

#define _MAX_FUNCTION_NAME 32
#define _MAX_INT_PARAMS 4
#define _MAX_DOUBLE_PARAMS 4

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
  // find call sites that have constant integer or double values.
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
        bool isQuantumModuleCall = false;         
        for(unsigned iop=0;iop < CI->getNumArgOperands(); iop++) {
          if (CI->getArgOperand(iop)->getType()->isPointerTy())
            if(CI->getArgOperand(iop)->getType()->getPointerElementType()->isIntegerTy(16))
              isQuantumModuleCall = true;
          if (CI->getArgOperand(iop)->getType()->isIntegerTy(16))
            isQuantumModuleCall = true;
        }        
        if (!CI->getCalledFunction()->isIntrinsic() && !CI->getCalledFunction()->isDeclaration() && isQuantumModuleCall) {
          // first, find the argument positions of all integers and doubles (regardless of being contstant or not)
          std::vector<unsigned> posOfInt;
          std::vector<unsigned> posOfDouble;
          for(unsigned iop=0;iop < CI->getNumArgOperands(); iop++) {
            if(dyn_cast<ConstantInt>(CI->getArgOperand(iop)) || CI->getArgOperand(iop)->getType() == Type::getInt32Ty(CI->getContext()))
              posOfInt.push_back(iop);
            if(dyn_cast<ConstantFP>(CI->getArgOperand(iop)) || CI->getArgOperand(iop)->getType() == Type::getDoubleTy(CI->getContext()))
              posOfDouble.push_back(iop);
          }

          // second, scan for constant int or double arguments and save the argument position with the constant value
          std::map<unsigned, int> valueOfInt; 
          std::map<unsigned, double> valueOfDouble; 
          for(unsigned iop=0;iop < CI->getNumArgOperands(); iop++) {
            if(ConstantInt *CInt = dyn_cast<ConstantInt>(CI->getArgOperand(iop)))
              valueOfInt[iop] = CInt->getZExtValue();
            if(ConstantFP *CDouble = dyn_cast<ConstantFP>(CI->getArgOperand(iop)))
              // round to 4 decimanl points
              valueOfDouble[iop] = floorf(CDouble->getValueAPF().convertToDouble() * 10000 + 0.5)/10000;              
            }

          // now if there were constant arguments present, clone.
          if (!valueOfInt.empty() || !valueOfDouble.empty()) {
            Function *F = CI->getCalledFunction();
            std::string originalName = F->getName().str();    
            if(debugCloning)
              errs() << "\n\toriginalName: " << originalName << "\n";
       
            // what's the name without all the parameters?
            std::string::size_type originalCoreEnd;
            originalCoreEnd = originalName.find(std::string("_IP"));
            if (originalCoreEnd == std::string::npos) // maybe it's all doubles?
              originalCoreEnd = originalName.find(std::string("_DP"));        
            std::string originalCore = originalName.substr(0, originalCoreEnd);
            if(debugCloning)
              errs() << "\t\toriginalCore: " << originalCore << "\n";

            // check to see if this function has itself been cloned before as a result of this same pass
            // if not, pad with "_IPx", _MAX_INT_PARAMS times / "_DPx", _MAX_DOUBLE_PARAMS times.
            // (it's like a fence to ensure after this point all to-be-cloned modules got IPx_.._IPx_DPx_.._DPX placeholder in their names)              
            if (originalName == originalCore){
                std::stringstream padding;
                for (unsigned j=0; j<_MAX_INT_PARAMS; j++)
                  padding<<"_IPx";
                for (unsigned k=0; k<_MAX_DOUBLE_PARAMS; k++) 
                  padding<<"_DPx";                
                originalName = originalCore + padding.str();
            }
            
            // read the current value of integers and doubles from the originalName string
            std::string::size_type found_pos_begin, found_pos_end, found_pos_begin_new;
            std::vector<std::string> originalInts; 
            std::vector<std::string> originalDoubles;

            // for IntParams
            found_pos_begin = originalName.find(std::string("_IP"));
            while (found_pos_begin != std::string::npos){
              //because there might be numbers more than 1 digit long, need to find begin and end
              found_pos_end = originalName.find_first_not_of("012345679-x",found_pos_begin+3);
              std::string intString = originalName.substr(found_pos_begin+3, found_pos_end-(found_pos_begin+3));
              //if(intString!=std::string("x"))
              //  originalInts.push_back(atoi(intString.c_str()));
              originalInts.push_back(intString);
              //next one
              found_pos_begin_new = originalName.find("_IP",found_pos_end);  
              found_pos_begin = found_pos_begin_new;
            }

            // for DoubleParams
            found_pos_begin = originalName.find(std::string("_DP"));
            while (found_pos_begin != std::string::npos){
              //because there might be numbers more than 1 digit long, need to find begin and end
              found_pos_end = originalName.find_first_not_of("0123456789e.-x",found_pos_begin+3);
              std::string doubleString = originalName.substr(found_pos_begin+3, found_pos_end-(found_pos_begin+3));
              //if(doubleString!=std::string("x"))              
              //  originalDoubles.push_back(atof(doubleString.c_str()));
              originalDoubles.push_back(doubleString);              
              //next one
              found_pos_begin_new = originalName.find("_DP",found_pos_end);  
              found_pos_begin = found_pos_begin_new;
            }

            /*if (debugCloning)
              for (std::map<unsigned, int>::iterator i = valueOfInt.begin(), e = valueOfInt.end(); i!=e; ++i)
                errs()<<"\t\tConstIntPosition: "<<i->first<<", ConstIntValue: "<<i->second<<"\n";            
            if (debugCloning)
              for (std::vector<unsigned>::iterator i = posOfInt.begin(), e = posOfInt.end(); i!=e; ++i)
                errs()<<"\t\tIntPosition: "<<*i<<"\n";                
            if (debugCloning)
              for (std::map<unsigned, double>::iterator i = valueOfDouble.begin(), e = valueOfDouble.end(); i!=e; ++i)
                errs()<<"\t\tConstDoublePosition: "<<i->first<<", ConstDoubleValue: "<<i->second<<"\n";            
            if (debugCloning)
              for (std::vector<unsigned>::iterator i = posOfDouble.begin(), e = posOfDouble.end(); i!=e; ++i)
                errs()<<"\t\tDoublePosition: "<<*i<<"\n"; */

            // Construct the specializedName string. All the above work was done because this has to preserve the same order of arguments.
            // Check in posOfInt to see where this constant argument lies (0, 1, 2, .., _MAX_NUM_INTS-1). Add current argument in that place.
            std::stringstream ss; 
            
            // fill in the originalInts and originalDoubles with the new constant values of this call site, at the correct position
            for (std::map<unsigned, int>::iterator i = valueOfInt.begin(), e = valueOfInt.end(); i!=e; ++i) {
              unsigned pos = std::find(posOfInt.begin(), posOfInt.end(), i->first) - posOfInt.begin();
              std::stringstream tmp1;
              tmp1 << i->second;   // in order to convert int to string
              originalInts[pos] = tmp1.str();
            }  

            for (std::map<unsigned, double>::iterator i = valueOfDouble.begin(), e = valueOfDouble.end(); i!=e; ++i) {
              unsigned pos = std::find(posOfDouble.begin(), posOfDouble.end(), i->first) - posOfDouble.begin();
              std::stringstream tmp2;
              tmp2 << i->second;
              originalDoubles[pos] = tmp2.str(); 
            }           

            for (std::vector<std::string>::iterator i = originalInts.begin(), e = originalInts.end(); i!=e; ++i)
              ss << "_IP" << *i;
            for (std::vector<std::string>::iterator j = originalDoubles.begin(), e = originalDoubles.end(); j!=e; ++j){
              std::replace( j->begin(), j->end(), '.', '_');
              std::replace( j->begin(), j->end(), '-', '_');
              ss << "_DP" << *j; 
            }
            
            std::string specializedName = originalCore + ss.str();

            // process specializedName string to convert dots into underscores (for flat qasm generation purposes)
            //for (unsigned long i = 0; i < specializedName.length(); ++i)
            //  if (specializedName[i] == '.' || specializedName[i] == '-')
            //    specializedName[i] = '_';

            // don't clone if it has been before
            if (M.getFunction(specializedName)) {
              if (debugCloning)
                errs() << "\t\tAlready Cloned: " << specializedName << "\n";
              insertNewCallSite(CI, specializedName, &M);
              instErase.push_back((Instruction*)CI);
              continue;
            }

            ValueMap<const Value*, WeakVH> VMap;
            Function *specializedFunction = CloneFunctionInfo(F, VMap, &M); 
            specializedFunction->setName(specializedName);

            if (debugCloning)
              errs() << "\t\tCloned Function: " << specializedFunction->getName() << "\n";

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



