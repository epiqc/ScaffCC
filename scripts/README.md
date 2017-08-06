Scaffold Compiler Working Group 
===============================

(https://github.com/epiqc/ScaffCC)

Contents:
---------

  Example scripts to test some of ScaffCC's functionalities.

Important Note:
---------------

  The assumption in these scripts is that they are separated from the ScaffCC directory by only one directory level. Therefore let the scripts remain in ScaffCC/scripts

### 1. Generating LLVM Intermediate Format: ./gen-ll.sh

Lowers .scaffold source file to .ll file (intermediary LLVM format).
Creates &lt;algorithm&gt;.ll The .ll file is the main file in LLVM on
which all transformations, optimizations and analysis are performed.

### 2. Critical Path Estimation: ./gen-cp.sh

Finds critical path information for several different flattening
thresholds by doing the following:

1.  Finding module sizes using the ResourceCount2 pass.

2.  Flattening modules based on the found module sizes and the
    requested threshold.

3.  Finds length of critical path, in terms of number of operations
    on it. Look for the number in front of “main” in the output.

#### flattening\_thresh.py

Divides modules into different buckets based on their size, to be used
for flattening decision purposes.

### 3. Module Call Frequency Estimation: ./gen-freq-estimate.sh

Generates an estimate of how many times each module is called, which can
guide flattening decisions.

### 4. Generate Longest-Path-First-Schedule (LPFS): ./gen-lpfs.sh

Generates LPFS schedules with different options as specified below.

Options in the script: K=number of SIMD regions / D=capacity of each
region / th=flattening thresholds

Calls the following scripts:

#### ./regress.sh

Runs the 3 different communication-aware schedulers, LPFS, RCP, SS, with
different scheduler configurations. Look in ./sched.pl for configuration
options. For example using -m gives metrics only, while -s outputs
entire schedule.

#### ./sched.pl

The main scheduler code for LPFS and RCP.

#### ./comm\_aware.pl

Applies the communication penalty to timesteps.

All output files are placed in a new directory to avoid cluttering.

### 5. Rotation Generator: gen\_rotation/

This is a C++ implemetation of the *library construction* method for generating *Rz* rotations. The main features of this generator include:
  - Powered by [gridsynth](http://www.mathstat.dal.ca/~selinger/newsynth/), the package can generate rotation sequences that approximate arbitray *Rz* angles, up to given precision.
  - Generate libraries of rotation sequences given use-defined precision and storage requirements, trading storage for execution time.
  - Dynamically concatenate rotation sequences at run time using generated libraries.

Detailed description and usage can be found in the subdirectory [scripts/gen_rotations/](https://github.com/epiqc/ScaffCC/tree/master/scripts/gen_rotations).

### 6. Test Correctness of RKQC Programs: RKQCVerifier/

RKQCVerifier is a tool that helps verify the correctness of RKQC applications. For detailed description and usage, please refer to the subdirectory [scripts/RKQCVerifier/](https://github.com/epiqc/ScaffCC/tree/master/scripts/RKQCVerifier).

### 7. Braidflash

Braidflash is a software for efficient simulation of braids in the context of surface error correction of quantum applications. For further explanation and to cite this tool, please refer to the following publication:

> A. Javadi-Abhari, P. Gokhale, A. Holmes, D. Franklin, K. R. Brown, M. R. Martonosi, F. T. Chong, “Optimized Surface Code Communication in Superconducting Quantum Computers,” IEEE/ACM MICRO, Cambridge, MA, 2017

Detailed description and usage can be found in the subdirectory [braidflash/](https://github.com/epiqc/ScaffCC/tree/master/braidflash).