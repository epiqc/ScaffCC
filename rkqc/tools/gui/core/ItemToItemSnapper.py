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

from PyQt4.QtCore import QObject

from core.BaseItem import *

class ItemToItemSnapper( QObject ):
    def __init__( self, scene ):
        QObject.__init__( self, scene )

        self.threshold = 20

        self.scene = scene
        self.scene.item_added.connect( self.onItemAdded )

        self.linex = self.scene.addLine( 0, 0, 1, 1, QPen( QColor( 'Cornflowerblue' ) ) )
        self.liney = self.scene.addLine( 0, 0, 1, 1, QPen( QColor( 'Cornflowerblue' ) ) )

        self.linex.setVisible( False )
        self.liney.setVisible( False )

    def onItemAdded( self, item ):
        if not isinstance( item.toGraphicsObject(), BaseItem ): return

        item.toGraphicsObject().installEventFilter( self )

    def eventFilter( self, obj, event ):
        if event.type() == QEvent.GraphicsSceneMouseMove:
            newX = obj.pos().x()
            newY = obj.pos().y()

            xitems = [ item for item in self.scene.graph.items() if item != obj and abs( item.x() - obj.pos().x() ) <= self.threshold ]
            if len( xitems ) > 0:
                view = self.scene.views()[0]

                item = min( xitems, key = lambda x: abs( x.x() - obj.pos().x() ) )
                self.linex.setLine( item.x(), view.mapToScene( 0, 0 ).y(), item.x(), view.mapToScene( 0, view.height() ).y() )
                self.linex.setVisible( True )
            else:
                self.linex.setVisible( False )

            yitems = [ item for item in self.scene.graph.items() if item != obj and abs( item.y() - obj.pos().y() ) <= self.threshold ]
            if len( yitems ) > 0:
                view = self.scene.views()[0]

                item = min( yitems, key = lambda y: abs( y.y() - obj.pos().y() ) )
                self.liney.setLine( view.mapToScene( 0, 0 ).x(), item.y(), view.mapToScene( view.width(), 0 ).x(), item.y() )
                self.liney.setVisible( True )
            else:
                self.liney.setVisible( False )

        elif event.type() == QEvent.GraphicsSceneMouseRelease:
            newX = obj.pos().x()
            newY = obj.pos().y()

            if self.linex.isVisible():
                newX = self.linex.line().x1()
                self.linex.setVisible( False )

            if self.liney.isVisible():
                newY = self.liney.line().y1()
                self.liney.setVisible( False )

            obj.setPos( newX, newY )

        return QObject.eventFilter( self, obj, event )

