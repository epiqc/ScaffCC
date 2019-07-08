#!/bin/bash

export SCAFFCC_PATH=/n/fs/qdb/ScaffCC
TEST_NAME=${1%.scaffassert}

# CLEANUP
$SCAFFCC_PATH/scaffold.sh -c $1
rm *.breakpoint_*.scaffold
rm *.breakpoint_*.ll
rm *.breakpoint_*.tmp
rm *.breakpoint_*.resources
rm *.breakpoint_*.qasmh
rm *.breakpoint_*.qasmf
rm *.breakpoint_*.qc
rm *.breakpoint_*.out
rm *.breakpoint_*.err
rm *.breakpoint_*.csv
rm *.breakpoint_*.bash

# split into breakpoints
BREAKPOINTS=$($SCAFFCC_PATH/scripts/simulation/scaffassert.py $1)
# number of measurements we want for each breakpoint
ENSEMBLE=8

# get QX Simulator
if [ ! -e $SCAFFCC_PATH/../qx_simulator_linux_x86_64/qx_simulator_1.0.beta_linux_x86_64 ]
then
    wget -nc -P $SCAFFCC_PATH/../ http://quantum-studio.net/qx_simulator_linux_x86_64.tar.gz
    tar -xvf $SCAFFCC_PATH/../qx_simulator_linux_x86_64.tar.gz -C $SCAFFCC_PATH/../
fi

######################
# Begin work section #
######################

for ((bpIndx=1;bpIndx<=$BREAKPOINTS;bpIndx++))
do
  # compile into QASM code
  $SCAFFCC_PATH/scaffold.sh -s $TEST_NAME.breakpoint_${bpIndx}.scaffold

  # SIMULATE the whole ensemble
  # map the ensemble of measurements into several runs of the simulator
  for ((ensIndx=1;ensIndx<=$ENSEMBLE;ensIndx++))
  do
    $SCAFFCC_PATH/../qx_simulator_linux_x86_64/qx_simulator_1.0.beta_linux_x86_64 \
    $TEST_NAME.breakpoint_${bpIndx}.qc > \
    $TEST_NAME.breakpoint_${bpIndx}.trial_${ensIndx}.out
  done

  # reduce the measurement results into a summary CSV file
  $SCAFFCC_PATH/scripts/simulation/register_value_csv.py \
  $TEST_NAME.breakpoint_${bpIndx}.qc \
  $TEST_NAME.breakpoint_${bpIndx}.csv

  # do statistical tests on the summary CSV files
  # check for the assertions as recorded in the bash file
  bash $TEST_NAME.breakpoint_${bpIndx}.bash
  # scripts/simulation/assert_uniform.py ${source%.scaffold}.csv 4
  # scripts/simulation/assert_integer.py ${source%.scaffold}.csv 5 30
  # scripts/simulation/assert_product.py ${source%.scaffold}.csv

done
