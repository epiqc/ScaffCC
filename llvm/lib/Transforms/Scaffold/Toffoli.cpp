//===------------------------- Toffoli.cpp --------------------------------===//
//
// This pass implements a fault tolerant implementation of Toffoli gates.
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <cstdio>
#include <cstdlib>

#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/IR/CallSite.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/InstVisitor.h"
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
        std::vector<Type *> ArgTypes(3);
        for (int i = 0; i < 3; i++) {
          ArgTypes[i] = I.getArgOperand(i)->getType();
          if (ArgTypes[i] == Type::getInt16Ty(CF->getContext()))
            implName.append("_Q");
          else
            implName.append("_A");
        }

        Function *ToffoliImpl = M->getFunction(implName);
        if (!ToffoliImpl) {
          FunctionType *FuncType =
              FunctionType::get(Type::getVoidTy(CF->getContext()),
                                ArrayRef<Type *>(ArgTypes), false);
          ToffoliImpl = Function::Create(
              FuncType, GlobalVariable::ExternalLinkage, implName, M);
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

          // Create BasicBlock
          BasicBlock *BB =
              BasicBlock::Create(CF->getContext(), "", ToffoliImpl, 0);

          // -- Improved T-depth Toffoli gate (Amy et al.)
          // CNOT shorthands

          std::vector<Value *> C1C2;
          C1C2.push_back(Control2);
          C1C2.push_back(Control1);
          std::vector<Type *> C1C2ty;
          C1C2ty.push_back(ArgTypes[2]);
          C1C2ty.push_back(ArgTypes[1]);

          std::vector<Value *> C2T;
          C2T.push_back(Target);
          C2T.push_back(Control2);
          std::vector<Type *> C2Tty;
          C2Tty.push_back(ArgTypes[0]);
          C2Tty.push_back(ArgTypes[2]);

          std::vector<Value *> TC1;
          TC1.push_back(Control1);
          TC1.push_back(Target);
          std::vector<Type *> TC1ty;
          TC1ty.push_back(ArgTypes[1]);
          TC1ty.push_back(ArgTypes[0]);

          std::vector<Value *> C2C1;
          C2C1.push_back(Control1);
          C2C1.push_back(Control2);
          std::vector<Type *> C2C1ty;
          C2C1ty.push_back(ArgTypes[1]);
          C2C1ty.push_back(ArgTypes[2]);

          // Construct circuit

          // t is Tdag, T is T
          CallInst::Create(Intrinsic::getDeclaration(
                               M, Intrinsic::H, ArrayRef<Type *>(ArgTypes[2])),
                           ArrayRef<Value *>(Target), "", BB)
              ->setTailCall();

          CallInst::Create(
              Intrinsic::getDeclaration(M, Intrinsic::Tdag,
                                        ArrayRef<Type *>(ArgTypes[1])),
              ArrayRef<Value *>(Control1), "", BB)
              ->setTailCall();

          CallInst::Create(Intrinsic::getDeclaration(
                               M, Intrinsic::T, ArrayRef<Type *>(ArgTypes[2])),
                           ArrayRef<Value *>(Control2), "", BB)
              ->setTailCall();

          CallInst::Create(Intrinsic::getDeclaration(
                               M, Intrinsic::T, ArrayRef<Type *>(ArgTypes[0])),
                           ArrayRef<Value *>(Target), "", BB)
              ->setTailCall();

          CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::CNOT,
                                                     ArrayRef<Type *>(C1C2ty)),
                           ArrayRef<Value *>(C1C2), "", BB)
              ->setTailCall();

          CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::CNOT,
                                                     ArrayRef<Type *>(TC1ty)),
                           ArrayRef<Value *>(TC1), "", BB)
              ->setTailCall();

          CallInst::Create(
              Intrinsic::getDeclaration(M, Intrinsic::Tdag,
                                        ArrayRef<Type *>(ArgTypes[1])),
              ArrayRef<Value *>(Control1), "", BB)
              ->setTailCall();

          CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::CNOT,
                                                     ArrayRef<Type *>(C2Tty)),
                           ArrayRef<Value *>(C2T), "", BB)
              ->setTailCall();

          CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::CNOT,
                                                     ArrayRef<Type *>(C2C1ty)),
                           ArrayRef<Value *>(C2C1), "", BB)
              ->setTailCall();

          CallInst::Create(
              Intrinsic::getDeclaration(M, Intrinsic::Tdag,
                                        ArrayRef<Type *>(ArgTypes[1])),
              ArrayRef<Value *>(Control1), "", BB)
              ->setTailCall();

          CallInst::Create(
              Intrinsic::getDeclaration(M, Intrinsic::Tdag,
                                        ArrayRef<Type *>(ArgTypes[2])),
              ArrayRef<Value *>(Control2), "", BB)
              ->setTailCall();

          CallInst::Create(Intrinsic::getDeclaration(
                               M, Intrinsic::T, ArrayRef<Type *>(ArgTypes[0])),
                           ArrayRef<Value *>(Target), "", BB)
              ->setTailCall();

          CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::CNOT,
                                                     ArrayRef<Type *>(TC1ty)),
                           ArrayRef<Value *>(TC1), "", BB)
              ->setTailCall();

          CallInst::Create(Intrinsic::getDeclaration(
                               M, Intrinsic::S, ArrayRef<Type *>(ArgTypes[1])),
                           ArrayRef<Value *>(Control1), "", BB)
              ->setTailCall();

          CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::CNOT,
                                                     ArrayRef<Type *>(C2Tty)),
                           ArrayRef<Value *>(C2T), "", BB)
              ->setTailCall();

          CallInst::Create(Intrinsic::getDeclaration(M, Intrinsic::CNOT,
                                                     ArrayRef<Type *>(C1C2ty)),
                           ArrayRef<Value *>(C1C2), "", BB)
              ->setTailCall();

          CallInst::Create(Intrinsic::getDeclaration(
                               M, Intrinsic::H, ArrayRef<Type *>(ArgTypes[0])),
                           ArrayRef<Value *>(Target), "", BB)
              ->setTailCall();
          // end TODO

          ReturnInst::Create(CF->getContext(), 0, BB);
        } // end !ToffoliImpl
        std::vector<Value *> Args(3);
        for (int i = 0; i < 3; i++)
          Args[i] = I.getArgOperand(i);
        BasicBlock::iterator ii(&I);
        ReplaceInstWithInst(
            I.getParent()->getInstList(), ii,
            CallInst::Create(ToffoliImpl, ArrayRef<Value *>(Args)));

      } // endif 'found Toffoli'
    }   // visitCallInst()
  };    // struct ToffoliVisitor

  virtual bool runOnModule(Module &M) {
    ToffoliVisitor TV(&M);
    TV.visit(M);

    return true;
  } // runOnModule()

}; // struct ToffoliReplace
} // namespace

char ToffoliReplace::ID = 0;
static RegisterPass<ToffoliReplace> X("ToffoliReplace", "Toffoli Replacer",
                                      false, false);
