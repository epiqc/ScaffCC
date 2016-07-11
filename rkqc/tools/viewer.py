#!/home/adam/Desktop/EPiQC_Releases/Full_Install_Testing/July10_ScaffCC/rkqc/python
#!/home/adam/Desktop/EPiQC_Releases/Full_Install_Testing/EPiQC_Scaffold/rkqc/python
#!/home/adam/Desktop/EPiQC_Releases/Full_Install_Testing/EPiQC_Scaffold/rkqc/python
#!/home/adam/Desktop/Development/RKQC/python
#!/home/adam/New_Installs/Scaffold_New/rkqc/python
#!/home/adam/Documents/revkit-1.3/python
#!/usr/bin/python

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

import os, sys
sys.path.append(os.path.dirname(sys.path[0]))
from revkit import *
from revkitui import *

from PyQt4.QtCore import *
from PyQt4.QtGui import *

class InfoDialog( QDialog ):
    def __init__( self, circ, parent ):
        QDialog.__init__( self, parent, Qt.Dialog )

        self.setWindowTitle( 'Circuit details' )
        self.resize( 20, 20 )

        self.setLayout( QVBoxLayout() )
        self.layout().setMargin( 0 )

        widget = QWidget()

        layout = QFormLayout()
        widget.setLayout( layout )
        layout.addRow( QLabel( 'Gate count:', widget ), QLabel( str( costs( circ, gate_costs() ) ), widget ) )
        layout.addRow( QLabel( 'Line count:', widget ), QLabel( str( costs( circ, line_costs() ) ), widget ) )
        layout.addRow( QLabel( 'Quantum cost:', widget ), QLabel( str( costs( circ, quantum_costs() ) ), widget ) )
        layout.addRow( QLabel( 'Transistor cost:', widget ), QLabel( str( costs( circ, transistor_costs() ) ), widget ) )

        widget2 = QWidget()
        widget2.setLayout( QHBoxLayout() )
        widget2.layout().addStretch()
        button = QPushButton( 'Close' )
        button.setAutoDefault( True )
        widget2.layout().addWidget( button )

        self.connect( button, SIGNAL( 'clicked()' ), self.close )

        self.layout().addWidget( widget )
        self.layout().addWidget( widget2 )

class AboutDialog( QDialog ):
    def __init__( self, parent ):
        QDialog.__init__( self, parent, Qt.Dialog )

        self.setWindowTitle( 'About RevKit Viewer' )
        self.resize( 20, 20 )

        self.setLayout( QVBoxLayout() )
        self.layout().setMargin( 0 )

        widget2 = QWidget()
        widget2.setLayout( QHBoxLayout() )
        widget2.layout().addStretch()
        button = QPushButton( 'Close' )
        button.setAutoDefault( True )
        widget2.layout().addWidget( button )

        self.connect( button, SIGNAL( 'clicked()' ), self.close )

        self.layout().addWidget( QLabel( '(c) 2009-2011 by the RevKit Developers' ) )
        self.layout().addWidget( widget2 )

class SmallerTreeView( QTreeView ):
    def __init__( self, parent = None ):
        QTreeView.__init__( self, parent )

    def sizeHint( self ):
        return QSize( 200, 200 )

