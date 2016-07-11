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

import inspect
import pkgutil
import sys, os
dirnamesyspath0=os.path.dirname(sys.path[0])
sys.path.append(dirnamesyspath0) 
sys.path.append(os.path.dirname(dirnamesyspath0)) 

from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from revkit import *
from revkitui import *
#from revlib import *

from core.BaseItem import *

from core.ButtonBar import *
from core.FlowContainer import *
from core.MessagesWidget import *
from core.SettingsDialog import *

from items.AddingLinesItem import *
from items.CreateSimulationPatternItem import *
from items.ComparatorItem import *
from items.DDSynthesisItem import *
from items.EmbeddingItem import *
from items.EquivalenceCheckItem import *
from items.ESOPSynthesisItem import *
from items.ExactSynthesisItem import *
from items.LineReductionItem import *
from items.PathBenchmarksItem import *
from items.PatternInputItem import *
from items.PLAFileItem import *
from items.RealFileItem import *
from items.ResultsTableItem import *
#from items.RevLibFunctions import *
from items.SpecFileItem import *
from items.TransformationBasedSynthesisItem import *
from items.TranspositionBasedSynthesisItem import *
from items.ViewerItem import *
from items.WindowOptimizationItem import *
from items.WriteRealItem import *
from items.WriteRealToPathItem import *

from core.MainWidget import *

class MainWindow( QMainWindow ):
    def __init__( self, parent = None ):
        QMainWindow.__init__( self, parent )

        self.setupWidgets()
        self.setupDockWidgets()
        self.setupTools()
        self.setupActions()
        self.setupToolBars()

        self.widget.tabWidget.slotNew()

    def setupWidgets( self ):
        self.widget = MainWidget( self )
        self.setCentralWidget( self.widget )

    def setupDockWidgets( self ):
        self.messagesWidget = MessagesWidget()
        self.runWidget = QProgressBar()

        self.widget.buttonBar.addWidget( self.messagesWidget, QIcon.fromTheme( "utilities-system-monitor" ), "Messages" )
        self.widget.buttonBar.addWidget( self.runWidget, QIcon.fromTheme( "run-build" ), "Run" )

        self.messagesWidget.message_added.connect( lambda: self.widget.buttonBar.setCurrentIndex( 0 ) )

    def setupTools( self ):
        #self.widget.addToolCategory( "Sources", [ PathBenchmarksItem, RevLibFunctions, PLAFileItem, RealFileItem, SpecFileItem, PatternInputItem ] )
        self.widget.addToolCategory( "Sources", [ PathBenchmarksItem, PLAFileItem, RealFileItem, SpecFileItem, PatternInputItem ] )
        self.widget.addToolCategory( "Sinks", [ ResultsTableItem, ViewerItem, WriteRealToPathItem, WriteRealItem ] )
        self.widget.addToolCategory( "Synthesis", [ DDSynthesisItem, ESOPSynthesisItem, TransformationBasedSynthesisItem, ExactSynthesisItem, TranspositionBasedSynthesisItem, EmbeddingItem ] )
        self.widget.addToolCategory( "Optimization", [ AddingLinesItem, LineReductionItem, WindowOptimizationItem ] )
        self.widget.addToolCategory( "Verification / Simulation", [ EquivalenceCheckItem, CreateSimulationPatternItem ] )
        self.widget.addToolCategory( "Utilities", [ ComparatorItem ] )

        unstable_items = []
        import items.unstable
        package = items.unstable

        for _, name, _ in pkgutil.iter_modules( package.__path__ ):
            __import__( package.__name__ + "." + name )
            unstable_items.append( eval( "%s.%s.%s" % (package.__name__, name, name) ) )

        if len( unstable_items ) > 0:
            self.widget.addToolCategory( "Unstable", unstable_items )

        self.widget.toolBox.expandAll()

    def setupActions( self ):
        self.settingsAction = QAction( QIcon.fromTheme( "configure" ), "Settings", self )

        self.connect( self.settingsAction, SIGNAL( 'triggered()' ), self.configure )

    def setupToolBars( self ):
        self.mainBar = self.addToolBar( "main" )
        self.mainBar.setIconSize( QSize( 22, 22 ) )
        self.mainBar.setToolButtonStyle( Qt.ToolButtonTextBesideIcon )

        self.mainBar.addAction( self.widget.tabWidget.newAction )
        self.mainBar.addAction( self.widget.tabWidget.openAction )
        self.mainBar.addAction( self.widget.tabWidget.saveAction )
        self.mainBar.addAction( self.widget.tabWidget.saveAsAction )
        self.mainBar.addSeparator()
        self.mainBar.addAction( self.settingsAction )
        self.mainBar.addSeparator()
        self.mainBar.addAction( self.widget.tabWidget.runAction )

    def configure( self ):
        Settings().exec_()

if __name__ == "__main__":
    app = QApplication( [] )

    QCoreApplication.setOrganizationName( "Group of Computer Architecture, University of Bremen" )
    QCoreApplication.setOrganizationDomain( "revkit.org" )
    QCoreApplication.setApplicationName( "RevKit" )

    w = MainWindow()
    w.setWindowTitle( "RevKit Graphical User Interface" )
    w.setWindowState( Qt.WindowMaximized )
    w.show()

    # Open files on startup
    for i, f in enumerate( sys.argv[1:] ):
        if i != 0:
            w.widget.tabWidget.slotNew()
        w.widget.tabWidget.currentWidget().scene().load( QString( f ) )
        w.widget.tabWidget.currentWidget().scene().setFilename( f )

    sys.exit( app.exec_() )

