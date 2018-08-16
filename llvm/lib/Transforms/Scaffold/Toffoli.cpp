// Scaffold
// This pass implements a fault tolerant implementation of Toffoli gates
// Update Aug 2018: Toffoli(ctrl1, ctrl2, targ)
//

#include <cstdlib>
#include <cstdio>

#include "llvm/ADT/ArrayRef.h"

#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Intrinsics.h"
#include "llvm/LLVMContext.h"
#include "llvm/Pass.h"

#include "llvm/Support/CallSite.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

namespace {
	// We need to use a ModulePass in order to create new Functions
	struct ToffoliReplace : public ModulePass {
		static char ID;
		ToffoliReplace() : ModulePass(ID) {}

		struct ToffoliVisitor : public InstVisitor<ToffoliVisitor> {
			// The Toffoli implementation will be a Function in M's FunctionList
			Module *M;
			// The constructor is called once per module (in runOnModule)
			ToffoliVisitor(Module *module) : M(module) {}

			void visitCallInst(CallInst &I) {
				// Determine whether this is a Toffoli gate
				Function *CF = I.getCalledFunction();
				if (CF->isIntrinsic() && CF->getIntrinsicID() == Intrinsic::Toffoli) {
					// Retrieve the FT Toffoli implementation
					std::string implName = "ToffoliImpl";
		            std::vector<Type*> ArgTypes(3);
				    for (int i=0; i<3; i++){
					    ArgTypes[i] = I.getArgOperand(i)->getType();//Type::getInt16Ty(getGlobalContext());
                        if(ArgTypes[i] == Type::getInt16Ty(getGlobalContext()))
                            implName.append("_Q");
                        else
                            implName.append("_A");
                    }
                        
                    Function *ToffoliImpl = M->getFunction(implName);
                    if(!ToffoliImpl){
						FunctionType *FuncType = FunctionType::get(
								Type::getVoidTy(getGlobalContext()),
								ArrayRef<Type*>(ArgTypes),
								false);
						ToffoliImpl = Function::Create(FuncType,
								GlobalVariable::ExternalLinkage,
								implName,
								M);
						ToffoliImpl->addFnAttr(Attribute::AlwaysInline);

						// Build Function;
						// Fetch arguments: Toffoli(ctrl1, ctrl2, targ)
						Function::arg_iterator arg_it = ToffoliImpl->arg_begin();
						Value *Control1 = arg_it++;
						Value *Control2 = arg_it++;
						Value *Target = arg_it;	            
            
                        Control1->setName("control1");
                        Control2->setName("control2");
                        Target->setName("target");

						// Fetch gate definitions
						//Function* gate_H = Intrinsic::getDeclaration(M, Intrinsic::H);
						//Function* gate_T = Intrinsic::getDeclaration(M, Intrinsic::T);
						//Function* gate_t = Intrinsic::getDeclaration(M, Intrinsic::Tdag);
						//Function* gate_S = Intrinsic::getDeclaration(M, Intrinsic::S);
						//Function* gate_CNOT = Intrinsic::getDeclaration(M, Intrinsic::CNOT);

						// Create BasicBlock
						BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "", ToffoliImpl, 0);
            
            /*
            // -- The standard implementation of Toffoli gates (Mike and Ike)
            // CNOT shorthands
						std::vector<Value*> C2T; C2T.push_back(Target); C2T.push_back(Control2);
						std::vector<Value*> C1T; C1T.push_back(Target); C1T.push_back(Control1);
						std::vector<Value*> C1C2; C1C2.push_back(Control2); C1C2.push_back(Control1);
						// Construct circuit
						CallInst::Create(gate_H, ArrayRef<Value*>(Target), "", BB)->setTailCall();
						CallInst::Create(gate_CNOT, ArrayRef<Value*>(C2T), "", BB)->setTailCall();
						CallInst::Create(gate_t, ArrayRef<Value*>(Target), "", BB)->setTailCall();
						CallInst::Create(gate_CNOT, ArrayRef<Value*>(C1T), "", BB)->setTailCall();
						CallInst::Create(gate_T, ArrayRef<Value*>(Target), "", BB)->setTailCall();
						CallInst::Create(gate_CNOT, ArrayRef<Value*>(C2T), "", BB)->setTailCall();
						CallInst::Create(gate_t, ArrayRef<Value*>(Target), "", BB)->setTailCall();
						CallInst::Create(gate_CNOT, ArrayRef<Value*>(C1T), "", BB)->setTailCall();
						CallInst::Create(gate_t, ArrayRef<Value*>(Control2), "", BB)->setTailCall();
						CallInst::Create(gate_T, ArrayRef<Value*>(Target), "", BB)->setTailCall();
						CallInst::Create(gate_CNOT, ArrayRef<Value*>(C1C2), "", BB)->setTailCall();
						CallInst::Create(gate_H, ArrayRef<Value*>(Target), "", BB)->setTailCall();
						CallInst::Create(gate_t, ArrayRef<Value*>(Control2), "", BB)->setTailCall();
						CallInst::Create(gate_CNOT, ArrayRef<Value*>(C1C2), "", BB)->setTailCall();
						CallInst::Create(gate_T, ArrayRef<Value*>(Control1), "", BB)->setTailCall();
						CallInst::Create(gate_S, ArrayRef<Value*>(Control2), "", BB)->setTailCall();
            */

            // -- Improved T-depth Toffoli gate (Amy et al. (http://arxiv.org/abs/1206.0758v3))
            // CNOT shorthands

						std::vector<Value*> C1C2; C1C2.push_back(Control1); C1C2.push_back(Control2); 
						std::vector<Type*> C1C2ty; C1C2ty.push_back(ArgTypes[0]); C1C2ty.push_back(ArgTypes[1]);
						std::vector<Value*> C2C1; C2C1.push_back(Control2); C2C1.push_back(Control1);   
						std::vector<Type*> C2C1ty; C2C1ty.push_back(ArgTypes[1]); C2C1ty.push_back(ArgTypes[0]);            

						std::vector<Value*> C1T; C1T.push_back(Control1); C1T.push_back(Target);
						std::vector<Type*> C1Tty; C1Tty.push_back(ArgTypes[0]); C1Tty.push_back(ArgTypes[2]);
						std::vector<Value*> C2T; C2T.push_back(Control2); C2T.push_back(Target);
						std::vector<Type*> C2Tty; C2Tty.push_back(ArgTypes[1]); C2Tty.push_back(ArgTypes[2]);


						std::vector<Value*> TC1; TC1.push_back(Target); TC1.push_back(Control1);
						std::vector<Type*> TC1ty; TC1ty.push_back(ArgTypes[2]); TC1ty.push_back(ArgTypes[0]);
						std::vector<Value*> TC2; TC2.push_back(Target); TC2.push_back(Control2);
						std::vector<Type*> TC2ty; TC2ty.push_back(ArgTypes[2]); TC2ty.push_back(ArgTypes[1]);
            
						
                        // Construct circuit

                        //t is Tdag, T is T
						// H (targ)
						CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::H, ArrayRef<Type*>(ArgTypes[2])), 
                                         ArrayRef<Value*>(Target), "", BB)->setTailCall();

