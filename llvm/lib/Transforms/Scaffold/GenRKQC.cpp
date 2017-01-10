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
#include "llvm/Argument.h"
#include "llvm/Module.h"

#include "llvm/Support/CallSite.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

namespace {
	// We need to use a ModulePass in order to create new Functions
	struct GenRKQC : public ModulePass {
		static char ID;
		GenRKQC() : ModulePass(ID) {}

		struct RKQCVisitor : public InstVisitor<RKQCVisitor> {
			// The Toffoli implementation will be a Function in M's FunctionList
			Module *M;
			// The constructor is called once per module (in runOnModule)
			RKQCVisitor(Module *module) : M(module) {}

			Value* createAncilla(std::string& name, BasicBlock* BB ){
				std::string anc_name = "ancilla_"+name;
				Type *abit_type = IntegerType::getInt8Ty(getGlobalContext());
        		Constant *val = ConstantInt::get(Type::getInt32Ty(getGlobalContext()), 1, false);       
				Value* Idx[2];	  
				Idx[0] = Constant::getNullValue(Type::getInt32Ty(getGlobalContext()));  
				Idx[1] = ConstantInt::get(Type::getInt32Ty(getGlobalContext()),0);
				ArrayType *arrayType = ArrayType::get(abit_type, 1);
				AllocaInst *anc = new AllocaInst(arrayType,anc_name, BB);
				anc->setAlignment(8);
        		Value *intArrPtr = GetElementPtrInst::CreateInBounds(anc,Idx,"",BB);
				Value *value = new LoadInst(intArrPtr, "", BB);
				return value;
			}

			void create_a_swap_b(CallInst& I, Function* RKQC_Func, std::string& rkqcName){
				if(!RKQC_Func){
					std::vector<Type*> ArgTypes(2);
					for(int i=0;i<2;i++) ArgTypes[i] = I.getArgOperand(i)->getType();//Type::getInt16Ty(getGlobalContext());
					FunctionType *FuncType = FunctionType::get(Type::getVoidTy(getGlobalContext()),
						ArrayRef<Type*>(ArgTypes), false);
					RKQC_Func = Function::Create(FuncType,GlobalVariable::ExternalLinkage,rkqcName,M);
					RKQC_Func->addFnAttr(Attribute::AlwaysInline);

					Function::arg_iterator arg_it = RKQC_Func->arg_begin();
					Value *Target0 = arg_it;
					Type *Type0 = arg_it->getType();
					arg_it++;
					Value *Target1 = arg_it;
					Type *Type1 = arg_it->getType();

					Target0->setName("target0");
					Target1->setName("target1");


					BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "", RKQC_Func, 0);

					std::vector<Type*> Types0; Types0.push_back(Type0); Types0.push_back(Type1);
					std::vector<Type*> Types1; Types1.push_back(Type1); Types1.push_back(Type0);
					std::vector<Value*> T0T1; T0T1.push_back(Target0); T0T1.push_back(Target1);
					std::vector<Value*> T1T0; T1T0.push_back(Target1); T1T0.push_back(Target0);

					Function* gate_CNOT_T0T1 = Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(Types0));
					Function* gate_CNOT_T1T0 = Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(Types1));

					CallInst::Create(gate_CNOT_T0T1, ArrayRef<Value*>(T0T1), "", BB)->setTailCall();
					CallInst::Create(gate_CNOT_T1T0, ArrayRef<Value*>(T1T0), "", BB)->setTailCall();
					CallInst::Create(gate_CNOT_T0T1, ArrayRef<Value*>(T0T1), "", BB)->setTailCall();

