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

from revkit import binary_truth_table, embed_truth_table, read_pla

from core.BaseItem import *

from helpers.RevKitHelper import *

from ui.DesignerWidget import DesignerWidget
from ui.Embedding import Ui_Embedding

class Embedding( DesignerWidget ):
    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_Embedding, parent )

@item( "Embedding",
       requires = "PLA", provides = "Truth Table",
       widget = { 'class': Embedding, 'size': (200, 100) },
       properties = [ 'garbage_name' ] )
class EmbeddingItem( BaseItem ):
    """This item provides a simple embedding method. Embedding needs to be processed in order to transform a PLA function into a Truth Table. Optionally, the name of the garbage outputs can be defined. After the item has been processed, the enlarged item reports the run-time needed to perform the embedding."""
    def onCreate( self ):
        self.setText( "Embedding" )
        self.setState( self.CONFIGURED )

    def executeEvent( self, inputs ):
        filename = inputs[0]
        spec = binary_truth_table()
        read_pla( spec, filename )
        spec2 = binary_truth_table()
        res = embed_truth_table( spec2, spec, garbage_name = str( self.garbage_name ) )
        if type( res ) == dict:
            self.widget.runtime.setText( "%.2f s" % res['runtime'] )
            circuit_add_runtime( spec2, res['runtime'] )
            spec2.name = filename[filename.rfind('/') + 1:filename.rfind('.')]
        else:
            return res
        return [ spec2 ]
