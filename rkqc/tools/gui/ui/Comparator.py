# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'Comparator.ui'
#
# Created: Thu May 19 15:49:21 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_Comparator(object):
    def setupUi(self, Comparator):
        Comparator.setObjectName(_fromUtf8("Comparator"))
        Comparator.resize(400, 300)
        Comparator.setWindowTitle(_fromUtf8(""))
        self.verticalLayout_2 = QtGui.QVBoxLayout(Comparator)
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.widget = QtGui.QWidget(Comparator)
        self.widget.setObjectName(_fromUtf8("widget"))
        self.horizontalLayout = QtGui.QHBoxLayout(self.widget)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.add_row = QtGui.QToolButton(self.widget)
        self.add_row.setToolButtonStyle(QtCore.Qt.ToolButtonTextBesideIcon)
        self.add_row.setObjectName(_fromUtf8("add_row"))
        self.horizontalLayout.addWidget(self.add_row)
        spacerItem = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.label = QtGui.QLabel(self.widget)
        self.label.setObjectName(_fromUtf8("label"))
        self.horizontalLayout.addWidget(self.label)
        self.add_input = QtGui.QToolButton(self.widget)
        self.add_input.setObjectName(_fromUtf8("add_input"))
        self.horizontalLayout.addWidget(self.add_input)
        self.remove_input = QtGui.QToolButton(self.widget)
        self.remove_input.setEnabled(False)
        self.remove_input.setObjectName(_fromUtf8("remove_input"))
        self.horizontalLayout.addWidget(self.remove_input)
        self.verticalLayout_2.addWidget(self.widget)
        self.lines = QtGui.QWidget(Comparator)
        self.lines.setObjectName(_fromUtf8("lines"))
        self.verticalLayout = QtGui.QVBoxLayout(self.lines)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.first_line = ComparatorLine(self.lines)
        self.first_line.setObjectName(_fromUtf8("first_line"))
        self.verticalLayout.addWidget(self.first_line)
        spacerItem1 = QtGui.QSpacerItem(20, 247, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout.addItem(spacerItem1)
        self.verticalLayout_2.addWidget(self.lines)

        self.retranslateUi(Comparator)
        QtCore.QMetaObject.connectSlotsByName(Comparator)

    def retranslateUi(self, Comparator):
        self.add_row.setText(QtGui.QApplication.translate("Comparator", "Add Row", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("Comparator", "Inputs:", None, QtGui.QApplication.UnicodeUTF8))
        self.add_input.setToolTip(QtGui.QApplication.translate("Comparator", "Add Input", None, QtGui.QApplication.UnicodeUTF8))
        self.add_input.setText(QtGui.QApplication.translate("Comparator", "Add Input", None, QtGui.QApplication.UnicodeUTF8))
        self.remove_input.setToolTip(QtGui.QApplication.translate("Comparator", "Remove Input", None, QtGui.QApplication.UnicodeUTF8))
        self.remove_input.setText(QtGui.QApplication.translate("Comparator", "Remove Input", None, QtGui.QApplication.UnicodeUTF8))

from core.ComparatorLine import ComparatorLine
