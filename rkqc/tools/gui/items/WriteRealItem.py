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
from revkit import write_realization

@item( "Write Circuit to File",
       iconname = "text-x-readme",
       requires = "Circuit",
       properties = [ "filename" ] )
class WriteRealItem( BaseItem ):
    """This item dumps a given circuit a *.real-file to a given path. A right-click on the item opens a file browser, where the respective file to which the circuit should be stored  can be defined."""
    def onCreate( self ):
        self.setText( "no filename specified" )

    def executeEvent( self, inputs ):
        write_realization( inputs[0], str( self.filename ) )
        return []

    @action( "Specify file name", "document-save" )
    def setWriteFilename( self ):
        self.setFilename( QFileDialog.getSaveFileName( None, 'Write Realization', '', 'RevLib Realization (*.real)' ) )

    def onFilenameChanged( self, value ):
        if not value.isEmpty():
            self.setText( QFileInfo( value ).fileName() )
            self.setState( self.CONFIGURED )
