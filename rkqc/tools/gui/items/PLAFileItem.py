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

from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from revkit import *

from core.BaseItem import *

from core.SpecificationTable import *

from ui.PLAFile import *
from ui.DesignerWidget import *

class PLAFile( DesignerWidget ):
    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_PLAFile, parent )

        self.filename.specificationChanged.connect( self.onSpecificationChanged )

    def onSpecificationChanged( self ):
        self.numInputs.setText( str( self.filename.spec.num_inputs ) )
        self.numOutputs.setText( str( self.filename.spec.num_outputs ) )
        self.numCubes.setText( str( sum( 1 for _ in self.filename.spec.entries ) ) )

@item( "Function (*.pla)",
       iconname = "document-export",
       provides = "PLA",
       properties = [ "filename" ],
       widget = PLAFile )
class PLAFileItem( BaseItem ):
    """This item opens a single function provided in a given *.pla-file. A right-click on the item opens a file browser, where the file can be selected."""
    def onCreate( self ):
        self.setText( "(Unknown PLA)" )

    def executeEvent( self, inputs ):
        return [ str( self.filename ) ]

    def onFilenameChanged( self, value ):
        if not value.isEmpty():
            self.setText( QFileInfo( value ).fileName() )
            self.setState( self.CONFIGURED )

    @action( "Load PLA", "document-open" )
    def loadPLA( self ):
        self.setFilename( QFileDialog.getOpenFileName( None, 'Open PLA', '', 'PLA file (*.pla)' ) )
