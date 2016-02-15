// Scaffold
// This pass implements a fault tolerant implementation of Toffoli gates
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
					Function *ToffoliImpl = M->getFunction("ToffoliImpl");
					// If it doesn't exist yet, create it
					if (!ToffoliImpl) {
						std::vector<Type*> ArgTypes(3);
						for (int i=0; i<3; i++)
							ArgTypes[i] = Type::getInt16Ty(getGlobalContext());
						FunctionType *FuncType = FunctionType::get(
								Type::getVoidTy(getGlobalContext()),
								ArrayRef<Type*>(ArgTypes),
								false);
						ToffoliImpl = Function::Create(FuncType,
								GlobalVariable::ExternalLinkage,
								"ToffoliImpl",
								M);
						ToffoliImpl->addFnAttr(Attribute::AlwaysInline);

						// Build Function;
						// Fetch arguments
						Function::arg_iterator arg_it = ToffoliImpl->arg_begin();
						Value *Target = arg_it++;	            
						Value *Control1 = arg_it++;
						Value *Control2 = arg_it;
            
            Control1->setName("control1");
            Control2->setName("control2");
            Target->setName("target");

						// Fetch gate definitions
						Function* gate_H = Intrinsic::getDeclaration(M, Intrinsic::H);
						Function* gate_T = Intrinsic::getDeclaration(M, Intrinsic::T);
						Function* gate_t = Intrinsic::getDeclaration(M, Intrinsic::Tdag);
						Function* gate_S = Intrinsic::getDeclaration(M, Intrinsic::S);
						Function* gate_CNOT = Intrinsic::getDeclaration(M, Intrinsic::CNOT);

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

            // -- Improved T-depth Toffoli gate (Amy et al.)
            // CNOT shorthands
						std::vector<Value*> C1C2; C1C2.push_back(Control2); C1C2.push_back(Control1);                        
						std::vector<Value*> C2T; C2T.push_back(Target); C2T.push_back(Control2);
						std::vector<Value*> TC1; TC1.push_back(Control1); TC1.push_back(Target);
						std::vector<Value*> C2C1; C2C1.push_back(Control1); C2C1.push_back(Control2);            
            
						// Construct circuit
						CallInst::Create(gate_H, ArrayRef<Value*>(Target), "", BB)->setTailCall();
						CallInst::Create(gate_t, ArrayRef<Value*>(Control1), "", BB)->setTailCall();
						CallInst::Create(gate_T, ArrayRef<Value*>(Control2), "", BB)->setTailCall();
						CallInst::Create(gate_T, ArrayRef<Value*>(Target), "", BB)->setTailCall();
						CallInst::Create(gate_CNOT, ArrayRef<Value*>(C1C2), "", BB)->setTailCall();
						CallInst::Create(gate_CNOT, ArrayRef<Value*>(TC1), "", BB)->setTailCall();
						CallInst::Create(gate_t, ArrayRef<Value*>(Control1), "", BB)->setTailCall();
						CallInst::Create(gate_CNOT, ArrayRef<Value*>(C2T), "", BB)->setTailCall();
						CallInst::Create(gate_CNOT, ArrayRef<Value*>(C2C1), "", BB)->setTailCall();
						CallInst::Create(gate_t, ArrayRef<Value*>(Control1), "", BB)->setTailCall();
						CallInst::Create(gate_t, ArrayRef<Value*>(Control2), "", BB)->setTailCall();
						CallInst::Create(gate_T, ArrayRef<Value*>(Target), "", BB)->setTailCall();
						CallInst::Create(gate_CNOT, ArrayRef<Value*>(TC1), "", BB)->setTailCall();
						CallInst::Create(gate_S, ArrayRef<Value*>(Control1), "", BB)->setTailCall();
						CallInst::Create(gate_CNOT, ArrayRef<Value*>(C2T), "", BB)->setTailCall();
						CallInst::Create(gate_CNOT, ArrayRef<Value*>(C1C2), "", BB)->setTailCall();
						CallInst::Create(gate_H, ArrayRef<Value*>(Target), "", BB)->setTailCall();

						ReturnInst::Create(getGlobalContext(), 0, BB);
					}
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

