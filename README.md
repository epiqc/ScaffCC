***Important Note:*** starting with version 4.0, the argument orders of `CNOT` and `Toffoli` have been changed and standardized to `CNOT(control, target)` and `Toffoli(control1, control2, target)` respectively. Previously, the target argument was first. All of the benchmarks (in [Algorithms/](https://github.com/epiqc/ScaffCC/tree/master/Algorithms)) now conform to the new ordering. Also starting with version 4.0, rotation decomposition and Toffoli decomposition are disabled by default. They can be enabled with the `-R` and `-T` flags.


What Is ScaffCC?
================
ScaffCC is a compiler and scheduler for the Scaffold programing language. It is written using the LLVM open-source infrastructure. It is for the purpose of writing and analyzing code for quantum computing applications.

ScaffCC enables researchers to compile quantum applications written in Scaffold to a low-level quantum assembly format (QASM), apply error correction, and generate time and area metrics. It is written to be scalable up to problem sizes in which quantum algorithms outperform classical ones, and as such provide valuable insight into the overheads involved and possible optimizations for a realistic implementation on a future device technology.

If you use ScaffCC in your publications, please cite this work as follows:

> Ali JavadiAbhari, Shruti Patil, Daniel Kudrow, Jeff Heckey, Alexey Lvov, Frederic Chong and Margaret Martonosi, ScaffCC: A Framework for Compilation and Analysis of Quantum Computing Programs, ACM International Conference on Computing Frontiers (CF 2014), Cagliari, Italy, May 2014


Release Information
===================

Current Release
---------------

-   Version 4.0

-   Release Date: June 28, 2018

Supported Operating Systems
---------------------------

ScaffCC currently offers support for the following operating systems:

-   “Ubuntu"

-   “Red Hat"

-   "OS X"

This list will continue to grow in the future!

Installation 
============

**Note**: if you have trouble setting up ScaffCC, you can use our [Docker](https://www.docker.com/) image instead. Simply install Docker and run `docker pull epiqc/scaffcc`.

Getting ScaffCC
---------------

1.  Go to https://github.com/epiqc/ScaffCC

2.  For the **Unix build**, download the repository:

           git clone https://github.com/epiqc/ScaffCC.git [dir]

3.  For the **OS X build**, download the repository:

           git clone -b ScaffCC_OSX https://github.com/epiqc/ScaffCC.git [dir]

Building ScaffCC
----------------

### Prerequisites

#### For OS X build

1.  If not already installed, Xcode command line tool is required in order to build ScaffCC. 

            xcode-select --install

2.  Python 2.7: Python is usually bundled with OS X. The built-in version should be sufficient.
 
#### For Unix build

First you need to install the following dependencies. For each one, you
can either install by building from source, or use the package manager
of your system (“yum" on Red Hat or “apt-get" on Ubuntu).

1.  Static libraries for libstdc++ and glibc

    -   “Ubuntu": Install GNU gold linker

        You can check if you have this now by doing ‘ld -v’ and if it
        says ‘GNU gold’ you have it

                sudo apt-get install binutils-gold
                

    -   “Red Hat"

                sudo yum install libstdc++-static 
                sudo yum install glibc-static
                

2.  GCC 4.5 or higher: if you need to preserve an older build,
    consider using ‘update-alternatives’ as system-wide method for
    preserving and maintaining these.

3.  Clang++ 3.5 or higher

4.  Boost 1.48

    -   “Source Build": Boost installation instructions are here:
        <http://www.boost.org/doc/libs/1_48_0/doc/html/bbv2/installation.html>

                wget http://sourceforge.net/projects/boost/files/boost/1.48.0/boost_1_48_0.tar.gz
                tar zxf boost_1_48_0.tar.gz && cd boost_1_48_0/
                sudo ./bootstrap.sh
                sudo ./b2 install --prefix=/usr/local
                

        Alternatively, Ubuntu users can install the current Boost
        version via:

                sudo apt-get install libboost-all-dev
                

5.  The GNU Multiple Precision Arithmetic Library (gmp and gmpxx)

    -   “Ubuntu": Use tab-completion to verify the correct packages

                sudo apt-get install libgmp-dev libgmpxx4ldbl
                

    -   “Source Build"

                wget https://ftp.gnu.org/gnu/gmp/gmp-6.0.0a.tar.bz2
                sudo ./configure --disable-shared --enable-static --enable-cxx
                sudo make && sudo make check && sudo make install
                

6.  The GNU MPFR Library (mpfr)

    -   “Ubuntu"

                sudo apt-get install libmpfr-dev
                

    -   “Source Build"

                wget http://www.mpfr.org/mpfr-current/mpfr-3.1.4.tar.bz2
                sudo ./configure --disable-shared --enable-static
                sudo make && sudo make check && sudo make install
                

7.  Python 2.7

8.  CMake (For Integrating RKQC Functionality)

    -   “Ubuntu"

                sudo apt-get install cmake
                

    -   “Source Build"

        There are instructions for downloading and building CMake from
        source at:
        <https://cmake.org/install>

### Installing
Once you have all of the required libraries, simply run

        make

or

        make USE_GCC=1

at the root of the repository. The `USE_GCC` flag will force the
Makefile to use GCC to compile instead, and this has been seen to be
faster on some systems.

Verifying Installation
----------------------

Included with ScaffCC is a suite of tests designed to verify the
integrity of new installations. To access these tests and verify that
the installation process completed successfully:

1.  Enter the scripts/ directory:

                cd scripts/
            

2.  Run the Regression Test Suite:

                ./regression_test.sh
            

This invocation will compile three small benchmarks, and verify that the
generated files match those precompiled on an existing system, which are
included in the test cases directory. If the three tests complete with a
“Succeeded", the installation was successful.

Using ScaffCC 
=============

Types
-----

In programming a quantum algorithm, the Scaffold programming language now offers three distinct data types to the user: qubit, abit, and cbit. The first of these is the traditional qubit, for use in computation and throughout applications. Abit types are specifically "ancilla qubits", or those qubits that are used as intermediary steps in computation. To this end, the function afree() has been provided, which allows for ancilla bits (abits) to be freed explicitly by the programmer. This allows programmers to explicitly conserve usage of ancilla bits throughout their algorithm. The last of these is cbit, for the classical bit of information.


Running the Compiler
--------------------

To run the compiler, simply use the `scaffold.sh` script in the main
directory, with the name of the program and optional compiler flags.

### Compiler Options

To see a list of compiler options which can be passed as flags, run:

    ./scaffold.sh -h

    Usage: ./scaffold.sh [-hv] [-rqfRFckdso] [-l #] [-P #] <filename>.scaffold
        -r   Generate resource estimate (default)
        -q   Generate QASM
        -f   Generate flattened QASM
        -b   Generate OpenQASM
        -R   Enable rotation decomposition
        -T   Enable Toffoli decomposition
        -l   Levels of recursion to run (default=1)
        -P   Set precision of rotation decomposition in decimal digits (default=10)
        -F   Force running all steps
        -c   Clean all files (no other actions)
        -k   Keep all intermediate files (default only keeps specified output,
             but requires recompilation for any new output)
        -d   Dry-run; show all commands to be run, but do not execute
        -s   Generate QX Simulator input file 
        -o   Generate optimized QASM
        -v   Show current ScaffCC version

### Basic Example:

The command below runs the compiler with default options on the Binary
Welded Tree algorithm, with n=100 and s=100 as problem sizes. The
default compiler option is to generate resource estimations (number of
qubits and gates).

    ./scaffold.sh Algorithms/Binary_Welded_Tree/binary_welded_tree.n100s100.scaffold

Sample Scripts
--------------

This section describes some of the example scripts contained in the ‘scripts/’ directory. They are written to test the various functionalities of ScaffCC, as detailed below. Each of them automates the process of running multiple compiler passes on an input file to perform the required analysis or optimization.

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

Built-in Quantum Applications 
=============================

This section describes the apps provided with this software, in the
‘Algorithms/’ directory.

1.  Cat-State Preparation: Prepares an n-bit quantum register in the
    maximally entangled Cat-State. The app is parameterized by n.

2.  Quantum Fourier Transform (QFT): Performs quantum Fourier transform
    on an n-bit number. The app is parameterized by n.

3.  Square Root: Uses a quantum concept called *amplitude amplification*
    to find the square root of an n-bit number with the Grover’s
    search technique. The app is parameterized by n.

4.  Binary Welded Tree: Uses quantum random walk algorithm to find a
    path between an entry and exit node of a binary welded tree
    . The app is parameterized by the height of the tree (n)
    and a time parameter (s) within which to find the solution.

5.  Ground State Estimation: Uses quantum phase estimation algorithm to
    estimate the ground state energy of a molecule. The app
    is parameterized by the size of the molecule in terms of its
    molecular weight (M).

6.  Triangle Finding Problem: Finds a triangle within a dense,
    undirected graph. The app is parameterized by the number
    of nodes n in the graph.

7.  Boolean Formula: Uses the quantum algorithm described
    in the citation in the full documentation, to compute a winning strategy for the
    game of Hex. The app is parameterized by size of the Hex board
    (x,y).

8.  Class Number: A problem from computational algebraic number theory,
    to compute the class group of a real quadratic number field
    . The app is parameterized by p, the
    number of digits after the radix point for floating point numbers
    used in computation.

9.  Secure Hash Algorithm 1: An implementation of the reverse
    cryptographic hash function. The message is decrypted by
    using the SHA-1 function as the oracle in a Grovers
    search algorithm. The app is parameterized by the size of the
    message in bits (n).

10. Shor’s Factoring Algorithm: Performs factorization using the Quantum
    Fourier Transform. The app is parameterized by n, the size
    in bits of the number to factor.

QX: Quantum Computer Simulator
==============================

New compatibility has been added to allow Scaffold to compile algorithms down to files with acceptable formatting to act as inputs to the QX Quantum Computer Simulator by TU Delft. 

> Available at: http://qutech.nl/qx-quantum-computer-simulator/ 

Using the "-s" compile flag will tell Scaffold to compile the desired algorithm to flat QASM, and transform that QASM output into an acceptable format for the QX Simulator. Note: the simulator only supports up to tens of qubits at the moment (~30), and does not support specific primitive gates built into Scaffold. The QX Transform flag will emit a notification if these parameters are not satisfied, and the algorithm is unsuitable for simulation.

RKQC: RevKit For Quantum Computation 
====================================

RKQC is a compiler for reversible logic circuitry. The framework has
been developed to compile high level circuit descriptions down to
assembly language instructions, primarily for quantum computing
machines. Specifically, input files to the RKQC compiler contain
descriptions of reversible circuits, and the output files are the
assembly instructions for the circuit, in the “.qasm” format.


In many important quantum computing algorithms, a large portion of the
modules use only classical reversible logic operations that can be
decomposed into the universal set of NOT, CNOT, and Toffoli gates. Often
these are referred to as “classical oracles.” These oracles can also be
simulated on a conventional computer.


RKQC is used by the Scaffold quantum circuits compiler as a subroutine
for the compilation of purely classical reversible logic modules, or
oracles. It has also been designed to operate as a stand alone tool, and
can be used in this fashion. It was also developed as a full conversion
of the RevKit platform. 

The documentation describing installation and usage of RKQC is included
in the [docs/](https://github.com/epiqc/ScaffCC/tree/master/docs) directory, with the full documentation.

Expanding ScaffCC 
=================

ScaffCC is completely open-source and may be expanded to accomodate
future needs of quantum circuit analysis and optimization.

At the core of ScaffCC are the LLVM compiler passes, which operate on
LLVM IR (.ll) code. All LLVM passes are stored in:

    llvm/lib/Transforms/

Passes specific to ScaffCC are stored in:

    llvm/lib/Transforms/Scaffold

In general, to run a pass in LLVM, you invoke the opt program as
follows:

    build/Release+Asserts/bin/opt -S -load build/Release+Asserts/lib/Scaffold.so {pass_name} {input_ll_file} > {output_ll_file} 2> {log_file}

Note that "pass\_name" refers to the unique name of the pass by which
it is registered in the LLVM system, by invoking the following in the
implementation of the pass:

    static RegisterPass<{pass_name}> X({pass_name}, {description_of_functionality});

To write a new pass, start by looking at the previously implemented
examples in this directory, and consulting the LLVM Documentation:
<http://llvm.org/docs/WritingAnLLVMPass.html>
