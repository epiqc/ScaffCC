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

import sys
import commands, os
sys.path.append(os.path.dirname(sys.path[0]))

from PyKDE4.kdecore import *
from PyKDE4.kio import *
from PyKDE4.kdeui import *
from PyKDE4.kparts import *
from PyKDE4.ktexteditor import *

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from revkit import *

class MainWindow( KMainWindow ):
    def __init__( self ):
        KMainWindow.__init__( self )

        self.resize( 640, 480 )
        self.setWindowTitle( "Realization to LaTeX" )
        self.setWindowIcon( KIcon( "okular" ) )

        mainWidget = QWidget( self )
        layout = QVBoxLayout()
        layout.setMargin( 0 )
        layout.setSpacing( KDialog.spacingHint() )
        mainWidget.setLayout( layout )

        splitter = QSplitter( Qt.Vertical, mainWidget )

        toolBar = KToolBar( mainWidget )
        open_action = toolBar.addAction( KIcon( "document-open" ), i18n( "Open" ) )
        save_action = toolBar.addAction( KIcon( "document-save" ), i18n( "Save" ) )
        refresh_action = toolBar.addAction( KIcon( "view-refresh" ), i18n( "Refresh" ) )
        refresh_action.setShortcut( Qt.CTRL + Qt.Key_F5 )

        tabBar = KTabWidget( splitter )

        self.part = KLibLoader.self().factory( "okularpart" ).create( tabBar )
        self.editor = KTextEditor.EditorChooser.editor()
        self.editor.readConfig()
        self.doc = self.editor.createDocument( tabBar )
        self.doc.setMode( "LaTeX" )
        self.doc.setReadWrite( False )
        self.view = self.doc.createView( tabBar )

        tabBar.addTab( self.part.widget(), i18n( "Preview" ) )
        tabBar.addTab( self.view, i18n( "Source code" ) )

        tabBarEdit = KTabWidget( splitter )

        # Real Editor
        self.docReal = self.editor.createDocument( tabBarEdit )
        self.viewReal = self.docReal.createView( tabBarEdit )
        tabBarEdit.addTab( self.viewReal, i18n( "Realization" ) )

        # Before Editor
        self.docBefore = self.editor.createDocument( tabBarEdit )
        self.docBefore.setMode( "LaTeX" )
        self.viewBefore = self.docBefore.createView( tabBarEdit )
        tabBarEdit.addTab( self.viewBefore, i18n( "Before" ) )
        self.docInBetween = self.editor.createDocument( tabBarEdit )
        self.docInBetween.setMode( "LaTeX" )
        self.viewInBetween = self.docInBetween.createView( tabBarEdit )
        tabBarEdit.addTab( self.viewInBetween, i18n( "In between" ) )
        self.docAfter = self.editor.createDocument( tabBarEdit )
        self.docAfter.setMode( "LaTeX" )
        self.viewAfter = self.docAfter.createView( tabBarEdit )
        tabBarEdit.addTab( self.viewAfter, i18n( "After" ) )

        splitter.addWidget( tabBar )
        splitter.addWidget( tabBarEdit )

        layout.addWidget( toolBar )
        layout.addWidget( splitter )

        # Actions
        QObject.connect( open_action, SIGNAL( "triggered()" ), self.open )
        QObject.connect( save_action, SIGNAL( "triggered()" ), self.save )
        QObject.connect( refresh_action, SIGNAL( "triggered()" ), self.refresh )

        self.setCentralWidget( mainWidget )

    def open(self):
        filename = str( KFileDialog.getOpenFileName() )
        with open( filename, mode = "r" ) as f:
            self.docReal.setText( f.read() )
        self.refresh()

    def save(self):
        filename = str( KFileDialog.getSaveFileName() )
        with open( filename, mode = "w" ) as f:
            f.write( self.doc.text() )

    def refresh(self):
        with open( 'pytex.real', mode = "w" ) as f:
            f.write( self.docReal.text() )

        circ = circuit()
        read_realization( circ, 'pytex.real' ) or sys.exit ("Cannot read pytext.real")
        settings = create_tikz_settings()
        settings.draw_before_text = str( self.docBefore.text() )
        settings.draw_in_between_text = str( self.docInBetween.text() )
        settings.draw_after_text = str( self.docAfter.text() )
        text = create_image( circ, settings )

        # LaTeX Document text
        self.doc.setReadWrite( True )
        self.doc.setText( text )
        self.doc.setReadWrite( False )

        # Create PDF
        with open( 'pytex.tex', mode = "w" ) as f:
            f.write( '\\documentclass[a4paper,landscape]{article}\n' )
            f.write( '\\usepackage{tikz}\n' )
            f.write( '\\usetikzlibrary{backgrounds,fit,decorations.pathreplacing}\n' )
            f.write( '\\usepackage[active,tightpage]{preview}\n' )
            f.write( '\\PreviewEnvironment{tikzpicture}\n' )
            f.write( '\\setlength\\PreviewBorder{5pt}\n' )
            f.write( '\\usepackage[left=1cm,right=1cm]{geometry}\n' )
            f.write( '\\pagestyle{empty}\n' )
            f.write( '\\begin{document}\n' )
            f.write( text + '\n' )
            f.write( '\\end{document}\n' )

        os.system( 'pdflatex -file-line-error -halt-on-error pytex.tex' )

        self.part.openUrl( KUrl.fromPath( 'pytex.pdf' ) )

if __name__ == '__main__':
    appName     = "KApplication"
    catalog     = ""
    programName = ki18n ("KApplication")
    version     = "1.0"
    description = ki18n ("KApplication/KMainWindow/KAboutData example")
    license     = KAboutData.License_GPL
    copyright   = ki18n ("(c) 2007 Jim Bublitz")
    text        = ki18n ("none")
    homePage    = "www.riverbankcomputing.com"
    bugEmail    = "jbublitz@nwinternet.com"
 
    aboutData   = KAboutData (appName, catalog, programName, version, description,
                              license, copyright, text, homePage, bugEmail)

    KCmdLineArgs.init( sys.argv, aboutData )

    app = KApplication()
    mainWindow = MainWindow()
    mainWindow.show()
    sys.exit(app.exec_())
