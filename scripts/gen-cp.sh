#!/bin/bash

DIR=$(dirname $0)
ROOT=$DIR/..
OPT=$ROOT/build/Release+Asserts/bin/opt
SCAF=$ROOT/build/Release+Asserts/lib/Scaffold.so

THRESHOLDS=(001k 010k 2M)

# Generate .ll file if not done already
for f in $*; do
  b=$(basename $f .scaffold)
  echo "[gen-cp.sh] $b: Compiling ..."
  if [ ! -e ${b}.ll ]; then
    # Generate compiled files
    $ROOT/scaffold.sh -r $f
    mv ${b}11.ll ${b}11.ll.keep_me
    # clean intermediary compilation files (comment out for speed)
    $ROOT/scaffold.sh -c $f
    # Keep the final output for the compilation
    mv ${b}11.ll.keep_me ${b}.ll
  fi
done

for f in $*; do
    b=$(basename $f .scaffold)  
    echo "[gen-cp.sh] $b"
    echo "[gen-cp.sh] Computing module gate counts ..."
    $OPT -S -load $SCAF -ResourceCount2 ${b}.ll > /dev/null 2> ${b}.out

    python flattening_thresh.py ${b}
    for th in ${THRESHOLDS[@]}
    do
        echo "[gen-cp.sh] Flattening modules smaller than Threshold = $th ..."
        cp ${b}_flat${th}.txt flat_info.txt
        $OPT -S -load $SCAF -FlattenModule -dce -internalize -globaldce ${b}.ll -o ${b}_flat${th}.ll
        echo "[gen-cp.sh] Critical path calculation ..."        
        $OPT -load $SCAF -GetCriticalPath ${b}_flat${th}.ll >/dev/null 2> ${b}_flat${th}.cp
    done
    rm ${b}.out *flat*txt
    echo "[gen-cp.sh] Critical path lengths written to ${b}*.cp"
done
