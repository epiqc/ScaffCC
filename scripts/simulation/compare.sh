#!/bin/sh

# get QX Simulator
if [ ! -e ../qx_simulator_linux_x86_64/qx_simulator_1.0.beta_linux_x86_64 ]
then
    wget -nc -P ../ http://quantum-studio.net/qx_simulator_linux_x86_64.tar.gz
    tar -xvf ../qx_simulator_linux_x86_64.tar.gz -C ../
fi

test_bench=../qx_simulator_linux_x86_64/examples/qft_8q.qc
test_target=Library/QFT/QFT_test.scaffold

../qx_simulator_linux_x86_64/qx_simulator_1.0.beta_linux_x86_64 ../qx_simulator_linux_x86_64/examples/qft_8q.qc > qft_8q.global_phase.output
python scripts/simulation/remove_global_phase.py qft_8q.global_phase.output qft_8q.no_global_phase.output

./scaffold.sh -c $test_target
./scaffold.sh -s $test_target
source=${test_target##*/}
echo ${source%.scaffold}
../qx_simulator_linux_x86_64/qx_simulator_1.0.beta_linux_x86_64 ${source%.scaffold}.qc > ${source%.scaffold}.global_phase.output
python scripts/simulation/remove_global_phase.py ${source%.scaffold}.global_phase.output ${source%.scaffold}.no_global_phase.output

python scripts/simulation/trace_distance.py qft_8q.no_global_phase.output ${source%.scaffold}.no_global_phase.output
