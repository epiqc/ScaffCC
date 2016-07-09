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
from PyQt4.QtCore import QFileInfo
from PyQt4.QtGui import QFileDialog

from core.BaseItem import *
from revkit import binary_truth_table, read_specification

@item( "Truth Table (*.spec)",
       iconname = "document-export",
       provides = "Truth Table",
       properties = [ "filename" ] )
class SpecFileItem( BaseItem ):
    """This item opens a single function provided in a given *.spec-file. A right-click on the item opens a file browser, where the file can be selected."""
    def onCreate( self ):
        self.setText( "no truth table loaded" )

    def executeEvent( self, inputs ):
        spec = binary_truth_table()
        read_specification( spec, str( self.filename ) )
        return [ spec ]

    @action( "Load truth table", "document-open" )
    def loadSpecification( self ):
        self.setFilename( QFileDialog.getOpenFileName( None, 'Open Specification', '', 'RevLib Specification (*.spec)' ) )

    def onFilenameChanged( self, value ):
        if not value.isEmpty():
            self.setText( QFileInfo( value ).fileName() )
            self.setState( self.CONFIGURED )
