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

from BaseItemButton import *

class BaseItemProxyWidget( QGraphicsProxyWidget ):
    def __init__( self, parent = None ):
        QGraphicsProxyWidget.__init__( self, parent )

        self.setWidget( QWidget() )
        self.widget().setAttribute( Qt.WA_OpaquePaintEvent )
        self.widget().hide()

class BaseItemButtonMaximize( BaseItemButton ):
    maximized = pyqtSignal()
    minimized = pyqtSignal()

    def __init__( self, parent = None ):
        BaseItemButton.__init__( self, parent )

        self.setPos( 63, -30 )
        self.setOpacity( 0 )
        self.setToolTip( "Maximize" )

        self._widget = BaseItemProxyWidget( parent )

        # Animations
        self.posx_animation = QPropertyAnimation( parent, "x" )
        self.posy_animation = QPropertyAnimation( parent, "y" )
        self.height_animation = QPropertyAnimation( parent, "height" )
        self.width_animation = QPropertyAnimation( parent, "width" )
        self.button_maximize_animation = QPropertyAnimation( self, "x" )
        self.button_maximize_animation.setStartValue( self.x() )
        self.button_close_animation = QPropertyAnimation( parent.close_button, "x" )
        self.button_close_animation.setStartValue( parent.close_button.x() )

        self.animation = QParallelAnimationGroup( self )
        self.animation.addAnimation( self.posx_animation )
        self.animation.addAnimation( self.posy_animation )
        self.animation.addAnimation( self.height_animation )
        self.animation.addAnimation( self.width_animation )
        self.animation.addAnimation( self.button_maximize_animation )
        self.animation.addAnimation( self.button_close_animation )

        for i in range( self.animation.animationCount() ):
            self.animation.animationAt( i ).setDuration( 200 )

        self.connect( self.animation, SIGNAL( 'finished()' ), self.maximizeAnimationFinished )

        self.portAnimations = dict()
        parent.portAdded.connect( self.onPortAdded )
        parent.portRemoved.connect( self.onPortRemoved )

    def onPortAdded( self, port ):
        if port.direction == Qt.AlignTop: return

        portAnimation = QPropertyAnimation( port, "y" )
        portAnimation.setStartValue( 20 )
        portAnimation.setDuration( 200 )

        self.animation.addAnimation( portAnimation )
        self.portAnimations[port] = portAnimation

    def onPortRemoved( self, port ):
        if port.direction == Qt.AlignTop: return

        self.animation.removeAnimation( self.portAnimations[port] )
        del self.portAnimation[port]

    def paint( self, painter, option, widget = None ):
        BaseItemButton.paint( self, painter, option, widget )

        painter.setBrush( QColor( Qt.gray ) )
        painter.setPen( QColor( Qt.gray ).darker() )
        if self.animation.direction() == QAbstractAnimation.Forward:
            painter.drawRect( QRectF( 6, 6, 8, 8 ) )
        else:
            painter.drawLine( QLineF( 6, 14, 14, 14 ) )

    def mousePressEvent( self, event ):
        if self.animation.direction() == QAbstractAnimation.Forward:
            # (Re-)initialize start and end values
            view = self.parentObject().scene().views()[0]
            lt = view.mapToScene( 20, 20 )
            rb = view.mapToScene( view.width() - 40, view.height() - 40 )

            # where to move? small or big expanding?
            custom_size = self.parentObject().custom_size

            new_x = self.parentObject().x() if custom_size is not None else ( lt.x() + rb.x() ) / 2
            new_y = self.parentObject().y() if custom_size is not None else lt.y() + 20
            new_width = custom_size.width() if custom_size is not None else rb.x() - lt.x()
            new_height = custom_size.height() if custom_size is not None else rb.y() - lt.y()

            self.posx_animation.setStartValue( self.parentObject().x() )
            self.posx_animation.setEndValue( new_x )
            self.posy_animation.setStartValue( self.parentObject().y() )
            self.posy_animation.setEndValue( new_y )
            self.height_animation.setStartValue( self.parentObject()._height )
            self.height_animation.setEndValue( new_height )
            self.width_animation.setStartValue( self.parentObject()._width )
            self.width_animation.setEndValue( new_width )
            self.button_maximize_animation.setEndValue( new_width / 2 - 37 )
            self.button_close_animation.setEndValue( new_width / 2 - 10 )

            for portAnimation in self.portAnimations.values():
                portAnimation.setEndValue( new_height - 20 )

            self.parentObject().setZValue( 10 ) # Above everything
            self.parentObject().setFlag( QGraphicsItem.ItemIsMovable, False )

        self.animation.start()

    def isMaximized( self ):
        return self.animation.direction() == QAbstractAnimation.Backward

    def maximizeAnimationFinished( self ):
        if self.animation.direction() == QAbstractAnimation.Forward:
            self.animation.setDirection( QAbstractAnimation.Backward )
            self._widget.widget().show()
            self._widget.setPos( -self.parentObject()._width / 2, 0 )
            self._widget.widget().resize( self.parentObject()._width, self.parentObject()._height - 20 )

            # Item Appearance and Scene
            self.setToolTip( "Minimize" )
            view = self.parentObject().scene().views()[0]
            view.setHorizontalScrollBarPolicy( Qt.ScrollBarAlwaysOff )
            view.setVerticalScrollBarPolicy( Qt.ScrollBarAlwaysOff )

            self.maximized.emit()
        else:
            self.parentObject().setZValue( 0 )
            self.parentObject().setFlag( QGraphicsItem.ItemIsMovable, True )
            self.animation.setDirection( QAbstractAnimation.Forward )
            self._widget.widget().hide()

            self.setToolTip( "Maximize" )
            view = self.parentObject().scene().views()[0]
            view.setHorizontalScrollBarPolicy( Qt.ScrollBarAsNeeded )
            view.setVerticalScrollBarPolicy( Qt.ScrollBarAsNeeded )

            self.minimized.emit()
        self.update()
        self.scene().update()

    def hoverLeaveEvent( self, event ):
        # Protext when maximized
        if not self.isMaximized():
            BaseItemButton.hoverLeaveEvent( self, event )

    def widget( self, otherWidget = None ):
        if otherWidget is not None:
            w = self._widget.widget()
            self._widget.setWidget( otherWidget )
            self._widget.widget().setAttribute( Qt.WA_OpaquePaintEvent )
            self._widget.widget().hide()
            w.deleteLater()
        return self._widget.widget()

