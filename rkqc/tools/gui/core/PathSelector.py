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
from PyQt4.QtCore import QString, pyqtSignal, SIGNAL
from PyQt4.QtGui import QCompleter, QDirModel, QFileDialog, QHBoxLayout, QIcon, QLineEdit, QToolButton, QWidget

class PathSelector( QWidget ):
    pathChanged = pyqtSignal( QString )

    def __init__( self, parent = None ):
        QWidget.__init__( self, parent )

        # Components
        self.path_edit = QLineEdit( self )
        self.path_edit.setReadOnly( True )

        self.button = QToolButton( self )
        self.button.setIcon( QIcon.fromTheme( "document-open-folder" ) )

        # Completer
        completer = QCompleter( self )
        completer.setModel( QDirModel( completer ) )
        self.path_edit.setCompleter( completer )

        # Setup Widget
        self.setLayout( QHBoxLayout() )
        self.layout().setMargin( 0 )
        self.layout().setSpacing( 0 )
        self.layout().addWidget( self.path_edit )
        self.layout().addWidget( self.button )

        # Actions
        self.connect( self.button, SIGNAL( 'clicked()' ), self.selectPath )

    def selectPath( self ):
        self.path_edit.setText( QFileDialog.getExistingDirectory( self, "Open Directory" ) )
        self.pathChanged.emit( self.path_edit.text() )

    def path( self ):
        return self.path_edit.text()

    def setPath( self, p ):
        self.path_edit.setText( p )
