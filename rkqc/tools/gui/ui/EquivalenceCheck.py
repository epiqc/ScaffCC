# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'EquivalenceCheck.ui'
#
# Created: Thu May 12 10:07:44 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_EquivalenceCheck(object):
    def setupUi(self, EquivalenceCheck):
        EquivalenceCheck.setObjectName(_fromUtf8("EquivalenceCheck"))
        EquivalenceCheck.resize(400, 300)
        EquivalenceCheck.setWindowTitle(_fromUtf8(""))
        self.verticalLayout = QtGui.QVBoxLayout(EquivalenceCheck)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.tableWidget = QtGui.QTableWidget(EquivalenceCheck)
        self.tableWidget.setObjectName(_fromUtf8("tableWidget"))
        self.tableWidget.setColumnCount(3)
        self.tableWidget.setRowCount(0)
        item = QtGui.QTableWidgetItem()
        self.tableWidget.setHorizontalHeaderItem(0, item)
        item = QtGui.QTableWidgetItem()
        self.tableWidget.setHorizontalHeaderItem(1, item)
        item = QtGui.QTableWidgetItem()
        self.tableWidget.setHorizontalHeaderItem(2, item)
        self.verticalLayout.addWidget(self.tableWidget)

        self.retranslateUi(EquivalenceCheck)
        QtCore.QMetaObject.connectSlotsByName(EquivalenceCheck)

    def retranslateUi(self, EquivalenceCheck):
        self.tableWidget.horizontalHeaderItem(0).setText(QtGui.QApplication.translate("EquivalenceCheck", "Circuit 1", None, QtGui.QApplication.UnicodeUTF8))
        self.tableWidget.horizontalHeaderItem(1).setText(QtGui.QApplication.translate("EquivalenceCheck", "Circuit 2", None, QtGui.QApplication.UnicodeUTF8))
        self.tableWidget.horizontalHeaderItem(2).setText(QtGui.QApplication.translate("EquivalenceCheck", "Equivalent?", None, QtGui.QApplication.UnicodeUTF8))

