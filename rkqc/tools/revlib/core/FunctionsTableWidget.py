from PyQt4.QtGui import QTableWidget, QTableWidgetItem

class FunctionsTableWidget( QTableWidget ):
    def __init__( self, parent = None ):
        QTableWidget.__init__( self, parent )

    def setItems( self, items ):
        self.clear()
        self.setColumnCount( 7 )
        self.setRowCount( len( items ) )
        self.setHorizontalHeaderLabels( [ "Name", "Inputs", "Outputs", "Cubes", "Category", "URL", "Description" ] )

        for row, item in enumerate( items ):
            for column, content in enumerate( item ):
                self.setItem( row, column, QTableWidgetItem( str( content ) ) )

