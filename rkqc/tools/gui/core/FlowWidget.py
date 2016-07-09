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
from PyQt4.QtGui import QGraphicsView, QMenu, QPainter

from FlowItemTreeWidget import *
from FlowScene import *
from ItemAutoConnecter import ItemAutoConnecter
from ScrollControl import *

class FlowWidget( QGraphicsView ):
    def __init__( self, parent = None ):
        QGraphicsView.__init__( self, parent )

        self.setAcceptDrops( True )
        self.setScene( FlowScene( self ) )
        self.setRenderHints( QPainter.Antialiasing | QPainter.SmoothPixmapTransform )
        self.setAlignment( Qt.AlignLeft | Qt.AlignTop )

        self.autoConnecter = ItemAutoConnecter( self.scene() )

        self.setupMenu()

        #self.scrollControl = ScrollControl( self )
        #self.scrollControl.setScrollingEnabled( False )

    def setupMenu( self ):
        self.setContextMenuPolicy( Qt.CustomContextMenu )

        self.menu = QMenu( self )

        self.connect( self, SIGNAL( 'customContextMenuRequested( const QPoint& )' ), self.slotContextMenu )

    def slotContextMenu( self, pos ):
        self.menu.clear()

        item = self.itemAt( pos )
        if isinstance( item, FlowSceneGraphEdge ):
            self.menu.addAction( item.delete_action )

        self.menu.addAction( self.autoConnecter.action() )
        self.autoConnecter.updateAction()
        self.menu.exec_( self.mapToGlobal( pos ) )

    # Zoom by scroll event
    # def wheelEvent( self, event ):
    #     factor = 1.2
    #     if event.delta() < 0:
    #         factor = 1.0 / factor
    #     self.scale( factor, factor )
    #     #self.updateZoomValue()
    #     return QGraphicsView.wheelEvent( self, event )

    def dragEnterEvent( self, event ):
        if isinstance( event.source(), FlowItemTreeWidget ):
            event.acceptProposedAction()

    def dragMoveEvent( self, event ):
        pass

    def dropEvent( self, event ):
        type_ = event.source().selectedIndexes()[0].data( Qt.UserRole ).toPyObject()
        item = type_()
        item.setParent( self )
        item.setPos( self.mapToScene( event.pos() ) )
        self.scene().addItem( item )
