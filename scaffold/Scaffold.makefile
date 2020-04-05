SHELL=/bin/bash
PERL=/usr/bin/perl
PYTHON=/usr/bin/python

ROOT=".."
DIRNAME=""
FILENAME=""
FILE=""
CFILE=""
TOFF=0
CTQG=0
ROTATIONS=0

#BUILD=ROOT
BUILD=$(ROOT)/build

SQCTPATH=$(ROOT)/Rotations/sqct/rotZ
GRIDSYNTHPATH=$(ROOT)/Rotations/gridsynth/gridsynth
ROTATIONPATH=$(GRIDSYNTHPATH) # select rotation decomposition tool
SCRIPTSPATH=$(ROOT)/scripts/ # select path to scripts
PRECISION=""
OPTIMIZE=0

CC=$(BUILD)/bin/clang
OPT=$(BUILD)/bin/opt

CC_FLAGS=-c -emit-llvm -Xclang -disable-O0-optnone -I/usr/include -I/usr/include/x86_64-linux-gnu -I/usr/lib/gcc/x86_64-linux-gnu/4.8/include -I$(DIRNAME)

OSX_FLAGS=""

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
SCAFFOLD_LIB=$(ROOT)/build/lib/LLVMScaffold.so
endif
ifeq ($(UNAME_S),Darwin)
SCAFFOLD_LIB=$(ROOT)/build/lib/LLVMScaffold.dylib
OSX_FLAGS=-isysroot $(shell xcrun --sdk macosx --show-sdk-path)
endif


################################
# Resource Count Estimation
################################
resources: $(FILE).resources

################################
# Flat QASM generation
################################
flat: $(FILE).qasmf

################################
# OpenQASM generation
################################
openqasm: $(FILE).qasm

################################
# QASM generation
################################
qasm: $(FILE).qasmh

################################
# QASM optimization
################################
optimize: $(FILE)_optimized.qasmf

################################
# QX Simulator Generation
################################
qc: $(FILE).qc


.PHONY: res_count qasm flat optimize qc

################################
# Intermediate targets
################################
# Pre-process CTQG
$(FILE)_merged.scaffold: $(FILENAME)
	@if [ $(CTQG) -eq 1 ]; then \
		echo "[Scaffold.makefile] Extracting CTQG from Scaffold ..."; \
		$(PERL) $(ROOT)/ctqg/trans/pre_process.pl $(FILENAME); \
		echo "[Scaffold.makefile] Compiling CTQG ..."; \
		$(ROOT)/ctqg/CTQG/ctqg $(CFILE).ctqg; \
		echo "[Scaffold.makefile] Merging CTQG output back into Scaffold ..."; \
	$(PERL) $(ROOT)/ctqg/trans/trans.pl $(CFILE).qasm > trans.qasm; \
	mv trans.qasm $(CFILE).qasm; \
		$(PERL) $(ROOT)/ctqg/trans/merge.pl $(CFILE).qasm; \
	else \
		cp $(FILENAME) $(FILE)_merged.scaffold; \
	fi

$(FILE)_merged_reverse_insert.scaffold: $(FILE)_merged.scaffold
	@echo "[Scaffold.makefile] Inserting reverse function signatures"
	python $(ROOT)/scaffold/insert_reverse.py $(FILE)_merged.scaffold
	@cat $(FILE)_merged.scaffold >> $(FILE)_merged_reverse_insert.scaffold

# Compile Scaffold to LLVM bytecode
$(FILE).ll: $(FILE)_merged_reverse_insert.scaffold
	@echo "[Scaffold.makefile] Compiling $(FILE)_merged_reverse_insert.scaffold ..."
	@$(CC) $(FILE)_merged_reverse_insert.scaffold -ffast-math $(CC_FLAGS) $(OSX_FLAGS) -o $(FILE).ll

