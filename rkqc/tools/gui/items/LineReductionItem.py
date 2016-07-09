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
from revkit import circuit, line_reduction

from helpers.RevKitHelper import *

from ui.DesignerWidget import DesignerWidget
from ui.LineReduction import Ui_LineReduction

class LineReduction( DesignerWidget ):
    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_LineReduction, parent )

@item( "Line Reduction",
       iconname = "format-indent-less",
       requires = "Circuit", provides = "Circuit",
       properties = [ "max_window_lines", "max_grow_up_window_lines", "window_variables_threshold" ],
       widget = { 'class': LineReduction, 'size': (240, 140) } )
class LineReductionItem( BaseItem ):
    """This item provides the circuit line reduction method. After the item has been processed, the enlarged item reports the run-time needed to perform the optimization."""
    def onCreate( self ):
        self.setText( "Line Reduction" )
        self.setState( self.CONFIGURED )

    def executeEvent( self, inputs ):
        circ = circuit()
        res = line_reduction( circ, inputs[0],
                              max_window_lines = int( self.max_window_lines ),
                              max_grow_up_window_lines = int( self.max_grow_up_window_lines ),
                              window_variables_threshold = int( self.window_variables_threshold ) )
        if type( res ) == dict:
            self.widget.runtime.setText( "%.2f s" % res['runtime'] )
            circuit_add_runtime( circ, res['runtime'] )
        else:
            return res
        return [ circ ]
