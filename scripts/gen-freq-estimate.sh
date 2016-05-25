#!/bin/sh

DIR=$(dirname $0)
ROOT=$DIR/..
BIN=$ROOT/build/Release+Asserts/bin
OPT=$BIN/opt
CLANG=$BIN/clang
I_FLAGS="-I/usr/include -I/usr/include/x86_64-linux-gnu -I/usr/lib/gcc/x86_64-linux-gnu/4.8/include"
LLVM_LINK=$BIN/llvm-link
LLI=$BIN/lli
SCAF=$ROOT/build/Release+Asserts/lib/Scaffold.so
fname=$1

# Create directory to put all byproduct and output files in
for f in $*; do
  b=$(basename $f .scaffold)  
  echo "[gen-freq-estimate.sh] $b: Creating output directory"
  mkdir -p "$b"

  echo "[gen-freq-estimate.sh] Compiling frequency-estimation-hybrid.c" >&2
  $CLANG -c -O1 -emit-llvm $I_FLAGS frequency-estimation-hybrid.c -o frequency-estimation-hybrid.bc

  # if: file is compiled before (possibly flattened/unrolled/cloned also), use that compiled .ll file.
  # else: do simple compilation to get .ll file (without any flattening/unrolling/cloning)    
  if [ -e ${b}/${b}.ll ]; then
    echo "[gen-freq-estimate.sh] Using previously compiled ${b}/${b}.ll as ${b}/${b}_dynamic.ll"
    cp ${b}/${b}.ll ${b}/${b}_dynamic.ll
  else
    echo "[gen-freq-estimate.sh] Simple compiling of ${f} into ${b}/${b}_dynamic.ll" >&2
    $CLANG -cc1 -emit-llvm $I_FLAGS ${f} -o ${b}/${b}_dynamic.ll
  fi
  
  echo "[gen-freq-estimate.sh] Decomposing Toffolis" >&2
  $OPT -S -load $SCAF -ToffoliReplace ${b}/${b}_dynamic.ll -o ${b}/${b}_dynamic.ll

  echo "[gen-freq-estimate.sh] Identifying Loops to retain..." >&2
  $OPT -S -mem2reg -instcombine -loop-simplify -loop-rotate -indvars ${b}/${b}_dynamic.ll -o ${b}/${b}_marked.ll

  echo "[gen-freq-estimate.sh] Rolling up Loops" >&2
  $OPT -S -load $SCAF -dyn-rollup-loops ${b}/${b}_marked.ll -o ${b}/${b}_rolled.ll

  echo "[gen-freq-estimate.sh] Linking frequency-estimation-hybrid.bc and ${b}/${b}_rolled.ll" >&2
  $LLVM_LINK frequency-estimation-hybrid.bc ${b}/${b}_rolled.ll -S -o=${b}/${b}_linked.ll

  echo "[gen-freq-estimate.sh] Instrumenting ${b}/${b}_linked.ll" >&2
  $OPT -S -load $SCAF -runtime-frequency-estimation-hybrid ${b}/${b}_linked.ll -o ${b}/${b}_instr.ll

  $OPT -S -dce -dse -dce ${b}/${b}_instr.ll -o ${b}/${b}_instr2.ll

  $OPT -S -O1 ${b}/${b}_instr2.ll -o ${b}/${b}_instr.ll

  echo "[gen-freq-estimate.sh] Executing ${b}/${b}_instr.ll with lli" >&2
  $LLI ${b}/${b}_instr.ll > ${b}/${b}.freq_estimate

  echo "[gen-freq-estimate.sh] Critical path lengths written to ${b}.freq_estimate"
  #rm frequency-estimation-hybrid.bc ${b}/${b}_dynamic.ll ${b}/${b}_marked.ll ${b}/${b}_rolled.ll ${b}/${b}_linked.ll ${b}/${b}_instr.ll ${b}/${b}_instr2.ll
done
