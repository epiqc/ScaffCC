# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ResultsTableDialog.ui'
#
# Created: Thu May 19 16:32:25 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_ResultsTableDialog(object):
    def setupUi(self, ResultsTableDialog):
        ResultsTableDialog.setObjectName(_fromUtf8("ResultsTableDialog"))
        ResultsTableDialog.resize(518, 501)
        self.verticalLayout_2 = QtGui.QVBoxLayout(ResultsTableDialog)
        self.verticalLayout_2.setMargin(0)
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.widget_2 = QtGui.QWidget(ResultsTableDialog)
        self.widget_2.setObjectName(_fromUtf8("widget_2"))
        self.horizontalLayout = QtGui.QHBoxLayout(self.widget_2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.tableWidget = QtGui.QTableWidget(self.widget_2)
        self.tableWidget.setObjectName(_fromUtf8("tableWidget"))
        self.tableWidget.setColumnCount(2)
        self.tableWidget.setRowCount(0)
        item = QtGui.QTableWidgetItem()
        self.tableWidget.setHorizontalHeaderItem(0, item)
        item = QtGui.QTableWidgetItem()
        self.tableWidget.setHorizontalHeaderItem(1, item)
        self.horizontalLayout.addWidget(self.tableWidget)
        self.widget = QtGui.QWidget(self.widget_2)
        self.widget.setMaximumSize(QtCore.QSize(120, 16777215))
        self.widget.setObjectName(_fromUtf8("widget"))
        self.verticalLayout = QtGui.QVBoxLayout(self.widget)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.add = QtGui.QPushButton(self.widget)
        self.add.setObjectName(_fromUtf8("add"))
        self.verticalLayout.addWidget(self.add)
        self.remove = QtGui.QPushButton(self.widget)
        self.remove.setEnabled(False)
        self.remove.setObjectName(_fromUtf8("remove"))
        self.verticalLayout.addWidget(self.remove)
        spacerItem = QtGui.QSpacerItem(20, 282, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout.addItem(spacerItem)
        self.horizontalLayout.addWidget(self.widget)
        self.verticalLayout_2.addWidget(self.widget_2)
        self.buttonBox = QtGui.QDialogButtonBox(ResultsTableDialog)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Close|QtGui.QDialogButtonBox.Help)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.verticalLayout_2.addWidget(self.buttonBox)

        self.retranslateUi(ResultsTableDialog)
        QtCore.QMetaObject.connectSlotsByName(ResultsTableDialog)
        ResultsTableDialog.setTabOrder(self.add, self.remove)
        ResultsTableDialog.setTabOrder(self.remove, self.tableWidget)
        ResultsTableDialog.setTabOrder(self.tableWidget, self.buttonBox)

    def retranslateUi(self, ResultsTableDialog):
        ResultsTableDialog.setWindowTitle(QtGui.QApplication.translate("ResultsTableDialog", "Configure Extra Columns", None, QtGui.QApplication.UnicodeUTF8))
        self.tableWidget.setWhatsThis(QtGui.QApplication.translate("ResultsTableDialog", "The expressions are JavaScript expressions. Additionally, the values from the table columns can be fetched with the following <i>special variables</i>:\n"
"\n"
"<table border=\"0\" cellpadding=\"3\">\n"
"  <tr><td><b>LINES[x]</b></td><td>Lines from table <i>x</i></td></tr>\n"
"  <tr><td><b>GATES[x]</b></td><td>Gates from table <i>x</i></td></tr>\n"
"  <tr><td><b>QC[x]</b></td><td>Quantum costs from table <i>x</i></td></tr>\n"
"  <tr><td><b>TC[x]</b></td><td>Transistor costs from table <i>x</i></td></tr>\n"
"</table>\n"
"\n"
"The index variable always starts from 0. When processing the table the respective value from the current row is inserted.\n"
"", None, QtGui.QApplication.UnicodeUTF8))
        self.tableWidget.horizontalHeaderItem(0).setText(QtGui.QApplication.translate("ResultsTableDialog", "Column", None, QtGui.QApplication.UnicodeUTF8))
        self.tableWidget.horizontalHeaderItem(1).setText(QtGui.QApplication.translate("ResultsTableDialog", "Expression", None, QtGui.QApplication.UnicodeUTF8))
        self.add.setText(QtGui.QApplication.translate("ResultsTableDialog", "Add", None, QtGui.QApplication.UnicodeUTF8))
        self.remove.setText(QtGui.QApplication.translate("ResultsTableDialog", "Remove", None, QtGui.QApplication.UnicodeUTF8))

