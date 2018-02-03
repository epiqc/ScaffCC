#!/bin/bash

DIR=$(dirname $0)
ROOT=$DIR/..
BIN=$ROOT/build/Release+Asserts/bin
LIB=$ROOT/build/Release+Asserts/lib
SCAF=$LIB/Scaffold.so
OPT=$BIN/opt
CLANG=$BIN/clang
LLVM_LINK=$BIN/llvm-link
LLI=$BIN/lli
I_FLAGS="-I/usr/include -I/usr/include/x86_64-linux-gnu -I/usr/lib/gcc/x86_64-linux-gnu/4.8/include"

# Capacity of each SIMD region
D=(1024)
# Number of SIMD regions
K=(2)
# Module flattening thresholds: must be picked from the set in scripts/flattening_thresh.py
THRESHOLDS=(010k)
# Full schedule? otherwise only generates metrics (faster)
FULL_SCHED=1

# Create directory to put all byproduct and output files in
for f in $*; do
  b=$(basename $f .scaffold)  
  echo "[gen-lpfs.sh] $b: Creating output directory ..."
  mkdir -p "$b"
  mv ./*${b}* ${b} 2>/dev/null
done

# Generate .ll file if not done already
for f in $*; do
  b=$(basename $f .scaffold)
  echo "[gen-lpfs.sh] $b: Compiling ..."
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
  echo "[gen-lpfs.sh] $b: Computing module gate counts ..."  
  $OPT -S -load $SCAF -ResourceCount2 ${b}/${b}.ll > /dev/null 2> ${b}.out  
  python $DIR/flattening_thresh.py ${b}  
  for th in ${THRESHOLDS[@]}; do      
    if [ ! -e ${b}/${b}.flat${th}.ll ]; then
      echo "[gen-lpfs.sh] $b.flat${th}: Flattening ..."      
      mv ${b}.flat${th}.txt flat_info.txt
      $OPT -S -load $SCAF -FlattenModule -dce -internalize -globaldce ${b}/${b}.ll -o ${b}/${b}.flat${th}.ll
    fi
  done
  rm -f *flat*.txt ${b}.out     
done

# Perform resource estimation 
for f in $*; do
  b=$(basename $f .scaffold)
  for th in ${THRESHOLDS[@]}; do      
    echo "[gen-lpfs.sh] $b.flat${th}: Resource count ..."    
    if [ -n ${b}/${b}.flat${th}.resources ]; then
      $OPT -S -load $SCAF -ResourceCount ${b}/${b}.flat${th}.ll > /dev/null 2> ${b}/${b}.flat${th}.resources
    fi
  done
done

# For different K and D values specified above, generate MultiSIMD schedules
# Turn on opp_simd (opportunistic simd) for more efficient schedules, but much slower. Refer to paper.
for f in $*; do
  b=$(basename $f .scaffold)
  for d in ${D[@]}; do
    for k in ${K[@]}; do
      for th in ${THRESHOLDS[@]}; do
        echo "[gen-lpfs.sh] $b.flat${th}: Generating SIMD K=$k D=$d leaves ..."        
        if [ ! -e ${b}/${b}.flat${th}.simd.${k}.${d}.leaves.local ]; then
          $OPT -load $SCAF -GenLPFSSchedule -simd-kconstraint-lpfs $k -simd-dconstraint-lpfs $d -simd_l 1 -full_sched $FULL_SCHED -local_mem 1 -opp_simd 0 ${b}/${b}.flat${th}.ll > /dev/null 2> ${b}/${b}.flat${th}.simd.${k}.${d}.leaves.local
        fi
      done
    done
  done
done

# Take into account the penalty of ballistic communication
for f in $*; do
  b=$(basename $f .scaffold)
  cd ${b}
  echo "[gen-lpfs.sh] $b: Adding communication latencies ..."
  ../${DIR}/comm_aware.pl ${b}*.leaves.local
  cd ..
done

# Obtain coarse-grain schedules by co-scheduling modules
for f in $*; do
  b=$(basename $f .scaffold)
  cd ${b}
  for th in ${THRESHOLDS[@]}; do      
    echo "[gen-lpfs.sh] $b.flat${th}: Coarse-grain schedule ..."    
    for c in comm_aware_schedule.txt.${b}.flat${th}_*; do
      k=$(perl -e '$ARGV[0] =~ /_K(\d+)/; print $1' $c)
      d=$(perl -e '$ARGV[0] =~ /_D(\d+)/; print $1' $c)
      x=$(perl -e '$ARGV[0] =~ /.*_(.+)/; print $1' $c)
      th=$(perl -e '$ARGV[0] =~ /.flat(\d+[a-zA-Z])/; print $1' $c)    
      mv $c comm_aware_schedule.txt
      if [ ! -e ${b}/${b}.flat${th}.simd.${k}.${d}.${x}.time ]; then
        ../$OPT -load ../$SCAF -GenCGSIMDSchedule -simd-kconstraint-cg $k -simd-dconstraint-cg $d ${b}.flat${th}.ll > /dev/null 2> ${b}.flat${th}.simd.${k}.${d}.${x}.time
      fi
    done
  done
  rm -f comm_aware_schedule.txt histogram_data.txt
  cd ..
done

# Rename to simple names
for f in $*; do
  b=$(basename $f .scaffold)  
  rename -f 's/\.simd\.(\d)\.(\d+)\.leaves\.local/\.lpfs/' ${b}/*leaves.local
  rename -f 's/\.simd\.(\d)\.(\d+)\.local\.time/\.cg/' ${b}/*time
done

# Perform module frequency estimation
for f in $*; do
  b=$(basename $f .scaffold)  
  b_dir=$(dirname "$(readlink -f $f)")
  echo "[gen-lpfs.sh] Compiling frequency-estimation-hybrid.c" >&2
  $CLANG -c -O1 -emit-llvm $DIR/frequency-estimation-hybrid.c -o $DIR/frequency-estimation-hybrid.bc
  echo "[gen-lpfs.sh] $b: Frequency count ..." >&2
  if [ -n ${b}/${b}.freq ]; then
    cp ${b}/${b}.ll ${b}/${b}_dynamic.ll          
    echo -e "\t[gen-lpfs.sh] Decomposing Toffolis" >&2
    $OPT -S -load $SCAF -ToffoliReplace ${b}/${b}_dynamic.ll -o ${b}/${b}_dynamic.ll
    echo -e "\t[gen-lpfs.sh] Identifying Loops to retain..." >&2
    $OPT -S -mem2reg -instcombine -loop-simplify -loop-rotate -indvars ${b}/${b}_dynamic.ll -o ${b}/${b}_marked.ll
    echo -e "\t[gen-lpfs.sh] Rolling up Loops" >&2
    $OPT -S -load $SCAF -dyn-rollup-loops ${b}/${b}_marked.ll -o ${b}/${b}_rolled.ll
    echo -e "\t[gen-lpfs.sh] Linking frequency-estimation-hybrid.bc and ${b}/${b}_rolled.ll" >&2
    $LLVM_LINK $DIR/frequency-estimation-hybrid.bc ${b}/${b}_rolled.ll -S -o=${b}/${b}_linked.ll
    echo -e "\t[gen-lpfs.sh] Instrumenting ${b}/${b}_linked.ll" >&2
    $OPT -S -load $SCAF -runtime-frequency-estimation-hybrid ${b}/${b}_linked.ll -o ${b}/${b}_instr.ll
    $OPT -S -dce -dse -dce ${b}/${b}_instr.ll -o ${b}/${b}_instr2.ll
    $OPT -S -O1 ${b}/${b}_instr2.ll -o ${b}/${b}_instr.ll
    echo -e "\t[gen-lpfs.sh] Executing ${b}/${b}_instr.ll with lli" >&2
    $LLI ${b}/${b}_instr.ll > ${b}/${b}.flat${th}.freq
    echo -e "\t[gen-lpfs.sh] Frequency estimates written to ${b}.flat${th}.freq"
  fi
  rm $DIR/frequency-estimation-hybrid.bc ${b}/${b}*_dynamic.ll ${b}/${b}*_marked.ll ${b}/${b}*_rolled.ll ${b}/${b}*_linked.ll ${b}/${b}*_instr.ll ${b}/${b}*_instr2.ll ${b}/${b}.ll
done
