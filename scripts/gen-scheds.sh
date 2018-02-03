#!/bin/bash

DIR=$(dirname $0)
ROOT=$DIR/..
OPT=$ROOT/build/Release+Asserts/bin/opt
SCAF=$ROOT/build/Release+Asserts/lib/Scaffold.so

# Capacity of each SIMD region
D=(1024)
# Number of SIMD regions
K=(2)
# Module flattening threshold: must be picked from the set in scripts/flattening_thresh.py
THRESHOLDS=(2M)
# Full schedule? otherwise only generates metrics (faster)
FULL_SCHED=true

# Create directory to put all byproduct and output files in
for f in $*; do
  b=$(basename $f .scaffold)  
  echo "[gen-scheds.sh] $b: Creating output directory ..."
  mkdir -p "$b"
  #mv ./*${b}* ${b} 2>/dev/null
done

# Generate .ll file if not done already
for f in $*; do
  b=$(basename $f .scaffold)
  echo "[gen-scheds.sh] $b: Compiling ..."
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

# Module flattening pass with different thresholds
for f in $*; do
  b=$(basename $f .scaffold)
  echo "[gen-scheds.sh] $b: Flattening ..."
  echo "[gen-scheds.sh] Computing module gate counts ..."  
  $OPT -S -load $SCAF -ResourceCount2 ${b}/${b}.ll > /dev/null 2> ${b}.out  
  python $DIR/flattening_thresh.py ${b}  
  for th in ${THRESHOLDS[@]}; do      
    if [ ! -e ${b}/${b}.flat${th}.ll ]; then
      echo "[gen-scheds.sh] Flattening modules smaller than Threshold = $th ..."    
      mv ${b}.flat${th}.txt flat_info.txt
      $OPT -S -load $SCAF -FlattenModule -dce -internalize -globaldce ${b}/${b}.ll -o ${b}/${b}.flat${th}.ll
    fi
  done
  rm -f *flat*.txt ${b}.out     
done

# Perform resource estimation 
for f in $*; do
  b=$(basename $f .scaffold)
  echo "[gen-scheds.sh] $b: Resource count ..."
  for th in ${THRESHOLDS[@]}; do      
    if [ -n ${b}/${b}.flat${th}.resources ]; then
      echo "[gen-scheds.sh] Resource count for Threshold = $th flattening ..."
      $OPT -S -load $SCAF -ResourceCount ${b}/${b}.flat${th}.ll > /dev/null 2> ${b}/${b}.flat${th}.resources
    fi
  done
done

# For different K and D values specified above, generate MultiSIMD schedules
for f in $*; do
  b=$(basename $f .scaffold)
  for d in ${D[@]}; do
    for k in ${K[@]}; do
      echo "[gen-scheds.sh] $b: Generating SIMD K=$k D=$d leaves ..."
      for th in ${THRESHOLDS[@]}; do
        if [ ! -e ${b}/${b}.flat${th}.simd.${k}.${d}.leaves ]; then
          echo "[gen-scheds.sh] GenSIMD for Threshold = $th flattening ..."
          $OPT -load $SCAF -GenSIMDSchedule -simd-kconstraint $k -simd-dconstraint $d ${b}/${b}.flat${th}.ll > /dev/null 2> ${b}/${b}.flat${th}.simd.${k}.${d}
          ${DIR}/leaves.pl ${b}/${b}.flat${th}.simd.${k}.${d} > ${b}/${b}.flat${th}.simd.${k}.${d}.leaves
        fi
      done
    done
  done
done

# Perform different kinds of LPFS, RCP, SS scheduling, as specified in the regress.sh file
for f in $*; do
  b=$(basename $f .scaffold)
  echo "[gen-scheds.sh] $b: Generating LPFS, RCP, SS leaves ..."
  cd ${b}
  if [ "$FULL_SCHED" = true ]; then
    ../${DIR}/full_sched_regress.sh ${b}*.leaves
  else
    ../${DIR}/regress.sh ${b}*.leaves  
  fi
  cd ..
done

# Take into account the penalty of ballistic communication
for f in $*; do
  b=$(basename $f .scaffold)
  cd ${b}
  echo "[gen-scheds.sh] $b: Adding communication latencies ..."
  ../${DIR}/comm_aware.pl ${b}*.ss ${b}*.lpfs ${b}*.rcp
  cd ..
done

# Obtain coarse-grain schedules by co-scheduling modules
for f in $*; do
  b=$(basename $f .scaffold)
  cd ${b}
  for c in comm_aware_schedule.txt.${b}.*; do
    k=$(perl -e '$ARGV[0] =~ /_K(\d+)/; print $1' $c)
    d=$(perl -e '$ARGV[0] =~ /_D(\d+)/; print $1' $c)
    x=$(perl -e '$ARGV[0] =~ /.*_(.+)/; print $1' $c)
    th=$(perl -e '$ARGV[0] =~ /.flat(\d+[a-zA-Z])/; print $1' $c)    
    echo "[gen-scheds.sh] $b: Coarse-grain schedule ..."
    mv $c comm_aware_schedule.txt
    if [ ! -e ${b}.flat${th}.simd.${k}.${d}.${x}.time ]; then
      ../$OPT -load ../$SCAF -GenCGSIMDSchedule -simd-kconstraint-cg $k -simd-dconstraint-cg $d ${b}.flat${th}.ll > /dev/null 2> ${b}.flat${th}.simd.${k}.${d}.${x}.cg
    fi

    # Now do 0-communication cost
    #if [ ! -e ${b}.flat${th}.simd.${k}.${d}.w0.${x}.time ]; then
    #  ../$OPT -load ../$SCAF -GenCGSIMDSchedule -move-weight-cg 0 -simd-kconstraint-cg $k -simd-dconstraint-cg $d ${b}.flat${th}.ll > /dev/null 2> ${b}.flat${th}.simd.${k}.${d}.w0.${x}.time
    #fi
  done
  rm -f comm_aware_schedule.txt
  cd ..
done


