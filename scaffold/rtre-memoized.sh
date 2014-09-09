#!/bin/sh

fname=$1

date

echo "Compiling qasm-resource-est.c"
../build/Release+Asserts/bin/clang -cc1 -O1 -emit-llvm qasm-resource-est-memoized.c -o qasm-resource-est-memoized.bc

echo "Compiling scaffold code"
../build/Release+Asserts/bin/clang -cc1 -emit-llvm ${fname}.scaffold -o ${fname}_compile.bc

echo "Linking qasm-resource-est.c and ${fname}_compile.bc"
../build/Release+Asserts/bin/llvm-link qasm-resource-est-memoized.bc ${fname}_compile.bc -S -o=${fname}_res_linked.bc

echo "Instrumenting ${fname}_res_linked.bc"
../build/Release+Asserts/bin/opt -load ../build/Release+Asserts/lib/Scaffold.dylib -runtime-resource-estimation-memoized ${fname}_res_linked.bc -o ${fname}_res_instr.bc

../build/Release+Asserts/bin/llvm-dis ${fname}_res_instr.bc -o ${fname}_instr.ll

../build/Release+Asserts/bin/opt -dce -dse ${fname}_res_instr.bc -o ${fname}_res_instr2.bc

../build/Release+Asserts/bin/llvm-dis ${fname}_res_instr2.bc -o ${fname}_instr2.ll

echo "Executing ${fname}_res_instr.bc"
../build/Release+Asserts/bin/lli ${fname}_res_instr2.bc

echo "Done"

date

rm qasm-resource-est-memoized.bc ${fname}_compile.bc ${fname}_res_linked.bc ${fname}_res_instr.bc ${fname}_res_instr2.bc
echo "Check output ll file in ${fname}_instr.ll"
