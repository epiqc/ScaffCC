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

from PyQt4.QtGui import QTabWidget

from revkitui import CircuitView

from core.BaseItem import *

@item( "Circuit Viewer",
       iconname = "application-x-qet-element",
       requires = "Circuit",
       widget = QTabWidget )
class ViewerItem( BaseItem ):
    """This item displays a given circuit or a set of circuits. The viewer is the same component used in the <i>RevKit Viewer</i> tool.
Therefore, with a right-click the circuit can be exported as *.tex-file and you can zoom in and out of the circuit via the mouse wheel."""
    def onCreate( self ):
        self.setText( "Circuit Viewer" )
        self.setState( self.CONFIGURED )

    def initialize( self ):
        self.widget.clear()
        self.numCircuits = 0

    def executeEvent( self, inputs ):
        self.numCircuits += 1

        view = CircuitView( None, self.widget )
        view.load( inputs[0] )
        self.widget.addTab( view, inputs[0].circuit_name )
        return []