					ReturnInst::Create(getGlobalContext(), 0, BB);
				}
				std::vector<Value*>  Args(2);
				for (int i=0; i<3; i++) Args[i] = I.getArgOperand(i);
				BasicBlock::iterator ii(&I);
				ReplaceInstWithInst(I.getParent()->getInstList(), ii,
					CallInst::Create(RKQC_Func, ArrayRef<Value*>(Args)));
			}


			void create_assign_value_of_b_to_a(CallInst& I, Function* RKQC_Func, std::string& rkqcName){
				if(!RKQC_Func){
					std::vector<Type*> ArgTypes(2);
					ArgTypes[0] = Type::getInt16Ty(getGlobalContext());
					ArgTypes[1] = Type::getInt16Ty(getGlobalContext());
					FunctionType *FuncType = FunctionType::get(Type::getVoidTy(getGlobalContext()),
						ArrayRef<Type*>(ArgTypes),false);
					RKQC_Func = Function::Create(FuncType,GlobalVariable::ExternalLinkage,rkqcName,M);
					RKQC_Func->addFnAttr(Attribute::AlwaysInline);

					Function::arg_iterator arg_it = RKQC_Func->arg_begin();
					Value *Target = arg_it;
					Type *Type0 = arg_it->getType();
					arg_it++;
					Value *Control = arg_it;
					Type *Type1 = arg_it->getType();

					Target->setName("target");
					Control->setName("control");


					BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "", RKQC_Func, 0);
					std::string name = "zg";
					Value* ancilla = createAncilla(name,BB);	
					Type* Type_A = ancilla->getType();

					std::vector<Value*> TA; TA.push_back(Target); TA.push_back(ancilla);
					std::vector<Type*> TA_V; TA_V.push_back(Type0); TA_V.push_back(Type_A);

					std::vector<Value*> AT; AT.push_back(ancilla); AT.push_back(Target);
					std::vector<Type*> AT_V; AT_V.push_back(Type_A); AT_V.push_back(Type0);
					
					std::vector<Value*> BA; BA.push_back(Control); BA.push_back(Target);
					std::vector<Type*> BA_V; BA_V.push_back(Type1); BA_V.push_back(Type0);
					
					Function* gate_CNOT_TA = Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(TA_V));
					Function* gate_CNOT_AT = Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(AT_V));
					Function* gate_CNOT_BA = Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(BA_V));


					CallInst::Create(gate_CNOT_TA, ArrayRef<Value*>(TA), "", BB)->setTailCall();
					CallInst::Create(gate_CNOT_AT, ArrayRef<Value*>(AT), "", BB)->setTailCall();
					CallInst::Create(gate_CNOT_BA, ArrayRef<Value*>(BA), "", BB)->setTailCall();
					ReturnInst::Create(getGlobalContext(), 0, BB);
				}
				std::vector<Value*>  Args(2);
				for (int i=0; i<2; i++) Args[i] = I.getArgOperand(i);
				BasicBlock::iterator ii(&I);
				ReplaceInstWithInst(I.getParent()->getInstList(), ii, CallInst::Create(RKQC_Func, ArrayRef<Value*>(Args)));
			}

			void create_assign_value_of_0_to_a(CallInst& I, Function* RKQC_Func, std::string& rkqcName){
				if(!RKQC_Func){
					std::vector<Type*> ArgTypes(2);
					ArgTypes[0] = Type::getInt16Ty(getGlobalContext());
					ArgTypes[1] = Type::getInt32Ty(getGlobalContext());
					FunctionType *FuncType = FunctionType::get(Type::getVoidTy(getGlobalContext()),
						ArrayRef<Type*>(ArgTypes),false);
					RKQC_Func = Function::Create(FuncType,GlobalVariable::ExternalLinkage,rkqcName,M);
					RKQC_Func->addFnAttr(Attribute::AlwaysInline);

					Function::arg_iterator arg_it = RKQC_Func->arg_begin();
					Value *Target = arg_it;
					Type *Target_T = arg_it->getType();

					Target->setName("target");


					BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "", RKQC_Func, 0);

					std::string name = "zero_zero";
					Value* Control = createAncilla(name,BB);	
					Type* Control_T = Control->getType();
					std::string name2= "zero_garbage";
					Value* ancilla = createAncilla(name2,BB);	
					Type* ancilla_T = ancilla->getType();

					std::vector<Value*> TA; TA.push_back(Target); TA.push_back(ancilla);
					std::vector<Type*> TA_V; TA_V.push_back(Target_T); TA_V.push_back(ancilla_T);

					std::vector<Value*> AT; AT.push_back(ancilla); AT.push_back(Target);
					std::vector<Type*> AT_V; AT_V.push_back(ancilla_T); AT_V.push_back(Target_T);

					std::vector<Value*> BA; BA.push_back(Control); BA.push_back(Target);
					std::vector<Type*> BA_V; BA_V.push_back(Control_T); BA_V.push_back(Target_T);

					
					Function* gate_CNOT_TA = Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(TA_V));
					Function* gate_CNOT_AT = Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(AT_V));
					Function* gate_CNOT_BA = Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(BA_V));
					
					CallInst::Create(gate_CNOT_TA, ArrayRef<Value*>(TA), "", BB)->setTailCall();
					CallInst::Create(gate_CNOT_AT, ArrayRef<Value*>(AT), "", BB)->setTailCall();
					CallInst::Create(gate_CNOT_BA, ArrayRef<Value*>(BA), "", BB)->setTailCall();
					ReturnInst::Create(getGlobalContext(), 0, BB);
  				}
				std::vector<Value*>  Args(2);
				for (int i=0; i<2; i++) Args[i] = I.getArgOperand(i);
