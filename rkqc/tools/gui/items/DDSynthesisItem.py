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

import os, sys
from tempfile import NamedTemporaryFile

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from revkit import *

from core.BaseItem import *

from helpers.RevKitHelper import *

from ui.DDSynthesis import *
from ui.DesignerWidget import *

class DDSynthesis( DesignerWidget ):
    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_DDSynthesis, parent )

        self.splitter.setSizes( [1000,200] )
        self.connect( self.dd_type, SIGNAL( 'currentIndexChanged(int)' ), self.setTitle )

    def setTitle( self, index ):
        self.groupBox.setTitle( "%s Synthesis" % [ "BDD", "KFDD" ][index] )

    def maybeDrawBDD( self, dotname, name ):
        # Add Image to Scene
        if self.draw_bdd.isChecked():
            fpng = NamedTemporaryFile( delete = False )
            fpng.close()
            os.system( "dot -Tpng %s > %s" % ( dotname, fpng.name ) )

            view = QGraphicsView( self.tabWidget )
            view.setScene( QGraphicsScene( view ) )
            view.scene().clear()
            view.scene().addPixmap( QPixmap( fpng.name ) )
            self.tabWidget.addTab( view, name )

            # delete temporary files
            os.unlink( fpng.name )

@item( "DD Synthesis",
       requires = "PLA", provides = "Circuit",
       properties = [ "dd_type", "complemented", "reordering", "default_decomposition", "kfdd_reordering", "sifting_factor", "sifting_growth_limit", "sifting_method", "draw_bdd" ],
       widget = DDSynthesis )
class DDSynthesisItem( BaseItem ):
    """This item provides the BDD-based synthesis method as well as the KFDD-based synthesis method. The respective synthesis approach can be selected on the right-hand side of the enlarged item. Additionally, the reordering strategy and whether complement edges should be applied or not can be specified. Optionally, the applied DD structure can be displayed. After the item has been processed, the enlarged item reports the run-time needed to perform the synthesis."""
    def onCreate( self ):
        self.setText( "BDD Synthesis" )
        self.setState( self.CONFIGURED )

    def onDdTypeChanged( self, value ):
        self.setText( "%s Synthesis" % [ "BDD", "KFDD" ][int( value )] )

    def initialize( self ):
        self.widget.tabWidget.clear()

    def executeEvent( self, inputs ):
        circ = circuit()

        # Draw BDD?
        if bool( int( self.draw_bdd ) ):
            fdot = NamedTemporaryFile( delete = False )
            fdot.close()
            dotfilename = fdot.name
        else:
            dotfilename = ""

        filename = inputs[0]
        if int( self.dd_type ) == 0:
            res = bdd_synthesis( circ, filename,
                                 complemented_edges = bool( int( self.complemented ) ),
                                 reordering = int( self.reordering ),
                                 dotfilename = dotfilename )
        else:
            res = kfdd_synthesis( circ, filename,
                                  default_decomposition = int( self.default_decomposition ),
                                  reordering = int( self.kfdd_reordering ),
                                  sift_factor = float( self.sifting_factor ),
                                  sifting_growth_limit = [ 'r', 'a' ][int( self.sifting_growth_limit )],
                                  sifting_method = [ 'r', 'i', 'g', 'l', 'v' ][int( self.sifting_method )],
                                  dotfilename = dotfilename )

        if type( res ) == dict:
            circuit_set_name( circ, filename )

            self.widget.runtime.setText( "%.2f s" % res['runtime'] )
            circuit_add_runtime( circ, res['runtime'] )

            if dotfilename != "":
                self.widget.maybeDrawBDD( fdot.name, circ.circuit_name )
                os.unlink( dotfilename )
        else:
            return res

        return [ circ ]
