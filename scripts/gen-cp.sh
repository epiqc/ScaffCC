#!/bin/bash

DIR=$(dirname $0)
ROOT=$DIR/..
OPT=$ROOT/build/Release+Asserts/bin/opt
SCAF=$ROOT/build/Release+Asserts/lib/Scaffold.so

THRESHOLDS=(005k 010k 2M)

# Generate .ll file if not done already
for f in $*; do
  b=$(basename $f .scaffold)
  echo "[gen-cp.sh] $b: Compiling ..."
  if [! -e ${b}.ll ]; then
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
    echo -e "\t[gen-cp.sh] Computing module gate counts..."
    $OPT -S -load $SCAF -ResourceCount2 ${b}.ll > /dev/null 2> ${b}.out

    for th in ${THRESHOLDS[@]}
    do
        echo -e "\t[gen-cp.sh] Flattening modules smaller than Th..."
        python flattening_thresh.py ${b}
        mv ${b}_inline${th}.txt inline_info.txt
        $OPT -S -load $SCAF -InlineModule -dce -internalize -globaldce ${b}.ll -o ${b}_inline${th}.ll
        echo -e "\t[gen-cp.sh] Critical Path Calculation..."
        time $OPT -load $SCAF -GetCriticalPath ${b}_inline${th}.ll >/dev/null 2> ${b}_inline${th}.cp
    done
    rm ${b}.out *txt
done
