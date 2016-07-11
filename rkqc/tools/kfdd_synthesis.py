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
opts.add_write_realization_option()
opts.add_option( "filename", "PLA or BLIF filename (must have the corresponding extension *.pla or *.blif)" ) \
    .add_option( "default_decomposition", 0, "Default Decomposition:\n0: Shannon\n1: positive Davao\n2: negative Davio" ) \
    .add_option( "reordering", 7, "Reordering Strategy:\n0: No Reordering\n1: Exact for DTL and Order (Friedman)\n2: Exact for DTL and Order (Permutation)\n3: Sifiting for DTL and Order\n4: Exact for Order (Friedman)\n5: Exact for Order (Permutation)\n6: Sifting for Order\n7: Sifting for Order (6) followed by Sifting for DTL and Order (3)\n8: Inversion" ) \
    .add_option( "dotfilename", "[Statistics] Dump DOT representation of BDD to this file" )
opts.parse( sys.argv )

if not opts.good() or not opts.is_set( "filename" ):
    print opts
    exit( 1 )

circ = circuit()

dotfilename = ""
if opts.is_set( "dotfilename" ): dotfilename = opts["dotfilename"]

r = kfdd_synthesis( circ, opts["filename"], \
                        default_decomposition = opts["default_decomposition"], \
                        reordering = opts["reordering"], \
                        dotfilename = dotfilename )
#                        sift_factor = opts["sift_factor"], \
#                        sifting_growth_limit = opts["sifting_growth_limit"], \
#                        sifting_method = opts["sifting_method"] )

if opts.is_write_realization_filename_set():
    header = "This file has been generated using RevKit %s (www.revkit.org)\nCommand Line:\n%s\nBased on the approach proposed in M. Soeken, R. Wille, and R. Drechsler. Hierarchical synthesis of reversible circuits using positive and negative Davio decomposition. In Workshop on Reversible Computation, 2010." % ( revkit_version(), " ".join( sys.argv ) )
    write_realization( circ, opts.write_realization_filename() , header = header )

print_statistics( circ, runtime = r["runtime"], main_template = print_statistics_settings().main_template + "Nodes:            " + str( r["node_count"] ) + "\n" )
