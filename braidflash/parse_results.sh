#!/bin/bash

# usage: $ bash parse_result.sh {benchmark1} {benchmark2} 

#compile
#rm -f router && make

# change the desired thresholds below. that's the only change needed.
error_rates=(5)
th_yx=(8)
th_drop=(20)
layout=("" "--opt")
technologies=("sup")
priority_policy=(0 1 2 3 4 5 6)

#run
for bench in $*; do
  bench_dir=$(dirname $bench)
  bench_name=$(basename $bench)
  echo $bench_dir $bench_name
  output_simulation=$bench_dir/braid_simulation/$bench_name
  output_plot=$bench_dir/braid_simulation/braid_plot/$bench_name
  mkdir -p $bench_dir/braid_simulation/braid_plot
  
  for tech in "${technologies[@]}" 
  do
    for p in "${error_rates[@]}" 
    do
      # Excel Sheet 'BraidFlash'
      echo -n "" > ${output_plot}.${p}.${tech}.clk  
      for pri in "${priority_policy[@]}"      
      do
        Ser1=$(grep "SerialCLOCK" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.br | awk '{sum+=$2} END {print sum}')  
        Par1=$(grep "ParallelCLOCK" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.br | awk '{sum+=$2} END {print sum}')  
        Cri1=$(grep "CriticalCLOCK" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.br | awk '{sum+=$2} END {print sum}')    
        tots1=$(grep "total_success" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.br | awk '{sum+=$2} END {print sum}')  
        totc1=$(grep "total_conflict" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.br | awk '{sum+=$2} END {print sum}') 
        unic1=$(grep "unique_conflict" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.br | awk '{sum+=$2} END {print sum}')   

        Ser2=$(grep "SerialCLOCK" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.opt.br | awk '{sum+=$2} END {print sum}')  
        Par2=$(grep "ParallelCLOCK" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.opt.br | awk '{sum+=$2} END {print sum}')  
        Cri2=$(grep "CriticalCLOCK" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.opt.br | awk '{sum+=$2} END {print sum}')    
        tots2=$(grep "total_success" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.opt.br | awk '{sum+=$2} END {print sum}')  
        totc2=$(grep "total_conflict" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.opt.br | awk '{sum+=$2} END {print sum}') 
        unic2=$(grep "unique_conflict" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.opt.br | awk '{sum+=$2} END {print sum}')  
        echo $tots1 $totc1 $unic1 $Ser1 $Par1 $Cri1 >> ${output_plot}.${p}.${tech}.clk
        echo $tots2 $totc2 $unic2 $Ser2 $Par2 $Cri2 >> ${output_plot}.${p}.${tech}.clk
      done

      # Excel Sheet 'DroppedGates'
      echo -n "" > ${output_plot}.${p}.${tech}.dropped                        
      echo "TotalDroppedGates" "UniqueDroppedGates" 
      totd1=$(grep "total_dropped_gates" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.br | awk '{sum+=$2} END {print sum}')  
      unid1=$(grep "unique_dropped_gates" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.br | awk '{sum+=$2} END {print sum}')    
      totd2=$(grep "total_dropped_gates" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.opt.br | awk '{sum+=$2} END {print sum}')  
      unid2=$(grep "unique_dropped_gates" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.opt.br | awk '{sum+=$2} END {print sum}')      
      echo $totd1 $unid1 >> ${output_plot}.${p}.${tech}.dropped
      echo $totd2 $unid2 >> ${output_plot}.${p}.${tech}.dropped

      # Excel Sheet 'ConflictedAttempts'
      echo -n "" > ${output_plot}.${p}.${tech}.attempts            
      echo "Attempts"  
      for a in {0..40}
      do
        att_sum=$(grep "attempt\t$a" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.br | awk '{sum+=$3} END {print sum}') 
        att_sum_opt=$(grep "attempt\t$a" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.opt.br | awk '{sum+=$3} END {print sum}') 
        echo $a $att_sum $att_sum_opt >> ${output_plot}.${p}.${tech}.attempts
      done  

      # Excel Sheet 'ManhattanCost'
      echo -n "" > ${output_plot}.${p}.${tech}.mcost            
      echo "ManhattanCost" "ManhattanCost.Opt"
      mcost1=$(grep "mcost:" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.opt.br | awk '{print $2}')    
      mcost2=$(grep "mcost_opt:" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.opt.br | awk '{print $2}')      
      echo $mcost1 $mcost2 >> ${output_plot}.${p}.${tech}.mcost

      # Excel Sheet 'Area'
      echo -n "" > ${output_plot}.${p}.${tech}.area      
      echo "CodeDistance" "MaxLogicalQbits" "MaxPhysicalQbits"
      d=$(grep "code_distance(d):" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.br | awk '{print $2}')
      Q=$(grep "num_logical_qubits:" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.br | awk '{print $2}')
      q=$(grep "num_physical_qubits:" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.br | awk '{print $2}')
      echo $d $Q $q >> ${output_plot}.${p}.${tech}.area
      # Extra Info 
      echo -n "" > ${output_plot}.${p}.${tech}.ecount      
      echo "EventCount.CNOT" "EventCount.CNOT+H"
      ecount1=$(grep "event_count:" ${output_simulation}.p.$p.yx.$th_yx.drop.$th_drop.pri.$pri.$tech.opt.br | awk '{print $2}')        
      echo $ecount1 >> ${output_plot}.${p}.${tech}.ecount
    done
  done
done
