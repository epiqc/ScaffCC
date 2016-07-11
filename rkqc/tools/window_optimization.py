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
opts.add_read_realization_option()
opts.add_write_realization_option()
opts.add_costs_option()
opts.add_option( "select_window", 0, "0: Shift Window Selection (SWS)\n1: Line Window Selection (LWS)" ) \
    .add_option( "window_length", 10, "With SWS, this determines the length of the considered window" ) \
    .add_option( "offset", 1, "With SWS, this determines the offset by which the window is shifted" )

opts.parse( sys.argv )

if not opts.good():
    print opts
    exit( 1 )

circ = circuit()
read_realization( circ, opts.read_realization_filename() ) or sys.exit ("Cannot read " + opts.read_realization_filename())

newcirc = circuit()

r = window_optimization( newcirc, circ, \
                         select_window = shift_window_selection_func( window_length = opts["window_length"], offset = opts["offset"] ) if opts["select_window"] == 0 else line_window_selection_func(), \
                         cf = opts.costs() )

if type(r) == dict:
    if opts.is_write_realization_filename_set():
        write_realization( newcirc, opts.write_realization_filename() )

    print "Original Circuit:"
    print_statistics( circ )
    
    print
    print "Optimized Circuit:"
    print_statistics( newcirc, r["runtime"] )
else:
    print r
