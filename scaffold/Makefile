CXX := g++
LLVMCOMPONENTS := cppbackend
RTTIFLAG := -fno-rtti
LLVMCONFIG := ../build/Debug+Asserts/bin/llvm-config

CXXFLAGS := -I../clang/include \
	-I../build/tools/clang/include \
	-I../llvm/tools/clang/include \
	$(shell $(LLVMCONFIG) --cxxflags) $(RTTIFLAG)
LLVMLDFLAGS := $(shell $(LLVMCONFIG) --ldflags --libs $(LLVMCOMPONENTS))

SOURCES = scaffold.cpp 

OBJECTS = $(SOURCES:.cpp=.o)
EXES = $(OBJECTS:.o=)
CLANGLIBS = \
	-lclangTooling -lclangFrontendTool -lclangFrontend -lclangDriver \
	-lclangSerialization -lclangCodeGen -lclangParse -lclangSema \
	-lclangStaticAnalyzerFrontend -lclangStaticAnalyzerCheckers \
	-lclangStaticAnalyzerCore -lclangAnalysis -lclangARCMigrate \
	-lclangEdit -lclangAST -lclangLex -lclangBasic -lLLVMAsmParser \
	-lLLVMInstrumentation -lLLVMLinker -lLLVMArchive -lLLVMBitReader \
	-lLLVMDebugInfo -lLLVMJIT -lLLVMipo -lLLVMVectorize -lLLVMBitWriter \
	-lLLVMTableGen -lLLVMHexagonCodeGen -lLLVMHexagonDesc \
	-lLLVMHexagonAsmPrinter -lLLVMHexagonInfo -lLLVMPTXCodeGen \
	-lLLVMPTXDesc -lLLVMPTXInfo -lLLVMPTXAsmPrinter -lLLVMMBlazeCodeGen \
	-lLLVMMBlazeAsmParser -lLLVMMBlazeDisassembler -lLLVMMBlazeDesc \
	-lLLVMMBlazeAsmPrinter -lLLVMMBlazeInfo -lLLVMCppBackendCodeGen \
	-lLLVMCppBackendInfo -lLLVMMSP430CodeGen -lLLVMMSP430Desc \
	-lLLVMMSP430Info -lLLVMMSP430AsmPrinter -lLLVMXCoreCodeGen \
	-lLLVMXCoreDesc -lLLVMXCoreInfo -lLLVMCellSPUCodeGen -lLLVMCellSPUDesc \
	-lLLVMCellSPUInfo -lLLVMMipsDisassembler -lLLVMMipsAsmParser \
	-lLLVMMipsCodeGen -lLLVMMipsDesc -lLLVMMipsInfo -lLLVMMipsAsmPrinter \
	-lLLVMARMDisassembler -lLLVMARMAsmParser -lLLVMARMCodeGen \
	-lLLVMARMDesc -lLLVMARMAsmPrinter -lLLVMARMInfo -lLLVMPowerPCCodeGen \
	-lLLVMPowerPCDesc -lLLVMPowerPCAsmPrinter -lLLVMPowerPCInfo \
	-lLLVMSparcCodeGen -lLLVMSparcDesc -lLLVMSparcInfo \
	-lLLVMX86Disassembler -lLLVMX86CodeGen -lLLVMSelectionDAG \
	-lLLVMAsmPrinter -lLLVMX86AsmParser -lLLVMX86Desc -lLLVMX86AsmPrinter \
	-lLLVMX86Utils -lLLVMX86Info -lgtest_main -lgtest \
	-lLLVMMCDisassembler -lLLVMMCParser -lLLVMInterpreter -lLLVMCodeGen \
	-lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa \
	-lLLVMAnalysis -lLLVMMCJIT -lLLVMRuntimeDyld -lLLVMExecutionEngine \
	-lLLVMTarget -lLLVMMC -lLLVMObject -lLLVMCore -lLLVMSupport

all: $(OBJECTS) $(EXES)

%: %.o
	$(CXX) -o $@ $< $(CLANGLIBS) $(LLVMLDFLAGS)

clean:
	-rm -f $(EXES) $(OBJECTS) *~
