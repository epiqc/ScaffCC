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

from PyQt4.QtCore import SIGNAL
from PyQt4.QtGui import QLineEdit, QTabBar, QTabWidget

class RenamableTabWidget( QTabWidget ):
    class RenamableTabBar( QTabBar ):
        class FramelessLineEdit( QLineEdit ):
            def __init__( self, parent = None ):
                QLineEdit.__init__( self, parent )

                self.setFrame( False )
                self.hide()

            def focusOutEvent( self, _ ):
                self.hide()

        def __init__( self, parent = None ):
            QTabBar.__init__( self, parent )

            self.lineEdit = self.FramelessLineEdit( self )
            self.currentTab = -1

            self.connect( self.lineEdit, SIGNAL( 'returnPressed()' ), self.slotReturnPressed )

        def slotReturnPressed( self ):
            self.setTabText( self.currentTab, self.lineEdit.text() )
            self.lineEdit.hide()

        def mousePressEvent( self, event ):
            self.lineEdit.hide()
            QTabBar.mousePressEvent( self, event )

        def resizeEvent( self, event ):
            self.lineEdit.hide()
            QTabBar.resizeEvent( self, event )

        def mouseDoubleClickEvent( self, event ):
            QTabBar.mouseDoubleClickEvent( self, event )
            index = self.tabAt( event.pos() )
            if index < 0:
                self.lineEdit.hide()
                return

            self.currentTab = index
            self.lineEdit.show()
            self.lineEdit.setGeometry( self.tabRect( index ) )
            self.lineEdit.setText( self.tabText( index ) )
            self.lineEdit.setFocus()
            self.lineEdit.selectAll()

    def __init__( self, parent = None ):
        QTabWidget.__init__( self, parent )
        self.setTabBar( self.RenamableTabBar( self ) )

    def mousePressEvent( self, event ):
        self.tabBar().lineEdit.hide()
        QTabWidget.mousePressEvent( self, event )
