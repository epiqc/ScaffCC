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

from core.BaseItem import *

from revkit import circuit, transposition_based_synthesis

from helpers.RevKitHelper import *

@item( "Transposition-based Synthesis",
       requires = "Truth Table", provides = "Circuit" )
class TranspositionBasedSynthesisItem( BaseItem ):
    """TODO"""
    def onCreate( self ):
        self.setText( "Transposition-based" )
        self.setState( self.CONFIGURED )

    def executeEvent( self, inputs ):
        circ = circuit()
        res = transposition_based_synthesis( circ, inputs[0] )
        if type( res ) == dict:
            try:
                circ.circuit_name = inputs[0].name
            except: pass
            circuit_add_runtime( circ, res['runtime'] )
        else:
            return res
        return [ circ ]
