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

from PyQt4.QtCore import QSize
from PyQt4.QtGui import QCursor, QIcon

import os
from tempfile import NamedTemporaryFile

from revkit import *

from core.BaseItem import *

from ui.CreateSimulationPattern import *
from ui.DesignerWidget import *

class CreateSimulationPattern( DesignerWidget ):
    filename = None
    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_CreateSimulationPattern, parent )

        self.open.setIcon( QIcon.fromTheme( "document-open" ) )

        self.connect( self.open, SIGNAL( 'released()' ), self.slotOpen )

    def setWaveFile( self, filename ):
        self.filename = filename
        self.open.setEnabled( True )

    def reset( self ):
        self.filename = None
        self.open.setEnabled( False )

    @pyqtSlot()
    def slotOpen( self ):
        if self.filename:
            os.system( "dinotrace %s" % self.filename )

@item( "Sequential Simulation",
       requires = [ "Pattern File", "Circuit" ],
       widget = { 'class': CreateSimulationPattern, 'size': (200, 80) } )
class CreateSimulationPatternItem( BaseItem ):
    """This item provides a simulation engine. It gets a circuit and a pattern file. After the item has been processed, the <i>dinotrace</i> viewer can be invoked from the enlarged item in order to display the simulation waveform. The usage of the simulation item is explicitly illustrated by means of a tutorial-video at the <i>www.revkit.org</i> website."""
    def onCreate( self ):
        self.setText( "Sequential Simulation" )
        self.setState( self.CONFIGURED )

    @action( "Show Wave Form", "document-open" )
    def showWaveForm( self ):
        self.widet.slotOpen()

    def initialize( self ):
        self.widget.reset()

    def executeEvent( self, inputs ):
        r = create_simulation_pattern( inputs[0], inputs[1] )
        outputs = []
        vcd_file = NamedTemporaryFile( 'w', suffix = ".vcd", delete = False )
        vcd_file.close()
        sequential_simulation( outputs, inputs[1], r['pattern'], r['init'], vcd_filename = vcd_file.name )
        self.widget.setWaveFile( vcd_file.name )
        return []
