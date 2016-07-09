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

from PyQt4.QtCore import SIGNAL
from PyQt4.QtGui import QIcon, QWhatsThis

from ui.DesignerWidget import *
from ui.MainWidget import *

from FlowItemTreeWidget import *

class MainWidget( DesignerWidget ):
    def __init__( self, parent = None ):
        DesignerWidget.__init__( self, Ui_MainWidget, parent )

        self.helpItems.setText( "What's This?" )
        self.helpItems.setIcon( QIcon.fromTheme( "help-contextual" ) )
        self.connect( self.helpItems, SIGNAL( 'released()' ), QWhatsThis.enterWhatsThisMode )

        self.splitter.setSizes( [ 200, 1000] )
        self.splitter_2.setSizes( [1000, 200] )

        self.connect( self.filterLine, SIGNAL( 'textChanged(const QString&)' ), self.toolBox.setFilter )

        try:
            self.filterLine.setPlaceholderText( "Filter Items" )
        except: pass

    def addToolCategory( self, name, items ):
        self.toolBox.addCategory( name, items )
