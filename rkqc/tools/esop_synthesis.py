#!/home/adam/Desktop/EPiQC_Releases/Full_Install_Testing/July10_ScaffCC/rkqc/python
#!/home/adam/Desktop/EPiQC_Releases/Full_Install_Testing/EPiQC_Scaffold/rkqc/python
#!/home/adam/Desktop/EPiQC_Releases/Full_Install_Testing/EPiQC_Scaffold/rkqc/python
#!/home/adam/Desktop/Development/RKQC/python
#!/home/adam/New_Installs/Scaffold_New/rkqc/python
#!/home/adam/Documents/revkit-1.3/python
#!/usr/bin/python

# RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
# Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import sys, os
sys.path.append(os.path.dirname(sys.path[0]))
from revkit import *

opts = program_options()
opts.add_write_realization_option() \
    .add_option( "filename", "filename to the esop file") \
    .add_option ("separate_polarities", False, "False: Basic approach\nTrue: Using both literals (no reorering required)") \
    .add_option ("reordering", 1, "0: No Reordering\n1: Weigthted reordering")  \
    .add_option( "print", "prints the circuit" ) 

opts.parse( sys.argv )

if not opts.good() or not opts.is_set("filename"):
    print opts
    exit()

circ = circuit()

reordering = no_reordering()

if opts["reordering"] == 1:
  reordering = weighted_reordering()

r = esop_synthesis ( circ, opts["filename"], \
    separate_polarities = opts["separate_polarities"], \
    reordering = reordering )

if type(r) == dict:
  if opts.is_set( "print" ):
    print circ

  if opts.is_write_realization_filename_set():
    header = "This file has been generated using RevKit %s (www.revkit.org)\nCommand Line:\n%s\nBased on the approach proposed in K. Fazel, M. A. Thornton, and J. E. Rice, ESOP-based Toffoli Gate Cascade Generation, In IEEE Pacific Rim Conference on Communications, Computers and Signal Processing, pages 206-209, 2007." % ( revkit_version(), " ".join( sys.argv ) ) 
    write_realization( circ, opts.write_realization_filename(), header = header )

  print_statistics( circ, r["runtime"] )
else:
  print r