						// T (ctrl1)
						CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::T, ArrayRef<Type*>(ArgTypes[0])), 
                                         ArrayRef<Value*>(Control1), "", BB)->setTailCall();

						// T (ctrl2)
						CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::T, ArrayRef<Type*>(ArgTypes[1])),
                                         ArrayRef<Value*>(Control2), "", BB)->setTailCall();

						// T (targ)
						CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::T, ArrayRef<Type*>(ArgTypes[2])),
                                         ArrayRef<Value*>(Target), "", BB)->setTailCall();
                                   
						// CNOT (ctrl2, ctrl1)
						CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(C2C1ty)),
						                 ArrayRef<Value*>(C2C1), "", BB)->setTailCall();

						// CNOT (targ, ctrl2)
						CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(TC2ty)),
						                 ArrayRef<Value*>(TC2), "", BB)->setTailCall();
            
						// CNOT (ctrl1, targ)
						CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(C1Tty)),
						                 ArrayRef<Value*>(C1T), "", BB)->setTailCall();
  
						// Tdag (ctrl2)
            CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::Tdag, ArrayRef<Type*>(ArgTypes[1])),
						                 ArrayRef<Value*>(Control2), "", BB)->setTailCall();
            
						// T (targ)
            CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::T, ArrayRef<Type*>(ArgTypes[2])),
						                 ArrayRef<Value*>(Target), "", BB)->setTailCall();
            
						// CNOT (ctrl1, ctrl2)
            CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(C1C2ty)),
						                 ArrayRef<Value*>(C1C2), "", BB)->setTailCall();
            
						// Tdag (ctrl1)
            CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::Tdag, ArrayRef<Type*>(ArgTypes[0])),
						                 ArrayRef<Value*>(Control1), "", BB)->setTailCall();
            
						// Tdag (ctrl2)
            CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::Tdag, ArrayRef<Type*>(ArgTypes[1])),
						                 ArrayRef<Value*>(Control2), "", BB)->setTailCall();
            
						// CNOT (targ, ctrl2)
            CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(TC2ty)),
						                 ArrayRef<Value*>(TC2), "", BB)->setTailCall();
           
						// CNOT (ctrl1, targ)
            CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(C1Tty)),
						                 ArrayRef<Value*>(C1T), "", BB)->setTailCall();
            
						// CNOT (ctrl2, ctrl1)
            CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(C2C1ty)),
						                 ArrayRef<Value*>(C2C1), "", BB)->setTailCall();
            
						// H (targ)
            CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::H, ArrayRef<Type*>(ArgTypes[2])),
						                 ArrayRef<Value*>(Target), "", BB)->setTailCall();
                        //end TODO

						ReturnInst::Create(getGlobalContext(), 0, BB);
					}//end !ToffoliImpl
					std::vector<Value*>  Args(3);
					for (int i=0; i<3; i++)
						Args[i] = I.getArgOperand(i);
					BasicBlock::iterator ii(&I);
					ReplaceInstWithInst(I.getParent()->getInstList(), ii,
						CallInst::Create(ToffoliImpl, ArrayRef<Value*>(Args)));

				} // endif 'found Toffoli'
			} // visitCallInst()
		}; // struct ToffoliVisitor

		virtual bool runOnModule(Module &M) {
			ToffoliVisitor TV(&M);
			TV.visit(M);

			return true;
		} // runOnModule()
		
	}; // struct ToffoliReplace
} // namespace

char ToffoliReplace::ID = 0;
static RegisterPass<ToffoliReplace> X("ToffoliReplace", "Toffoli Replacer", false, false);

