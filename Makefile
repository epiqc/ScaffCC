SVN=/usr/bin/svn

LLVM_RELEASE="RELEASE_31/final"
CLANG_RELEASE="RELEASE_31/final"

LLVM_URL="http://llvm.org/svn/llvm-project/llvm"
CLANG_URL="http://llvm.org/svn/llvm-project/cfe"

#CLANG=../build/Debug+Asserts/bin/clang++
CLANG=g++

# LDFLAGS is modified from `llvm-config --libs`
# -- certain libs were not found for some reason or another...
LDFLAGS=-lclangFrontend \
		-lclangDriver \
		-lclangSerialization \
		-lclangParse \
		-lclangSema \
		-lclangAnalysis \
		-lclangRewrite \
		-lclangEdit \
		-lclangAST \
		-lclangLex \
		-lclangBasic \
		-lLLVMAsmParser \
		-lLLVMInstrumentation \
		-lLLVMLinker \
		-lLLVMArchive \
		-lLLVMBitReader \
		-lLLVMDebugInfo \
		-lLLVMJIT \
		-lLLVMipo \
		-lLLVMVectorize \
		-lLLVMBitWriter \
		-lLLVMTableGen \
		-lLLVMHexagonCodeGen \
		-lLLVMHexagonDesc \
		-lLLVMHexagonInfo \
		-lLLVMHexagonAsmPrinter \
		-lLLVMMBlazeDisassembler \
		-lLLVMMBlazeAsmParser \
		-lLLVMMBlazeCodeGen \
		-lLLVMMBlazeDesc \
		-lLLVMMBlazeInfo \
		-lLLVMMBlazeAsmPrinter \
		-lLLVMCppBackendCodeGen \
		-lLLVMCppBackendInfo \
		-lLLVMMSP430CodeGen \
		-lLLVMMSP430Desc \
		-lLLVMMSP430Info \
		-lLLVMMSP430AsmPrinter \
		-lLLVMXCoreCodeGen \
		-lLLVMXCoreDesc \
		-lLLVMXCoreInfo \
		-lLLVMCellSPUCodeGen \
		-lLLVMCellSPUDesc \
		-lLLVMCellSPUInfo \
		-lLLVMMipsCodeGen \
		-lLLVMMipsAsmParser \
		-lLLVMMipsDisassembler \
		-lLLVMMipsDesc \
		-lLLVMMipsInfo \
		-lLLVMMipsAsmPrinter \
		-lLLVMARMDisassembler \
		-lLLVMARMAsmParser \
		-lLLVMARMCodeGen \
		-lLLVMARMDesc \
		-lLLVMARMInfo \
		-lLLVMARMAsmPrinter \
		-lLLVMPowerPCCodeGen \
		-lLLVMPowerPCDesc \
		-lLLVMPowerPCInfo \
		-lLLVMPowerPCAsmPrinter \
		-lLLVMSparcCodeGen \
		-lLLVMSparcDesc \
		-lLLVMSparcInfo \
		-lLLVMX86AsmParser \
		-lLLVMX86Disassembler \
		-lLLVMX86CodeGen \
		-lLLVMSelectionDAG \
		-lLLVMAsmPrinter \
		-lLLVMX86Desc \
		-lLLVMX86Info \
		-lLLVMX86AsmPrinter \
		-lLLVMX86Utils \
		-lgtest_main \
		-lgtest \
		-lLLVMMCDisassembler \
		-lLLVMMCParser \
		-lLLVMInterpreter \
		-lLLVMCodeGen \
		-lLLVMScalarOpts \
		-lLLVMInstCombine \
		-lLLVMTransformUtils \
		-lLLVMipa \
		-lLLVMAnalysis \
		-lLLVMMCJIT \
		-lLLVMRuntimeDyld \
		-lLLVMExecutionEngine \
		-lLLVMTarget \
		-lLLVMMC \
		-lLLVMObject \
		-lLLVMCore \
		-lLLVMSupport

CLANGFLAGS=`../build/Debug+Asserts/bin/llvm-config --cxxflags --ldflags`

CFLAGS=-L ../build/Debug+Asserts/lib \
		-I`../build/Debug+Asserts/bin/llvm-config --includedir` \
		-I ../llvm/tools/clang/include \
		-I ../build/include \
		-I ../build/tools/clang/include

SCAFFOLD=scaffold

UNAME_S := $(shell uname -s)

all: Clang

Clang: llvm build
	@cd llvm/tools && /bin/rm -f clang && /bin/ln -s ../../clang;
	@cd clang && /bin/rm -f build && /bin/ln -s ../build;
	@if [ -z $(USE_GCC) ]; then \
		if [ "$(UNAME_S)" = "Darwin" ]; then \
	        cd build && ../llvm/configure --disable-debug-symbols && make ENABLE_LIBCPP=1; \
	    else \
	        cd build && ../llvm/configure --disable-debug-symbols && make ; \
		fi \
	else \
		mkdir -p build && cd build && ../llvm/configure --disable-debug-symbols CC=gcc CXX=g++ && make ; fi
	@if [ -z `echo ${PATH} | grep ${PWD}/Debug+Asserts/bin` ]; then \
		export PATH=${PATH}:${PWD}/Debug+Asserts/bin; \
	else true; fi
	@if [ -z `echo ${PATH} | grep ${PWD}/Debug+Asserts/bin` ]; then \
		export PATH=${PATH}:${PWD}/Debug+Asserts/bin; \
	else true; fi

build: llvm
	@mkdir -p build

llvm:
	@if [ ! -e $(SVN) ]; then \
		echo "Please install Subversion: 'sudo apt-get install subversion'"; exit 2; \
	else true; fi
	@$(SVN) checkout --force $(LLVM_URL)/tags/$(LLVM_RELEASE) llvm;

Scaffold:
	@cd scaffold && make;

clean:
	#cd scaffold && make clean
	@if [ -d build ]; then cd build && make clean; fi

.PHONY: clean Scaffold Clang
