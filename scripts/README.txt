=================================================================
                Scaffold Compiler Working Group
              https://github.com/epiqc/ScaffCC

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

=================================================================


$ ./gen-ll.sh
-------------
Lowers .scaffold source file to .ll file (intermediary LLVM format). Creates <algorithm>.ll
The .ll file is the main file in LLVM on which all transformations, optimizations and analysis are performed.

NB: In general, to run a pass in LLVM, you invoke the opt program as follows:
    build/Release+Asserts/bin/opt -S -load build/Release+Asserts/lib/Scaffold.so <desired_pass_name> <input_ll_file> > <output_ll_file> 2> <log_file>
    (These scripts basically automate the above command for several passes)


$ ./gen-cp.sh
-------------
Finds critical path information for several different flattening thresholds by doing the following:
1- Finding module sizes using the ResourceCount2 pass.
2- Flattening modules based on the found module sizes and the requested threshold.
   flattening_thresh.py: divides modules into different buckets based on their size, to be used for flattening decision purposes.
3- Finds length of critical path, in terms of number of operations on it. Look for the number in front of "main" in the output. 


$ ./gen-scheds.sh 
-----------------
This is the wrapper script around all the different schedulers.
Generates communication-unaware Multi-SIMD schedules and commnication-aware LPFS, RCP and SS scheudles.
Options: 
  K=number of SIMD regions / D=capacity of each region
Calls the following scripts:
  
  $ ./regress.sh
  --------------
  Runs the 3 different communication-aware schedulers, LPFS, RCP, SS, with different scheduler configurations.
  Look in ./sched.pl for configuration options. For example using -m gives metrics only, while -s outputs entire schedule.

  $ ./sched.pl
  ------------
  The main scheduler code for LPFS and RCP.

  $ ./leaves.pl
  -------------
  Finds leaf (i.e. flat) modules.

  $ ./comm_aware.pl
  -----------------
  Applies the communication penalty to timesteps.

All output files are placed in a new directory to avoid cluttering.