//				Args[0] = I.getArgOperand(0);
				BasicBlock::iterator ii(&I);
				ReplaceInstWithInst(I.getParent()->getInstList(), ii, CallInst::Create(RKQC_Func, ArrayRef<Value*>(Args)));
			}

			void create_assign_value_of_1_to_a(CallInst& I, Function* RKQC_Func, std::string& rkqcName){
				if(!RKQC_Func){
					std::vector<Type*> ArgTypes(2);
					ArgTypes[0] = Type::getInt16Ty(getGlobalContext());
					ArgTypes[1] = Type::getInt32Ty(getGlobalContext());
					FunctionType *FuncType = FunctionType::get(Type::getVoidTy(getGlobalContext()),
						ArrayRef<Type*>(ArgTypes),false);
					RKQC_Func = Function::Create(FuncType,GlobalVariable::ExternalLinkage,rkqcName,M);
					RKQC_Func->addFnAttr(Attribute::AlwaysInline);

					Function::arg_iterator arg_it = RKQC_Func->arg_begin();
					Value *Target = arg_it;
					Type *Target_T = arg_it->getType();

					Target->setName("target");

					BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "", RKQC_Func, 0);

					std::string name = "one_one";
					Value* Control = createAncilla(name,BB);	
					Type* Control_T = Control->getType();
					std::string name2 = "zero_garbage";
					Value* ancilla = createAncilla(name2,BB);	
					Type* ancilla_T = ancilla->getType();


					std::vector<Value*> TA; TA.push_back(Target); TA.push_back(ancilla);
					std::vector<Type*> TA_V; TA_V.push_back(Target_T); TA_V.push_back(ancilla_T);

					std::vector<Value*> AT; AT.push_back(ancilla); AT.push_back(Target);
					std::vector<Type*> AT_V; AT_V.push_back(ancilla_T); AT_V.push_back(Target_T);

					std::vector<Value*> BA; BA.push_back(Control); BA.push_back(Target);
					std::vector<Type*> BA_V; BA_V.push_back(Control_T); BA_V.push_back(Target_T);

					Function* gate_CNOT_TA = Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(TA_V));
					Function* gate_CNOT_AT = Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(AT_V));
					Function* gate_CNOT_BA = Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(BA_V));

					CallInst::Create(gate_CNOT_TA, ArrayRef<Value*>(TA), "", BB)->setTailCall();
					CallInst::Create(gate_CNOT_AT, ArrayRef<Value*>(AT), "", BB)->setTailCall();
					CallInst::Create(gate_CNOT_BA, ArrayRef<Value*>(BA), "", BB)->setTailCall();

					ReturnInst::Create(getGlobalContext(), 0, BB);
				}
				std::vector<Value*>  Args(2);
				for (int i=0; i<2; i++) Args[i] = I.getArgOperand(i);
				BasicBlock::iterator ii(&I);
				ReplaceInstWithInst(I.getParent()->getInstList(), ii, CallInst::Create(RKQC_Func, ArrayRef<Value*>(Args)));
			}



			void create_a_eq_a_plus_b(CallInst& I, Function* RKQC_Func, std::string& rkqcName){
				if(!RKQC_Func){
					std::vector<Type*> ArgTypes(3);
					ArgTypes[0] = Type::getInt16Ty(getGlobalContext());
					ArgTypes[1] = Type::getInt16Ty(getGlobalContext());
					ArgTypes[2] = Type::getInt32Ty(getGlobalContext());
					FunctionType *FuncType = FunctionType::get(Type::getVoidTy(getGlobalContext()),
						ArrayRef<Type*>(ArgTypes),false);
					RKQC_Func = Function::Create(FuncType,GlobalVariable::ExternalLinkage,rkqcName,M);
					RKQC_Func->addFnAttr(Attribute::AlwaysInline);

					Function::arg_iterator arg_it = RKQC_Func->arg_begin();
					Value *A = arg_it;
					Type *A_T = arg_it->getType();
					arg_it++;
					Value *B = arg_it;
					Type *B_T = arg_it->getType();

					A->setName("target");
					B->setName("control");

					BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "", RKQC_Func, 0);
					std::string ancilla_garbage = "zg";
					std::string ancilla_zero = "zz";
					Value* ancillaG = createAncilla(ancilla_garbage,BB);	
					Type *ancillaG_T = ancillaG->getType();
					Value* ancillaZ = createAncilla(ancilla_zero,BB);	
					Type *ancillaZ_T = ancillaZ->getType();

					// Main Addition Circuit
					std::vector<Value*> AG; AG.push_back(A); AG.push_back(ancillaG);
					std::vector<Type*> AG_T; AG_T.push_back(A_T); AG_T.push_back(ancillaG_T);

					std::vector<Value*> BZ; BZ.push_back(B); BZ.push_back(ancillaZ);
					std::vector<Type*> BZ_T; BZ_T.push_back(B_T); BZ_T.push_back(ancillaZ_T);

					std::vector<Value*> AB; AB.push_back(A); AB.push_back(B);
					std::vector<Type*> AB_T; AB_T.push_back(A_T); AB_T.push_back(B_T);

					std::vector<Value*> BGA; BGA.push_back(B); BGA.push_back(ancillaG); BGA.push_back(A);
					std::vector<Type*> BGA_T; BGA_T.push_back(B_T); BGA_T.push_back(ancillaG_T); BGA_T.push_back(A_T);

					std::vector<Value*> ZG; ZG.push_back(ancillaZ); ZG.push_back(ancillaG);
					std::vector<Type*> ZG_T; ZG_T.push_back(ancillaZ_T); ZG_T.push_back(ancillaG_T);

					std::vector<Value*> ZB; ZB.push_back(ancillaZ); ZB.push_back(B);
					std::vector<Type*> ZB_T; ZB_T.push_back(ancillaZ_T); ZB_T.push_back(B_T);

					// Register Renaming Circuit
					std::vector<Value*> GA; GA.push_back(ancillaG); GA.push_back(A);
					std::vector<Type*> GA_T; GA_T.push_back(ancillaG_T); GA_T.push_back(A_T);

					Function* gate_CNOT_qbit_anc = Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(AG_T));
					Function* gate_CNOT_qbit_qbit = Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(AB_T));
					Function* gate_CNOT_anc_qbit = Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(ZB_T));
					Function* gate_CNOT_anc_anc = Intrinsic::getDeclaration(M, Intrinsic::CNOT, ArrayRef<Type*>(ZG_T));
					Function* gate_Toff = Intrinsic::getDeclaration(M, Intrinsic::Toffoli, ArrayRef<Type*>(BGA_T));

					CallInst::Create(gate_CNOT_qbit_anc, ArrayRef<Value*>(AG), "", BB)->setTailCall();
					CallInst::Create(gate_CNOT_qbit_anc, ArrayRef<Value*>(BZ), "", BB)->setTailCall();
					CallInst::Create(gate_CNOT_qbit_qbit, ArrayRef<Value*>(AB), "", BB)->setTailCall();
					CallInst::Create(gate_Toff, ArrayRef<Value*>(BGA), "", BB)->setTailCall();
					CallInst::Create(gate_CNOT_anc_anc, ArrayRef<Value*>(ZG), "", BB)->setTailCall();
					CallInst::Create(gate_CNOT_anc_qbit, ArrayRef<Value*>(ZB), "", BB)->setTailCall();

					CallInst::Create(gate_CNOT_anc_qbit, ArrayRef<Value*>(ZB), "", BB)->setTailCall();
    				CallInst::Create(gate_CNOT_qbit_anc, ArrayRef<Value*>(BZ), "", BB)->setTailCall();
					CallInst::Create(gate_CNOT_anc_qbit, ArrayRef<Value*>(ZB), "", BB)->setTailCall();

					CallInst::Create(gate_CNOT_anc_qbit, ArrayRef<Value*>(GA), "", BB)->setTailCall();
    				CallInst::Create(gate_CNOT_qbit_anc, ArrayRef<Value*>(AG), "", BB)->setTailCall();
					CallInst::Create(gate_CNOT_anc_qbit, ArrayRef<Value*>(GA), "", BB)->setTailCall();

					ReturnInst::Create(getGlobalContext(), 0, BB);
				}
				std::vector<Value*>  Args(3);
				for (int i=0; i<3; i++) Args[i] = I.getArgOperand(i);
				BasicBlock::iterator ii(&I);
				ReplaceInstWithInst(I.getParent()->getInstList(), ii,
					CallInst::Create(RKQC_Func, ArrayRef<Value*>(Args)));
			}

			void visitCallInst(CallInst &I) {
				// Determine whether this is an RKQC function 
				Function *CF = I.getCalledFunction();
				if (CF->isIntrinsic()){
					std::string name = CF->getName();
					if(name.find("rkqc")!=std::string::npos) {
					// Retrieve name of function
						std::size_t func_loc = name.find("rkqc");
						std::size_t func_name_end = name.find_first_of("(");
						std::string rkqcFuncName = name.substr(func_loc+5, func_name_end-func_loc); 
						errs() << rkqcFuncName << "\n";
						std::string rkqcName = rkqcFuncName + "_impl";
						Function* RKQC_Func = M->getFunction(rkqcName);

						if(rkqcFuncName.find("a_swap_b") != std::string::npos){
							create_a_swap_b(I, RKQC_Func, rkqcName);
						}
						else if(rkqcFuncName.find( "assign_value_of_b_to_a") != std::string::npos){
							create_assign_value_of_b_to_a(I, RKQC_Func, rkqcName);
						}
						else if(rkqcFuncName.find( "a_eq_a_plus_b") != std::string::npos){
    						create_a_eq_a_plus_b(I, RKQC_Func, rkqcName);
    					}
						else if(rkqcFuncName.find("assign_value_of_0_to_a") != std::string::npos){
							create_assign_value_of_0_to_a(I, RKQC_Func, rkqcName);
						}
						else if(rkqcFuncName.find("assign_value_of_1_to_a") != std::string::npos){
							create_assign_value_of_1_to_a(I, RKQC_Func, rkqcName);
						}
						else if(rkqcFuncName.find("toffoli") != std::string::npos){
							std::vector<Value*> Args(3);
							std::vector<Type*> Types(3);
							for (int i=0; i<3; i++){
								Args[i] = I.getArgOperand(i);
								Types[i] = I.getArgOperand(i)->getType();
							}
							BasicBlock::iterator ii(&I);
							Function* gate_Toffoli = Intrinsic::getDeclaration(M, Intrinsic::Toffoli, ArrayRef<Type*>(Types));
							ReplaceInstWithInst(I.getParent()->getInstList(), ii,
								CallInst::Create(gate_Toffoli, ArrayRef<Value*>(Args)));
						}
						else if(rkqcFuncName.find("NOT") != std::string::npos){

							std::vector<Value*> Args(1);
							std::vector<Type*> Types(1);
							for (int i=0; i<1; i++) {
								Args[i] = I.getArgOperand(i);
								Types[i] = I.getArgOperand(i)->getType();
							}
							BasicBlock::iterator ii(&I);
							Function* gate_X = Intrinsic::getDeclaration(M, Intrinsic::X, ArrayRef<Type*>(Types));
							ReplaceInstWithInst(I.getParent()->getInstList(), ii,
								CallInst::Create(gate_X, ArrayRef<Value*>(Args)));

						}
						else if(rkqcFuncName.find("cnot") != std::string::npos){
							std::vector<Value*> Args(2);
							std::vector<Type*> Types(2);
							for (int i=0; i<2; i++){
								Args[i] = I.getArgOperand(i);
								Types[i] = I.getArgOperand(i)->getType();
							}
							BasicBlock::iterator ii(&I);
							Function* gate_X = Intrinsic::getDeclaration(M, Intrinsic::X, ArrayRef<Type*>(Types));
							ReplaceInstWithInst(I.getParent()->getInstList(), ii,
								CallInst::Create(gate_X, ArrayRef<Value*>(Args)));
						}

					}// endif 'found RKQC'
				} 	
			} // visitCallInst()
		}; // struct RKQCVisitor

		virtual bool runOnModule(Module &M) {
			RKQCVisitor RV(&M);
			RV.visit(M);
			return true;
		} // runOnModule()
		
	}; // struct GenRKQC
} // namespace

char GenRKQC::ID = 0;
static RegisterPass<GenRKQC> X("GenRKQC", "RKQC Generator", false, false);

