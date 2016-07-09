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

from PyQt4.QtCore import QSettings, QVariant, SIGNAL
from PyQt4.QtGui import QColor, QDialogButtonBox

from core.ColorComboBox import ColorComboBox

from ui.DesignerDialog import DesignerDialog
from ui.SettingsDialog import Ui_SettingsDialog

class SettingsDialog( DesignerDialog ):
    prefix = "setting_"

    # Old Defaults
    # defaults = { "color_configured": QColor( "#7777ff" ),
    #              "color_unconfigured": QColor( "#99ccff" ),
    #              "color_processing": QColor( "#ff9900" ),
    #              "color_finished": QColor( "#77ff77" ) }

    defaults = { "color_configured": QColor( "Gainsboro" ),
                 "color_unconfigured": QColor( "Aliceblue" ),
                 "color_processing": QColor( "#ff9900" ),
                 "color_finished": QColor( "Darkgreen" ),
                 "color_error": QColor( "#ff0000" ) }


    single = None

    def __init__( self, parent = None ):
        if SettingsDialog.single:
            raise SettingsDialog.single
        else:
            DesignerDialog.__init__( self, Ui_SettingsDialog )

            # Fix margin because of documentMode tabWidget
            self.layout().setMargin( 0 )

            self.settings = QSettings()

            self.readValues()

            # Slots
            self.connect( self.buttonBox, SIGNAL( 'rejected()' ), self.close )
            self.connect( self.buttonBox, SIGNAL( 'accepted()' ), self.onAccepted )
            self.connect( self.buttonBox, SIGNAL( 'clicked( QAbstractButton* )' ), self.buttonClicked )

            SettingsDialog.single = self

    def readValues( self ):
        for key, value in self.ui.__dict__.items():
            if key.startswith( self.prefix ):
                setting = key[len( self.prefix ):]
                control = getattr( self, key )

                v = self.settings.value( setting, self.defaults[setting] )

                if isinstance( control, ColorComboBox ):
                    control.setColor( v.toPyObject() )

    def writeValues( self ):
        for key, value in self.ui.__dict__.items():
            if key.startswith( self.prefix ):
                setting = key[len( self.prefix ):]
                control = getattr( self, key )

                if isinstance( control, ColorComboBox ):
                    self.settings.setValue( setting, QVariant( control.color() ) )

    def restoreDefaults( self ):
        for key, value in self.ui.__dict__.items():
            if key.startswith( self.prefix ):
                value = self.defaults[key[len( self.prefix ):]]
                control = getattr( self, key )

                if isinstance( control, ColorComboBox ):
                    control.setColor( value )

    def onAccepted( self ):
        self.writeValues()
        self.close()

    def buttonClicked( self, button ):
        if button == self.buttonBox.button( QDialogButtonBox.Apply ):
            self.writeValues()
        elif button == self.buttonBox.button( QDialogButtonBox.RestoreDefaults ):
            self.restoreDefaults()

    def __getitem__( self, key ):
        return self.settings.value( key, self.defaults[key] ).toPyObject()

def Settings():
    if SettingsDialog.single:
        return SettingsDialog.single
    else:
        return SettingsDialog()



