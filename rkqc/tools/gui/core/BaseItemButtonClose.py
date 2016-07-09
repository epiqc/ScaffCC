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

class BaseItemButtonClose( BaseItemButton ):
    def __init__( self, parent = None ):
        BaseItemButton.__init__( self, parent )

        self.setPos( 90, -30 )
        self.setOpacity( 0 )
        self.setToolTip( "Delete Item from Graph" )

        # Appear/Delete Animation
        self.parentAppearAnimation = QPropertyAnimation( parent, "scale" )
        self.parentAppearAnimation.setDuration( 250 )
        self.parentAppearAnimation.setStartValue( 0.1 )
        self.parentAppearAnimation.setEndValue( 1.0 )
        self.parentAppearAnimation.setEasingCurve( QEasingCurve.OutBack )

    def backgroundColor( self ):
        return QColor( "#aa0000" )

    def paint( self, painter, option, widget = None ):
        BaseItemButton.paint( self, painter, option, widget )

        # Cross
        painter.setPen( QPen( QColor( Qt.gray ), 2 ) )
        painter.drawLine( 7, 7, 13, 13 )
        painter.drawLine( 13, 7, 7, 13 )

    def mousePressEvent( self, event ):
        self.parentObject().scene().beforeDelete( self.parentObject() )
        self.parentAppearAnimation.setDirection( QAbstractAnimation.Backward )
        self.parentAppearAnimation.start()
        self.connect( self.parentAppearAnimation, SIGNAL( 'finished()' ), self.parentObject().deleteLater )
