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

from revkit import circuit, adding_lines

from helpers.RevKitHelper import *

from ui.DesignerWidget import *
from ui.AddingLines import *

class AddingLines( DesignerWidget ):
    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_AddingLines, parent )

@item( "Adding Lines Optimization",
       iconname = "format-indent-more",
       requires = "Circuit", provides = "Circuit",
       properties = [ "additional_lines" ],
       widget = { 'class': AddingLines, 'size': (200, 100) } )
class AddingLinesItem( BaseItem ):
    """This item provides the adding lines optimization method. It requires to define the number of lines that should be added. After the item has been processed, the enlarged item reports the run-time needed to perform the optimization."""
    def onCreate( self ):
        self.setState( self.CONFIGURED )

    def onAdditionalLinesChanged( self, value ):
        self.setText( "Adding Lines (%d)" % int( value ) )

    def executeEvent( self, inputs ):
        circ = circuit()
        res = adding_lines( circ, inputs[0], additional_lines = int( self.additional_lines ) )
        if type( res ) == dict:
            self.widget.runtime.setText( "%.2f s" % res['runtime'] )
            circuit_add_runtime( circ, res['runtime'] )
        else:
            return res
        return [ circ ]
