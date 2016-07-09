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

from PyQt4.QtCore import Qt, pyqtSignal
from PyQt4.QtGui import QTableWidget, QTableWidgetItem

from revkit import binary_truth_table, read_pla

class SpecificationTable( QTableWidget ):
    specificationChanged = pyqtSignal()

    def __init__( self, parent = None ):
        QTableWidget.__init__( self, parent )
        self.verticalHeader().hide()

        self.spec = None

    def load( self, filename ):
        if filename.isEmpty(): return

        self.spec = binary_truth_table()
        read_pla( self.spec, str( filename ), extend = False )

        self.clear()
        self.setColumnCount( self.spec.num_inputs + self.spec.num_outputs )
        self.setRowCount( sum( 1 for _ in self.spec.entries ) )
        self.setHorizontalHeaderLabels( self.spec.inputs + self.spec.outputs )

        for row, entry in enumerate( self.spec.entries ):
            for column, entry in enumerate( entry[0] + entry[1] ):
                item = QTableWidgetItem( "-" if entry is None else str( int( entry ) ) )
                item.setFlags( Qt.ItemIsSelectable | Qt.ItemIsEnabled )
                if column >= self.spec.num_inputs:
                    item.setBackground( Qt.gray )
                self.setItem( row, column, item )

        self.resizeColumnsToContents()
        self.specificationChanged.emit()

