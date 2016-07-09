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

class PortItem( QGraphicsObject ):
    radius = 8
    valueChanged = pyqtSignal()
    connectionsChanged = pyqtSignal()

    def __init__( self, datatype, direction = Qt.AlignTop, parent = None ):
        QGraphicsObject.__init__( self, parent )

        self.setCacheMode( QGraphicsItem.DeviceCoordinateCache )

        self.datatype = datatype
        self.direction = direction
        self.datatypeWidth = 200
        self.datatypeHeight = 15
        self._value = None

    def boundingRect( self ):
        y = -self.radius / 2 if self.direction != Qt.AlignTop else -self.radius / 2 - 20 - self.datatypeHeight
        return QRectF( -self.datatypeWidth / 2, y, self.datatypeWidth, self.radius + 20 + self.datatypeHeight )

    def paint( self, painter, option, widget = None ):
        painter.setBrush( QColor( Qt.black ).lighter() )
        painter.setPen( Qt.black )
        painter.drawRect( QRectF( -self.radius / 2, -self.radius / 2, self.radius, self.radius ) )
        self.datatypeWidth = painter.fontMetrics().width( self.datatype )
        self.datatypeHeight = painter.fontMetrics().height()
        y = self.radius / 2 - self.datatypeHeight if self.direction == Qt.AlignTop else self.radius + 8
        painter.drawText( -self.datatypeWidth / 2.0, y, self.datatype )

    def setValue( self, v ):
        self._value = v
        if self._value is not None: self.valueChanged.emit()

    def value( self ):
        return self._value
