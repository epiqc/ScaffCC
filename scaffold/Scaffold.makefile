SHELL=/bin/bash
PERL=/usr/bin/perl
PYTHON=/usr/bin/python

ROOT=".."
FILENAME=""
FILE=""
CFILE=""

BUILD=$(ROOT)/build/Release+Asserts
SQCTPATH=$(ROOT)/Rotations/sqct

CC=$(BUILD)/bin/clang
OPT=$(BUILD)/bin/opt

CC_FLAGS=-c -cc1 -emit-llvm -I$(BUILD)/lib/clang/3.1/include/ -I/usr/include/ -I/usr/local/include

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
SCAFFOLD_LIB=$(ROOT)/build/Release+Asserts/lib/Scaffold.so
endif
ifeq ($(UNAME_S),Darwin)
SCAFFOLD_LIB=$(ROOT)/build/Release+Asserts/lib/Scaffold.dylib
endif

CTQG=0
ROTATIONS=1
TOFF=1
SQCT_LEVELS=1

################################
# Resource Count Estimation
################################
resources: $(FILE)_res_count.log
	@cat $(FILE)_res_count.log

################################
# Flat QASM generation
################################
flat: $(FILE).qasm

################################
# QASM generation
################################
qasm: $(FILE).qhf

.PHONY: res_count qasm flat

################################
# Intermediate targets
################################
# Pre-process CTQG
$(FILE)_merged.scaffold: $(FILENAME)
	@if [ $(CTQG) -eq 1 ]; then \
		echo "Extracting CTQG from Scaffold..."; \
		$(PERL) $(ROOT)/ctqg/trans/pre_process.pl $(FILENAME); \
		echo "Compiling CTQG..."; \
		$(ROOT)/ctqg/CTQG/ctqg $(CFILE).ctqg; \
		echo "Merging CTQG output back into Scaffold..."; \
		$(PERL) $(ROOT)/ctqg/trans/merge.pl $(CFILE).qasm; \
	else \
		cp $(FILENAME) $(FILE)_merged.scaffold; \
	fi

# Compile Scaffold to LLVM bytecode
$(FILE).ll: $(FILE)_merged.scaffold
	@echo "Compiling $(FILE)_merged.scaffold..."
	@$(CC) -cc1 -emit-llvm -I$(BUILD)/lib/clang/3.1/include/ -I/usr/include/ -I/usr/local/include $(FILE)_merged.scaffold -o $(FILE).ll

$(FILE)1.ll: $(FILE).ll
	@echo "Transforming cbits..."
	@$(OPT) -S -load $(SCAFFOLD_LIB) -xform-cbit-stores $(FILE).ll -o $(FILE)1.ll > /dev/null

# Perform normal C++ optimization routines
$(FILE)4.ll: $(FILE)1.ll
	@echo "O1 optimizations..."
	@$(OPT) -S $(FILE)1.ll -no-aa -tbaa -targetlibinfo -basicaa -o $(FILE)1a.ll > /dev/null
	@$(OPT) -S $(FILE)1a.ll -simplifycfg -domtree -o $(FILE)1b.ll > /dev/null
	@$(OPT) -S $(FILE)1b.ll -early-cse -lower-expect -o $(FILE)2.ll > /dev/null
	@$(OPT) -S $(FILE)2.ll -targetlibinfo -no-aa -tbaa -basicaa -globalopt -ipsccp -o $(FILE)3.ll > /dev/null
	@$(OPT) -S $(FILE)3.ll -instcombine -simplifycfg -basiccg -prune-eh -always-inline -functionattrs -domtree -early-cse -lazy-value-info -jump-threading -correlated-propagation -simplifycfg -instcombine -tailcallelim -simplifycfg -reassociate -domtree -loops -loop-simplify -lcssa -loop-rotate -licm -lcssa -loop-unswitch -instcombine -scalar-evolution -loop-simplify -lcssa -iv-users -indvars -loop-idiom -loop-deletion -loop-unroll -memdep -memcpyopt -sccp -instcombine -lazy-value-info -jump-threading -correlated-propagation -domtree -memdep -dse -adce -simplifycfg -instcombine -strip-dead-prototypes -preverify -domtree -verify -o $(FILE)3a.ll > /dev/null
	@$(OPT) -S -load $(SCAFFOLD_LIB) -InlineModule -sccp $(FILE)3a.ll -o $(FILE)4.ll > /dev/null

