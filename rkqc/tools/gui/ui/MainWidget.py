# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'MainWidget.ui'
#
# Created: Wed May 25 16:24:56 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_MainWidget(object):
    def setupUi(self, MainWidget):
        MainWidget.setObjectName(_fromUtf8("MainWidget"))
        MainWidget.resize(404, 573)
        MainWidget.setWindowTitle(_fromUtf8(""))
        self.verticalLayout_2 = QtGui.QVBoxLayout(MainWidget)
        self.verticalLayout_2.setMargin(0)
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.splitter_2 = QtGui.QSplitter(MainWidget)
        self.splitter_2.setOrientation(QtCore.Qt.Vertical)
        self.splitter_2.setObjectName(_fromUtf8("splitter_2"))
        self.splitter = QtGui.QSplitter(self.splitter_2)
        self.splitter.setOrientation(QtCore.Qt.Horizontal)
        self.splitter.setObjectName(_fromUtf8("splitter"))
        self.widget = QtGui.QWidget(self.splitter)
        self.widget.setObjectName(_fromUtf8("widget"))
        self.verticalLayout = QtGui.QVBoxLayout(self.widget)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.widget_2 = QtGui.QWidget(self.widget)
        self.widget_2.setObjectName(_fromUtf8("widget_2"))
        self.horizontalLayout = QtGui.QHBoxLayout(self.widget_2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.filterLine = QtGui.QLineEdit(self.widget_2)
        self.filterLine.setObjectName(_fromUtf8("filterLine"))
        self.horizontalLayout.addWidget(self.filterLine)
        self.helpItems = QtGui.QToolButton(self.widget_2)
        self.helpItems.setText(_fromUtf8(""))
        self.helpItems.setAutoRaise(True)
        self.helpItems.setObjectName(_fromUtf8("helpItems"))
        self.horizontalLayout.addWidget(self.helpItems)
        self.verticalLayout.addWidget(self.widget_2)
        self.toolBox = FlowItemTreeWidget(self.widget)
        self.toolBox.setObjectName(_fromUtf8("toolBox"))
        self.verticalLayout.addWidget(self.toolBox)
        self.tabWidget = FlowContainer(self.splitter)
        self.tabWidget.setObjectName(_fromUtf8("tabWidget"))
        self.buttonBar = ButtonBar(self.splitter_2)
        self.buttonBar.setObjectName(_fromUtf8("buttonBar"))
        self.verticalLayout_2.addWidget(self.splitter_2)

        self.retranslateUi(MainWidget)
        QtCore.QMetaObject.connectSlotsByName(MainWidget)

    def retranslateUi(self, MainWidget):
        pass

from core.ButtonBar import ButtonBar
from core.FlowItemTreeWidget import FlowItemTreeWidget
from core.FlowContainer import FlowContainer
