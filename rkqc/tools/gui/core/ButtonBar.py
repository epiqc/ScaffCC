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

class ButtonBar( QWidget ):
    def __init__( self, parent = None ):
        QWidget.__init__( self, parent )

        self.buttons = QWidget( self )
        self.buttons.setLayout( QHBoxLayout() )
        self.buttons.layout().setMargin( 0 )
        self.buttons.layout().setSpacing( 0 )
        self.buttons.layout().addStretch()

        self.stack = QStackedWidget( self )

        self.setLayout( QVBoxLayout() )
        self.layout().setMargin( 0 )
        self.layout().setSpacing( 0 )
        self.layout().addWidget( self.stack )
        self.layout().addWidget( self.buttons )

    def addWidget( self, page, icon, text ):
        button = QPushButton( icon, text, self.buttons )
        button.setCheckable( True )
        button.setFlat( True )
        button.installEventFilter( self )

        # Check the first added button
        if len( self.buttons.children() ) == 2:
            button.setChecked( True )
        self.buttons.layout().insertWidget( len( self.buttons.children() ) - 2, button )
        self.stack.addWidget( page )

    def eventFilter( self, obj, event ):
        if isinstance( obj, QPushButton ) and event.type() == QEvent.MouseButtonPress:
            index = self.buttons.children().index( obj ) - 1
            for button in filter( lambda b: isinstance( b, QPushButton ), self.buttons.children() ):
                button.setChecked( False )

            if self.currentIndex() == index:
                self.stack.setVisible( not self.stack.isVisible() )
                obj.setChecked( self.stack.isVisible() )
            else:
                self.stack.setVisible( True )
                obj.setChecked( True )

            self.stack.setCurrentIndex( index )
            return True

        return QWidget.eventFilter( self, obj, event )


    def setCurrentIndex( self, index ):
        if index == self.stack.currentIndex(): return

        self.eventFilter( self.buttons.children()[1 + index], QEvent( QEvent.MouseButtonPress ) )
        self.stack.setCurrentIndex( index )

    def currentIndex( self ):
        return self.stack.currentIndex()
