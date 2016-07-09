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

from PyQt4.QtCore import QSize, pyqtProperty

from revkit import circuit, esop_synthesis, weighted_reordering

from core.BaseItem import *

from helpers.RevKitHelper import *

from ui.DesignerWidget import DesignerWidget
from ui.ESOPSynthesis import Ui_ESOPSynthesis

class ESOPSynthesis( DesignerWidget ):
    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_ESOPSynthesis, parent )

@item( "ESOP Synthesis",
       requires = "PLA", provides = "Circuit",
       properties = [ "separate_polarities", "reordering", "alpha", "beta", "garbage_name" ],
       widget = { 'class': ESOPSynthesis, 'size': (350, 200) } )
class ESOPSynthesisItem( BaseItem ):
    """This item provides the ESOP-based synthesis method. After the item has been processed, the enlarged item reports the run-time needed to perform the synthesis."""
    def onCreate( self ):
        self.setText( "ESOP Synthesis" )
        self.setState( self.CONFIGURED )

    def executeEvent( self, inputs ):
        circ = circuit()
        res = esop_synthesis( circ, inputs[0],
                              separate_polarities = bool( int( self.separate_polarities ) ),
                              reordering = weighted_reordering( float( self.alpha ), float( self.beta ) ),
                              garbage_name = str( self.garbage_name ) )
        if type( res ) == dict:
            circuit_set_name( circ, inputs[0] )

            self.widget.runtime.setText( "%.2f s" % res['runtime'] )
            circuit_add_runtime( circ, res['runtime'] )
        else:
            return res
        return [ circ ]
