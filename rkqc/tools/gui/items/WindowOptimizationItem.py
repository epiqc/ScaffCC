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

from PyQt4.QtCore import QSize, pyqtProperty, SIGNAL

from revkit import *

from core.BaseItem import *

from helpers.RevKitHelper import *

from ui.DesignerWidget import DesignerWidget
from ui.WindowOptimization import Ui_WindowOptimization

class WindowOptimization( DesignerWidget ):
    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_WindowOptimization, parent )

        self.connect( self.window_selection, SIGNAL( 'currentIndexChanged(int)' ), self.window_selection_changed )

    def window_selection_changed( self, value ):
        for c in [ self.window_length_label, self.window_length, self.offset_label, self.offset ]:
            c.setEnabled( value == 0 )

@item( "Window Optimization",
       iconname = "window-duplicate",
       provides = "Circuit", requires = "Circuit",
       properties = [ "window_selection", "window_length", "offset", "synthesis_method", "costs_function" ],
       widget = { 'class': WindowOptimization, 'size': (350, 240) } )
class WindowOptimizationItem( BaseItem ):
    """This item provides the window optimization method. After the item has been processed, the enlarged item reports the run-time needed to perform the optimization."""
    def onCreate( self ):
        self.setText( "Window Optimization" )
        self.setState( self.CONFIGURED )

    def executeEvent( self, inputs ):
        circ = circuit()

        # Select Window Function
        if int( self.window_selection ) == 0:
            select_window = shift_window_selection_func( int( self.window_length ), int( self.offset ) )
        else:
            select_window = line_window_selection_func()

        # Synthesis Function
        if int( self.synthesis_method ) == 0:
            optimization = resynthesis_optimization_func( synthesis = transformation_based_synthesis_func() )
        else:
            optimization = resynthesis_optimization_func( synthesis = exact_synthesis_func() )

        # Costs Function
        cf = [ gate_costs, quantum_costs, transistor_costs ][int( self.costs_function )]()

        res = window_optimization( circ, inputs[0],
                                   select_window = select_window,
                                   optimization = optimization,
                                   cf = cf )
        if type( res ) == dict:
            self.widget.runtime.setText( "%.2f s" % res['runtime'] )
            circuit_add_runtime( circ, res['runtime'] )
        else:
            return res
        return [ circ ]