# Perform loop unrolling until completely unrolled
#
# Gory details:
# Unroll until the 'diff' of *4 and *6tmp is NULL (i.e., no differences)
# *4 is used to create *6tmp; *6tmp is copied over *4 for each iteration to retry 'diff'
# As a weird consquence, we need to first rename *4 to *6tmp and create an empty *4 to diff
# This screws with the intermediate results, but they are mostly for debugging anyway
$(FILE)5a.ll: $(FILE)4.ll
	@UCNT=0; \
	mv $(FILE)4.ll $(FILE)5tmp.ll; \
	touch $(FILE)4.ll; \
	while [ -n "$$(diff -q $(FILE)4.ll $(FILE)5tmp.ll)" ]; do \
		UCNT=$$(expr $$UCNT + 1); \
		echo "Unrolling Loops ($$UCNT)..."; \
		cp $(FILE)5tmp.ll $(FILE)4.ll; \
		$(OPT) -S $(FILE)4.ll -mem2reg -loops -loop-simplify -loop-rotate -lcssa -loop-unroll -unroll-threshold=4294967295 -simplifycfg -internalize -globaldce -sccp -time-passes -o $(FILE)5.ll > /dev/null && \
		echo "Cloning Functions ($$UCNT)..." && \
		$(OPT) -S -load $(SCAFFOLD_LIB) -FunctionClone -internalize -globaldce -sccp -time-passes $(FILE)5.ll -o $(FILE)5tmp.ll > /dev/null;  \
	done && \
	mv $(FILE)5tmp.ll $(FILE)5a.ll

# Remove all unused functions
$(FILE)6.ll: $(FILE)5a.ll
	@echo "Dead Argument Elimination..." && \
	$(OPT) -S -deadargelim -internalize -globaldce -sccp $(FILE)5a.ll -o $(FILE)6.ll > /dev/null

# Perform rotation decomposition if requested and SQCT is built
$(FILE)7.ll: $(FILE)6.ll
	@if [ ! -e $(SQCTPATH)/rotZ ]; then \
		echo "SQCT not built, skipping rotation decomposition..."; \
		cp $(FILE)6.ll $(FILE)7.ll; \
	elif [ $(ROTATIONS) -eq 1 ]; then \
		echo "Decomposing Rotations..."; \
		if [ ! -e /tmp/epsilon-net.0.bin ]; then echo "Generating decomposition databases; this may take up to an hour"; fi; \
		export SQCTPATH=$(SQCTPATH); \
		$(OPT) -S -load $(SCAFFOLD_LIB) -Rotations -sqct-levels $(SQCT_LEVELS) $(FILE)6.ll -o $(FILE)7.ll > /dev/null; \
	else \
		cp $(FILE)6.ll $(FILE)7.ll; \
	fi

# Remove any code that is useless after optimizations
$(FILE)10.ll: $(FILE)7.ll
	@echo "Internalizing and Removing Unused Functions..."
	@$(OPT) -S $(FILE)7.ll -internalize -globaldce -deadargelim -o $(FILE)10.ll > /dev/null

# Perform gate decomposition if TOFF is 1
$(FILE)11.ll: $(FILE)10.ll
	@echo "Gate Decomposition..."
	@if [ $(TOFF) -eq 1 ]; then \
		$(OPT) -S -load $(SCAFFOLD_LIB) -ToffoliReplace $(FILE)10.ll -o $(FILE)11.ll > /dev/null; \
	else \
		cp $(FILE)10.ll $(FILE)11.ll; \
	fi

# Generate resource counts from final LLVM output
$(FILE)_res_count.log: $(FILE)11.ll
	@echo "Generating $(FILE)_res_count.log..."
	@$(OPT) -load $(SCAFFOLD_LIB) -ResourceCount $(FILE)11.ll 2> $(FILE)_res_count.log > /dev/null

# Generate hierarchical QASM
$(FILE).qhf: $(FILE)11.ll
	@echo "Generating $(FILE).qhf"
	@$(OPT) -load $(SCAFFOLD_LIB) -gen-qasm $(FILE)11.ll 2> $(FILE).qhf > /dev/null

# Translate hierarchical QASM back to C++ for flattening
$(FILE)_qasm.scaffold: $(FILE).qhf
	@echo Generating flattened QASM...
	@$(PYTHON) $(ROOT)/scaffold/flatten-qasm.py $(FILE).qhf

# Compile C++
$(FILE)_qasm: $(FILE)_qasm.scaffold
	@$(CC) $(FILE)_qasm.scaffold -o $(FILE)_qasm

# Execute hierchical QASM to flatten it
$(FILE).qasm: $(FILE)_qasm
	@./$(FILE)_qasm > $(FILE).tmp
	@cat fdecl.out $(FILE).tmp > $(FILE).qasm

# purge cleans temp files
purge:
	@rm -f $(FILE)_merged.scaffold $(FILE)_noctqg.scaffold $(FILE).ll $(FILE)1.ll $(FILE)1a.ll $(FILE)1b.ll $(FILE)2.ll $(FILE)3.ll $(FILE)3a.ll $(FILE)4.ll $(FILE)5.ll $(FILE)5a.ll $(FILE)6.ll $(FILE)7.ll $(FILE)8.ll $(FILE)9.ll $(FILE)10.ll $(FILE)11.ll $(FILE)tmp.ll $(FILE)_qasm $(FILE)_qasm.scaffold fdecl.out $(CFILE).ctqg $(CFILE).c $(CFILE).ctqg $(CFILE).qasm $(CFILE).signals $(FILE).tmp sim_$(CFILE) $(FILE).qhf

# clean removes all completed files
clean: purge
	@rm -f $(FILE)_res_count.log $(FILE)_qasm.flat $(FILE).qhf $(FILE).qasm

.PHONY: clean purge
