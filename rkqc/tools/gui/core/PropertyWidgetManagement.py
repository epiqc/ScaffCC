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
from PyQt4.QtCore import QObject, QString
from PyQt4.QtGui import QCheckBox, QComboBox, QDoubleSpinBox, QLineEdit, QSpinBox

from revkitui import RevLibEditor

from core.PathSelector import PathSelector
from core.SpecificationTable import SpecificationTable

def toCamelCase( s ):
    r = s[0].upper()
    i = 1
    while i < len( s ):
        if s[i] == '_':
            r += s[i+1].upper()
            i += 2
        else:
            r += s[i]
            i += 1
    return r

def widgetValueChanged( item, pname ):

    if isinstance( witem, QComboBox ):
        value = QString.number( witem.currentIndex() )
    elif isinstance( witem, PathSelector ):
        value = witem.getPath()

    setattr( item, pname, value )

    if hasattr( item, "on%sChanged" % toCamelCase( pname ) ):
        getattr( item, "on%sChanged" % toCamelCase( pname ) )( value )

class PropertyWidgetManager( QObject ):
    type_mapping = {
        QCheckBox:          ( lambda w: QString( "1" if w.isChecked() else "0" ), lambda w, v: w.setChecked( v == "1" ),      'stateChanged(int)' ),
        QComboBox:          ( lambda w: QString.number( w.currentIndex() ),       lambda w, v: w.setCurrentIndex( int( v ) ), 'currentIndexChanged(int)' ),
        QDoubleSpinBox:     ( lambda w: QString.number( w.value() ),              lambda w, v: w.setValue( float( v ) ),      'valueChanged(double)' ),
        QLineEdit:          ( QLineEdit.text,                                     QLineEdit.setText,                          'textChanged(const QString&)' ),
        QSpinBox:           ( lambda w: QString.number( w.value() ),              lambda w, v: w.setValue( int( v ) ),        'valueChanged(int)' ),
        PathSelector:       ( PathSelector.path,                                  PathSelector.setPath,                       PathSelector.pathChanged ),
        RevLibEditor:       ( None, lambda w, v:                                  w.load( str( v ) ),                         None ),
        SpecificationTable: ( None,                                               SpecificationTable.load,                    None )
        }

    def __init__( self, parent = None ):
        QObject.__init__( self, parent )

    def createSetter( self, pname ):
        item = self.parent()
        def f( value ):
            setattr( item, pname, value )

            # Update widgets
            if hasattr( item, "widget" ) and hasattr( item.widget, pname ):
                witem = getattr( item.widget, pname )
                key = type( witem )
                if PropertyWidgetManager.type_mapping.has_key( key ) and PropertyWidgetManager.type_mapping[key][1] is not None:
                    PropertyWidgetManager.type_mapping[key][1]( witem, value )

            if hasattr( item, "on%sChanged" % toCamelCase( pname ) ):
                getattr( item, "on%sChanged" % toCamelCase( pname ) )( value )
        return f

    def updateValue( self, pname ):
        self.parent().scene().setModified( True )

        witem = getattr( self.parent().widget, pname )

        key = type( witem )
        if PropertyWidgetManager.type_mapping.has_key( key ) and PropertyWidgetManager.type_mapping[key][0] is not None:
            value = PropertyWidgetManager.type_mapping[key][0]( witem )
            setattr( self.parent(), pname, value )

            if hasattr( self.parent(), "on%sChanged" % toCamelCase( pname ) ):
                getattr( self.parent(), "on%sChanged" % toCamelCase( pname ) )( value )

    def widgetValueChanged( self ):
        pname = [ n for (n,w) in self.parent().widget.__dict__.items() if w == self.sender() ][0]
        self.updateValue( pname )
