//===----------------- QtmGateArg.cpp ----------------------===//
// This file implements the Scaffold Quantum Gate Argument Class
//
//
//
//        This file was created by Scaffold Compiler Working Group
//
//
//
//
//===----------------------------------------------------------------------===//


#include "llvm/Support/raw_ostream.h"
#include "llvm/Constants.h"

using namespace llvm;

namespace {
  class QtmGateArg{ //arguments to qgate calls

  public: 
    Value* argPtr;
    int argNum;
    bool isQbit;
    bool isCbit;
    bool isUndef;
    bool isPtr;
    int valOrIndex; //Value if not Qbit, Index if Qbit & not a Ptr
    double angle;

    QtmGateArg(): argPtr(NULL), argNum(-1), isQbit(false), isCbit(false), isUndef(false), isPtr(false), valOrIndex(-1), angle(0.0){ }

    void dump() //originally print_qgateArg
    {
      errs()<< "Printing QGate Argument:\n";
      if(argPtr) errs() << "  Name: "<<argPtr->getName()<<"\n";
      errs() << "  Arg Num: "<<argNum<<"\n"
	     << "  isUndef: "<<isUndef
	     << "  isQbit: "<<isQbit
	     << "  isCbit: "<<isCbit
	     << "  isPtr: "<<isPtr << "\n"
	     << "  Value or Index: "<<valOrIndex<<"\n";
    }                       
  }; // End of struct QtmGateArg
} // End of anonymous namespace