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

from PyQt4 import QtCore
from PyQt4.QtCore import QEvent, QObject

class ScrollControl( QObject ):
    def __init__( self, parent = None ):
        QObject.__init__( self, parent )

        parent.installEventFilter( self )

        self._scrollingEnabled = True

    def eventFilter( self, obj, event ):
        if self._scrollingEnabled:
            return QObject.eventFilter( self, obj, event )
        else:
            if event.type() == QEvent.Wheel:
                return True
            else:
                print event.type()
                return QObject.eventFilter( self, obj, event )

    def getScrollingEnabled( self ):
        return self._scrollingEnabled

    def setScrollingEnabled( self, value ):
        self._scrollingEnabled = value

    scrollingEnabled = QtCore.pyqtProperty( "bool", getScrollingEnabled, setScrollingEnabled )

