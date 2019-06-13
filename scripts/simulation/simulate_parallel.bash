#!/bin/bash

export SCAFFCC_PATH=/n/fs/qdb/ScaffCC
TEST_NAME=${1%.scaffassert}

# CLEANUP
$SCAFFCC_PATH/scaffold.sh -c $1
rm *.breakpoint_*.scaffold
rm sbatch.bash
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
ENSEMBLE=1

# compilation without slurm:
# for ((i=1;i<=$BREAKPOINTS;i++))
# do
#   $SCAFFCC_PATH/scaffold.sh -s $TEST_NAME.breakpoint_$i.scaffold
# done

# get QX Simulator
if [ ! -e $SCAFFCC_PATH/../qx_simulator_linux_x86_64/qx_simulator_1.0.beta_linux_x86_64 ]
then
    wget -nc -P $SCAFFCC_PATH/../ http://quantum-studio.net/qx_simulator_linux_x86_64.tar.gz
    tar -xvf $SCAFFCC_PATH/../qx_simulator_linux_x86_64.tar.gz -C $SCAFFCC_PATH/../
fi

SBATCH_SCRIPT="sbatch.bash"
/bin/cat <<EOM >$SBATCH_SCRIPT
#!/bin/bash

#SBATCH --job-name=$TEST_NAME.simulate
#SBATCH --time=00:01:00

#SBATCH --mail-type=ALL
#SBATCH --mail-user=yipengh@cs.princeton.edu

echo "HEAD =" $HOSTNAME
echo "LOCAL =" \$HOSTNAME
echo "SLURM_ARRAY_JOB_ID =" \$SLURM_ARRAY_JOB_ID
echo "SLURM_ARRAY_TASK_ID =" \$SLURM_ARRAY_TASK_ID
echo "SLURM_CLUSTER_NAME =" \$SLURM_CLUSTER_NAME
echo "SLURM_CPUS_PER_TASK =" \$SLURM_CPUS_PER_TASK
echo "SLURM_JOB_ACCOUNT =" \$SLURM_JOB_ACCOUNT
echo "SLURM_JOB_ID =" \$SLURM_JOB_ID
echo "SLURM_JOB_NAME =" \$SLURM_JOB_NAME
echo "SLURM_JOB_NODELIST =" \$SLURM_JOB_NODELIST
echo "SLURM_JOB_NUM_NODES =" \$SLURM_JOB_NUM_NODES
echo "SLURM_JOB_PARTITION =" \$SLURM_JOB_PARTITION
echo "SLURM_JOB_UID =" \$SLURM_JOB_UID
echo "SLURM_JOB_USER =" \$SLURM_JOB_USER
echo "SLURM_STEP_ID =" \$SLURM_STEP_ID

######################
# Begin work section #
######################

SCRATCH_PATH=/scratch/$USER/$TEST_NAME.breakpoint_\$SLURM_ARRAY_TASK_ID
mkdir -p \$SCRATCH_PATH
cd \$SCRATCH_PATH

# COMPILE
# compile Scaffold to QASM to QX input
$SCAFFCC_PATH/scaffold.sh -s \$SLURM_SUBMIT_DIR/$TEST_NAME.breakpoint_\$SLURM_ARRAY_TASK_ID.scaffold
cp $TEST_NAME.breakpoint_\$SLURM_ARRAY_TASK_ID.qc \$SLURM_SUBMIT_DIR
cd \$SLURM_SUBMIT_DIR

# noise model
# error_model depolarizing_channel, 0.001

# SIMULATE
# map
srun \
--ntasks=$ENSEMBLE \
--output=$TEST_NAME.breakpoint_%a.trial_%n.%A.%N.out \
--error=$TEST_NAME.breakpoint_%a.trial_%n.%A.%N.err \
$SCAFFCC_PATH/../qx_simulator_linux_x86_64/qx_simulator_1.0.beta_linux_x86_64 $TEST_NAME.breakpoint_\$SLURM_ARRAY_TASK_ID.qc

# reduce
$SCAFFCC_PATH/scripts/simulation/register_value_csv.py \
$TEST_NAME.breakpoint_\$SLURM_ARRAY_TASK_ID.qc \
$TEST_NAME.breakpoint_\$SLURM_ARRAY_TASK_ID.csv

bash $TEST_NAME.breakpoint_\$SLURM_ARRAY_TASK_ID.bash

# scripts/simulation/assert_uniform.py ${source%.scaffold}.csv 4
# scripts/simulation/assert_integer.py ${source%.scaffold}.csv 5 30
# scripts/simulation/assert_product.py ${source%.scaffold}.csv

# users are expected to clean up their files at the end of each job
rm -r \$SCRATCH_PATH

EOM

sbatch \
--array=1-$BREAKPOINTS \
-N $((ENSEMBLE)) \
--output=$TEST_NAME.breakpoint_%a.compile.%A.%N.out \
--error=$TEST_NAME.breakpoint_%a.compile.%A.%N.err \
$SBATCH_SCRIPT
