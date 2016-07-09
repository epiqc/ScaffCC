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

from revkit import circuit, read_realization

from core.BaseItem import *

from ui.DesignerWidget import DesignerWidget
from ui.RealFile import Ui_RealFile

class RealFile( DesignerWidget ):
    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_RealFile, parent )

@item( "Circuit Realization (*.real)",
       iconname = "document-export",
       provides = "Circuit",
       properties = [ "filename" ],
       widget = RealFile )
class RealFileItem( BaseItem ):
    """This item opens a single circuit provided in a given *.real-file. A right-click on the item opens a file browser, where the file can be selected."""
    def onCreate( self ):
        self.setText( "no circuit loaded" )

    def executeEvent( self, inputs ):
        circ = circuit()
        filename = str( self.filename )
        ret = read_realization( circ, filename )
        if isinstance( ret, str ):
            return ret
        circ.circuit_name = filename[filename.rfind('/') + 1:filename.rfind('.')]
        return [ circ ]

    def onFilenameChanged( self, filename ):
        if not filename.isEmpty():
            self.setText( QFileInfo( filename ).fileName() )
            self.setState( self.CONFIGURED )

    @action( "Load circuit", "document-open" )
    def loadCircuit( self ):
        filename = QFileDialog.getOpenFileName( None, 'Open Realization', '', 'RevLib Realization (*.real)' )
        self.setFilename( filename )
