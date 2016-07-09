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

from PyQt4.QtCore import Qt, QEvent, QObject, QTimer, SIGNAL
from PyQt4.QtGui import QFormLayout, QGridLayout, QWidgetItem

class FormLayoutWidgetItem( QWidgetItem ):
    def __init__( self, widget, formLayout, itemRole ):
        QWidgetItem.__init__( self, widget )
        self.m_width = -1
        self.m_formLayout = formLayout
        self.m_itemRole = itemRole

    def sizeHint( self ):
        size = QWidgetItem.sizeHint( self )
        if self.m_width != -1:
            size.setWidth( self.m_width )
        return size

    def minimumSize( self ):
        size = QWidgetItem.minimumSize( self )
        if self.m_width != -1:
            size.setWidth( self.m_width )
        return size

    def maximumSize( self ):
        size = QWidgetItem.maximumSize( self )
        if self.m_width != -1:
            size.setWidth( self.m_width )
        return size

    def setWidth( self, width ):
        if width != self.m_width:
            self.m_width = width
            self.invalidate()

    def setGeometry( self, _rect ):
        rect = _rect
        width = self.widget().sizeHint().width()
        if self.m_itemRole == QFormLayout.LabelRole and self.m_formLayout.labelAlignment() & Qt.AlignRight:
            rect.setLeft( rect.right() - width )
        QWidgetItem.setGeometry( self, rect )

    def formLayout( self ):
        return self.m_formLayout

class ColumnResizer( QObject ):
    class ColumnResizerPrivate:
        widgets = []
        wrWidgetItemList = []
        gridColumnInfoList = []

        def __init__( self, q ):
            self.q = q
            self.updateTimer = QTimer( q )
            self.updateTimer.setSingleShot( True )
            self.updateTimer.setInterval( 0 )
            QObject.connect( self.updateTimer, SIGNAL( 'timeout()' ), q.updateWidth )

        def scheduleWithUpdate( self ):
            self.updateTimer.start()

    def __init__( self, parent = None ):
        QObject.__init__( self, parent )
        self.d = self.ColumnResizerPrivate( self )

    def addWidget( self, widget ):
        self.d.widgets.append( widget )
        widget.installEventFilter( self )
        self.d.scheduleWithUpdate()

    def updateWidth( self ):
        width = 0
        for widget in self.d.widgets:
            width = max( widget.sizeHint().width(), width )
        for item in self.d.wrWidgetItemList:
            item.setWidth( width )
            item.formLayout().update()
        for (l, i) in self.d.gridColumnInfoList:
            l.setColumnMinimumWidth( i, width )

    def eventFilter( self, obj, event ):
        if event.type() == QEvent.Resize:
            self.d.scheduleWithUpdate()
        return False

    def addWidgetsFromLayout( self, layout, column ):
        assert( column >= 0 )
        if isinstance( layout, QGridLayout ):
            self.addWidgetsFromGridLayout( layout, column )
        elif isinstance( layout, QFormLayout ):
            if column <= QFormLayout.SpanningRole:
                self.addWidgetsFromFormLayout( layout, column )

    def addWidgetsFromGridLayout( self, layout, column ):
        for row in range( layout.rowCount() ):
            item = layout.itemAtPosition( row, column )
            if item is None: continue
            widget = item.widget()
            if widget is None: continue
            self.addWidget( widget )
        self.d.gridColumnInfoList.append( (layout, column) )

    def addWidgetsFromFormLayout( self, layout, role ):
        for row in range( layout.rowCount() ):
            item = layout.itemAt( row, role )
            if item is None: continue
            widget = item.widget()
            if widget is None: continue
            layout.removeItem( item )
            del item
            newItem = FormLayoutWidgetItem( widget, layout, role )
            layout.setItem( row, role, newItem )
            self.addWidget( widget )
            self.d.wrWidgetItemList.append( newItem )

