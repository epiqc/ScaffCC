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
import re, inspect

from revkitui import RevLibEditor

from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from core.BaseItemButton import *
from core.BaseItemButtonMaximize import *
from core.BaseItemButtonClose import *
from core.PathSelector import *
from core.PortItem import *
from core.PropertyWidgetManagement import PropertyWidgetManager, toCamelCase

from core.SettingsDialog import *

def item( description, iconname = "unknown", requires = None, provides = None, properties = [], widget = None ):
    def f( cls ):
        def wrap( *args, **kwargs ):
            cls.description = description
            cls.iconname = iconname

            self = cls( *args, **kwargs )

            # Ports
            if requires is not None:
                self.requires( [ requires ] if isinstance( requires, str ) else requires )
            if provides is not None:
                self.provides( [ provides ] if isinstance( provides, str ) else provides )

            # Actions
            self.actions = [ [ m.icon, m.label, m ] for (_,m) in inspect.getmembers( self ) if hasattr( m, 'is_action' ) ]
            self.actions.sort( key = lambda x: x[2].order )

            # Properties
            self.manager = PropertyWidgetManager( self )
            for p in properties:
                setattr( self, p, QString() )

                if not hasattr( self, "set%s" % toCamelCase( p ) ):
                    setattr( self, "set%s" % toCamelCase( p ), self.manager.createSetter( p ) )

                self.properties.append( p )

            # Widget
            if widget is not None:
                # Tuple or class
                if isinstance( widget, dict ):
                    self._widget_class = widget['class']
                    self.custom_size = QSize( widget['size'][0], widget['size'][1] )
                else:
                    self._widget_class = widget
            return self
        return wrap
    return f

def action( label, icon = None ):
    def wrap( f ):
        f.order = int( re.search( 'line (\d+)', str( f.__code__ ) ).group( 1 ) )
        f.is_action = True

        f.label = label
        f.icon  = icon

        return f
    return wrap

