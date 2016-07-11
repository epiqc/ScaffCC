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
opts.add_read_realization_option() \
    .add_option( "texname", "LaTeX file-name to write result" ) \
    .add_option( "format", 0, "0: tikZ\n1: PsTricks" ) \
    .add_double_option( "elem_width", "Element Width" )
opts.parse( sys.argv )

if not opts.good():
    print opts
    exit( 1 )

circ = circuit()
read_realization( circ, opts.read_realization_filename() ) or sys.exit ("Cannot read " + opts.read_realization_filename())

if opts["format"] == 1:
    settings = create_pstricks_settings()
else:
    settings = create_tikz_settings()

output = create_image( circ, \
                           generator = settings, \
                           elem_width = opts["elem_width"] if opts.is_set( "elem_width" ) else 0.5 )

if opts.is_set( "texname" ):
    with open( opts["texname"], 'w' ) as f:
        f.write( output )
else:
    print output
