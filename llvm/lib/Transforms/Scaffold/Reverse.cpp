//===------------------------------ Reverse.cpp ---------------------------===//
//
// This file implements the Scaffold Pass of reversing functions. In the 
// initial stage of the compiler, we search all of the IR
// code for functions beginning with "_reverse_". For any of these functions,
// we see which function they are meant to reverse; either an intrinsic 
// function, or a user-defined function. For intrinsic functions, we already 
// know their inverses. Otherwise, we can reverse user-defined functions 
// with an "Instruction Visitor," an llvm construct which allows us to 
// manipulate each individual instruction. Using this visitor, we place the 
// inverse of each original instruction in reverse of the original order
// of the instructions.
//
// For reasons inherent to the uncertainty of quantum computation, this
// functionality is often crucial for optimal use of memory, specifically
// for automatic garbage-collection.
//
//        This file was created by Scaffold Compiler Working Group
//
//===---------------------------------------------------------------------===//
//
#include <map>

#include "llvm/ADT/ValueMap.h" 

#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Intrinsics.h"
#include "llvm/LLVMContext.h"
#include "llvm/Pass.h"

#include "llvm/Support/InstIterator.h"
#include "llvm/Support/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

namespace {
    // We need to use a ModulePass in order to insert new 
    // Functions into the target code
    class FunctionReverse : public ModulePass {
      public:
        static char ID;
        // Prefix of functions that should be replaced with the reverse 
        // of the function with the prefix dropped
        static const std::string REVERSE_PREFIX;
        FunctionReverse() : ModulePass(ID) {}
        // an InstVisitor is an LLVM construct which allows to manipulate
        // each instruction in our function
        struct InsertReverseFunctionsVisitor : 
        public InstVisitor<InsertReverseFunctionsVisitor> {
            // The reversed function implementations will be Functions 
            // in M's FunctionList
            Module *M;
            // We need to know the inverse of every intrinsic function 
            // before reversing any functions
            ValueMap<Function *, Function *> IntrinsicInverses;

            // The constructor is called once per module (in runOnModule)
            InsertReverseFunctionsVisitor(Module *module) : M(module) {
                // This table maps each intrinsic function to its inverse
                std::vector<std::vector<Type*> > onePerm;
                std::vector<std::vector<Type*> > twoPerms;
                std::vector<std::vector<Type*> > threePerms;
                //std::vector<ArrayRef<Type*> > prepTys;
                //std::vector<ArrayRef<Type*> > RTys;
                std::vector<Type*> v;
                //errs() << "About to get type pointers\n";
                Type *aa = Type::getInt8Ty(M->getContext());
                Type *qq = Type::getInt16Ty(M->getContext());
                Type *dd = Type::getDoubleTy(M->getContext());
                Type *ii = Type::getInt32Ty(M->getContext());
                /*
                v.push_back(aa);
                v.push_back(dd);
                RTys.push_back(makeArrayRef(v));
                v.pop_back();
                v.pop_back();
                v.push_back(qq);
                v.push_back(dd);
                RTys.push_back(makeArrayRef(v));


                v.push_back(aa);
                v.push_back(ii);
                prepTys.push_back(makeArrayRef(v));
                v.pop_back();
                v.pop_back();
                v.push_back(qq);
                v.push_back(ii);
                prepTys.push_back(makeArrayRef(v));
                */
                //errs() << "About to make onePerm\n";

                v.push_back(aa);
                onePerm.push_back(v);//a
                v.pop_back();

                v.push_back(qq);
                onePerm.push_back(v);//q
                v.pop_back();

                //errs() << "About to make twoPerms\n";

                v.push_back(aa);
                v.push_back(aa);
                twoPerms.push_back(v);//aa
                v.pop_back();
                v.push_back(qq);
                twoPerms.push_back(v);//aq
                v.pop_back();
                v.pop_back();
                v.push_back(qq);
                v.push_back(aa);
                twoPerms.push_back(v);//qa
                v.pop_back();
                v.push_back(qq);
                twoPerms.push_back(v);//qq

                //errs() << "About to do threePerms\n";

                v.push_back(qq);
                threePerms.push_back(v);//qqq
                v.pop_back();

                v.push_back(aa);
                threePerms.push_back(v);//qqa
                v.pop_back();
                v.pop_back();

                v.push_back(aa);
                v.push_back(aa);
                threePerms.push_back(v);//qaa
                v.pop_back();

                v.push_back(qq);
                threePerms.push_back(v);//qaq
                v.pop_back();
                v.pop_back();
                v.pop_back();

                v.push_back(aa);
                v.push_back(qq);
                v.push_back(qq);
                threePerms.push_back(v);//aqq
                v.pop_back();

                v.push_back(aa);
                threePerms.push_back(v);//aqa
                v.pop_back();
                v.pop_back();

                v.push_back(aa);
                v.push_back(aa);
                threePerms.push_back(v);//aaa
                v.pop_back();

                v.push_back(qq);
                threePerms.push_back(v);//aaq

                for(int i=0; i<2; i++){
                  llvm::ArrayRef<Type*> ar = llvm::makeArrayRef(onePerm[i]);

                  //errs() << "doing one perm getDeclaration for " << i;

                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::H, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::H, ar);

                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::MeasX, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::MeasX, ar);

                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::MeasZ, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::MeasZ, ar);

                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::PrepX, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::PrepX, ar);

                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::PrepZ, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::PrepZ, ar);
                
                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::Rx, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::Rx, ar);
                
                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::Ry, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::Ry, ar);
                
                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::Rz, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::Rz, ar);
                 
                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::S, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::Sdag, ar);

                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::Sdag, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::S, ar);

                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::T, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::Tdag, ar);

                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::Tdag, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::T, ar);

                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::X, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::X, ar);

                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::Y, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::Y, ar);

                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::Z, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::Z, ar);
                }

                //errs() << "2perms getDeclaration\n";
                
                for(int i=0; i<4; i++){
                  llvm::ArrayRef<Type*> ar = llvm::makeArrayRef(twoPerms[i]);
                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::CNOT, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::CNOT, ar);
                }
                //errs() << "3perms getDeclaration\n";
                
                for(int i=0; i<8; i++){
                  llvm::ArrayRef<Type*> ar = llvm::makeArrayRef(threePerms[i]);
                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::Toffoli, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::Toffoli, ar);

                  IntrinsicInverses[Intrinsic::getDeclaration
                  (M, Intrinsic::Fredkin, ar)] = Intrinsic::getDeclaration
                  (M, Intrinsic::Fredkin, ar);
                }
                
                //errs() << "store_cbit getDeclaration\n";
                IntrinsicInverses[M->getFunction("store_cbit")] 
                = M->getFunction("store_cbit");
            }
            // Returns a pointer to the inverse of the specified
            // function and creates it if it doesn't exist yet
            Function *GetOrCreateInverseFunction(Function *Func) {
                if (IntrinsicInverses.count(Func)) {
                    return IntrinsicInverses[Func];
                }
                const std::string ReverseFuncName = 
                    Func->getName().str() + "_Reverse";
                Function *ReverseFunc = M->getFunction(ReverseFuncName);
                // If it doesn't exist yet, create it
                if (!ReverseFunc) {
                    ValueMap<const Value*, WeakVH> VMap;
                    ReverseFunc = CloneFunction
                       (Func, VMap, false /* ModuleLevelChanges */);
                    ReverseFunc->setName(ReverseFuncName);
                    BasicBlock &BB = ReverseFunc->front();
                    // Finds all of the call instructions 
                    // in the cloned reverse function
                    std::vector<CallInst *> V;
                    BasicBlock::iterator It;
                    BasicBlock::iterator E = BB.end();
                    CallInst *CI;
                    
                    for (It = BB.begin(); It != E; It++) {
                        if ( (CI = dyn_cast<CallInst>(&*It)) ) {
                            V.push_back(CI);
                        }
                    }
                    // Iterates backwards through the list of call 
                    // instructions, inverts the functions call by the 
                    // instructions and moves them to the end of the 
                    // reversed function
                    std::vector<CallInst *>::reverse_iterator I;
                    Function *Inverse;
                    std::vector<CallInst *>::reverse_iterator F = V.rend();
                    for (I = V.rbegin(); I != F; I++) {
                        Inverse = GetOrCreateInverseFunction
                            ((*I)->getCalledFunction());
                        (*I)->setCalledFunction(Inverse);
                        (*I)->removeFromParent();
                        BB.getInstList().insert(&BB.back(), *I);
                    }
                    M->getFunctionList().push_back(ReverseFunc);
                }
                return ReverseFunc;
            } // GetOrCreateInverseFunction()

            // Gets called very time the target LLVM IR code  
            // has a function call
            // Overrides visitCallInst in the InstVisitor class
            void visitCallInst(CallInst &I) {
                // Get the current function
                Function *CF = I.getCalledFunction();
                if (CF->getName().startswith(REVERSE_PREFIX)) {
                    // Stores the number of times a reverse function is 
                    // prefixed with '_reverse_'; If this number is odd, it 
                    // it will be replaced by the function's reverse 
                    // otherwiseit will be replaced by the function itself
                    int numReverses = 0;
                    StringRef ForwardFunctionName = CF->getName();
                    
                    while(ForwardFunctionName.startswith(REVERSE_PREFIX)){
                        ForwardFunctionName = ForwardFunctionName.drop_front
                            (REVERSE_PREFIX.size());
                        ++numReverses;
                    }
                    
                    Function *ForwardFunction;
                    // The Toffoli function is special becuase it has 
                    // been rewritten to have a different function name
                    if (ForwardFunctionName == "Toffoli")
                        ForwardFunction = M->getFunction("ToffoliImpl");
                    else
                        ForwardFunction = M->getFunction(ForwardFunctionName);
                    
                    // If the function name was not found in the current 
                    // module we can search for it in the intrinsics by 
                    // prepending "llvm." to it
                    if (ForwardFunction == NULL) {
                        const std::string IntrinsicForwardFunctionName 
                            = "llvm." + ForwardFunctionName.str();
                        ForwardFunction = M->getFunction
                            (IntrinsicForwardFunctionName);
                    }

                    // If no function exists for the base after the 
                    // _reverse_, print an error message
                    if (ForwardFunction == NULL) {
                        errs() << 
                        "Error: Could not invert non-existent function " 
                        << ForwardFunctionName << "\n";
                    }
                    // If a function exists, but different arguments are 
                    // supplied to the call site than the type of the 
                    // function, print an error message
                    else if (CF->getFunctionType() != ForwardFunction->
                        getFunctionType()) {
                        errs() << "Error: reversed function " << CF->getName()
                               << " did not match type of foward function " 
                               << ForwardFunctionName << "\n";
                    }

                    Function *ReplacementFunction = numReverses % 2 == 1 ? 
                        GetOrCreateInverseFunction(ForwardFunction) 
                        : ForwardFunction;
                    // Now that we have the replacement function, this 
                    // performs the insertion into the target code
                    const FunctionType *FuncType = ReplacementFunction
                        ->getFunctionType();
                    std::vector<Value*> Args(FuncType->getNumParams());
                    unsigned i;
                    for (i=0; i<FuncType->getNumParams(); i++) {
                        Args[i] = I.getArgOperand(i);
                    }
                    CallInst::Create(ReplacementFunction, 
                        ArrayRef<Value*>(Args), "", &I);
                    I.eraseFromParent();
                }
            } // visitCallInst()
        }; // struct InsertReverseFunctionsVisitor


        virtual bool runOnModule(Module &M) {
                InsertReverseFunctionsVisitor IRFV(&M);
                IRFV.visit(M);

                return true;
            } // runOnModule()

    }; // struct FunctionReverse
} // namespace

char FunctionReverse::ID = 0;
const std::string FunctionReverse::REVERSE_PREFIX = "_reverse_";
static RegisterPass<FunctionReverse> X("FunctionReverse", 
    "Function Reverser", false, false);
