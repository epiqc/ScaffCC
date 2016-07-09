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

from revkit import circuit, exact_synthesis

from core.BaseItem import *

from helpers.RevKitHelper import *

from ui.DesignerWidget import DesignerWidget
from ui.ExactSynthesis import Ui_ExactSynthesis

class ExactSynthesis( DesignerWidget ):
    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_ExactSynthesis, parent )

@item( "Exact Synthesis",
       requires = "Truth Table", provides = "Circuit",
       properties = [ "spec_incremental", "max_depth" ],
       widget = { 'class': ExactSynthesis, 'size': (250, 120) } )
class ExactSynthesisItem( BaseItem ):
    """This item provides the exact synthesis method. It can be specified whether incremental SAT techniques should be applied or not. Furthermore, the maximum number of gates to be considered can be defined. After the item has been processed, the enlarged item reports the run-time needed to perform the synthesis."""
    def onCreate( self ):
        self.setText( "Exact Synthesis" )
        self.setState( self.CONFIGURED )

    def executeEvent( self, inputs ):
        circ = circuit()
        res = exact_synthesis( circ, inputs[0],
                               max_depth = int( self.max_depth ),
                               spec_incremental = bool( int( self.spec_incremental ) ) )
        if type( res ) == dict:
            try:
                circ.circuit_name = inputs[0].name
            except: pass
            self.widget.runtime.setText( "%.2f s" % res['runtime'] )
            circuit_add_runtime( circ, res['runtime'] )
        else:
            return res
        return [ circ ]
