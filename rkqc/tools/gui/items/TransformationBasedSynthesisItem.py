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

from PyQt4.QtCore import SIGNAL, pyqtProperty

from core.BaseItem import *

from revkit import circuit, reed_muller_synthesis_func, swop, transformation_based_synthesis_func, gate_costs, quantum_costs, transistor_costs

from helpers.RevKitHelper import *

from ui.DesignerWidget import DesignerWidget
from ui.TransformationBasedSynthesis import Ui_TransformationBasedSynthesis

class TransformationBasedSynthesis( DesignerWidget ):
    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_TransformationBasedSynthesis, parent )

        self.connect( self.swop, SIGNAL( 'currentIndexChanged(int)' ), self.swopChanged )

    def swopChanged( self, index ):
        self.cost_function.setEnabled( index > 0 )
        self.cost_function_label.setEnabled( index > 0 )

@item( "Transformation-based Synthesis",
       requires = "Truth Table", provides = "Circuit",
       properties = [ "variant", "bidi_synthesis", "swop", "cost_function" ],
       widget = { 'class': TransformationBasedSynthesis, 'size': (300, 175) } )
class TransformationBasedSynthesisItem( BaseItem ):
    """This item provides the transformation-based synthesis method as well as the corresponding synthesis with output permutation method. The respective synthesis approach can be selected in the pull-down menu (in case of synthesis with output permutation additionally the optimization criteria can be defined). Furthermore, it can be specified whether bi-directional synthesis should be applied or not. After the item has been processed, the enlarged item reports the run-time needed to perform the synthesis."""
    def onCreate( self ):
        self.setState( self.CONFIGURED )

    def executeEvent( self, inputs ):
        circ = circuit()
        cf = [ gate_costs, quantum_costs, transistor_costs ][int( self.cost_function )]()
        synthesis = [ transformation_based_synthesis_func, reed_muller_synthesis_func ][int( self.variant )]
        res = swop( circ, inputs[0],
                    enable = int( self.swop ) > 0,
                    exhaustive = int( self.swop ) == 1,
                    synthesis = synthesis( bidirectional = bool( int( self.bidi_synthesis ) ) ),
                    cf = cf )
        if type( res ) == dict:
            try:
                circ.circuit_name = inputs[0].name
            except: pass
            self.widget.runtime.setText( "%.2f s" % res['runtime'] )
            circuit_add_runtime( circ, res['runtime'] )
        else:
            return res
        return [ circ ]

    def onVariantChanged( self, value ):
        suffix = [ "TT", "RMS" ][int( value )]
        self.setText( "Transformation-based (%s)" % suffix )

