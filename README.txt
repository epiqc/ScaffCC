Contents:
=========

Getting Started - How to get access and download the repo
Running Scaffold - How to compile, install and run Scaffold
Using Git - A brief guide to the important git commands

Getting ScaffCC:
=================

1. Go to https://github.com/ajavadia/ScaffCC
2. Download the repository:
   % git clone https://github.com/ajavadia/ScaffCC.git [dir]

Building ScaffCC:
==================

First you need to install the following:
1. binutils-dev binutils-gold
    You can check if you have this now by doing `ld -v' and if it says 'GNU gold' you have it
    You can get this with `sudo apt-get install binutils-gold'
1. GCC 4.5 or higher
    NOTE: if you need to preserve an older build, consider using `update-alternatives' as system-wide method
    for preserving and maintaining these.
2. Boost 1.48
    Boost installation instructions are here: http://www.boost.org/doc/libs/1_48_0/doc/html/bbv2/installation.html
    % wget http://sourceforge.net/projects/boost/files/boost/1.48.0/boost_1_48_0.tar.gz
    % tar zxf boost_1_48_0.tar.gz
    % sudo ./bootstrap.sh --layout=tagged --threading=multi
    % sudo ./b2
    % sudo ./bjam install --prefix=/usr/local
3. The GNU Multiple Precision Arithmetic Library (gmp and gmpxx)
    Use tab-completion to verify the correct packages (e.g. libgmp<tab><tab>)
    % sudo apt-get install libgmp-dev libgmpxx4ldbl
4. The GNU MPFR Library (mpfr)
    % sudo apt-get install libmpfr-dev


Compiling
---------

Once you have all of the required libraries, simply run `make' or `make 
USE_GCC=1' at the root of the repository. The USE_GCC flag will force the 
Makefile to use GCC to compile instead, and this has been seen to be faster 
on some systems.

Running
-------

Prior to the first run, please select the variation of QASM that you want by 
doing the following:

% cd <repo>/scaffold/
% cp flatten-qasm-<usc|qcs>.py flatten-qasm.py
% cd -

Run the 'scaffold.sh' command with the path and name of the scaffold file to
compile.

Example:
% ./scaffold.sh Algorithms/Binary Welded Tree/Binary_Welded_Tree.scaffold

Using Git:
==========

Notes: All of these commands are executed in the Git repo directory (or a 
subdirectory) on your local machine

Syncing to latest repo resources
    % git fetch
    % git merge
    -- or --
    % git pull

Committing changes
    # Stage all modified/deleted files for commit
    % git add -u
    % git commit
    # Push changes to remote repository (make available for everyone)
    % git push

Creating a branch (use for local or experimental changes)
    % git checkout -b <new_branch>

Switching between branches
    % git checkout <existing_branch>

Listing local branches
    % git branch

Merging to the 'master' branch (main trunk) after committing changes in your 
local branch
    % git checkout master
    % git merge <my_local_branch>
    % git commit
    % git push

References:
http://gitref.org/
http://jonas.nitro.dk/git/quick-reference.html
