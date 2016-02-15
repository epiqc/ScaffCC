=================================================================
                Scaffold Compiler Working Group
              https://github.com/ajavadia/ScaffCC

                  Software README Document
=================================================================

Contents:
=========

Intro - About this software
Getting ScaffCC - How to get access and download from the repo
Building ScaffCC - How to build the compiler
Running the Compiler - How to compile Scaffold programs


Intro:
======

ScaffCC is a compiler and scheduler for the Scaffold programing language. It is written using the LLVM open-source infrastructure.
It is for the purpose of writing and analyzing code for quantum computing applications.

Different parts of this software have been published in the following papers:
http://www.princeton.edu/~ajavadia/ScaffCC.pdf
http://mrmgroup.cs.princeton.edu/papers/QPE_IISWC13.pdf
http://www.princeton.edu/~ajavadia/ASPLOS15.pdf

To cite this software, please use the following:
A. JavadiAbhari et al. "ScaffCC: A Framework for Compilation and Analysis of Quantum Computing Programs," Computing Frontiers, 2014


Getting ScaffCC:
=================

1. Go to https://github.com/ajavadia/ScaffCC
2. Download the repository:
   % git clone https://github.com/ajavadia/ScaffCC.git [dir]


Building ScaffCC:
==================
First you need to install the following dependencies. For each one, you can either install by building from source,
or use the package manager of your system ("yum" on Red Hat or "apt-get" on Ubuntu).  

1. Static libraries for libstdc++ and glibc
  
    "Ubuntu"
    Install GNU gold linker
    You can check if you have this now by doing `ld -v' and if it says 'GNU gold' you have it
    % sudo apt-get install binutils-gold

    "Red Hat"
    % sudo yum install libstdc++-static 
    % sudo yum install glibc-static

2. GCC 4.5 or higher
    NOTE: if you need to preserve an older build, consider using `update-alternatives' as system-wide method
    for preserving and maintaining these.

3. Boost 1.48
    
    "Source Build"
    Boost installation instructions are here: http://www.boost.org/doc/libs/1_48_0/doc/html/bbv2/installation.html
    % wget http://sourceforge.net/projects/boost/files/boost/1.48.0/boost_1_48_0.tar.gz
    % tar zxf boost_1_48_0.tar.gz && cd boost_1_48_0/
    % sudo ./bootstrap.sh
    % sudo ./b2 install --prefix=/usr/local


4. The GNU Multiple Precision Arithmetic Library (gmp and gmpxx)
    
    "Ubuntu"
    Use tab-completion to verify the correct packages (e.g. libgmp<tab><tab>)
    % sudo apt-get install libgmp-dev libgmpxx4ldbl

    "Source Build"
    % wget https://ftp.gnu.org/gnu/gmp/gmp-6.0.0a.tar.bz2
    % sudo ./configure --disable-shared --enable-static --enable-cxx
    % sudo make && sudo make check && sudo make install

5. The GNU MPFR Library (mpfr)
    
    "Ubuntu"
    % sudo apt-get install libmpfr-dev

    "Source Build"
    % wget http://www.mpfr.org/mpfr-current/mpfr-3.1.2.tar.bz2
    % sudo ./configure --disable-shared --enable-static
    % sudo make && sudo make check && sudo make install
      

Once you have all of the required libraries, simply run `make' or `make 
USE_GCC=1' at the root of the repository. The USE_GCC flag will force the 
Makefile to use GCC to compile instead, and this has been seen to be faster 
on some systems.

Running the Compiler:
=====================

Run the 'scaffold.sh' command with the path and name of the scaffold file to
compile.

Example:
% ./scaffold.sh Algorithms/Binary Welded Tree/Binary_Welded_Tree.scaffold

Scripts:
========
A number of example scripts have been provided in the ./scripts/ directory.
You can use these to do understand how to do a number of common things with the compiler,
such as generating intermediary .ll files, finding critical path lengths, and scheduling for architectures.
The README.txt file in that directory contains further instructions.
