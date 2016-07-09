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

from PyQt4.QtCore import Qt
from PyQt4.QtGui import QCheckBox, QCursor, QDialogButtonBox, QHBoxLayout, QTableWidgetItem, QToolButton, QWhatsThis
from PyQt4.QtScript import QScriptEngine

from tempfile import NamedTemporaryFile
import os, re

from core.BaseItem import *

from revkit import costs, quantum_costs, transistor_costs

from core.ColumnResizer import ColumnResizer
from core.ResultsTableWidget import *

from ui.DesignerWidget import *
from ui.DesignerDialog import *
from ui.ResultsTable import *
from ui.ResultsTableDialog import *

class ResultsTableDialog( DesignerDialog ):
    def __init__( self, parent = None ):
        DesignerDialog.__init__( self, Ui_ResultsTableDialog, parent )
        self.setModal( True )

        self.add.setIcon( QIcon.fromTheme( 'list-add' ) )
        self.remove.setIcon( QIcon.fromTheme( 'list-remove' ) )

        self.connect( self.add, SIGNAL( 'clicked()' ), self.add_clicked )
        self.connect( self.remove, SIGNAL( 'clicked()' ), self.remove_clicked )
        self.connect( self.buttonBox.button( QDialogButtonBox.Close ), SIGNAL( 'clicked()' ), self.close )
        self.connect( self.buttonBox.button( QDialogButtonBox.Help ), SIGNAL( 'clicked()' ), self.show_help )
        self.connect( self.tableWidget, SIGNAL( 'itemSelectionChanged()' ), self.update_buttons )

    def add_clicked( self ):
        self.add_entry( "New", "\"N/A\"" )

    def add_entry( self, name, expr ):
        self.tableWidget.setRowCount( self.tableWidget.rowCount() + 1 )
        row = self.tableWidget.rowCount() - 1
        self.tableWidget.setItem( row, 0, QTableWidgetItem( name ) )
        self.tableWidget.setItem( row, 1, QTableWidgetItem( expr ) )

    def remove_clicked( self ):
        map( self.tableWidget.removeRow, reversed( list( set( map( QTableWidgetItem.row, self.tableWidget.selectedItems() ) ) ) ) )

    def update_buttons( self ):
        self.remove.setEnabled( len( self.tableWidget.selectedItems() ) > 0 )

    def columns( self ):
        return map( lambda row: ( self.tableWidget.item( row, 0 ).text(),
                                  self.tableWidget.item( row, 1 ).text() ), range( self.tableWidget.rowCount() ) )

    def show_help( self ):
        QWhatsThis.showText( QCursor.pos(), self.tableWidget.whatsThis(), self )

