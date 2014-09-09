#include <iostream>
#include <set>
#include "llvm/Pass.h"
#include "llvm/CallGraphSCCPass.h"
#include "llvm/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ValueSymbolTable.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"

using namespace llvm;
using namespace std;

namespace {
  /* Func is the struct that stores resource usage information
   * of each function, as well as the call graph
   */
  struct Func {
   public:
    string name;  // Name of the function
    // Functions called by this function and number of times called
    map<string, int> calls;
    
    /* self_bits stores the amount of qbits used by this function.
     *
     * total_bits stores the amount of qbits used by this function and
     * all its children.
     */
    int self_bits, total_bits_min, total_bits_max, self_ops, total_ops;
    Func(string n) : name(n),
                     self_bits(0), total_bits_min(0), total_bits_max(0),
                     self_ops(0), total_ops(0) {}
  };
  map<string, Func*> cg;  // Map from function name to Func struct
  Func* root = NULL;      // The Func struct for the "main" function

  struct ScaffoldResourceAnalysis : public CallGraphSCCPass {
    static char ID;

    ScaffoldResourceAnalysis() : CallGraphSCCPass(ID) {}

    int analyzeAlloc(Instruction* inst) {
      if (const AllocaInst *AI = dyn_cast<AllocaInst>(inst)) {
        // AllocaInst means variable declaration
        Type* ty = AI->getAllocatedType();
        int sub_bits = 1;
        /* For array types, recursively walk through each dimention
         * and multiply all the sizes until we reach the last
         */
        while (ty->isArrayTy()) {
          sub_bits *= ty->getArrayNumElements();
          ty = ty->getArrayElementType();
        }
        // We only need to count qbits and cbits; skip integers
        if (ty->getPrimitiveSizeInBits() > 8)
          sub_bits = 0;
        return sub_bits;
      }
      return 0;
    }

    int analyzeGate(Instruction* inst) {
      if (const CallInst *CI = dyn_cast<CallInst>(inst)) {
        string name = CI->getCalledFunction()->getName();
        if (name.find("llvm.") != string::npos)
          return 1;
      }
      return 0;
    }

    virtual bool runOnSCC(CallGraphSCC &SCC) {
      /* Currently I don't know what does "Singular" mean in LLVM.
       * I need more complicated test cases to figure out.
       */
      assert(SCC.isSingular());

      // Walk through the call graph
      for (CallGraphSCC::iterator itr = SCC.begin(); itr != SCC.end(); itr++) {
        Function* f = (*itr)->getFunction();
        if (!f) // External nodes will give empty function
          continue;
        string name = f->getName();
        Func* fnode;
        if (cg.find(name) == cg.end()) {
          cg[name] = fnode = new Func(name);
          if (name == "main")
            root = fnode;
          if (!f->empty()) {  // Whether this function contain code
            int bits = 0, ops = 0;
            for (Function::iterator f_itr = f->begin(); f_itr != f->end(); f_itr++) {
              // Walk through each instruction in the basic block
              for (BasicBlock::iterator bb_itr = f_itr->begin();
                  bb_itr != f_itr->end(); bb_itr++) {
                bits += analyzeAlloc(bb_itr);
                ops += analyzeGate(bb_itr);
              }
            }
            fnode->self_bits = bits;
            fnode->self_ops = ops;
          }
        } else {
          fnode = cg[name];
        }
        // Insert call graph
        for (CallGraphNode::iterator node_itr = (*itr)->begin();
             node_itr != (*itr)->end(); node_itr++) {
          if (node_itr->second->getFunction())
            fnode->calls.insert(make_pair(
              node_itr->second->getFunction()->getName(),
              node_itr->second->getNumReferences()));
        }
      }
      return false;
    }

    void dump(Func* func, int tab, int ref) {
      for (int i = 0; i < tab; i++)
        std::cout << "  ";
      if (ref)
        cout << "(" << ref << ")";
      std::cout << func->name << " (SB="
                << func->self_bits << ", TBMin="
                << func->total_bits_min << ", TBMax="
                << func->total_bits_max << ", SOp="
                << func->self_ops << ", TOp="
                << func->total_ops << ")" << std::endl;
      for (map<string, int>::iterator itr = func->calls.begin();
           itr != func->calls.end(); itr++)
        dump(cg[itr->first], tab + 1, itr->second);
    }
    
    /* Calculate the total number of bits used by each function and
     * its children
     */
    void propagate_bits(Func* func) {
      func->total_bits_min = func->total_bits_max = func->self_bits;
      func->total_ops = func->self_ops;
      int max_bits = 0;
      for (map<string, int>::iterator itr = func->calls.begin();
           itr != func->calls.end(); itr++) {
        propagate_bits(cg[itr->first]);
        max_bits = max(max_bits, cg[itr->first]->total_bits_min);
        func->total_bits_max += itr->second * cg[itr->first]->total_bits_max;
        func->total_ops += itr->second * cg[itr->first]->total_ops;
      }
      func->total_bits_min += max_bits;
    }

    virtual bool doFinalization (CallGraph &CG) {
      assert(root);
      propagate_bits(root);
      dump(root, 0, 0);
      for (map<string, Func*>::iterator itr = cg.begin(); itr != cg.end(); itr++)
        delete itr->second;
      return false;
    }
  }; // end of struct ScaffoldResourceAnalysis
}  // end of anonymous namespace

char ScaffoldResourceAnalysis::ID = 0;

static RegisterPass<ScaffoldResourceAnalysis> X("sra", "Scaffold resource analysis",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);