class Viewer( QMainWindow ):
    def __init__( self ):
        QMainWindow.__init__( self )

        self.setWindowTitle( "RevKit Viewer" )

        self.setupDockWidgets()
        self.setupActions()
        self.setupMenuBar()
        self.setupToolBar()

        # CircuitView
        self.setCentralWidget( QStackedWidget( self ) )
        self.connect( self.centralWidget(), SIGNAL( 'currentChanged(int)' ), self.updateStatusBar )

        self.setupStatusBar()

        self.resize( 600, 300 )

    def setupDockWidgets( self ):
        self.hierarchyDock = QDockWidget( "Hierarchy", self )
        self.hierarchyView = SmallerTreeView( self )
        self.hierarchyView.setExpandsOnDoubleClick( False )
        self.hierarchyDock.setWidget( self.hierarchyView )
        self.hierarchyDock.setVisible( False )
        self.hierarchyDock.setFeatures( QDockWidget.DockWidgetClosable )
        self.addDockWidget( Qt.LeftDockWidgetArea, self.hierarchyDock )

        # Actions
        self.connect( self.hierarchyView, SIGNAL( 'doubleClicked(QModelIndex)' ), self.loadCircuitFromHierarchy )

    def setupActions( self ):
        path = os.path.realpath( os.path.abspath( __file__ ) )
        path = path.replace( os.path.basename( __file__ ), 'icons/' )

        self.openAction = QAction( QIcon( path + 'document-open.png' ), '&Open...', self )
        self.openAction.setStatusTip( 'Open a circuit realization in RevLib format' )
        self.imageAction = QAction( QIcon( path + 'image-x-generic.png' ), 'Save as &Image...', self )
        self.imageAction.setStatusTip( 'Saves the circuit as an image file (PNG or JPG)' )
        self.latexAction = QAction( QIcon( path + 'text-x-tex.png' ), 'Save as &LaTeX...', self )
        self.latexAction.setStatusTip( 'Saves the LaTeX code to generate this circuit' )
        self.exitAction = QAction( QIcon( path + 'application-exit.png' ), '&Quit', self )
        self.exitAction.setStatusTip( 'Quits the program' )

        self.infoAction = QAction( QIcon( path + 'dialog-information.png' ), '&Circuit details', self )
        self.infoAction.setStatusTip( 'Opens a dialog with circuit information' )
        self.specAction = QAction( QIcon( path + 'view-form-table.png' ), '&View truth table', self )
        self.specAction.setStatusTip( 'Displays the full truth table of the circuit, obtained by simulation' )
        self.partialAction = QAction( QIcon( path + 'view-form-table.png' ), '&View partial truth table', self )
        self.partialAction.setStatusTip( 'Displays a truth table only for non-constant and non-garbage signals' )

        self.aboutAction = QAction( QIcon( path + 'help-about.png' ), '&About', self )
        self.aboutAction.setStatusTip( 'Displays information about the RevKit Viewer' )

        # Dock Widgets
        self.hierarchyDock.toggleViewAction().setIcon( QIcon( path + 'view-sidetree.png' ) )

        self.connect( self.openAction, SIGNAL( 'triggered()' ), self.open )
        self.connect( self.imageAction, SIGNAL( 'triggered()' ), self.saveImage )
        self.connect( self.latexAction, SIGNAL( 'triggered()' ), self.saveLatex )
        self.connect( self.exitAction, SIGNAL( 'triggered()' ), SLOT( 'close()' ) )

        self.connect( self.infoAction, SIGNAL( 'triggered()' ), self.info )
        self.connect( self.specAction, SIGNAL( 'triggered()' ), self.truthTable )
        self.connect( self.partialAction, SIGNAL( 'triggered()' ), self.partialTable )

        self.connect( self.aboutAction, SIGNAL( 'triggered()' ), self.about )

    def setupMenuBar( self ):
        menubar = self.menuBar()

        file = menubar.addMenu( '&File' )
        file.addAction( self.openAction )
        file.addAction( self.imageAction )
        file.addAction( self.latexAction )
        file.addSeparator()
        file.addAction( self.exitAction )

        view = menubar.addMenu( '&View' )
        view.addAction( self.infoAction )
        view.addSeparator()
        view.addAction( self.specAction )
        view.addAction( self.partialAction )

        help = menubar.addMenu( '&Help' )
        help.addAction( self.aboutAction )

    def setupToolBar( self ):
        toolbar = self.addToolBar( 'Main' )
        toolbar.setIconSize( QSize( 32, 32 ) )

        toolbar.addAction( self.openAction )
        toolbar.addAction( self.imageAction )
        toolbar.addSeparator()
        toolbar.addAction( self.infoAction )
        toolbar.addAction( self.partialAction )

        toolbarDock = QToolBar( self )
        self.addToolBar( Qt.LeftToolBarArea, toolbarDock )
        toolbarDock.setOrientation( Qt.Vertical )
        toolbarDock.setMovable( False )

        toolbarDock.addAction( self.hierarchyDock.toggleViewAction() )

    def setupStatusBar( self ):
        self.statusBar()
        self.updateStatusBar()

    zoom_widget = None # Pointer to the current zoom widget
    def updateStatusBar( self ):
        if self.zoom_widget is not None:
            self.statusBar().removeWidget( self.zoom_widget )
            self.zoom_widget = None

        if self.centralWidget().currentWidget():
            self.zoom_widget = self.centralWidget().currentWidget().zoomWidget()
            self.statusBar().addPermanentWidget( self.zoom_widget )
            self.zoom_widget.show()

    def open( self ):
        filename = str( QFileDialog.getOpenFileName( self, 'Open Realization', '', 'RevLib Realization (*.real)' ) )
        if len( filename ):
            self.openCircuitFromFilename( filename )

    def openCircuitFromFilename( self, filename, load_hierarchy = True ):
        circ = circuit()
        read_realization( circ, filename )
        self.openCircuit( circ )

    def openCircuit( self, circ ):
        # remove all views TODO make more efficient (also memory)
        while self.centralWidget().count():
            self.centralWidget().removeWidget( self.centralWidget().widget( 0 ) )

        # Save this, since all the other circuits are references
        self.circ = circ

        # hierarchy
        tree = hierarchy_tree()
        circuit_hierarchy( circ, tree )
        self.hierarchyView.setModel( HierarchyModel( tree ) )
        self.hierarchyView.setColumnWidth( 0, 150 )
        self.hierarchyView.resizeColumnToContents( 1 )
        self.hierarchyCurrentIndex = self.hierarchyView.model().index( 0, 0 )

        self.circuits = [ tree.node_circuit( i ) for i in range( tree.size() ) ]
        for c in self.circuits:
            view = CircuitView( c, self )
            view.gateDoubleClicked.connect( self.slotGateDoubleClicked )
            self.centralWidget().addWidget( view )

    def saveImage( self ):
        filename = QFileDialog.getSaveFileName( self, 'Save as Image', '', 'PNG image (*.png);;JPG image (*.jpg)' )
        if not filename.isEmpty():
            scene = self.centralWidget().currentWidget().scene()
            pixmap = QPixmap( scene.width(), scene.height() )
            painter = QPainter( pixmap )
            scene.render( painter )
            pixmap.save( filename )
            painter.end()

    def saveLatex( self ):
        filename = QFileDialog.getSaveFileName( self, 'Save as LaTeX', '', 'LaTeX file (*.tex)' )
        if not filename.isEmpty():
            with open( str( filename ), 'w' ) as f:
                f.write( create_image( self.circ ) )

    def info( self ):
        dialog = InfoDialog( self.circ, self )
        dialog.exec_()

    def truthTable( self ):
        dialog = QDialog( self, Qt.Dialog )
        dialog.setWindowTitle( 'Truth Table' )

        spec = binary_truth_table()
        flattened = circuit()
        flatten_circuit( self.circ, flattened )
        circuit_to_truth_table( flattened, spec )

        n = self.circ.lines

        table = QTableWidget( 2 ** n, 2 * n, dialog )
        table.setHorizontalHeaderLabels( self.circ.inputs + self.circ.outputs )
        table.setVerticalHeaderLabels( map( str, range( 0, 2 ** n ) ) )
        table.setAlternatingRowColors( True )
        table.setShowGrid( False )

        row = 0
        for entry in spec.entries:

            valid = True
            for c in range( 0, n ):
                if not self.circ.constants[c] is None and entry[0][c] != self.circ.constants[c]:
                    valid = False
                    break


            for col in range( 0, 2 * n ):
                item = QTableWidgetItem( '1' if ( entry[0] + entry[1] )[col] else '0' )
                flags = Qt.ItemIsSelectable
                if valid and not ( col >= n and self.circ.garbage[col % n] ) and not ( col < n and not self.circ.constants[col] is None ):
                    flags |= Qt.ItemIsEnabled
                item.setFlags( flags )
                if col >= n and not self.circ.garbage[col % n]:
                    item.setBackground( Qt.lightGray )
                table.setItem( row, col, item )
            row += 1

        table.resizeColumnsToContents()

        dialog.setLayout( QVBoxLayout() )
        dialog.layout().setMargin( 0 )
        dialog.layout().addWidget( table )

        dialog.exec_()

    def partialTable( self ):
        dialog = QDialog( self, Qt.Dialog )
        dialog.setWindowTitle( 'Partial Truth Table' )

        spec = binary_truth_table()
        settings = properties()
        settings.set_bool( "partial", True )
        flattened = circuit()
        flatten_circuit( self.circ, flattened )
        circuit_to_truth_table( flattened, spec, py_partial_simulation_func( settings ) )

        n = len( filter( lambda x: x is None, self.circ.constants ) )
        m = len( filter( lambda x: not x, self.circ.garbage ) )

        table = QTableWidget( 2 ** n, n + m, dialog )
        input_labels = map( lambda x: x[0], filter( lambda x: x[1] is None, map( lambda x, y: [x,y], self.circ.inputs, self.circ.constants ) ) )
        output_labels = map( lambda x: x[0], filter( lambda x: not x[1], map( lambda x, y: [x,y], self.circ.outputs, self.circ.garbage ) ) )
        table.setHorizontalHeaderLabels( input_labels + output_labels )
        table.setVerticalHeaderLabels( map( lambda x: "", range( 0, 2 ** n ) ) )
        table.setAlternatingRowColors( True )
        table.setShowGrid( False )

        row = 0
        for entry in spec.entries:
            for col in range( 0, n + m ):
                item = QTableWidgetItem( '1' if ( entry[0] + entry[1] )[col] else '0' )
                item.setFlags( Qt.ItemIsSelectable | Qt.ItemIsEnabled )
                if col >= n:
                    item.setBackground( Qt.lightGray )
                table.setItem( row, col, item )
            row += 1

        table.resizeColumnsToContents()

        dialog.setLayout( QVBoxLayout() )
        dialog.layout().setMargin( 0 )
        dialog.layout().addWidget( table )

        dialog.exec_()

    def about( self ):
        dialog = AboutDialog( self )
        dialog.exec_()

    def loadCircuitFromHierarchy( self, index ):
        self.hierarchyCurrentIndex = index
        self.centralWidget().setCurrentIndex( index.internalId() )

    def slotGateDoubleClicked( self, gate ):
        if gate.type == gate_type.module:
            rows = self.hierarchyView.model().rowCount( self.hierarchyCurrentIndex )
            for r in range( rows ):
                if str( self.hierarchyCurrentIndex.child( r, 0 ).data().toString() ) == gate.module_name:
                    self.centralWidget().setCurrentIndex( self.hierarchyCurrentIndex.child( r, 0 ).internalId() )
                    self.hierarchyCurrentIndex = self.hierarchyCurrentIndex.child( r, 0 )
                    return

if __name__ == '__main__':
    a = QApplication([])

    w = Viewer()
    w.show()

    if len( sys.argv ) == 2:
        w.openCircuitFromFilename( sys.argv[1] )

    sys.exit( a.exec_() )
