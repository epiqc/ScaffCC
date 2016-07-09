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
from PyQt4.QtGui import QColor, QColorDialog, QComboBox, QPainter, QPalette, QStyle, QStyleOptionComboBox, QStylePainter

class ColorComboBox( QComboBox ):
    def __init__( self, parent = None ):
        QComboBox.__init__( self, parent )

        self.custom = QColor( Qt.black )
        self.populateList()

        self.connect( self, SIGNAL( 'activated(int)' ), self.onActivated )

    def color( self ):
        return self.itemData( self.currentIndex(), Qt.DecorationRole ).toPyObject()

    def setColor( self, color ):
        index = self.findData( color, Qt.DecorationRole )
        if index >= 0:
            self.setCurrentIndex( index )
        else:
            self.setCurrentIndex( self.count() - 1 )
            self.custom = color
            self.updateCustomColor()

    def populateList( self ):
        colorNames = QColor.colorNames()
        for i, name in enumerate( colorNames ):
            self.insertItem( i, str( name ).capitalize() )
            self.setItemData( i, QColor( name ), Qt.DecorationRole )

        self.insertItem( len( colorNames ), "" )
        self.updateCustomColor()

    def updateCustomColor( self ):
        self.setItemText( self.count() - 1, "Custom (%s)" % self.custom.name() )
        self.setItemData( self.count() - 1, self.custom, Qt.DecorationRole )

    def onActivated( self, index ):
        if index == self.count() - 1:
            color = QColorDialog.getColor( self.custom, self )
            if color.isValid():
                self.custom = color
                self.updateCustomColor()

    def paintEvent( self, _ ):
        painter = QStylePainter( self )
        painter.setPen( self.palette().color( QPalette.Text ) )

        opt = QStyleOptionComboBox()
        self.initStyleOption( opt )
        painter.drawComplexControl( QStyle.CC_ComboBox, opt )

        frame = self.style().subControlRect( QStyle.CC_ComboBox, opt, QStyle.SC_ComboBoxEditField, self )
        painter.setRenderHint( QPainter.Antialiasing )
        painter.setPen( Qt.transparent )
        painter.setBrush( self.color() )
        painter.drawRoundedRect( frame.adjusted( 1, 1, -1, -2 ), 2, 2 )
