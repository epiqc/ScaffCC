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
opts.add_read_specification_option() \
    .add_write_realization_option() \
    .add_costs_option() \
    .add_option( "print", "prints the circuit" ) \
    .add_option( "max_depth", 100, "Maximum number of considered circuit gates (depth)." ) \
    .add_option( "spec_incremental", False, "Solve in spec incremental model.") \
    .add_option( "swop", 0, "0: No SWOP\n1: Exhaustive SWOP\n2: Heuristic SWOP" )

opts.parse( sys.argv )

if not opts.good():
    print opts
    exit()

circ = circuit()
tt = binary_truth_table()

read_specification( tt, opts.read_specification_filename() ) or sys.exit ("Cannot read " + opts.read_specification_filename())


r = swop( circ, tt, \
              enable = opts["swop"] > 0, \
              exhaustive = opts["swop"] == 1, \
              synthesis = exact_synthesis_func( solver = "MiniSAT", max_depth = opts["max_depth"], spec_incremental = opts["spec_incremental"] ), \
              cf = opts.costs() )


if type(r) == dict:
  if opts.is_set( "print" ):
    print_circuit( circ, print_inputs_and_outputs = True )
  if opts.is_write_realization_filename_set():
    write_realization( circ, opts.write_realization_filename() )
  print_statistics( circ, r["runtime"] )
else:
  print r
