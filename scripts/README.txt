=================================================================
                Scaffold Compiler Working Group
              https://github.com/ajavadia/ScaffCC

                    Scripts README Document
=================================================================

Contents:
=========
  example scripts to test some of ScaffCC's functionalities.
  please direct any questions/feedback to ajavadia@princeton.edu

Important Note:
===============
  The assumption in these scripts is that they are separated from the ScaffCC directory by only one directory level.
  Therefore let the scripts remain in ScaffCC/scripts

-----------------------------------------------------------------
$ cat file_list
To automate the process, put the name of any quantum algorithms you wish to test in this file (one per line).
All scripts will be executed on each of them.
With the current directory setup the .scaffold file should be present in the scripts directory, so copy it over from ../Algorithms/

$ bash gen-ll.sh
Lowers .scaffold source file to .ll file (intermediary LLVM format). Creates <algorithm>.ll
The .ll file is the main file in LLVM on which all transformations, optimizations and analysis are performed.
In general, to run a pass in LLVM, you call the opt program as follows:
    build/Release+Asserts/bin/opt -S -load build/Release+Asserts/lib/Scaffold.so <desired_pass_name> <input_ll_file> > <output_ll_file> 2> <log_file>
    (The following script basically automates the above command for several passes)

$ bash gen-cp.sh
Finds critical path information for several different flattening thresholds by doing the following:
1- Finding module sizes using the ResourceCount2 pass.
2- Inlining modules based on the found module sizes and the requested threshold (gen-flattening-files.py is a helper to this).
3- Finds length of critical path, in terms of number of operations on it. Look for the number in front of "main" in the output. 

$ bash 
