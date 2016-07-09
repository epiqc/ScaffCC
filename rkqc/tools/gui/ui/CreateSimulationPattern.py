# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'CreateSimulationPattern.ui'
#
# Created: Wed Feb 23 10:23:42 2011
#      by: PyQt4 UI code generator 4.8.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_CreateSimulationPattern(object):
    def setupUi(self, CreateSimulationPattern):
        CreateSimulationPattern.setObjectName(_fromUtf8("CreateSimulationPattern"))
        CreateSimulationPattern.resize(400, 300)
        self.verticalLayout = QtGui.QVBoxLayout(CreateSimulationPattern)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        spacerItem = QtGui.QSpacerItem(20, 123, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout.addItem(spacerItem)
        self.widget = QtGui.QWidget(CreateSimulationPattern)
        self.widget.setObjectName(_fromUtf8("widget"))
        self.horizontalLayout = QtGui.QHBoxLayout(self.widget)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        spacerItem1 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem1)
        self.open = QtGui.QPushButton(self.widget)
        self.open.setEnabled(False)
        self.open.setObjectName(_fromUtf8("open"))
        self.horizontalLayout.addWidget(self.open)
        spacerItem2 = QtGui.QSpacerItem(125, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem2)
        self.verticalLayout.addWidget(self.widget)
        spacerItem3 = QtGui.QSpacerItem(20, 123, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout.addItem(spacerItem3)

        self.retranslateUi(CreateSimulationPattern)
        QtCore.QMetaObject.connectSlotsByName(CreateSimulationPattern)

    def retranslateUi(self, CreateSimulationPattern):
        CreateSimulationPattern.setWindowTitle(QtGui.QApplication.translate("CreateSimulationPattern", "Form", None, QtGui.QApplication.UnicodeUTF8))
        self.open.setText(QtGui.QApplication.translate("CreateSimulationPattern", "Open Wave Form", None, QtGui.QApplication.UnicodeUTF8))

