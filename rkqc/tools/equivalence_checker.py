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
opts.add_option( "spec", "specification of a circuit" ) \
    .add_option( "impl", "implementation of a circuit") \

opts.parse( sys.argv )

if not opts.good() or not opts.is_set ("spec") or not opts.is_set ("impl"):
    print opts
    exit( 1 )

spec = circuit ()
impl = circuit ()
 
read_realization (spec, opts["spec"]) or sys.exit ("Cannot read " + opts["spec"])
read_realization (impl, opts["impl"]) or sys.exit ("Cannot read " + opts["impl"])

r = equivalence_check (spec, impl )

length = max ( map (len, r.keys()))

string = '{0:' + str(length + 1) + 's} {1:s} ' 

for key in r.keys():
  print string.format (key + ":", str(r[key])) 

#print statistics

