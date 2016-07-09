from PyQt4.QtGui import QComboBox

class CategoryComboBox( QComboBox ):
    def __init__( self, parent = None ):
        QComboBox.__init__( self, parent )

    def setEntries( self, entries ):
        self.clear()
        for id, name in entries:
            self.addItem( name )
            self.setItemData( self.count() - 1, id )

    def currentId( self ):
        if self.currentIndex() >= 0:
            return self.itemData( self.currentIndex() )
        else:
            return None

