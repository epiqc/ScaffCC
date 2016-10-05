SHELL=/bin/bash
PERL=/usr/bin/perl
PYTHON=/usr/bin/python

ROOT=".."
FILENAME=""
FILE=""
CFILE=""
DIRNAME=""

BUILD=$(ROOT)/build/Release+Asserts

SQCTPATH=$(ROOT)/Rotations/sqct/rotZ
GRIDSYNTHPATH=$(ROOT)/Rotations/gridsynth/gridsynth
ROTATIONPATH=$(GRIDSYNTHPATH) # select rotation decomposition tool
SCRIPTSPATH=$(ROOT)/scripts/ # select rotation decomposition tool

CC=$(BUILD)/bin/clang
OPT=$(BUILD)/bin/opt

CC_FLAGS=-c -emit-llvm -I/usr/include -I/usr/include/x86_64-linux-gnu -I/usr/lib/gcc/x86_64-linux-gnu/4.8/include -I$(DIRNAME)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
SCAFFOLD_LIB=$(ROOT)/build/Release+Asserts/lib/Scaffold.so
endif
ifeq ($(UNAME_S),Darwin)
SCAFFOLD_LIB=$(ROOT)/build/Release+Asserts/lib/Scaffold.dylib
endif


RKQC=0
ROTATIONS=0
TOFF=0

################################
# Resource Count Estimation
################################
resources: $(FILE).resources

################################
# Flat QASM generation
################################
flat: $(FILE).qasmf

################################
# QASM generation
################################
qasm: $(FILE).qasmh

################################
# QX Simulator generation
################################
qc: $(FILE).qc

.PHONY: resources qasm flat qc

################################
# Intermediate targets
################################
# Compile Scaffold to LLVM bytecode

$(FILE)_merged.scaffold: $(FILENAME)
	@cp $(FILENAME) $(FILE)_merged.scaffold

$(FILE).ll: $(FILE)_merged.scaffold
	@echo "[Scaffold.makefile] Compiling $(FILE).scaffold ..."
	@$(CC) $(FILE)_merged.scaffold $(CC_FLAGS) -o $(FILE).ll 

$(FILE)1.ll: $(FILE).ll
	@echo "[Scaffold.makefile] Transforming cbits ..."
	@$(OPT) -S -load $(SCAFFOLD_LIB) -xform-cbit-stores $(FILE).ll -o $(FILE)1.ll > /dev/null

# Perform normal C++ optimization routines
$(FILE)4.ll: $(FILE)1.ll
	@echo "[Scaffold.makefile] O1 optimizations ..."
	@$(OPT) -S $(FILE)1.ll -no-aa -tbaa -targetlibinfo -basicaa -o $(FILE)1a.ll > /dev/null
	@$(OPT) -S $(FILE)1a.ll -simplifycfg -domtree -o $(FILE)1b.ll > /dev/null
	@$(OPT) -S $(FILE)1b.ll -early-cse -lower-expect -o $(FILE)2.ll > /dev/null
	@$(OPT) -S $(FILE)2.ll -targetlibinfo -no-aa -tbaa -basicaa -globalopt -ipsccp -o $(FILE)3.ll > /dev/null
	@$(OPT) -S $(FILE)3.ll -instcombine -simplifycfg -basiccg -prune-eh -always-inline -functionattrs -domtree -early-cse -lazy-value-info -jump-threading -correlated-propagation -simplifycfg -instcombine -tailcallelim -simplifycfg -reassociate -domtree -loops -loop-simplify -lcssa -loop-rotate -licm -lcssa -loop-unswitch -instcombine -scalar-evolution -loop-simplify -lcssa -iv-users -indvars -loop-idiom -loop-deletion -loop-unroll -memdep -memcpyopt -sccp -instcombine -lazy-value-info -jump-threading -correlated-propagation -domtree -memdep -dse -adce -simplifycfg -instcombine -strip-dead-prototypes -preverify -domtree -verify -o $(FILE)4.ll > /dev/null

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
	$(OPT) -S $(FILE)6tmp.ll -internalize -globaldce -adce -o $(FILE)6.ll > /dev/null  
	

# Perform Rotation decomposition if requested and rotation decomp tool is built
$(FILE)7.ll: $(FILE)6.ll
	@if [ ! -e $(ROTATIONPATH) ]; then \
		echo "[Scaffold.makefile] Rotation tool not built, skipping rotation decomposition ..."; \
		cp $(FILE)6.ll $(FILE)7.ll; \
	elif [ $(ROTATIONS) -eq 1 ]; then \
		echo "[Scaffold.makefile] Decomposing Rotations ..."; \
		if [ ! -e /tmp/epsilon-net.0.bin ]; then echo "Generating decomposition databases; this may take up to an hour"; fi; \
		export ROTATIONPATH=$(ROTATIONPATH); \
		$(OPT) -S -load $(SCAFFOLD_LIB) -Rotations $(FILE)6.ll -o $(FILE)7.ll > /dev/null; \
	else \
		cp $(FILE)6.ll $(FILE)7.ll; \
	fi

# Remove any code that is useless after optimizations
$(FILE)8.ll: $(FILE)7.ll
	@echo "[Scaffold.makefile] Internalizing and Removing Unused Functions ..."
	@$(OPT) -S $(FILE)7.ll -internalize -globaldce -deadargelim -o $(FILE)8.ll > /dev/null

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
	@$(CC) $(FILE)_qasm.scaffold -o $(FILE)_qasm

# Execute hierchical QASM to flatten it
$(FILE).qasmf: $(FILE)_qasm
	@./$(FILE)_qasm > $(FILE).tmp
	@cat fdecl.out $(FILE).tmp > $(FILE).qasmf
	@echo "[Scaffold.makefile] Flat QASM written to $(FILE).qasmf ..."

$(FILE).qc: $(FILE).qasmf
	@echo "[Scaffold.makefile] Transforming flat QASM to QX Simulator input ..."
	@$(SHELL) $(ROOT)/scripts/qasmf2qc.sh $(FILE).qasmf
	@echo "[Scaffold.makefile] QX Simulator input written to $(FILE).qc ..." 

# purge cleans temp files
purge:
	@rm -f $(FILE)_merged.scaffold $(FILE)_no.scaffold $(FILE).ll $(FILE)1.ll $(FILE)1a.ll $(FILE)1b.ll $(FILE)2.ll $(FILE)3.ll $(FILE)4.ll $(FILE)5.ll $(FILE)5a.ll $(FILE)6.ll $(FILE)6tmp.ll $(FILE)7.ll $(FILE)8.ll $(FILE)9.ll $(FILE)10.ll $(FILE)11.ll $(FILE)12.ll $(FILE)tmp.ll $(FILE)_qasm $(FILE)_qasm.scaffold fdecl.out $(CFILE).ctqg $(CFILE).c $(CFILE).signals $(FILE).tmp sim_$(CFILE) $(FILE).*.qasm 

# clean removes all completed files
clean: purge
	@rm -f $(FILE).resources $(FILE).qasmh $(FILE).qasmf $(FILE).qc

.PHONY: clean purge
