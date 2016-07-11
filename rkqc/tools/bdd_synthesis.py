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
    .add_option( "complemented_edges", True, "build BDD with complemented edges" ) \
    .add_option( "reordering", 4, "reordering strategy\n 0: CUDD_REORDER_SAME\n 1: CUDD_REORDER_NONE\n 2: CUDD_REORDER_RANDOM\n 3: CUDD_REORDER_RANDOM_PIVOT\n 4: CUDD_REORDER_SIFT\n 5: CUDD_REORDER_SIFT_CONVERGE\n 6: CUDD_REORDER_SYMM_SIFT\n 7: CUDD_REORDER_SYMM_SIFT_CONV\n 8: CUDD_REORDER_WINDOW2\n 9: CUDD_REORDER_WINDOW3\n10: CUDD_REORDER_WINDOW4\n11: CUDD_REORDER_WINDOW2_CONV\n12: CUDD_REORDER_WINDOW3_CONV\n13: CUDD_REORDER_WINDOW4_CONV\n14: CUDD_REORDER_GROUP_SIFT\n15: CUDD_REORDER_GROUP_SIFT_CONV\n16: CUDD_REORDER_ANNEALING\n17: CUDD_REORDER_GENETIC\n18: CUDD_REORDER_LINEAR\n19: CUDD_REORDER_LINEAR_CONVERGE\n20: CUDD_REORDER_LAZY_SIFT\n21: CUDD_REORDER_EXACT" ) \
    .add_option( "use_quantum_library", False, "use quantum library instead of MCT" ) \
    .add_option( "dotfilename", "[Statistics] Dump DOT representation of BDD to this file" ) \
    .add_option( "infofilename", "[Statistics] Dump BDD information to this file" )
opts.parse( sys.argv )

if not opts.good() or not opts.is_set( "filename" ):
    print opts
    exit( 1 )

circ = circuit()

dotfilename = ""
if opts.is_set( "dotfilename" ): dotfilename = opts["dotfilename"]
infofilename = ""
if opts.is_set( "infofilename" ): infofilename = opts["infofilename"]

r = bdd_synthesis( circ, opts["filename"], \
                       complemented_edges = opts["complemented_edges"], \
                       reordering = opts["reordering"], \
                       dotfilename = dotfilename, \
                       infofilename = infofilename )

if opts.is_write_realization_filename_set():
    header = "This file has been generated using RevKit %s (www.revkit.org)\nCommand Line:\n%s\nBased on the approach proposed in R. Wille and R. Drechsler. BDD-based synthesis of reversible logic for large functions. In Design Automation Conf., pages 270-275, 2009." % ( revkit_version(), " ".join( sys.argv ) ) 
    write_realization( circ, opts.write_realization_filename() , header = header ) 

print_statistics( circ, runtime = r["runtime"], main_template = print_statistics_settings().main_template + "Nodes:            " + str( r["node_count"] ) + "\n" )

