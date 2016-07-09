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

from PyQt4.QtCore import Qt, QSize, QRegExp, QVariant
from PyQt4.QtGui import QIcon, QFrame, QIcon, QPalette, QSortFilterProxyModel, QStandardItem, QStandardItemModel, QTreeView

class FlowItemTreeWidget( QTreeView ):
    def __init__( self, parent = None ):
        QTreeView.__init__( self, parent )

        self.setAnimated( True )
        self.setDragEnabled( True )
        self.setRootIsDecorated( False )
        self.setFrameShape( QFrame.NoFrame )
        self.setIconSize( QSize( 22, 22 ) )
        self.header().setStretchLastSection( True )

        # background color
        palette = self.viewport().palette()
        palette.setColor( self.viewport().backgroundRole(), Qt.transparent )
        palette.setColor( self.viewport().foregroundRole(), palette.color( QPalette.WindowText ) )
        self.viewport().setPalette( palette )

        # Model
        self._model = QStandardItemModel()
        self.proxy = FlowItemSortFilterProxyModel( self )
        self.proxy.setSourceModel( self._model )
        self.setModel( self.proxy )

        self.header().hide()

    def addCategory( self, category, items ):
        # Items
        root = QStandardItem( category )
        root.setFlags( Qt.ItemIsEnabled | Qt.ItemIsSelectable )
        self._model.appendRow( root )

        # Styling
        root.setFlags( Qt.ItemIsEnabled )
        f = root.font()
        f.setPointSize( 10 )
        f.setBold( True )
        root.setFont( f )

        for item in items:
            ins = item()
            child = QStandardItem( QIcon.fromTheme( ins.iconname ), ins.description )
            child.setData( QVariant( item ), Qt.UserRole )
            child.setFlags( Qt.ItemIsEnabled | Qt.ItemIsSelectable | Qt.ItemIsDragEnabled )
            if ins.__doc__ is not None and len( ins.__doc__ ) > 0: child.setWhatsThis( ins.__doc__ )
            root.appendRow( child )

    def setFilter( self, pattern ):
        self.proxy.setFilterRegExp( QRegExp( pattern, Qt.CaseInsensitive, QRegExp.FixedString ) )
        self.expandAll()

class FlowItemSortFilterProxyModel( QSortFilterProxyModel ):
    def __init__( self, parent = None ):
        QSortFilterProxyModel.__init__( self, parent )

    def filterAcceptsRow( self, sourceRow, sourceParent ):
        if not sourceParent.isValid():
            return True

        index = self.sourceModel().index( sourceRow, 0, sourceParent )
        return self.sourceModel().data( index ).toString().contains( self.filterRegExp() )

