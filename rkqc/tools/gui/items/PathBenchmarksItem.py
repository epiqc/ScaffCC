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
from core.BaseItem import *

import glob
import os

from revkit import circuit, binary_truth_table, read_realization, read_specification

from ui.DesignerWidget import *
from ui.PathBenchmarks import *

class PathBenchmarksController:
    def __init__( self, item, widget ):
        self.item = item
        self.widget = widget

        self.filenames = None

    def setPath( self, path, guess = True ):
        # Guess file type, if port is unconnected
        if guess and len( self.item.scene().graph.outEdges( self.item.providesPorts[0] ) ) == 0:
            self.guessFileType( path )

        # Update
        if os.path.exists( path ):
            extensions = [ "*.pla", "*.real", "*.spec" ]
            names = [ "function", "circuit", "truth table" ]
            index = int( self.item.filetype )
            self.filenames = glob.glob( os.path.join( path, extensions[index] ) )
            self.widget.info.setText( "Found %d %s%s" % ( len( self.filenames ),
                                                          names[index],
                                                          "s" if len( self.filenames ) != 1 else "" ) )
            if len( self.filenames ) == 0:
                self.filenames = None
        else:
            self.filenames = None
            self.widget.info.setText( "Please choose a location" )

    def guessFileType( self, path ):
        if os.path.exists( path ):
            extensions = [ "*.pla", "*.real", "*.spec" ]
            counts = map( len, map( lambda ext: glob.glob( os.path.join( path, ext ) ), extensions ) )
            if counts.count( max( counts ) ) == 1:
                self.item.setFiletype( str( counts.index( max( counts ) ) ) )

class PathBenchmarks( DesignerWidget ):
    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_PathBenchmarks, parent )

@item( "Path Benchmarks",
       iconname = "document-open-folder",
       provides = "PLA",
       properties = [ "path", "filetype" ],
       widget = { 'class': PathBenchmarks, 'size': (320, 150) } )
class PathBenchmarksItem( BaseItem ):
    """This item opens a set of benchmarks (either Boolean functions provided in *.pla-files, Truth Tables provided in *.spec-files, or Circuits provided in *.real-files) and separately pass them to the succeeding items. It can be utilized, if a design process should be applied to a larger set of benchmarks. A right-click on the item opens a file browser, where a path including the respective files can be selected. When enlarging the item, the type of benchmark to consider can be selected."""
    def onCreate( self ):
        self.controller = PathBenchmarksController( self, self.widget )

        self.setText( "Functions from path" )
        self.setState( self.UNCONFIGURED )

        # Observe port
        self.providesPorts[0].connectionsChanged.connect( self.onConnectionsChanged )

    @action( "Choose path", "document-open-folder" )
    def choosePath( self ):
        self.widget.path.selectPath()

    def initialize( self ):
        self.currentIndex = 0

    def executeEvent( self, inputs ):
        index = int( self.filetype )
        filename = self.controller.filenames[self.currentIndex]
        self.currentIndex += 1
        if index == 0: # Functions
            return [ filename ]
        elif index == 1: # Circuit
            circ = circuit()
            read_realization( circ, filename )
            circ.circuit_name = filename[filename.rfind('/') + 1:filename.rfind('.')]
            return [ circ ]
        elif index == 2: # Specification
            spec = binary_truth_table()
            read_specification( spec, filename )
            return [ spec ]

    def numRuns( self ):
        return len( self.controller.filenames )

    def onPathChanged( self, value ):
        if not hasattr( self, "controller" ): return

        self.controller.setPath( str( value ) )
        self.setState( self.CONFIGURED if self.controller.filenames is not None else self.UNCONFIGURED )

    def onFiletypeChanged( self, value ):
        if not hasattr( self, "controller" ): return

        self.controller.setPath( str( self.path ), False )

        filetype = [ "PLA", "Circuit", "Truth Table" ][ int( value ) ]
        self.providesPorts[0].datatype = filetype
        self.providesPorts[0].datatypeWidth = 200
        self.providesPorts[0].update()

        self.setText( "%ss from path" % filetype[:].replace( "PLA", "Function" ) )

    def onConnectionsChanged( self ):
        self.widget.filetype.setEnabled( len( self.scene().graph.outEdges( self.providesPorts[0] ) ) == 0 )

