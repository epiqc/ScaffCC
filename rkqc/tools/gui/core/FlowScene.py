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

import math

from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtXml import QDomDocument, QDomElement

from core.BaseItem import *
from core.ItemToItemSnapper import ItemToItemSnapper

def topCenter( rect ):
    return QPointF( rect.center().x(), rect.top() )

def bottomCenter( rect ):
    return QPointF( rect.center().x(), rect.bottom() )

class FlowScenePortConnecter( QObject ):
    edgeCreated = pyqtSignal( PortItem, PortItem )

    def __init__( self, parent ):
        QObject.__init__( self, parent )

        parent.installEventFilter( self )
        self.startItem = None
        self.lineItem = parent.addLine( 0, 0, 0, 0 )
        self.lineItem.setZValue( 10 ) # Above of everything
        self.lineItem.setPen( QColor( Qt.blue ).darker() )
        self.lineItem.hide()

    def compatiblePort( self, pos ):
        assert( self.startItem )
        item = self.parent().itemAt( pos )
        if item:
            other = item.toGraphicsObject()
            if isinstance( other, PortItem ) and other.parentItem() != self.startItem.parentItem():
                # Check if it is compatible
                if other.datatype == self.startItem.datatype \
                  and other.direction != self.startItem.direction \
                  and ( other.direction == Qt.AlignBottom or len( self.parent().graph.inEdges( other ) ) == 0 ):
                    return other
        return None

    def eventFilter( self, obj, event ):
        # Start Connecting
        if isinstance( obj, PortItem ) and event.type() == QEvent.GraphicsSceneMousePress and not self.startItem:
            self.startItem = obj
            self.startItem.parentObject().setFlag( QGraphicsItem.ItemIsMovable, False )
            self.lineItem.setLine( 0, 0, 0, 0 )
            self.lineItem.show()

        # Break/Stop Connecting
        elif self.startItem and obj == self.parent() and event.type() in [ QEvent.GraphicsSceneMouseRelease ]:
            # Check if we can finish
            if event.type() == QEvent.GraphicsSceneMouseRelease:
                other = self.compatiblePort( event.scenePos() )
                if other:
                    source = self.startItem if self.startItem.direction != Qt.AlignTop else other
                    target = self.startItem if self.startItem.direction == Qt.AlignTop else other
                    self.edgeCreated.emit( source, target )
                    self.startItem.parentObject().setFlag( QGraphicsItem.ItemIsMovable, not self.startItem.parentObject().maximize_button.isMaximized() )
                    self.startItem = None

            # We did not finish
            if self.startItem:
                self.startItem.parentObject().setFlag( QGraphicsItem.ItemIsMovable, not self.startItem.parentObject().maximize_button.isMaximized() )
                self.startItem = None

            # Do not show the line
            self.lineItem.hide()

        # Draw Line
        if self.startItem and obj == self.parent() and event.type() == QEvent.GraphicsSceneMouseMove:
            self.lineItem.setLine( self.startItem.scenePos().x(), self.startItem.scenePos().y(), event.scenePos().x(), event.scenePos().y() )
            color = QColor( Qt.green ) if self.compatiblePort( event.scenePos() ) else QColor( Qt.blue )
            self.lineItem.setPen( color.darker() )

        return QObject.eventFilter( self, obj, event )

class FlowSceneGraphEdge( QGraphicsLineItem ):
    def __init__( self, line, graph, parent = None ):
        QGraphicsLineItem.__init__( self, line, parent )

        self.setAcceptHoverEvents( True )
        self.graph = graph

        # ContextMenu
        self.delete_action = QAction( QIcon.fromTheme( "edit-delete" ), "Delete Edge", graph.parent().views()[0] )
        graph.parent().connect( self.delete_action, SIGNAL( 'triggered(bool)' ), self.onDeleteEdge )

    def hoverEnterEvent( self, event ):
        self.setPen( Qt.red )

    def hoverLeaveEvent( self, event ):
        self.setPen( Qt.black )

    def onDeleteEdge( self ):
        self.graph.deleteEdge( self )

