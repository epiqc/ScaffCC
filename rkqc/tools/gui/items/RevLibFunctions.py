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
import os
from tempfile import NamedTemporaryFile

from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from revkit import *
from revlib import *
from revlibui import *

from core.BaseItem import *

@item( "RevLib Functions",
       iconname = "document-export",
       widget = FunctionsWidget,
       provides = "PLA" )
class RevLibFunctions( BaseItem ):
    def onCreate( self ):
        self.setText( "RevLib Functions" )
        self.setState( self.CONFIGURED )

        self.server = revlib_server( "http://localhost:3000" )

        self.loadFunctions()

        # Create temporary directory
        # TODO delete when finalize
        f = NamedTemporaryFile( delete = False )
        f.close()
        os.unlink( f.name )
        os.mkdir( f.name )
        self.pladir = f.name


    def loadFunctions( self ):
        self.widget.setItems( self.server.list_functions_by_categories() )

    def initialize( self ):
        self.currentIndex = 0
        self.functions = self.widget.tree.selectedItems()

    def executeEvent( self, inputs ):
        filename = self.functions[self.currentIndex]['url']
        destfile = str( '%s/%s.pla' % ( self.pladir, self.functions[self.currentIndex]['name'] ) )
        with open( destfile, 'w' ) as f:
            f.write( urllib2.urlopen( str( filename ) ).read() )
        self.currentIndex += 1
        return [ destfile ]

    def numRuns( self ):
        # This is called before initialize actually
        self.functions = self.widget.tree.selectedItems()
        return len( self.functions )

    ### Options
    def setInputsfrom( self, v ):
        self.inputsQuery.checkFrom.setChecked( v )

    def setInputsto( self, v ):
        self.inputsQuery.checkTo.setChecked( v )

    def setOutputsfrom( self, v ):
        self.outputsQuery.checkFrom.setChecked( v )

    def setOutputsto( self, v ):
        self.outputsQuery.checkTo.setChecked( v )

    def setCubesfrom( self, v ):
        self.cubesQuery.checkFrom.setChecked( v )

    def setCubesto( self, v ):
        self.cubesQuery.checkTo.setChecked( v )

    conf_inputsfrom = pyqtProperty( "bool", lambda x: x.inputsQuery.checkFrom.isChecked(), setInputsfrom )
    conf_inputsto = pyqtProperty( "bool", lambda x: x.inputsQuery.checkTo.isChecked(), setInputsto )
    conf_outputsfrom = pyqtProperty( "bool", lambda x: x.outputsQuery.checkFrom.isChecked(), setOutputsfrom )
    conf_outputsto = pyqtProperty( "bool", lambda x: x.outputsQuery.checkTo.isChecked(), setOutputsto )
    conf_cubesfrom = pyqtProperty( "bool", lambda x: x.cubesQuery.checkFrom.isChecked(), setCubesfrom )
    conf_cubesto = pyqtProperty( "bool", lambda x: x.cubesQuery.checkTo.isChecked(), setCubesto )