class ResultsTable( DesignerWidget ):
    tabAdded = pyqtSignal()
    tabRemoved = pyqtSignal(int)

    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_ResultsTable, parent )

        self.add_button = QToolButton( self )
        self.add_button.setIcon( QIcon.fromTheme( "list-add" ) )
        self.add_button.setAutoRaise( True )
        self.connect( self.add_button, SIGNAL( 'clicked()' ), self.add_button_clicked )
        self.tabWidget.setCornerWidget( self.add_button, Qt.TopLeftCorner )

        self.remove_button = QToolButton( self )
        self.remove_button.setIcon( QIcon.fromTheme( "list-remove" ) )
        self.remove_button.setAutoRaise( True )
        self.connect( self.remove_button, SIGNAL( 'clicked()' ), self.remove_button_clicked )
        self.tabWidget.setCornerWidget( self.remove_button, Qt.TopRightCorner )

        self.connect( self.tabWidget, SIGNAL( 'currentChanged(int)' ), self.update_buttons )

        self.configure_extra_columns.setIcon( QIcon.fromTheme( 'configure' ) )
        self.configure_dialog = ResultsTableDialog()

        self.connect( self.configure_extra_columns, SIGNAL( 'clicked()' ), self.configure_extra_columns_clicked )

        self.export_pdf.setIcon( QIcon.fromTheme( 'application-pdf' ) )
        self.export_latex.setIcon( QIcon.fromTheme( 'text-x-tex' ) )

        self.connect( self.export_pdf, SIGNAL( 'clicked()' ), self.export_as_pdf )
        self.connect( self.export_latex, SIGNAL( 'clicked()' ), self.export_as_latex )

        self.splitter.setSizes( [1000,200] )

        self.chk_global = [ self.global_lines, self.global_gates, self.global_qc, self.global_tc ]

        self.update_buttons()

    def onCreate( self ):
        self.addTab( "Original" )
        self.tabAdded.connect( self.ownerItem.onTabAdded )
        self.tabRemoved.connect( self.ownerItem.onTabRemoved )

    def addTab( self, name = "New" ):
        self.tabWidget.addTab( ResultsTableWidget( self ), name )
        self.tabWidget.setCurrentIndex( self.tabWidget.count() - 1 )

    def add_button_clicked( self ):
        self.addTab()
        self.tabAdded.emit()
        self.update_buttons()

    def remove_button_clicked( self ):
        index = self.tabWidget.currentIndex()
        self.tabWidget.removeTab( index )
        self.tabRemoved.emit( index )
        self.update_buttons()

    def update_buttons( self ):
        self.remove_button.setEnabled( self.tabWidget.count() > 1 )

    def write_table( self, f ):
        # Prepare Strings
        columns = [ 'r' ] * len( filter( QCheckBox.isChecked, self.chk_global ) )
        header = [ "Benchmark" ]
        header.extend( [ str( chk.text() ) for chk in filter( QCheckBox.isChecked, self.chk_global ) ] )

        for widget in map( self.tabWidget.widget, range( self.tabWidget.count() ) ):
            columns.append( '|' )
            columns.extend( [ 'r' for _ in filter( lambda x: not widget.isColumnHidden( x ), range( 1, widget.columnCount() ) ) ] )

            header.extend( [ str( widget.labels[i] ) for i in filter( lambda x: not widget.isColumnHidden( x ), range( 1, widget.columnCount() ) ) ] )

        if len( self.configure_dialog.columns() ) > 0:
            columns.append( '|' )
            columns.extend( ['r'] * len( self.configure_dialog.columns() ) )
            header.extend( [ str( c ) for (c, e) in self.configure_dialog.columns() ] )

        # Items
        script_engine = QScriptEngine()
        items = []
        for row in range( self.tabWidget.widget( 0 ).rowCount() ):
            # Benchmark
            item = [ self.tabWidget.widget( 0 ).item( row, 0 ).text() ]

            # Sticky (global) Columns
            item.extend( map( lambda i: self.tabWidget.widget( 0 ).item( row, i + 1 ).text(),
                              map( self.chk_global.index,
                                   filter( QCheckBox.isChecked, self.chk_global ) ) ) )

            # Tables
            for widget in map( self.tabWidget.widget, range( self.tabWidget.count() ) ):
                item.extend( [ widget.item( row, i ).text() for i in filter( lambda x: not widget.isColumnHidden( x ), range( 1, widget.columnCount() ) ) ] )

            # Extra Columns
            for (_,e) in self.configure_dialog.columns():
                e = str( e )

                # Modify
                ops = [ "LINES", "GATES", "QC", "TC" ]
                regexp = "(%s)\[(\d+)]" % '|'.join( ops )

                while True:
                    m = re.search( regexp, e )
                    if m is None: break

                    table_index = int( m.group( 2 ) )
                    if table_index >= self.tabWidget.count():
                        e = "\"N/A\""
                        break

                    column_index = ops.index( m.group( 1 ) ) + 1

                    e = e.replace( m.group( 0 ), str( self.tabWidget.widget( table_index ).item( row, column_index ).text() ) )

                item.append( script_engine.evaluate( e ).toString() )

            items.append( map( to_latex, map( str, item ) ) )

        # Captions
        captions = '& ' * len( filter( QCheckBox.isChecked, self.chk_global ) )
        for i, widget in enumerate( map( self.tabWidget.widget, range( self.tabWidget.count() ) ) ):
            captions += "& \\multicolumn{%d}{|c}{%s} " % (
                len( filter( lambda x: not widget.isColumnHidden( x ), range( 1, widget.columnCount() ) ) ),
                self.tabWidget.tabText( i ) )
        if len( self.configure_dialog.columns() ) > 0:
            captions += "& \\multicolumn{%d}{|c}{}" % len( self.configure_dialog.columns() )

        f.write( "\\documentclass[10pt]{article}\n" )
        f.write( "\\topmargin=-0.25in\n" )
        f.write( "\\oddsidemargin=14.32mm\n" )
        f.write( "\\evensidemargin=14.32mm\n" )
        f.write( "\\addtolength{\\oddsidemargin}{-1in}\n" )
        f.write( "\\addtolength{\\evensidemargin}{-1in}\n" )
        f.write( "\\textheight=9.25in\n" )
        f.write( "\\textwidth=43pc\n" )
        f.write( "\\tabcolsep 6pt\n" )

        f.write( "\\renewcommand{\\sfdefault}{phv}\n" )
        f.write( "\\renewcommand{\\rmdefault}{ptm}\n" )
        f.write( "\\renewcommand{\\ttdefault}{pcr}\n" )
        f.write( "\\makeatletter\n" )
        f.write( "\\def\\footnotesize{\\@setfontsize{\\footnotesize}{8}{9pt}}\n" )
        f.write( "\\makeatother\n" )

        f.write( "\\begin{document}\n" )
        f.write( "\\footnotesize\n" )
        f.write( "\\noindent\\begin{tabular}{l%s} \\hline\n" % ''.join( columns ) )
        f.write( "  %s \\\\\n" % captions )
        f.write( "  %s \\\\ \\hline\n" % ' & '.join( header ) )

        f.write( '\\\\\n'.join( map( ' & '.join, items ) ) )
        f.write( '\\\\ \\hline\n' )

        f.write( "\\end{tabular}\n" )
        f.write( "\\end{document}\n" )

    def export_as_pdf( self ):
        with NamedTemporaryFile( delete = False, suffix = ".tex" ) as f:
            self.write_table( f )

        os.system( "pdflatex -output-directory %s %s" % ( f.name[:f.name.rfind( '/' )], f.name ) )
        for ext in [ "tex", "aux", "log" ]:
            os.unlink( "%s%s" % ( f.name[:-3], ext ) )
        os.system( "xdg-open %spdf" % f.name[:-3] )

    def export_as_latex( self ):
        filename = QFileDialog.getSaveFileName( None, 'Write LaTeX Table', '', 'LaTeX2e document (*.tex)' )
        if not filename.isEmpty():
            with open( str( filename ), 'w' ) as f:
                self.write_table( f )

    def configure_extra_columns_clicked( self ):
        self.configure_dialog.exec_()
        self.update_extra_columns()

    def update_extra_columns( self ):
        columns = len( self.configure_dialog.columns() )
        extras = "s" if columns != 1 else ""
        self.extra_columns.setText( "No extra columns" if columns == 0 else "%s extra column%s" % ( columns, extras ) )