class FlowSceneGraph( QObject ):
    def __init__( self, parent ):
        QObject.__init__( self, parent )

        self.edges = []

    def addEdge( self, source, target ):
        line = FlowSceneGraphEdge( QLineF( source.scenePos(), target.scenePos() ), self )
        self.parent().addItem( line )
        line.setZValue( 9 ) # Below line to create new edges
        self.edges.append( [ source, target, line ] )
        self.connect( source.parentObject(), SIGNAL( 'xChanged()' ), self.positionChanged )
        self.connect( source.parentObject(), SIGNAL( 'yChanged()' ), self.positionChanged )
        self.connect( target.parentObject(), SIGNAL( 'xChanged()' ), self.positionChanged )
        self.connect( target.parentObject(), SIGNAL( 'yChanged()' ), self.positionChanged )
        self.connect( source, SIGNAL( 'yChanged()' ), self.positionChanged )
        source.connectionsChanged.emit()
        target.connectionsChanged.emit()

        if len( self.outEdges( source ) ) == 1:
            source.valueChanged.connect( self.onValueChanged )

    def outEdges( self, item ):
        if isinstance( item, PortItem ):
            return [ edge for edge in self.edges if edge[0] == item ]
        elif isinstance( item, BaseItem ):
            return [ edge for edge in self.edges if ( edge[0] in item.providesPorts ) ]
        else:
            assert( False )

    def inEdges( self, item ):
        if isinstance( item, PortItem ):
            return [ edge for edge in self.edges if edge[1] == item ]
        elif isinstance( item, BaseItem ):
            return [ edge for edge in self.edges if ( edge[1] in item.requiresPorts ) ]
        else:
            assert( False )

    def items( self ):
        return [ item.toGraphicsObject() for item in self.parent().items() if isinstance( item.toGraphicsObject(), BaseItem ) ]

    def rootItems( self ):
        return [ item for item in self.items() if len( item.requiresPorts ) == 0 ]

    def unconnectedItems( self ):
        items = []
        for item in self.items():
            unconnected = False
            for port in item.requiresPorts:
                if len( self.inEdges( port ) ) == 0:
                    unconnected = True
                    break
            if not unconnected:
                for port in item.providesPorts:
                    if len( self.outEdges( port ) ) == 0:
                        unconnected = True
                        break

            if unconnected:
                items.append( item )

        return items

    def unconfiguredItems( self ):
        return [ item for item in self.items() if item.getState() == BaseItem.UNCONFIGURED ]

    def positionChanged( self ):
        self.updateEdges( self.sender() )

    def updateEdges( self, item ):
        # Update edge
        for edge in self.inEdges( item ) + self.outEdges( item ):
            edge[2].setLine( edge[0].scenePos().x(), edge[0].scenePos().y(), edge[1].scenePos().x(), edge[1].scenePos().y() )

    def onValueChanged( self ):
        for edge in self.outEdges( self.sender() ):
            edge[1].setValue( self.sender().value() )

    def deleteEdge( self, edge ):
        for e in filter( lambda x: x[2] == edge, self.edges ):

            # Disconnect slot if last edge
            source = e[0]
            if len( self.outEdges( source ) ) == 1:
                source.valueChanged.disconnect( self.onValueChanged )

            self.edges.remove( e )
            e[0].connectionsChanged.emit()
            e[1].connectionsChanged.emit()

        self.parent().removeItem( edge )

