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

from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *

class BaseItemButton( QGraphicsObject ):
    def __init__( self, parent = None ):
        QGraphicsObject.__init__( self, parent )

        self.setAcceptHoverEvents( True )
        self.setCacheMode( QGraphicsItem.DeviceCoordinateCache )

        self.appearAnimation = QPropertyAnimation( self, "opacity" )
        self.appearAnimation.setDuration( 250 )
        self.appearAnimation.setStartValue( 0 )
        self.appearAnimation.setEndValue( 1 )

        # shadow
        effect = QGraphicsDropShadowEffect();
        effect.setOffset( 2, 2 )
        effect.setBlurRadius( 10 )
        self.setGraphicsEffect( effect )

    def boundingRect( self ):
        return QRectF( 0, 0, 20, 20 )

    def backgroundColor( self ):
        # default from parent
        try:
            return self.parentObject()._color
        except: pass

    def paint( self, painter, option, widget = None ):
        try:
            painter.setBrush( QBrush( self.parentObject().createButtonGradient( 20, self.backgroundColor().lighter( 125 ) ) ) )
            painter.setPen( self.backgroundColor() )
            painter.drawEllipse( option.rect )
        except: pass

    def show( self ):
        self.appearAnimation.stop()
        self.appearAnimation.setDirection( QAbstractAnimation.Forward )
        self.appearAnimation.start()

    def hide( self ):
        self.appearAnimation.stop()
        self.appearAnimation.setDirection( QAbstractAnimation.Backward )
        self.appearAnimation.start()

    def hoverLeaveEvent( self, event ):
        # If leaving out of the parent item,
        # then hide and hide other buttons as well
        if self.scene().itemAt( event.scenePos() ) != self.parentItem():
            self.hide() # this seems to work already