@item( "Results Table",
       iconname = "x-office-spreadsheet",
       requires = "Circuit",
       properties = [ "tabnames", "showboxes", "globalcolumns", "extracolumns" ],
       widget = ResultsTable )
class ResultsTableItem( BaseItem ):
    """This item generates a table summarizing the results of an applied process. In particular, this item finds application if more than one benchmark is considered (i.e. in combination with the item <i>Path Benchmarks</i>). The result table gets a set of circuits and  lists the name of the benchmark, the number of lines, the number of gates, the quantum cost, and the transistor cost of them.<br /><br />

Using the result table, also different sets of circuits (e.g. obtained by different synthesis approaches) can be compared. Therefore, further item-inputs have to be added by clicking on the <b>+</b>-symbol at the left-hand side of the enlarged item.
A double click on the respective tab button enables to name each input individually. Global columns (i.e. table columns which should be listed only once for all circuit sets) can be defined at the right-hand side of the enlarged item. Finally, extra columns can be defined using the button <i>Configure</i>. Here, JavaScript expressions can be defined in order to e.g. automatically compute improvements of certain values (e.g. the number of gates or the quantum cost). The resulting table can be exported either as *.pdf- or as *.tex-file.<br /><br />

The usage of the result table is explicitly illustrated by means of a tutorial-video at the <i>www.revkit.org</i> website."""
    def onCreate( self ):
        self.setText( "Results" )
        self.setState( self.CONFIGURED )

    def initialize( self ):
        for i in range( self.widget.tabWidget.count() ):
            self.widget.tabWidget.widget( i ).clearContents()
            self.widget.tabWidget.widget( i ).setRowCount( 0 )

    def executeEvent( self, inputs ):
        for i, circ in enumerate( inputs ):
            widget = self.widget.tabWidget.widget( i )

            # Add new row
            widget.setRowCount( widget.rowCount() + 1 )

            # Add data
            row = widget.rowCount() - 1
            widget.setItem( row, 0, QTableWidgetItem( circ.circuit_name ) )
            widget.setItem( row, 1, QTableWidgetItem( str( circ.lines ) ) )
            widget.setItem( row, 2, QTableWidgetItem( str( circ.num_gates ) ) )
            widget.setItem( row, 3, QTableWidgetItem( str( costs( circ, quantum_costs() ) ) ) )
            widget.setItem( row, 4, QTableWidgetItem( str( costs( circ, transistor_costs() ) ) ) )
            try:
                widget.setItem( row, 5, QTableWidgetItem( "%.2f" % circ.runtime ) )
            except:
                widget.setItem( row, 5, QTableWidgetItem( "N/A" ) )

            for column in range( 6 ):
                widget.item( row, column ).setFlags( Qt.ItemIsSelectable | Qt.ItemIsEnabled )
                if column > 0:
                    widget.item( row, column ).setTextAlignment( Qt.AlignVCenter | Qt.AlignRight )

        return []

    def onTabAdded( self ):
        self.addRequires( "Circuit" )

    def onTabRemoved( self, index ):
        self.removeRequires( index )

    def getShowboxes( self ):
        return '@*@'.join( map( lambda w: ','.join( map( str, map( w.isColumnHidden, range( 1, w.columnCount() ) ) ) ),
                                map( self.widget.tabWidget.widget, range( self.widget.tabWidget.count() ) ) ) )

    def onShowboxesChanged( self, v ):
        texts = str( v ).split( '@*@' )
        if self.widget.tabWidget.count() < len( texts ):
            for i in range( len( texts ) - self.widget.tabWidget.count() ):
                self.widget.addTab()
                self.onTabAdded()

        for i, text in enumerate( texts ):
            for j, val in enumerate( map( lambda x: x == 'True', text.split( ',' ) ) ):
                self.widget.tabWidget.widget( i ).setColumnHidden( j + 1, val )

        self.widget.tabWidget.setCurrentIndex( 0 )

    def getTabnames( self ):
        return '@*@'.join( map( str, map( self.widget.tabWidget.tabText, range( self.widget.tabWidget.count() ) ) ) )

    def onTabnamesChanged( self, v ):
        texts = v.split( '@*@' )
        if self.widget.tabWidget.count() < len( texts ):
            for i in range( len( texts ) - self.widget.tabWidget.count() ):
                self.widget.addTab()
                self.onTabAdded()

        for i, text in enumerate( texts ):
            self.widget.tabWidget.setTabText( i, text )

        self.widget.tabWidget.setCurrentIndex( 0 )

    def getGlobalcolumns( self ):
        return ','.join( map( str, map( QCheckBox.isChecked, self.widget.chk_global ) ) )

    def onGlobalcolumnsChanged( self, v ):
        for j, val in enumerate( map( lambda x: x == 'True', str( v ).split( ',' ) ) ):
            self.widget.chk_global[j].setChecked( val )

    def getExtracolumns( self ):
        return '@*@'.join( map( lambda p: "%s:::%s" % p, self.widget.configure_dialog.columns() ) )

    def onExtracolumnsChanged( self, v ):
        for p in str( v ).split( '@*@' ):
            e = p.split( ":::" )
            try:
                self.widget.configure_dialog.add_entry( e[0], e[1] )
            except: pass
        self.widget.update_extra_columns()

def to_latex( s ):
    return s.replace( '_', '\\_' ).replace( '%', '\\%' )
