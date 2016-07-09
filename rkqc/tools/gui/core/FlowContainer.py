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

from PyQt4.QtCore import SIGNAL, QFileInfo
from PyQt4.QtGui import QAction, QIcon, QMessageBox, QTabWidget

from FlowWidget import *

class FlowContainer( QTabWidget ):
    def __init__( self, parent = None ):
        QTabWidget.__init__( self, parent )

        self.setDocumentMode( True )
        self.setTabsClosable( True )
        self.setupActions()

        self.connect( self, SIGNAL( 'tabCloseRequested(int)' ), self.slotClose )

    def setupActions( self ):
        self.newAction = QAction( QIcon.fromTheme( "document-new" ), "New", self )
        self.openAction = QAction( QIcon.fromTheme( "document-open" ), "Open", self )
        self.saveAction = QAction( QIcon.fromTheme( "document-save" ), "Save", self )
        self.saveAsAction = QAction( QIcon.fromTheme( "document-save-as" ), "Save As", self )
        self.runAction = QAction( QIcon.fromTheme( "run-build" ), "Run", self )

        self.connect( self.newAction, SIGNAL( 'triggered()' ), self.slotNew )
        self.connect( self.openAction, SIGNAL( 'triggered()' ), self.slotOpen )
        self.connect( self.saveAction, SIGNAL( 'triggered()' ), self.slotSave )
        self.connect( self.saveAsAction, SIGNAL( 'triggered()' ), self.slotSaveAs )
        self.connect( self.runAction, SIGNAL( 'triggered()' ), self.slotRun )

    def updateActions( self ):
        self.saveAction.setEnabled( self.count() > 0 )
        self.saveAsAction.setEnabled( self.count() > 0 )
        self.runAction.setEnabled( self.count() > 0 )

    def slotNew( self ):
        w = FlowWidget( self )
        self.setCurrentIndex( self.addTab( w, QIcon.fromTheme( "text-rdf+xml" ), "New Graph" ) )

        w.scene().before_run.connect( self.window().messagesWidget.clear )
        w.scene().error_sent.connect( lambda m: self.window().messagesWidget.addMessage( m, "Flow Diagram", "error" ) )
        w.scene().item_error_sent.connect( lambda m, s: self.window().messagesWidget.addMessage( m, s, "error" ) )
        w.scene().filename_changed.connect( self.slotFilenameChanged )
        w.scene().modified_changed.connect( self.slotModifiedChanged )

        self.updateActions()

    def slotOpen( self ):
        # Overwrite
        new = False
        if not( self.count() > 0 and self.currentWidget().scene().filename is None and not self.currentWidget().scene().modified ):
            new = True
            self.slotNew()

        if not self.currentWidget().scene().slotLoad() and new:
            self.removeTab( self.count() - 1 )
            self.updateActions()

    def slotSave( self ):
        self.currentWidget().scene().slotSave()

    def slotSaveAs( self ):
        self.currentWidget().scene().slotSaveAs()

    def slotRun( self ):
        self.currentWidget().scene().run()

    def slotClose( self, index ):
        if self.widget( index ).scene().modified:
            messageBox = QMessageBox( QMessageBox.Question, "Modified Graph", "Do you wanna save the graph before closing?" )
            yesButton = messageBox.addButton( "Yes, save", QMessageBox.YesRole )
            noButton = messageBox.addButton( "No, close without saving", QMessageBox.NoRole )
            messageBox.setDefaultButton( yesButton )
            messageBox.exec_()
            if messageBox.clickedButton() == yesButton:
                if not self.widget( index ).scene().slotSave():
                    return
        self.removeTab( index )
        self.updateActions()

    def slotFilenameChanged( self ):
        w = self.sender().views()[0]
        self.setTabText( self.indexOf( w ), QFileInfo( self.sender().filename ).fileName() )

    def slotModifiedChanged( self ):
        w = self.sender().views()[0]
        self.tabBar().setTabTextColor( self.indexOf( w ), Qt.red if self.sender().modified else Qt.black )
