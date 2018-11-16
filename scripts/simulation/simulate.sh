#!/bin/sh

# get QX Simulator
if [ ! -e ../qx_simulator_linux_x86_64/qx_simulator_1.0.beta_linux_x86_64 ]
then
    wget -nc -P ../ http://quantum-studio.net/qx_simulator_linux_x86_64.tar.gz
    tar -xvf ../qx_simulator_linux_x86_64.tar.gz -C ../
fi

source=${1##*/}

./scaffold.sh -c $1

scripts/simulation/scaffassert.py $1
# ./scaffold.sh -s $1
# # echo ${source%.scaffold}
#
# # map
# rm ${source%.scaffold}.csv
# for i in `seq 1 1`;
# do
#   ../qx_simulator_linux_x86_64/qx_simulator_1.0.beta_linux_x86_64 ${source%.scaffold}.qc | scripts/simulation/remove_global_phase.py ${source%.scaffold}.qc ${source%.scaffold}.csv
# done
#
# # reduce
# # scripts/simulation/assert_uniform.py ${source%.scaffold}.csv 4
# # scripts/simulation/assert_integer.py ${source%.scaffold}.csv 5 30
# scripts/simulation/assert_product.py ${source%.scaffold}.csv
