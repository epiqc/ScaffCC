#!/bin/bash

if [ ! -e $1 ]; then
    echo "File not found: $1"
    exit 1
fi

fname=`basename -s .scaffold ${1}`

./gen-qasm-script ${1}
python flatten-qasm.py ${fname}.qasm
../build/Release+Asserts/bin/clang ${fname}_qasm.scaffold
./a.out > ${fname}_qasm1.flat
cat fdecl.out ${fname}_qasm1.flat > ${fname}_qasm.flat 
echo "Output is in ${fname}_qasm.flat"

