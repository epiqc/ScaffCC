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
from PyQt4.QtGui import QCursor, QIcon

import os
from tempfile import NamedTemporaryFile

from revkit import *

from core.BaseItem import *

from core.SpinBoxDelegate import *

from ui.PatternInput import *
from ui.DesignerWidget import *

class PatternInput( DesignerWidget ):
    deleteColumn = -1

    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_PatternInput, parent )

        self.filename = QString()
        self.openButton.setIcon( QIcon.fromTheme( "document-open" ) )

        self.connect( self.openButton, SIGNAL( 'released()' ), self.slotOpenFile )

        ### States
        self.stateTable.setItemDelegateForColumn( 1, SpinBoxDelegate( self ) )

        self.addState.setIcon( QIcon.fromTheme( "list-add" ) )
        self.removeState.setIcon( QIcon.fromTheme( "list-remove" ) )

        self.connect( self.stateTable, SIGNAL( 'itemSelectionChanged()' ), self.slotStateSelectionChanged )
        self.connect( self.addState, SIGNAL( 'released()' ), self.slotAddState )
        self.connect( self.removeState, SIGNAL( 'released()' ), self.slotRemoveState )

        ### Sequences
        self.sequencesTable.setItemDelegate( SpinBoxDelegate( self ) )

        self.addSequence.setIcon( QIcon.fromTheme( "list-add" ) )
        self.removeSequence.setIcon( QIcon.fromTheme( "list-remove" ) )

        self.connect( self.sequencesTable, SIGNAL( 'itemSelectionChanged()' ), self.slotSequenceSelectionChanged )
        self.connect( self.addSequence, SIGNAL( 'released()' ), self.slotAddSequence )
        self.connect( self.removeSequence, SIGNAL( 'released()' ), self.slotRemoveSequence )
        # Add Input Button
        self.addInputButton = QToolButton( self.sequencesTable )
        self.addInputButton.setText( "Add Input" )
        self.addInputButton.setIcon( QIcon.fromTheme( "list-add" ) )
        self.addInputButton.setAutoRaise( True )
        self.addInputButton.setToolButtonStyle( Qt.ToolButtonTextBesideIcon )
        self.addInputButton.move( 2, 2 )
        self.connect( self.addInputButton, SIGNAL( 'released()' ), self.slotAddInput )
        self.connect( self.sequencesTable.horizontalHeader(), SIGNAL( 'sectionResized(int, int, int)' ), self.slotAdjustAddInputPosition )
        self.connect( self.sequencesTable.horizontalHeader(), SIGNAL( 'sectionDoubleClicked( int )' ), self.slotRequestTextForInput )

        # Remove Input Button
        self.removeInputButton = QToolButton( self.sequencesTable )
        self.removeInputButton.setIcon( QIcon.fromTheme( "list-remove" ) )
        self.removeInputButton.setAutoRaise( True )
        self.removeInputButton.move( 2, 2 )
        self.removeInputButton.hide()
        self.connect( self.removeInputButton, SIGNAL( 'released()' ), self.slotRemoveInput )

        self.sequencesTable.horizontalHeader().installEventFilter( self )
        self.removeInputButton.installEventFilter( self )

    def write( self, filename ):
        # Check whether all states have a number
        if False in [ self.stateTable.model().index( row, 1 ).data().toInt()[1] for row in range( self.stateTable.rowCount() ) ]:
            # TODO emit error
            return False

        # Check whether all states have an identifier
        if 0 in [ len( self.stateTable.model().index( row, 0 ).data().toString() ) for row in range( self.stateTable.rowCount() ) ]:
            # TODO emit error
            return False

        # Check wheter inputs are specified and non-empty
        inputs = [ str( self.sequencesTable.horizontalHeaderItem( column ).text() ) for column in range( self.sequencesTable.columnCount() ) ]

        if len( inputs ) == 0:
            # TODO emit error
            return False

        if 0 in map( len, inputs ):
            # TODO emit error
            return False

        # Check whether pattern are non-empty
        if False in [ self.sequencesTable.model().index( row, column ).data().toInt()[1] for column in range( self.sequencesTable.columnCount() ) for row in range( self.sequencesTable.rowCount() ) ]:
            # TODO emit error
            return False

        ### Output
        with open( filename, 'w' ) as f:
            if self.stateTable.rowCount() > 0:
                f.write( '\n'.join( [ '.init %s %d' % ( self.stateTable.model().index( row, 0 ).data().toString(), self.stateTable.model().index( row, 1 ).data().toInt()[0] ) for row in range( self.stateTable.rowCount() ) ] ) + '\n' )
            f.write( '.inputs %s' % ' '.join( inputs ) + '\n' )
            f.write( '.begin\n' )

            pattern = [ [ self.sequencesTable.model().index( row, column ).data().toInt()[0] for column in range( self.sequencesTable.columnCount() ) ] for row in range( self.sequencesTable.rowCount() ) ]
            f.write( '\n'.join( [ ' '.join( map( str, p ) ) for p in pattern ] ) + '\n' )
            f.write( '.end\n' )

        return True

    def eventFilter( self, obj, event ):
        if obj == self.sequencesTable.horizontalHeader():
            if event.type() == QEvent.HoverMove:
                newColumn = self.sequencesTable.horizontalHeader().logicalIndexAt( event.pos() )
                if newColumn != self.deleteColumn:
                    if not self.removeInputButton.isVisible() and newColumn >= 0:
                        self.removeInputButton.show()
                    elif self.removeInputButton.isVisible() and newColumn == -1:
                        self.removeInputButton.hide()
                    self.deleteColumn = newColumn
                    if newColumn >= 0:
                        x = sum( map( self.sequencesTable.columnWidth, range( self.deleteColumn + 1 ) ) ) - 20
                        if self.sequencesTable.rowCount() > 0:
                            x += 15
                        self.removeInputButton.move( x, 2 )
            elif event.type() == QEvent.HoverLeave:
                child = self.childAt( self.mapFromGlobal( QCursor.pos() ) )
                if not isinstance( child, QToolButton ):
                    self.removeInputButton.hide()
                    self.deleteColumn = -1
        elif obj == self.removeInputButton:
            if event.type() == QEvent.HoverLeave:
                child = self.childAt( self.mapFromGlobal( QCursor.pos() ) )
                if isinstance( child, QToolButton ):
                    self.removeInputButton.hide()
                    self.deleteColumn = -1

        return DesignerWidget.eventFilter( self, obj, event )

    def deleteRows( self, table ):
        messageBox = QMessageBox( QMessageBox.Question, "Delete rows", "Do you want to delete all selected rows?" )
        yesButton = messageBox.addButton( "Yes, delete selected rows", QMessageBox.YesRole )
        noButton = messageBox.addButton( "No", QMessageBox.NoRole )
        messageBox.setDefaultButton( noButton )
        messageBox.exec_()
        if messageBox.clickedButton() == yesButton:
            for row in reversed( list( set( [ index.row() for index in table.selectedIndexes() ] ) ) ):
                table.removeRow( row )

    def load( self, filename ):
        if not filename.isEmpty():
            self.filename = filename
            p = pattern()
            if read_pattern( p, str( filename ) ) == True:
                self.stateTable.clearContents()
                self.stateTable.setRowCount( len( p.initializers ) )
                for row, entry in enumerate( p.initializers.items() ):
                    self.stateTable.setItem( row, 0, QTableWidgetItem( entry[0] ) )
                    value = QTableWidgetItem()
                    value.setData( Qt.DisplayRole, entry[1] )
                    self.stateTable.setItem( row, 1, value )

                self.sequencesTable.clear()
                self.sequencesTable.setColumnCount( len( p.inputs ) )
                self.sequencesTable.setRowCount( len( p.patterns ) )
                self.sequencesTable.setHorizontalHeaderLabels( p.inputs )

                for row, p in enumerate( p.patterns ):
                    for column, v in enumerate( p ):
                        value = QTableWidgetItem()
                        value.setData( Qt.DisplayRole, v )
                        self.sequencesTable.setItem( row, column, value )

                self.slotAdjustAddInputPosition()
                if self.sequencesTable.columnCount() > 0:
                    self.addSequence.setEnabled( True )
    @pyqtSlot()
    def slotOpenFile( self ):
        filename = QFileDialog.getOpenFileName( None, 'Open Pattern', '', 'RevLib Pattern (*.sim)' )
        self.load( filename )

    @pyqtSlot()
    def slotStateSelectionChanged( self ):
        self.removeState.setEnabled( len( self.stateTable.selectedIndexes() ) > 0 )

    @pyqtSlot()
    def slotAddState( self ):
        self.stateTable.insertRow( self.stateTable.rowCount() )

    @pyqtSlot()
    def slotRemoveState( self ):
        self.deleteRows( self.stateTable )

    @pyqtSlot()
    def slotSequenceSelectionChanged( self ):
        self.removeSequence.setEnabled( len( self.sequencesTable.selectedIndexes() ) > 0 )

    @pyqtSlot()
    def slotAddSequence( self ):
        self.sequencesTable.insertRow( self.sequencesTable.rowCount() )
        self.slotAdjustAddInputPosition()

    @pyqtSlot()
    def slotRemoveSequence( self ):
        self.deleteRows( self.sequencesTable )

    @pyqtSlot()
    def slotAddInput( self ):
        self.sequencesTable.insertColumn( self.sequencesTable.columnCount() )
        headerItem = QTableWidgetItem( str( self.sequencesTable.columnCount() ) )
        self.sequencesTable.setHorizontalHeaderItem( self.sequencesTable.columnCount() - 1, headerItem )
        self.slotAdjustAddInputPosition()
        self.slotRequestTextForInput( self.sequencesTable.columnCount() - 1 )
        self.addSequence.setEnabled( True )

    @pyqtSlot()
    def slotAdjustAddInputPosition( self ):
        x = sum( map( self.sequencesTable.columnWidth, range( self.sequencesTable.columnCount() ) ) ) + 3
        if self.sequencesTable.rowCount() > 0:
            x += 15
        self.addInputButton.move( x, 2 )

    @pyqtSlot()
    def slotRequestTextForInput( self, column ):
        item = self.sequencesTable.horizontalHeaderItem( column )
        ( s, ok ) = QInputDialog.getText( None, "Input", "You can change the name by double clicking on the column header.\n\nName of Input:", QLineEdit.Normal, item.text() )
        if ok:
            item.setText( str( s ) )

    @pyqtSlot()
    def slotRemoveInput( self ):
        messageBox = QMessageBox( QMessageBox.Question, "Delete input", "Do you want to delete the input?" )
        yesButton = messageBox.addButton( "Yes, delete input", QMessageBox.YesRole )
        noButton = messageBox.addButton( "No", QMessageBox.NoRole )
        messageBox.setDefaultButton( noButton )
        messageBox.exec_()
        if messageBox.clickedButton() == yesButton:
            self.sequencesTable.removeColumn( self.deleteColumn )
            self.removeInputButton.hide()
            self.slotAdjustAddInputPosition()

            if self.sequencesTable.columnCount() == 0:
                self.addSequence.setEnabled( False )
                self.sequencesTable.setRowCount( 0 )

@item( "Simulation Pattern (*.sim)",
       iconname = "document-export",
       provides = [ "Pattern File" ],
       properties = [ "filename" ],
       widget = PatternInput )
class PatternInputItem( BaseItem ):
    """This item enables the definition of simulation patterns that should be applied to a circuit. The pattern can be provided manually or in terms a *.sim file."""
    def onCreate( self ):
        self.setText( "Pattern" )
        self.setState( self.CONFIGURED )

    def executeEvent( self, inputs ):
        f = NamedTemporaryFile( 'w', delete = False )
        filename = f.name
        f.close()

        self.widget.write( filename )
        p = pattern()
        read_pattern( p, filename )
        os.unlink( filename )

        return [ p ]

    @action( "Open Pattern File", "document-open" )
    def loadPatternFile( self ):
        self.widget.slotOpenFile()

    def onFilenameChanged( self, value ):
        self.widget.load( value )

    def getFilename( self ):
        return self.widget.filename