$(FILE)1.ll: $(FILE).ll
	@echo "[Scaffold.makefile] Transforming cbits ..."
	@$(OPT) -S -load $(SCAFFOLD_LIB) -xform-cbit-stores $(FILE).ll -o $(FILE)1.ll > /dev/null

# Perform normal C++ optimization routines
$(FILE)4.ll: $(FILE)1.ll
	@if [ $(COPTIMIZATION) -eq 1 ]; then \
		echo "[Scaffold.makefile] O1 optimizations ..."; \
		$(OPT) -S $(FILE)1.ll -tbaa -targetlibinfo -basicaa -o $(FILE)1a.ll > /dev/null; \
		$(OPT) -S $(FILE)1a.ll -simplifycfg -domtree -o $(FILE)1b.ll > /dev/null; \
		$(OPT) -S $(FILE)1b.ll -early-cse -lower-expect -o $(FILE)2.ll > /dev/null; \
		$(OPT) -S $(FILE)2.ll -targetlibinfo -tbaa -basicaa -globalopt -ipsccp -o $(FILE)3.ll > /dev/null; \
		$(OPT) -S $(FILE)3.ll -instcombine -simplifycfg -basiccg -prune-eh -always-inline -functionattrs -domtree -early-cse -lazy-value-info -jump-threading -correlated-propagation -simplifycfg -instcombine -tailcallelim -simplifycfg -reassociate -domtree -loops -loop-simplify -lcssa -loop-rotate -licm -lcssa -loop-unswitch -instcombine -scalar-evolution -loop-simplify -lcssa -iv-users -indvars -loop-idiom -loop-deletion -loop-unroll -unroll-threshold=100000000 -memdep -memcpyopt -sccp -instcombine -lazy-value-info -jump-threading -correlated-propagation -domtree -memdep -dse -adce -simplifycfg -instcombine -strip-dead-prototypes -verify-each -domtree -verify -o $(FILE)4.ll > /dev/null; \
	else \
		cp $(FILE)1.ll $(FILE)4.ll; \
	fi

# Perform loop unrolling until completely unrolled, then remove dead code
#
# Gory details:
# Unroll until the 'diff' of *4 and *6tmp is NULL (i.e., no differences)
# *4 is used to create *6tmp; *6tmp is copied over *4 for each iteration to retry 'diff'
# As a weird consquence, we need to first rename *4 to *6tmp and create an empty *4 to diff
# This screws with the intermediate results, but they are mostly for debugging anyway
$(FILE)6.ll: $(FILE)4.ll
	@UCNT=0; \
	mv $(FILE)4.ll $(FILE)6tmp.ll; \
	touch $(FILE)4.ll; \
	while [ -n "$$(diff -q $(FILE)4.ll $(FILE)6tmp.ll)" ]; do \
		UCNT=$$(expr $$UCNT + 1); \
		echo "[Scaffold.makefile] Unrolling Loops ($$UCNT) ..."; \
		cp $(FILE)6tmp.ll $(FILE)4.ll; \
		$(OPT) -S $(FILE)4.ll -mem2reg -loops -loop-simplify -loop-rotate -lcssa -loop-unroll -unroll-threshold=100000000 -sccp -simplifycfg -o $(FILE)5.ll > /dev/null && \
		echo "[Scaffold.makefile] Cloning Functions ($$UCNT) ..." && \
		$(OPT) -S -load $(SCAFFOLD_LIB) -FunctionClone -sccp $(FILE)5.ll -o $(FILE)5a.ll > /dev/null && \
		echo "[Scaffold.makefile] Dead Argument Elimination ($$UCNT) ..." && \
		$(OPT) -S -deadargelim $(FILE)5a.ll -o $(FILE)6tmp.ll > /dev/null; \
	done && \
	$(OPT) -S $(FILE)6tmp.ll -internalize-public-api-list=main -internalize -globaldce -adce -o $(FILE)6.ll > /dev/null  
	

