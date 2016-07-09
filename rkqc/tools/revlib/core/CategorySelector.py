from PyQt4.QtCore import Qt, SIGNAL
from PyQt4.QtGui import QMenu, QToolButton

class CategorySelector( QToolButton ):
    def __init__( self, items, parent = None ):
        QToolButton.__init__( self, parent )
        self.setText( "All categories" )

        self.setPopupMode( QToolButton.InstantPopup )

        self.menu = QMenu( self )
        self.setMenu( self.menu )

        self.items = items
        self.actions = map( self.createAction, self.items )
        self.menu.addSeparator()
        self.menu.addAction( "Select All" )
        self.menu.addAction( "Select None" )

        self.connect( self.menu, SIGNAL( 'aboutToShow()' ), self.slotMenuAboutToShow )

    def createAction( self, item ):
        action = self.menu.addAction( item )
        action.setCheckable( True )
        action.setChecked( True )
        return action

    def slotMenuAboutToShow( self ):
        self.menu.setMinimumWidth( self.width() )

