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

import sys , os
sys.path.append(os.path.dirname(sys.path[0]))
from revkit import *

opts = program_options()
opts.add_read_realization_option()
opts.add_write_realization_option()
opts.add_option( "synthesis", 0, "Synthesis Approach:\n 0: Transformation Based Synthesis\n 1: Exact Synthesis" ) \
    .add_option( "max_window_lines", 6, "Initial maximum size for windows to be considered" ) \
    .add_option( "max_grow_up_window_lines", 9, "If no valid truth table can be found for max_window_lines size windows, they can grow up to this size" ) \
    .add_option( "window_variables_threshold", 17, "Number of primary inputs for pre-window-simulation which still allow the use of simple simulation. Afterwards, SMT - based simulation is used." ) \
    .add_option( "timeout", 0, "Timeout for window synthesis") 

opts.parse( sys.argv )

if not opts.good():
    print opts
    exit( 1 )

circ = circuit()
read_realization( circ, opts.read_realization_filename() ) or sys.exit ("Cannot read " + opts.read_realization_filename())

newcirc = circuit()

if opts["synthesis"] == 0:
    synthesis = embed_and_synthesize( synthesis = transformation_based_synthesis_func(), timeout = opts["timeout"] )
else:
    synthesis = embed_and_synthesize( synthesis = exact_synthesis_func(), timeout = opts["timeout"] )

r = line_reduction( newcirc, circ, \
                        max_window_lines = opts["max_window_lines"], \
                        max_grow_up_window_lines = opts["max_grow_up_window_lines"], \
                        window_variables_threshold = opts["window_variables_threshold"], \
                        window_synthesis = synthesis )

if opts.is_write_realization_filename_set():
    header = "This file has been generated using RevKit %s (www.revkit.org)\nCommand Line:\n%s\nBased on the approach proposed in R. Wille, M. Soeken, and R. Drechsler, Reducing the Number of Lines in Reversible Circuits, Design Automation Conference, pages 647-652, 2010." % ( revkit_version(), " ".join( sys.argv ) ) 
    write_realization( newcirc, opts.write_realization_filename(), header = header )

print "Original Circuit:"
print_statistics( circ )

print
print "Optimized Circuit:"
print_statistics( newcirc, runtime = r['runtime'], main_template = print_statistics_settings().main_template \
                           + "\nConsidered Windows:               " + str( r['num_considered_windows'] ) \
                           + "\nNo suiteable constant line found: " + str( r['skipped_no_constant_line'] ) \
                           + "\nSkipped due to ambiguous lines:   " + str( r['skipped_ambiguous_line'] ) \
                           + "\nSkipped due to max window lines:  " + str( r['skipped_max_window_lines'] ) \
                           + "\nSkipped due to synthesis fail:    " + str( r['skipped_synthesis_failed'] ) + "\n")