# Perform Rotation decomposition if requested and rotation decomp tool is built
$(FILE)7.ll: $(FILE)6.ll
	@if [ ! -e $(ROTATIONPATH) ]; then \
		echo "[Scaffold.makefile] Rotation tool not built, skipping rotation decomposition ..."; \
		cp $(FILE)6.ll $(FILE)7.ll; \
	elif [ $(ROTATIONS) -eq 1 ]; then \
		echo "[Scaffold.makefile] Decomposing Rotations ..." && \
		export ROTATIONPATH=$(ROTATIONPATH) && \
	export PRECISION=$(PRECISION); \
		$(OPT) -S -load $(SCAFFOLD_LIB) -Rotations $(FILE)6.ll -o $(FILE)7.ll > /dev/null; \
	else \
		cp $(FILE)6.ll $(FILE)7.ll; \
	fi

# Remove any code that is useless after optimizations
$(FILE)8.ll: $(FILE)7.ll
	@echo "[Scaffold.makefile] Internalizing and Removing Unused Functions ..."
	@$(OPT) -S $(FILE)7.ll -internalize-public-api-list=main -internalize -globaldce -deadargelim -o $(FILE)8.ll > /dev/null

# Compile RKQC 
$(FILE)9.ll: $(FILE)8.ll
	@if [ $(RKQC) -eq 1 ]; then \
		echo "[Scaffold.makefile] Compiling RKQC Functions ..."; \
		$(OPT) -S -load $(SCAFFOLD_LIB) -GenRKQC $(FILE)8.ll -o $(FILE)9.ll > /dev/null 2> $(FILE).errs; \
	else \
		mv $(FILE)8.ll $(FILE)9.ll; \
	fi

# Perform Toffoli decomposition if TOFF is 1
$(FILE)11.ll: $(FILE)9.ll
	@if [ $(TOFF) -eq 1 ]; then \
	echo "[Scaffold.makefile] Toffoli Decomposition ..."; \
		$(OPT) -S -load $(SCAFFOLD_LIB) -ToffoliReplace $(FILE)9.ll -o $(FILE)11.ll > /dev/null; \
	else \
		cp $(FILE)9.ll $(FILE)11.ll; \
	fi

# Insert reverse functions if REVERSE is 1
$(FILE)12.ll: $(FILE)11.ll
	@echo "[Scaffold.makefile] Inserting Reverse Functions..."
	@$(OPT) -S -load $(SCAFFOLD_LIB) -FunctionReverse $(FILE)11.ll -o $(FILE)12.ll > /dev/null

# Generate resource counts from final LLVM output
$(FILE).resources: $(FILE)12.ll
	@echo "[Scaffold.makefile] Generating resource count ..."	
	@$(OPT) -load $(SCAFFOLD_LIB) -ResourceCount $(FILE)12.ll 2> $(FILE).resources > /dev/null
	@echo "[Scaffold.makefile] Resources written to $(FILE).resources ..."  

# Generate hierarchical QASM
$(FILE).qasmh: $(FILE)12.ll
	@echo "[Scaffold.makefile] Generating hierarchical QASM ..."  
	@$(OPT) -load $(SCAFFOLD_LIB) -gen-qasm $(FILE)12.ll 2> $(FILE).qasmh > /dev/null
	@echo "[Scaffold.makefile] Hierarchical QASM written to $(FILE).qasmh ..."  

# Translate hierarchical QASM back to C++ for flattening
$(FILE)_qasm.scaffold: $(FILE).qasmh
	@echo "[Scaffold.makefile] Generating flattened QASM ..."
	@$(PYTHON) $(ROOT)/scaffold/flatten-qasm.py $(FILE).qasmh

# Compile C++
$(FILE)_qasm: $(FILE)_qasm.scaffold
	@$(CC) $(FILE)_qasm.scaffold -lm -o $(FILE)_qasm

