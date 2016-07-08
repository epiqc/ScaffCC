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

CTQG=0
ROTATIONS=0
TOFF=0

################################
# Resource Count Estimation
################################
resources: $(FILE).resources
	@cat $(FILE).resources

################################
# Flat QASM generation
################################
flat: $(FILE).qasmf

################################
# QASM generation
################################
qasm: $(FILE).qasmh

.PHONY: res_count qasm flat

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

# Compile Scaffold to LLVM bytecode
$(FILE).ll: $(FILE)_merged.scaffold
	@echo "[Scaffold.makefile] Compiling $(FILE)_merged.scaffold ..."
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
$(FILE)10.ll: $(FILE)7.ll
	@echo "[Scaffold.makefile] Internalizing and Removing Unused Functions ..."
	@$(OPT) -S $(FILE)7.ll -internalize -globaldce -deadargelim -o $(FILE)10.ll > /dev/null

# Perform Toffoli decomposition if TOFF is 1
$(FILE)11.ll: $(FILE)10.ll
	@if [ $(TOFF) -eq 1 ]; then \
    echo "[Scaffold.makefile] Toffoli Decomposition ..."; \
		$(OPT) -S -load $(SCAFFOLD_LIB) -ToffoliReplace $(FILE)10.ll -o $(FILE)11.ll > /dev/null; \
	else \
		cp $(FILE)10.ll $(FILE)11.ll; \
	fi

# Generate resource counts from final LLVM output
$(FILE).resources: $(FILE)11.ll
	@echo "[Scaffold.makefile] Generating resource count ..."    
	@$(OPT) -load $(SCAFFOLD_LIB) -ResourceCount $(FILE)11.ll 2> $(FILE).resources > /dev/null
	@echo "[Scaffold.makefile] Resources written to $(FILE).resources ..."  

# Generate hierarchical QASM
$(FILE).qasmh: $(FILE)11.ll
	@echo "[Scaffold.makefile] Generating hierarchical QASM ..."  
	@$(OPT) -load $(SCAFFOLD_LIB) -gen-qasm $(FILE)11.ll 2> $(FILE).qasmh > /dev/null
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

# purge cleans temp files
purge:
	@rm -f $(FILE)_merged.scaffold $(FILE)_noctqg.scaffold $(FILE).ll $(FILE)1.ll $(FILE)1a.ll $(FILE)1b.ll $(FILE)2.ll $(FILE)3.ll $(FILE)4.ll $(FILE)5.ll $(FILE)5a.ll $(FILE)6.ll $(FILE)6tmp.ll $(FILE)7.ll $(FILE)8.ll $(FILE)9.ll $(FILE)10.ll $(FILE)11.ll $(FILE)tmp.ll $(FILE)_qasm $(FILE)_qasm.scaffold fdecl.out $(CFILE).ctqg $(CFILE).c $(CFILE).signals $(FILE).tmp sim_$(CFILE) $(FILE).*.qasm

# clean removes all completed files
clean: purge
	@rm -f $(FILE).resources $(FILE).qasmh $(FILE).qasmf

.PHONY: clean purge
