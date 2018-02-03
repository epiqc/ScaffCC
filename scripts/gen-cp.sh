#!/bin/bash

DIR=$(dirname $0)
ROOT=$DIR/..
OPT=$ROOT/build/Release+Asserts/bin/opt
SCAF=$ROOT/build/Release+Asserts/lib/Scaffold.so

# Module flattening threshold
# note: thresholds must be picked from the set in scripts/flattening_thresh.py
THRESHOLDS=(010k 100k 2M 25M)

# Create directory to put all byproduct and output files in
for f in $*; do
  b=$(basename $f .scaffold)  
  echo "[gen-cp.sh] $b: Creating output directory ..."
  mkdir -p "$b"
  #mv ./*${b}* ${b} 2>/dev/null
done

# Generate .ll file if not done already
for f in $*; do
  b=$(basename $f .scaffold)
  echo "[gen-cp.sh] $b: Compiling ..."
  if [ ! -e ${b}/${b}.ll ]; then
    # Generate compiled files
    $ROOT/scaffold.sh -rk $f
    mv ${b}11.ll ${b}11.ll.keep_me
    # clean intermediary compilation files (comment out for speed)
    $ROOT/scaffold.sh -c $f
    # Keep the final output for the compilation
    mv ${b}11.ll.keep_me ${b}/${b}.ll
  fi
done

# Module flattening pass with different thresholds, then critical path calculation.
for f in $*; do
  b=$(basename $f .scaffold)  
  echo "[gen-cp.sh] $b: Flattening"
  echo "[gen-cp.sh] Computing module gate counts ..."
  $OPT -S -load $SCAF -ResourceCount2 ${b}/${b}.ll > /dev/null 2> ${b}.out
  python $DIR/flattening_thresh.py ${b}
  for th in ${THRESHOLDS[@]}; do    
    if [ ! -e ${b}/${b}.flat${th}.ll ]; then      
      echo "[gen-cp.sh] Flattening modules smaller than Threshold = $th ..."
      mv ${b}.flat${th}.txt flat_info.txt
      $OPT -S -load $SCAF -FlattenModule -dce -internalize -globaldce ${b}/${b}.ll -o ${b}/${b}.flat${th}.ll
    fi
    if [ ! -e ${b}/${b}.flat${th}.cp ]; then
      echo "[gen-cp.sh] Critical path calculation ..."        
      /usr/bin/time -v $OPT -load $SCAF -GetCriticalPath ${b}/${b}.flat${th}.ll >/dev/null 2> ${b}/${b}.flat${th}.cp
    fi
  done
  rm -f *flat*txt ${b}.out
  echo "[gen-cp.sh] Critical path lengths written to ${b}*.cp"
done
