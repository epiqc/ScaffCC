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

from PyQt4.QtCore import Qt, QSize
from PyQt4.QtGui import QBrush, QTableWidgetItem

from revkit import equivalence_check

from core.BaseItem import *

from ui.DesignerWidget import DesignerWidget
from ui.EquivalenceCheck import Ui_EquivalenceCheck

class EquivalenceCheck( DesignerWidget ):
    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_EquivalenceCheck, parent )

    def clear( self ):
        self.tableWidget.clearContents()
        self.tableWidget.setRowCount( 0 )

    def addRow( self, circ1, circ2, equivalent ):
        self.tableWidget.setRowCount( self.tableWidget.rowCount() + 1 )

        row = self.tableWidget.rowCount() - 1
        self.tableWidget.setItem( row, 0, QTableWidgetItem( circ1.circuit_name ) )
        self.tableWidget.setItem( row, 1, QTableWidgetItem( circ1.circuit_name ) )
        self.tableWidget.setItem( row, 2, QTableWidgetItem( str( equivalent ) ) )

        for column in range( 3 ):
            self.tableWidget.item( row, column ).setFlags( Qt.ItemIsSelectable | Qt.ItemIsEnabled )
        self.tableWidget.item( row, 2 ).setForeground( QBrush( Qt.green if equivalent else Qt.red ) )
        self.tableWidget.item( row, 2 ).setTextAlignment( Qt.AlignVCenter | Qt.AlignHCenter )

@item( "Equivalence Checking",
       iconname = "checkbox",
       requires = [ "Circuit", "Circuit" ],
       widget = { 'class': EquivalenceCheck, 'size': (500, 400) } )
class EquivalenceCheckItem( BaseItem ):
    """This items provide the SAT-based equivalence checker. It gets two circuits and returns <i>equivalent</i> if both circuits realizing the same function. The equivalence checker supports different configurations of constant inputs and garbage outputs in the considered circuits. If more than one benchmark, e.g. when using <i>Path Benchmarks</i>, a detailed overview is given when enlarging the item."""
    def onCreate( self ):
        self.setText( "Equivalence Checking" )
        self.setState( self.CONFIGURED )

    def initialize( self ):
        self.equivalent = True
        self.widget.clear()

    def executeEvent( self, inputs ):
        r = equivalence_check( inputs[0], inputs[1] )
        if type( r ) == dict:
            self.widget.addRow( inputs[0], inputs[1], r['equivalent'] )

            self.equivalent = self.equivalent and r['equivalent']
            self.setText( "Equivalent" if self.equivalent else "Not equivalent" )
        else:
            return r
        return []
