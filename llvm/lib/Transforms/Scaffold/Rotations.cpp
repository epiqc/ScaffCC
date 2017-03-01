// Scaffold
// This pass stops at Rz gates in the call graphs and decomposes them into
// sequences of clifford+T gates
//

#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <string>

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
#include "llvm/Support/CommandLine.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

static cl::opt<unsigned>
SqctLevels("sqct-levels", cl::init(1), cl::Hidden,
  cl::desc("The rotation decomposition precision"));


namespace {
	// We need to use a ModulePass in order to create new Functions
	struct Rotations : public ModulePass {
		static char ID;
		Rotations() : ModulePass(ID) {}

		struct RotationVisitor : public InstVisitor<RotationVisitor> {
			// All decompositions will be created as Functions in M's FunctionList
			Module *M;
			// The constructor is called once per module (in runOnModule)
			RotationVisitor(Module *module) : M(module) {}

			// private:
			std::string exec(const char* cmd) {
				FILE* pipe = popen(cmd, "r");
				if (!pipe) return "ERROR";
				char buffer[128];
				std::string result = "";
				while(!feof(pipe)) {
					if (fgets(buffer, 128, pipe) != NULL)
						result += buffer;
				}
				pclose(pipe);
				return result;
			} // exec()
			// public: 
			void visitCallInst(CallInst &I) {
        // Get Instruction iterator
        BasicBlock::iterator ii(&I);
        
				// Determine whether this is an Rz gate
				Function *CF = I.getCalledFunction();
				// Is this an intrinsic?
				if (!CF->isIntrinsic()) return;
				// If it is a rotation, what is the axis?
				std::string axis;
				switch (CF->getIntrinsicID()) {
					case Intrinsic::Rz:
						axis = std::string(" Z ");
						break;
					case Intrinsic::Rx:
						axis = std::string(" X ");
						break;
					case Intrinsic::Ry:
						axis = std::string(" Y ");
						break;
					default:
						return;
				}
				// Detemine whether we know the rotation angle
				if (!isa<ConstantFP>(I.getArgOperand(1))) {
					errs() << "Unknown rotation angle\n";
					return;
				}

				// Extract the target qubit from the CallInst
				Value *Target = I.getArgOperand(0);
				// Extract the rotation angle from the CallInst
				double Angle = cast<ConstantFP>(I.getArgOperand(1))
					->getValueAPF()
					.convertToDouble();

        // environment variables for rotation decomposition
        char *prec = getenv("PRECISION");
        char *path = getenv("ROTATIONPATH");
        if (!path) {
          errs() << "Rotation decomposer not found!\n";
          return;
        }

        // especial cases of rotations: replace with single gates
				// If the angle is 0, delete the rotation
        double pi = 3.1415926535897932384626433;
        // the rotation should be between 0 and 2pi
        while (Angle > 2*pi) Angle -= 2*pi;
        while (Angle < 0) Angle += 2*pi;
        
				if ( fabs(Angle - 0.0) < pow(10, -1.0 * atoi(prec)) ) {
          errs() << "Removing near-zero rotation.\n";
					I.eraseFromParent();
					return;
				}
        if ( fabs(Angle - pi) < pow(10, -1.0 * atoi(prec)) ) {// pi rotation
          errs() << "Synthesizing pi rotation (Z gate).\n";          
		  		ReplaceInstWithInst(I.getParent()->getInstList(), ii,
			  		  CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::Z, makeArrayRef(Target->getType())), 
                ArrayRef<Value*>(Target)));
          return;
        }
        if ( fabs(Angle - pi/2.0) < pow(10, -1.0 * atoi(prec)) ) {// pi/2 rotation
          errs() << "Synthesizing pi/2 rotation (S gate).\n";                    
		  		ReplaceInstWithInst(I.getParent()->getInstList(), ii,
			  		  CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::S, makeArrayRef(Target->getType())), 
                ArrayRef<Value*>(Target)));
          return;
        }
        if ( fabs(Angle - pi/4.0) < pow(10, -1.0 * atoi(prec)) ) {// pi/4 rotation
          errs() << "Synthesizing pi/4 rotation (T gate).\n";                              
		  		ReplaceInstWithInst(I.getParent()->getInstList(), ii,
			  		  CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::T, makeArrayRef(Target->getType())), 
                ArrayRef<Value*>(Target)));
          return;
        }

				// Create a unique function name (for lookup later)
				std::string buf; raw_string_ostream ss(buf);
				ss << "DecomposeRotation_" << Angle;
				std::string FuncName = ss.str();
				// Sanitize strings
				for (std::string::iterator iter = FuncName.begin(); iter < FuncName.end(); iter++) {
					switch (*iter) {
						case '-': FuncName.replace(iter,iter+1,"n"); break;
						case '+': FuncName.erase(iter); iter--; break;
						case '.': FuncName.replace(iter,iter+1,"_"); break;
						case '"': FuncName.erase(iter); iter--; break;
					}
				}
				// Create a FunctionType object with 'void' return type and one 'qbit'
				// parameter
				FunctionType *FuncType = FunctionType::get(
					Type::getVoidTy(getGlobalContext()),
					ArrayRef<Type*>(Type::getInt16Ty(getGlobalContext())),
					false);
				// Lookup the Function in the module
				Function *DR = M->getFunction(FuncName);
				if (!DR) {
					// Create the new function
					DR = Function::Create(FuncType, GlobalVariable::ExternalLinkage,
						FuncName, M);

					Function::arg_iterator args = DR->arg_begin(); //set name of variable
					Value* qArg = args;
					qArg->setName("q");

					// Create a BasicBlock and insert it at the end of the Function
					BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "", DR, 0);
					// Populate the BasicBlock
					// Build rotation decomposition command
					std::string buf; std::ostringstream ss2;        
          if (std::string(path).find("gridsynth") != std::string::npos) {
    			  ss2 << path << " \"(" << std::fixed << Angle << ")\"" << " -d " << prec;          
          }
          else if (std::string(path).find("sqct") != std::string::npos) {
            ss2 << path << " " << Angle << axis << SqctLevels;
          }
          else {
            errs() << "Invalid rotation decomposer!\n";
            return;
          }
          
          buf = ss2.str();
          raw_string_ostream ss3(buf);          
          std::string circuit;
					// Capture result          
					//errs() << "Calling '" << ss3.str() << "'\n";
          errs() << "Decomposing " << Angle << " radian rotation.\n";
					circuit = exec(ss3.str().c_str());

					// For each gate in decomposition:
          // (the decomposed string is given in the reverse order that ops must be applied)
					for (int i=circuit.length()-2; i>=0; i--) {
						Function *gate = NULL;
						switch(circuit[i]) {
							case 'T':
								gate = Intrinsic::getDeclaration(M, Intrinsic::T, makeArrayRef(Target->getType()));
								break;
							case 't':
								gate = Intrinsic::getDeclaration(M, Intrinsic::Tdag, makeArrayRef(Target->getType()));
								break;
							case 'S':
								gate = Intrinsic::getDeclaration(M, Intrinsic::S, makeArrayRef(Target->getType()));
								break;
							case 's':
								gate = Intrinsic::getDeclaration(M, Intrinsic::Sdag, makeArrayRef(Target->getType()));
								break;
							case 'H':
								gate = Intrinsic::getDeclaration(M, Intrinsic::H, makeArrayRef(Target->getType()));
								break;
							case 'X':
								gate = Intrinsic::getDeclaration(M, Intrinsic::X, makeArrayRef(Target->getType()));
								break;
							case 'Y':
								gate = Intrinsic::getDeclaration(M, Intrinsic::Y, makeArrayRef(Target->getType()));
								break;
							case 'Z':
								gate = Intrinsic::getDeclaration(M, Intrinsic::Z, makeArrayRef(Target->getType()));
								break;
							default:
								continue;
						}
						CallInst *newCallInst;
						newCallInst = CallInst::Create(gate, ArrayRef<Value*>(DR->arg_begin()),
								// Insert at front
								 //"", BB->front());
								// Insert at end
								"", BB);
						//newCallInst->setTailCall();
					}
					ReturnInst::Create(getGlobalContext(), 0, BB);
				} // endif 'decomposition not found'
				// Replace the old Rz call with the new call to Decomposed_Rotation
				ReplaceInstWithInst(I.getParent()->getInstList(), ii,
					CallInst::Create(DR, ArrayRef<Value*>(Target)));
				// } // endif 'found Rz'
			} // visitCallInst()

		}; // struct RotationVisitor

		virtual bool runOnModule(Module &M) {
			RotationVisitor RV(&M);
			RV.visit(M);

			return true;
		} // runOnModule()
		
	}; // struct Rotations
} // namespace

char Rotations::ID = 0;
static RegisterPass<Rotations> X("Rotations", "Rotation Decomposition", false, false);


