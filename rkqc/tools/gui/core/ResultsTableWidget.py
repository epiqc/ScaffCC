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

from PyQt4.QtCore import Qt, SIGNAL
from PyQt4.QtGui import QAction, QCursor, QMenu, QTableWidget

class ResultsTableWidget( QTableWidget ):
    def __init__( self, parent = None ):
        QTableWidget.__init__( self, parent )

        self.labels = [ "Name", "Lines", "Gates", "Quantum Costs", "Transistor Costs", "Run-Time" ]

        self.setColumnCount( len( self.labels ) )
        self.setHorizontalHeaderLabels( self.labels )
        self.setColumnHidden( 4, True )
        self.setColumnHidden( 5, True )

        self.horizontalHeader().setContextMenuPolicy( Qt.CustomContextMenu )
        self.connect( self.horizontalHeader(), SIGNAL( 'customContextMenuRequested( const QPoint& )' ), self.headerContextMenu )

    def headerContextMenu( self, pos ):
        actions = []
        for i, name in enumerate( self.labels[1:] ):
            action = QAction( name, self )
            action.setCheckable( True )
            action.setChecked( not self.isColumnHidden( i + 1 ) )
            actions.append( action )

        menu = QMenu()
        act = menu.exec_( actions, QCursor.pos() )

        if act is None: return

        index = actions.index( act ) + 1
        self.setColumnHidden( index, not self.isColumnHidden( index ) )

