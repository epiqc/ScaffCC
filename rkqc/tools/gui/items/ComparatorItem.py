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
from PyQt4.QtGui import QIcon

from revkit import costs, line_costs, gate_costs, quantum_costs, transistor_costs

from core.BaseItem import *
from core.ComparatorLine import ComparatorLine

from ui.DesignerWidget import DesignerWidget
from ui.Comparator import Ui_Comparator

class Comparator( DesignerWidget ):
    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_Comparator, parent )

        self.add_row.setIcon( QIcon.fromTheme( "list-add" ) )
        self.add_input.setIcon( QIcon.fromTheme( "list-add" ) )
        self.remove_input.setIcon( QIcon.fromTheme( "list-remove" ) )

        self.first_line.button.setEnabled( False )

        self.comparisons = [ self.first_line ]

        self.connect( self.add_row, SIGNAL( 'pressed()' ), self.addRow )

    def onCreate( self ):
        self.connect( self.add_input, SIGNAL( 'pressed()' ), self.ownerItem.addInput )
        self.connect( self.remove_input, SIGNAL( 'pressed()' ), self.ownerItem.removeInput )

    def addRow( self ):
        line = ComparatorLine()
        line.label.setText( "Then" )
        self.connect( line.button, SIGNAL( 'pressed()' ), self.removeRow )

        self.lines.layout().insertWidget( len( self.comparisons ), line )
        self.comparisons.append( line )

    def removeRow( self ):
        line = self.sender().parent()
        self.comparisons.remove( line )
        line.deleteLater()

@item( "Comparator",
       iconname = "system-switch-user",
       requires = [ "Circuit", "Circuit" ], provides = "Circuit",
       properties = [ "ports", "lines" ],
       widget = { 'class': Comparator, 'size': (330, 250) } )
class ComparatorItem( BaseItem ):
    """This item gets two circuits and passes the better one along depending on criteria which can be defined in the enlarged item."""
    def onCreate( self ):
        self.setText( "Comparator" )
        self.setState( self.CONFIGURED )

    @action( "Add input", "list-add" )
    def addInput( self ):
        self.addRequires( "Circuit" )
        self.widget.remove_input.setEnabled( len( self.requiresPorts ) > 2 )

    @action( "Remove input", "list-remove" )
    def removeInput( self ):
        self.removeRequires( len( self.requiresPorts ) - 1 )
        self.widget.remove_input.setEnabled( len( self.requiresPorts ) > 2 )

    def executeEvent( self, inputs ):
        for line in self.widget.comparisons:
            if len( inputs ) == 1: break # Determined already?

            compare_func = [ min, max ][ line.compare.currentIndex() ]
            cf = [ line_costs, gate_costs, quantum_costs, transistor_costs ][ line.what.currentIndex() ]

            compare_to = compare_func( [ costs( c, cf() ) for c in inputs ] )
            inputs = [ c for c in inputs if costs( c, cf() ) == compare_to ]

        return inputs[:1]

    def onPortsChanged( self, value ):
        for _ in range( 2, int( value ) ):
            self.addRequires( "Circuit" )
        self.widget.remove_input.setEnabled( len( self.requiresPorts ) > 2 )

    def getPorts( self ):
        return QString.number( len( self.requiresPorts ) )

    def onLinesChanged( self, v ):
        lines = str( v ).split( ';' )
        for _ in range( 1, len( lines ) ):
            self.widget.addRow()

        for i, line in enumerate( lines ):
            pair = map( int, line.split( ',' ) )
            self.widget.comparisons[i].compare.setCurrentIndex( pair[0] )
            self.widget.comparisons[i].what.setCurrentIndex( pair[1] )

    def getLines( self ):
        return ';'.join( [ '%d,%d' % ( l.compare.currentIndex(), l.what.currentIndex() ) for l in self.widget.comparisons ] )