class BaseItem( QGraphicsObject ):
    description = "BaseItem"
    iconname = "unknown"
    whatsthis = None
    actions = []
    has_widget = False
    custom_size = None

    requestTabWidget = pyqtSignal( QWidget, QString, QString )
    portAdded = pyqtSignal( PortItem )
    portRemoved = pyqtSignal( PortItem )

    UNCONFIGURED, CONFIGURED, RUNNING, DONE, ERROR = range( 5 )

    def __init__( self, parent = None ):
        QGraphicsObject.__init__( self, parent )

        self.setAcceptHoverEvents( True )
        self.setFlags( QGraphicsItem.ItemIsMovable )
        self.setCacheMode( QGraphicsItem.DeviceCoordinateCache )

        # properties
        self._color = Settings()["color_unconfigured"]
        self._text = QString()
        self._state = self.UNCONFIGURED
        self._height = 40
        self._width = 200

        self.properties = []

        # effect
        #effect = QGraphicsDropShadowEffect();
        #effect.setOffset( 2, 2 )
        #effect.setBlurRadius( 10 )
        #self.setGraphicsEffect( effect )

        # for the graph and actions
        self._requires = []
        self._provides = []
        self.requiresPorts = []
        self.providesPorts = []
        self.requiresMapping = dict()

        # Buttons
        self.close_button = BaseItemButtonClose( self )
        self.maximize_button = BaseItemButtonMaximize( self )
        self.maximize_button.maximized.connect( self.onMaximized )
        self.maximize_button.minimized.connect( self.onMinimized )

        self.buttons = [ self.maximize_button, self.close_button ]

        # default
        #self.setScale( 0.0 )

    # Ports
    def addPorts( self, l, direction = Qt.AlignTop ):
        ports = []
        factor = -1 if direction == Qt.AlignTop else 1
        y = factor * ( self._height / 2 )
        for i, name in enumerate( l ):
            port = PortItem( name, direction, self )
            port.setPos( ( ( i + 1.0 ) / ( len( l ) + 1 ) ) * 200 - 100, y )
            ports.append( port )
            self.portAdded.emit( port )
        return ports

    def addPort( self, name, direction = Qt.AlignTop ):
        l = self.requiresPorts if direction == Qt.AlignTop else self.providesPorts

        y = 0
        if self.maximize_button.isMaximized():
            y = -20 if direction == Qt.AlignTop else self._height - 20
        else:
            factor = -1 if direction == Qt.AlignTop else 1
            y = factor * ( self._height / 2 )

        # Move other
        for i, port in enumerate( l ):
            port.setPos( ( ( i + 1.0 ) / ( len( l ) + 2 ) ) * 200 - 100, y )

        new_port = PortItem( name, direction, self )
        new_port.setPos( ( len( l ) + 1.0 ) / ( len( l ) + 2 ) * 200 - 100, y )
        l.append( new_port )
        self.portAdded.emit( new_port )
        return new_port

    def addRequires( self, name ):
        port = self.addPort( name )
        port.valueChanged.connect( self.requiresValueChanged )
        return port

    def addProvides( self, name ):
        return self.addPort( name, Qt.AlignBottom )

    def removePort( self, index, direction = Qt.AlignTop ):
        l = self.requiresPorts if direction == Qt.AlignTop else self.providesPorts

        if index not in range( len( l ) ): return
        port = l[index]

        # remove edges
        edges = self.scene().graph.inEdges( port ) if direction == Qt.AlignTop else self.scene().graph.outEdges( port )
        for e in edges:
            self.scene().graph.deleteEdge( e[2] )

        # remove port
        self.portRemoved.emit( port )
        l.remove( port )

        # FIXME remove instead of making invisible
        #self.scene().removeItem( port )
        port.setVisible( False )

        # Update Positions
        for i, p in enumerate( l ):
            p.setPos( ( ( i + 1.0 ) / ( len( l ) + 1 ) ) * 200 - 100, p.pos().y() )

    def removeRequires( self, index ):
        self.removePort( index )

    def removeProvides( self, index ):
        self.removePort( index, Qt.AlignBottom )

    def requires( self, l ):
        self._requires = l
        self.requiresPorts = self.addPorts( l )

        # prepare for execution
        for port in self.requiresPorts:
            port.valueChanged.connect( self.requiresValueChanged )

    def provides( self, l ):
        self._provides = l
        self.providesPorts = self.addPorts( l, Qt.AlignBottom )

    def requiresValueChanged( self ):
        self.requiresMapping[self.sender()] = self.sender().value()

        if len( self.requiresMapping ) == len( self.requiresPorts ):
            # Execute
            inputs = []
            for port in self.requiresPorts:
                inputs.append( self.requiresMapping[port] )
                port.setValue( None )
            self.execute( inputs )
            self.requiresMapping.clear()

    def boundingRect( self ):
        return QRectF( -self._width / 2, -20, self._width, self._height )

    def paint( self, painter, option, widget = None ):
        # rectangle back ground
        painter.setBrush( QBrush( self.createButtonGradient( option.rect.height(), self._color ) ) )
        painter.setPen( self._color.darker() )
        painter.drawRoundedRect( option.rect, 7, 7 )

        # text
        tr = QRect( option.rect )
        tr.setHeight( 40 )

        tr2 = QRect( tr )
        tr2.translate( 1, 1 )

        painter.setFont( QFont( "Helvetica", 8, QFont.Bold ) )
        painter.setPen( QColor( "#303030" ) )
        painter.drawText( tr2, Qt.AlignCenter, self._text )
        painter.setPen( QColor( "#ffffff" ) )
        painter.drawText( tr, Qt.AlignCenter, self._text )

    def getText( self ):
        return self._text

    def setText( self, value ):
        self._text = value
        self.update()

    text = QtCore.pyqtProperty( "QString", getText, setText )

    def getColor( self ):
        return self._color

    def setColor( self, value ):
        self._color = value
        self.update()

    color = QtCore.pyqtProperty( "QColor", getColor, setColor )

    def getWidth( self ):
        return self._width

    def setWidth( self, value ):
        self._width = value
        self.update()

    width = QtCore.pyqtProperty( "int", getWidth, setWidth )

    def getHeight( self ):
        return self._height

    def setHeight( self, value ):
        self._height = value
        self.update()

    height = QtCore.pyqtProperty( "int", getHeight, setHeight )

    def getState( self ):
        return self._state

    def setState( self, value ):
        self._state = value

        new_color = None
        if self._state == self.UNCONFIGURED:
            new_color = Settings()["color_unconfigured"]
        elif self._state == self.CONFIGURED:
            new_color = Settings()["color_configured"]
        elif self._state == self.RUNNING:
            new_color = Settings()["color_processing"]
        elif self._state == self.DONE:
            new_color = Settings()["color_finished"]
        elif self._state == self.ERROR:
            new_color = Settings()["color_error"]
        else:
            assert( False )

        self.setColor( new_color )
        QApplication.processEvents()

    # Operation
    def create( self ):
        if hasattr( self, '_widget_class' ):
            self.widget = self.mainWidget( self._widget_class() )
            self.widget.ownerItem = self
            if hasattr( self.widget, "onCreate" ):
                self.widget.onCreate()

            # Connect UI with properties
            for p in self.properties:
                if hasattr( self, "widget" ) and hasattr( self.widget, p ):
                    widget = getattr( self.widget, p )
                    self.manager.updateValue( p ) # Initial Value

                    key = type( widget )
                    if PropertyWidgetManager.type_mapping.has_key( key ) and PropertyWidgetManager.type_mapping[key][2] is not None:
                        signal = PropertyWidgetManager.type_mapping[key][2]
                        if isinstance( signal, str ):
                            self.connect( widget, SIGNAL( signal ), self.manager.widgetValueChanged )
                        else: #pyqtSignal
                            signal.__get__( widget, widget.__class__ ).connect( self.manager.widgetValueChanged )
        self.onCreate()

    def onCreate( self ):
        pass

    def execute( self, inputs = [] ):
        self.setState( self.RUNNING )
        outputs = self.executeEvent( inputs )

        # Error Handling
        if isinstance( outputs, str ):
            self.setState( self.ERROR )
            self.scene().notifyError( self, outputs )
            return

        # Set data to the ports
        for i, o in enumerate( outputs ):
            self.providesPorts[i].setValue( o )
        self.setState( self.DONE )
        return outputs

    def executeEvent( self, inputs ):
        return []

    def initialize( self ): # Called in the beginning before execution
        pass

    # TODO when and how to call?
    def finalize( self ): # Called in the end after execution of all runs
        pass

    def numRuns( self ): # Number of runs
        return 1

    # Override
    def contextMenuEvent( self, event ):
        if len( self.actions ) > 0:
            menu = QMenu()
            for action in self.actions:
                if len( action ) == 2:
                    menu.addAction( action[0], action[1] )
                else:
                    menu.addAction( QIcon.fromTheme( action[0] ), action[1], action[2] )

            menu.exec_( event.screenPos() )

    def mouseDoubleClickEvent( self, event ):
        """If there is are actions, execute the first one by default on double click"""
        if len( self.actions ) > 0:
            self.actions[0][-1]()

    def hoverEnterEvent( self, event ):
        if self.maximize_button.isMaximized():
            return

        if self.has_widget:
            self.maximize_button.show()
        self.close_button.show()
        QGraphicsObject.hoverEnterEvent( self, event )

    def hoverLeaveEvent( self, event ):
        if self.maximize_button.isMaximized():
            return

        # Prevent items from closing if mouse leaves over them
        item = self.scene().itemAt( event.scenePos() )
        if item is not None:
            item = item.toGraphicsObject()
            if item in self.buttons:
                QGraphicsObject.hoverLeaveEvent( self, event )
                return

        if self.has_widget:
            self.maximize_button.hide()
        self.close_button.hide()
        QGraphicsObject.hoverLeaveEvent( self, event )

    def onMaximized( self ):
        if self.has_widget:
            self.maximize_button.show()
        self.close_button.hide()

    def onMinimized( self ):
        if self.has_widget:
            self.maximize_button.hide()
        self.close_button.hide()

    # Main Widget
    def mainWidget( self, otherWidget = None ):
        self.has_widget = True
        return self.maximize_button.widget( otherWidget )

    # Load/Save
    def load( self, properties ):
        for prop in [ properties.childNodes().at( i ).toElement() for i in range( properties.childNodes().length() ) ]:
            name = str( prop.attributeNode( "name" ).value() )
            value = prop.attributeNode( "value" ).value()

            # TODO
            #self.setProperty( name, value )
            t = self.metaObject().property( self.metaObject().indexOfProperty( "conf_%s" % name ) ).typeName()
            if t == "bool":
                eval( "self.set%s( value == 'true' )" % toCamelCase( name ) )
            else:
                eval( "self.set%s( value )" % toCamelCase( name ) )

    def save( self, elem ):
        elem.setAttribute( "type", self.__class__.__name__ )
        pos = elem.ownerDocument().createElement( "pos" )
        pos.setAttribute( "x", str( self.scenePos().x() ) )
        pos.setAttribute( "y", str( self.scenePos().y() ) )
        elem.appendChild( pos )

        # Properties
        properties = elem.ownerDocument().createElement( "properties" )
        for i in range( self.metaObject().propertyCount() ):
            name = str( self.metaObject().property( i ).name() )
            if name.startswith( "conf_" ):
                prop = elem.ownerDocument().createElement( "property" )
                prop.setAttribute( "name", name[5:] )
                prop.setAttribute( "value", self.metaObject().property( i ).read( self ).toString() )
                properties.appendChild( prop )

        # New properties
        for name in self.properties:
            prop = elem.ownerDocument().createElement( "property" )

            if hasattr( self, "get%s" % toCamelCase( name ) ):
                value = getattr( self, "get%s" % toCamelCase( name ) )()
            else:
                value = getattr( self, name )

            prop.setAttribute( "name", name )
            prop.setAttribute( "value", value )
            properties.appendChild( prop )
        elem.appendChild( properties )

        return elem

    #private
    def createButtonGradient( self, height, color ):
        grad = QLinearGradient( QPointF( 0, -height / 2 ), QPointF( 0, height / 2 ) )
        grad.setColorAt( 0, color.lighter( 120 ) )
        grad.setColorAt( 0.4, color.darker( 120 ) )
        grad.setColorAt( 1, color )
        return grad
