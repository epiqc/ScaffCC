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

from PyQt4.QtGui import QStyledItemDelegate, QSpinBox

class SpinBoxDelegate( QStyledItemDelegate ):
    def __init__( self, parent = None ):
        QStyledItemDelegate.__init__( self, parent )

    def createEditor( self, parent, option, index ):
        spinBox = QSpinBox( parent )
        spinBox.setMaximum( 65536 )
        return spinBox

    def setEditorData( self, editor, index ):
        editor.setValue( index.data().toInt()[0] )

    def setModelData( self, editor, model, index ):
        model.setData( index, editor.value() )

