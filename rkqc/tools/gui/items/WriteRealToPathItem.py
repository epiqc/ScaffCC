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

from PyQt4.QtCore import pyqtProperty, QFileInfo, QSize, QString
from PyQt4.QtGui import QFileDialog

from revkit import write_realization

from core.BaseItem import *

@item( "Write Circuits to Path",
       iconname = "folder-downloads",
       requires = "Circuit",
       properties = [ "pathname" ] )
class WriteRealToPathItem( BaseItem ):
    """This item dumps a given set of circuits as *.real-files to a given path. In particular, this item finds application if more than one benchmark is considered (i.e. in combination with the item <i>Path Benchmarks</i>). A right-click on the item opens a file browser, where the path to which the circuits should be stored  can be defined."""
    def onCreate( self ):
        self.setText( "Write to path" )

    def executeEvent( self, inputs ):
        filename = inputs[0].circuit_name
        pathname = str( self.pathname )

        # No circuit name
        if len( filename ) == 0: filename = "unknown"

        # Name taken
        if os.path.exists( "%s/%s.real" % (pathname, filename ) ):
            i = 0
            while os.path.exists( "%s/%s-%04d.real" % ( pathname, filename, i ) ):
                i += 1
            filename = "%s-%04d" % ( filename, i )

        # Write circuit
        write_realization( inputs[0], "%s/%s.real" % ( pathname, filename ) )

        return []

    @action( "Specify folder name", "document-open-folder" )
    def setWritePathname( self ):
        self.setPathname( QFileDialog.getExistingDirectory( None, "Open Directory" ) )

    def onPathnameChanged( self, value ):
        if not value.isEmpty():
            self.setText( QFileInfo( value ).fileName() )
            self.setState( self.CONFIGURED )
