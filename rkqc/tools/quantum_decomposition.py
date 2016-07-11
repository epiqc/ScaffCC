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
opts.add_option( "working_line_input", "w", "Input name of the working line, if added" ) \
    .add_option( "working_line_output", "w", "Output name of the working line, if added" )

opts.parse( sys.argv )

if not opts.good():
    print opts
    exit( 1 )

circ = circuit()
read_realization( circ, opts.read_realization_filename() )

newcirc = circuit()

r = quantum_decomposition( newcirc, circ, \
                        helper_line_input = opts["working_line_input"], \
                        helper_line_output = opts["working_line_output"] )

if opts.is_write_realization_filename_set():
    header = "This file has been generated using RevKit %s (www.revkit.org)\nCommand Line:\n%s\nBased on the approach proposed in D. Maslov and G. W. Dueck, Improved quantum cost for n-bit Toffoli gates, In Electronic Letters 39(25), pages 1790-1791, 2003 and A. Barenco, C. H. Bennett, R. Cleve, D. P. DiVincenzo, N. Margolus, P. Shor, T. Sleator, J. A. Smolin, and H. Weinfurter, Elementary gates for quantum computation, In Physical Review A 52(5), pages 3457-3467, 1995." % ( revkit_version(), " ".join( sys.argv ) ) 
    write_realization( newcirc, opts.write_realization_filename(), header = header )

print "Reversible Circuit:"
print_statistics( circ )

print
print "Quantum Circuit:"
print_statistics( newcirc )
