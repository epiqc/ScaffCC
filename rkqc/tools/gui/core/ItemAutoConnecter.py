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

from PyQt4.QtCore import QObject, pyqtSignal, SIGNAL
from PyQt4.QtGui import QAction

class ItemAutoConnecter( QObject ):
    def __init__( self, scene ):
        QObject.__init__( self, scene )
        self.scene = scene

        self._action = QAction( "Auto Connect Items", self )
        self.connect( self._action, SIGNAL( 'triggered( bool )' ), self.autoConnect )

    def action( self ):
        return self._action

    def updateAction( self ):
        self._action.setEnabled( self.canConnect() )

    def canConnect( self ):
        # Get all items sort by y position
        items = sorted( self.scene.graph.items(), key = lambda item: item.scenePos().y() )

        # We need at least two items
        if len( items ) < 2:
            #print "Less than two items"
            return False

        # Top is source, bottom is sink
        if len( items[0].requiresPorts ) != 0 or len( items[-1].providesPorts ) != 0:
            #print "Top must be bottom and bottom sink"
            return False

        # Only single port items in between
        if len( items ) > 2 and set( sum( [ [ len( item.requiresPorts ), len( item.providesPorts ) ] for item in items[1:-1] ], [] ) ) != set([1]):
            #print "There are items with multiple ports"
            return False

        # Ports sequence
        ports = [ items[0].providesPorts[0] ] + sum( [ [ item.requiresPorts[0], item.providesPorts[0] ] for item in items[1:-1] ], [] ) + [ items[-1].requiresPorts[0] ]
        # Cancel if ports are already connected
        if max( [ len( self.scene.graph.outEdges( p ) ) for p in ports[0::2] ] ) != 0 or \
           max( [ len( self.scene.graph.inEdges( p ) ) for p in ports[1::2] ] ) != 0:
            #print "Already connected"
            return False

        datatypes = [ p.datatype for p in ports ]
        if False in [ datatypes[i] == datatypes[i+1] for i in range( 0, len( datatypes ), 2 ) ]:
            return False

        return True

    # TODO allow more cases, respect already connected edges
    def autoConnect( self ):
        # Get all items sort by y position
        items = sorted( self.scene.graph.items(), key = lambda item: item.scenePos().y() )

        # Connect?
        if not self.canConnect():
            return

        ports = [ items[0].providesPorts[0] ] + sum( [ [ item.requiresPorts[0], item.providesPorts[0] ] for item in items[1:-1] ], [] ) + [ items[-1].requiresPorts[0] ]

        for i in range( 0, len( ports ), 2 ):
            self.scene.graph.addEdge( ports[i], ports[i+1] )



