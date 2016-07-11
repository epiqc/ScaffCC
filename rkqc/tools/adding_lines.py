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
opts.add_option( "additional_lines", 1, "Number of additional helper lines" )

opts.parse( sys.argv )

if not opts.good():
    print opts
    exit( 1 )

circ = circuit()
read_realization( circ, opts.read_realization_filename() ) or sys.exit ("Cannot read " + opts.read_realization_filename())

newcirc = circuit()

r = adding_lines( newcirc, circ, \
                      additional_lines = opts["additional_lines"] )

if type(r) == dict:
    if opts.is_write_realization_filename_set():
        header = "This file has been generated using RevKit %s (www.revkit.org)\nCommand Line:\n%s\nBased on the approach proposed in D. M. Miller, R. Wille, and R. Drechsler, Reducing Reversible Circuit Cost by Adding Lines, International Symposium on Multiple-Valued Logic, pages 647, 2010." % ( revkit_version(), " ".join( sys.argv ) ) 
        write_realization( newcirc, opts.write_realization_filename(), header = header )

    print "Original Circuit:"
    print_statistics( circ )

    print
    print "Optimized Circuit:"
    print_statistics( newcirc, r["runtime"] )
else:
    print r