# Generate flattened QASM 
$(FILE).qasmf: $(FILE)12.ll
	@echo "[Scaffold.makefile] Flattening modules ..." 
	@$(OPT) -S -load $(SCAFFOLD_LIB) -FlattenModule -all 1 $(FILE)12.ll -o $(FILE)12.inlined.ll 2> /dev/null
	@$(OPT) -load $(SCAFFOLD_LIB) -gen-qasm $(FILE)12.inlined.ll 2> $(FILE).qasmh > /dev/null
	@$(PYTHON) $(ROOT)/scaffold/flatten-qasm.py $(FILE).qasmh
	@$(CC) $(OSX_FLAGS) $(FILE)_qasm.scaffold -o $(FILE)_qasm
	@./$(FILE)_qasm > $(FILE).tmp
	@cat fdecl.out $(FILE).tmp > $(FILE).qasmf
	@echo "[Scaffold.makefile] Flat QASM written to $(FILE).qasmf ..."	

# Generate OpenQASM
$(FILE).qasm: $(FILE)12.ll
	@echo "[Scaffold.makefile] Flattening modules ..."
	@$(OPT) -S -load $(SCAFFOLD_LIB) -FlattenModule -all 1 $(FILE)12.ll -o $(FILE)12.inlined.ll 2> /dev/null
	@echo "[Scaffold.makefile] Generating OpenQASM ..."
	@$(OPT) -load $(SCAFFOLD_LIB) -gen-openqasm $(FILE)12.inlined.ll 2> $(FILE).qasm > /dev/null
	@echo "[Scaffold.makefile] OpenQASM written to $(FILE).qasm ..."

# Generate optimized QASM
$(FILE)_optimized.qasmf: $(FILE)12.ll
	@if [ $(OPTIMIZE) -eq 1 ]; then \
		echo "[Scaffold.makefile] Optimizing circuit ..."; \
		$(OPT) -S -load $(SCAFFOLD_LIB) -FlattenModule -all 1 $(FILE)12.ll -o $(FILE)12.inlined.ll 2> /dev/null; \
		$(OPT) -load $(SCAFFOLD_LIB) -Optimize $(FILE)12.inlined.ll 2> $(FILE)_optimized.qasmf > /dev/null; \
		echo "[Scaffold.makefile] Optimized circuit written to $(FILE)_optimized.qasmf ..."; \
	fi

# Generate simulation input
$(FILE).qc: $(FILE).qasmf
	@echo "[Scaffold.makefile] Transforming flat QASM to QX Simulator input ..."
	@$(SHELL) $(ROOT)/scripts/qasmf2qc.sh $(FILE).qasmf
	@echo "[Scaffold.makefile] QX Simulator input written to $(FILE).qc ..." 

# purge cleans temp files
purge:
	@rm -f $(FILE)_merged.scaffold $(FILE)_merged_reverse_insert.scaffold $(FILE)_no.scaffold $(FILE).ll $(FILE)1.ll $(FILE)1a.ll $(FILE)1b.ll $(FILE)2.ll $(FILE)3.ll $(FILE)4.ll $(FILE)5.ll $(FILE)5a.ll $(FILE)6.ll $(FILE)6tmp.ll $(FILE)7.ll $(FILE)8.ll $(FILE)9.ll $(FILE)10.ll $(FILE)11.ll $(FILE)12.ll $(FILE)12.inlined.ll $(FILE)tmp.ll $(FILE)_qasm $(FILE)_qasm.scaffold fdecl.out $(CFILE).ctqg $(CFILE).c $(CFILE).signals $(FILE).tmp sim_$(CFILE) $(FILE).*.qasm $(FILE)_args.scaffold

# clean removes all completed files
clean: purge
	@rm -f $(FILE).resources $(FILE).qasmh $(FILE).qasmf $(FILE).qasm $(FILE)_optimized.qasmf $(FILE).qc

.PHONY: clean purge