class FlowScene( QGraphicsScene ):
    # SIGNALS
    item_added = pyqtSignal( QGraphicsItem )
    before_run = pyqtSignal()
    error_sent = pyqtSignal( str )
    item_error_sent = pyqtSignal( str, str )
    filename_changed = pyqtSignal()
    modified_changed = pyqtSignal()

    def __init__( self, parent = None ):
        QGraphicsScene.__init__( self, parent )

        self.addRect( 0, 0, 1, 1, Qt.white )

        # Filename
        self.filename = None
        self.modified = False

        # Port Connecter and Graph
        self.graph = FlowSceneGraph( self )
        self.connecter = FlowScenePortConnecter( self )
        self.connecter.edgeCreated.connect( self.graph.addEdge )

        # ItemToItemSnapper
        self.snapper = ItemToItemSnapper( self )

        self.setupActions()

    def setFilename( self, filename ):
        self.filename = filename
        self.filename_changed.emit()

    def setModified( self, modified ):
        self.modified = modified
        self.modified_changed.emit()

    def setupActions( self ):
        self.loadAction = QAction( QIcon.fromTheme( "document-open" ), "Open", self )
        self.saveAction = QAction( QIcon.fromTheme( "document-save" ), "Save", self )
        self.saveAsAction = QAction( QIcon.fromTheme( "document-save-as" ), "Save As", self )
        self.runAction = QAction( QIcon.fromTheme( "run-build" ), "Run", self )

        self.connect( self.loadAction, SIGNAL( 'triggered()' ), self.slotLoad )
        self.connect( self.saveAction, SIGNAL( 'triggered()' ), self.slotSave )
        self.connect( self.saveAsAction, SIGNAL( 'triggered()' ), self.slotSaveAs )
        self.connect( self.runAction, SIGNAL( 'triggered()' ), self.run )

    def run( self ):
        self.before_run.emit()

        # Unconnected Items?
        if len( self.graph.unconnectedItems() ) > 0:
            self.error_sent.emit( "There are unconnected items in the graph" )
            return

        # Get Roots
        roots = self.graph.rootItems()
        if len( roots ) == 0:
            self.error_sent.emit( "There is no item in the graph" )
            return

        # Unconfigured Items?
        if len( self.graph.unconfiguredItems() ) > 0:
            self.error_sent.emit( "There are unconfigured items in the graph" )
            return

        # Root Items have same number of runs
        if len( set( map( lambda root: root.numRuns(), self.graph.rootItems() ) ) ) != 1:
            self.error_sent.emit( "The root items have different numbers of runs" )
            return

        # Before execute
        for item in self.graph.items():
            item.initialize()

        runs = roots[0].numRuns()
        # TODO better API
        window = self.views()[0].window()
        window.widget.buttonBar.setCurrentIndex( 1 )
        window.runWidget.setMinimum( 0 )
        window.runWidget.setMaximum( runs )
        window.runWidget.setValue( 0 )

        # Start execution
        for _ in range( runs ):
            # Put Items back to configured
            for item in self.graph.items():
                item.setState( BaseItem.CONFIGURED )

            for root in roots:
                root.execute()
                window.runWidget.setValue( window.runWidget.value() + 1 )
                QApplication.processEvents()

    def notifyError( self, item, error ):
        self.item_error_sent.emit( "Run-time Error: %s" % error, item.description )

    def beforeDelete( self, item ):
        for edge in self.graph.inEdges( item ) + self.graph.outEdges( item ):
            self.graph.deleteEdge( edge[2] )

    # Override addItem to get control
    def addItem( self, item ):
        QGraphicsScene.addItem( self, item )

        if isinstance( item, BaseItem ):
            # Add an event filter
            for port in [child.toGraphicsObject() for child in item.childItems() if isinstance( child.toGraphicsObject(), PortItem )]:
                port.installEventFilter( self.connecter )

            item.create()
            item.close_button.parentAppearAnimation.start()

        self.setModified( True )
        self.item_added.emit( item )

    # Saving and Loading
    def slotLoad( self ):
        filename = QFileDialog.getOpenFileName( None, 'Open Graph', '', 'RevKit Flow Graph (*.rfg)' )
        if not filename.isEmpty():
            self.setFilename( str( filename ) )
            self.load( self.filename )
            return True
        return False

    # MainWindow assures that it is always called on new, blank scenes
    def load( self, filename ):
        f = QFile( filename )
        f.open( QIODevice.ReadOnly )

        doc = QDomDocument()
        doc.setContent( f )

        root = doc.documentElement()
        items = root.childNodes().at( 0 ).toElement()
        connections = root.childNodes().at( 1 ).toElement()

        itemmap = dict()

        # Iterate through all children
        for n in [ items.childNodes().at( i ).toElement() for i in range( items.childNodes().length() ) ]:
            # type and pos
            _type = str( n.attributeNode( "type" ).value() )
            poselem = n.elementsByTagName( "pos" ).at( 0 ).toElement()
            _pos = QPointF( float( str( poselem.attributeNode( "x" ).value() ) ), float( str( poselem.attributeNode( "y" ).value() ) ) )

            exec( "from items.%s import *" % _type )
            item = eval( _type )()
            item.setPos( _pos )
            self.addItem( item )

            item.load( n.elementsByTagName( "properties" ).at( 0 ).toElement() )

            itemmap[int( str( n.attributeNode( "id" ).value() ) )] = item

        # Connections
        for n in [ connections.childNodes().at( i ).toElement() for i in range( connections.childNodes().length() ) ]:
            sourceelem = n.childNodes().at( 0 ).toElement()
            targetelem = n.childNodes().at( 1 ).toElement()

            sourceid = int( str( sourceelem.attributeNode( "id" ).value() ) )
            sourceport = int( str( sourceelem.attributeNode( "port" ).value() ) )

            targetid = int( str( targetelem.attributeNode( "id" ).value() ) )
            targetport = int( str( targetelem.attributeNode( "port" ).value() ) )

            self.graph.addEdge( itemmap[sourceid].providesPorts[sourceport], itemmap[targetid].requiresPorts[targetport] )

        QApplication.processEvents()
        self.update()
        QApplication.processEvents()

        for item in self.graph.items():
            self.graph.updateEdges( item )

        f.close()
        self.setModified( False )

    def slotSave( self ):
        if self.filename is None:
            return self.slotSaveAs()
        else:
            self.save( self.filename )
            return True

    def slotSaveAs( self ):
        filename = QFileDialog.getSaveFileName( None, 'Save Graph', '', 'RevKit Flow Graph (*.rfg)' )
        if not filename.isEmpty():
            self.setFilename( str( filename ) )
            self.save( self.filename )
            return True
        return False

    def save( self, filename ):
        doc = QDomDocument()
        root = doc.createElement( "graph" )
        doc.appendChild( root )

        items = doc.createElement( "items" )
        root.appendChild( items )

        # Items
        for index, item in enumerate( self.graph.items() ):
            elem = item.save( doc.createElement( "item" ) )
            elem.setAttribute( "id", index )
            items.appendChild( elem )

        # Connections
        connections = doc.createElement( "connections" )
        root.appendChild( connections )

        for edge in self.graph.edges:
            connection = doc.createElement( "connection" )

            # Source
            source = doc.createElement( "source" )
            source.setAttribute( "id", self.graph.items().index( edge[0].parentObject() ) )
            source.setAttribute( "port", edge[0].parentObject().providesPorts.index( edge[0] ) )
            connection.appendChild( source )

            # Target
            target = doc.createElement( "target" )
            target.setAttribute( "id", self.graph.items().index( edge[1].parentObject() ) )
            target.setAttribute( "port", edge[1].parentObject().requiresPorts.index( edge[1] ) )
            connection.appendChild( target )

            connections.appendChild( connection )

        # Write
        f = QFile( filename )
        f.open( QIODevice.WriteOnly | QIODevice.Text )
        stream = QTextStream( f )
        stream << doc.toString()
        f.close()

        self.setModified( False )
