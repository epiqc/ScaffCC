# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ComparatorLine.ui'
#
# Created: Thu May 12 11:01:59 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_ComparatorLine(object):
    def setupUi(self, ComparatorLine):
        ComparatorLine.setObjectName(_fromUtf8("ComparatorLine"))
        ComparatorLine.resize(288, 25)
        ComparatorLine.setWindowTitle(_fromUtf8(""))
        self.horizontalLayout = QtGui.QHBoxLayout(ComparatorLine)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.label = QtGui.QLabel(ComparatorLine)
        self.label.setMinimumSize(QtCore.QSize(41, 0))
        self.label.setObjectName(_fromUtf8("label"))
        self.horizontalLayout.addWidget(self.label)
        self.compare = QtGui.QComboBox(ComparatorLine)
        self.compare.setMinimumSize(QtCore.QSize(78, 0))
        self.compare.setMaximumSize(QtCore.QSize(78, 16777215))
        self.compare.setObjectName(_fromUtf8("compare"))
        self.compare.addItem(_fromUtf8(""))
        self.compare.addItem(_fromUtf8(""))
        self.horizontalLayout.addWidget(self.compare)
        self.what = QtGui.QComboBox(ComparatorLine)
        self.what.setObjectName(_fromUtf8("what"))
        self.what.addItem(_fromUtf8(""))
        self.what.addItem(_fromUtf8(""))
        self.what.addItem(_fromUtf8(""))
        self.what.addItem(_fromUtf8(""))
        self.horizontalLayout.addWidget(self.what)
        self.button = QtGui.QToolButton(ComparatorLine)
        self.button.setText(_fromUtf8(""))
        self.button.setObjectName(_fromUtf8("button"))
        self.horizontalLayout.addWidget(self.button)

        self.retranslateUi(ComparatorLine)
        QtCore.QMetaObject.connectSlotsByName(ComparatorLine)

    def retranslateUi(self, ComparatorLine):
        self.label.setText(QtGui.QApplication.translate("ComparatorLine", "Has", None, QtGui.QApplication.UnicodeUTF8))
        self.compare.setItemText(0, QtGui.QApplication.translate("ComparatorLine", "Lowest", None, QtGui.QApplication.UnicodeUTF8))
        self.compare.setItemText(1, QtGui.QApplication.translate("ComparatorLine", "Highest", None, QtGui.QApplication.UnicodeUTF8))
        self.what.setItemText(0, QtGui.QApplication.translate("ComparatorLine", "Circuit Lines", None, QtGui.QApplication.UnicodeUTF8))
        self.what.setItemText(1, QtGui.QApplication.translate("ComparatorLine", "Gate Count", None, QtGui.QApplication.UnicodeUTF8))
        self.what.setItemText(2, QtGui.QApplication.translate("ComparatorLine", "Quantum Costs", None, QtGui.QApplication.UnicodeUTF8))
        self.what.setItemText(3, QtGui.QApplication.translate("ComparatorLine", "Transistor Costs", None, QtGui.QApplication.UnicodeUTF8))

