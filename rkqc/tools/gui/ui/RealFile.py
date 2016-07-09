# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'RealFile.ui'
#
# Created: Wed Jun  8 10:18:16 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_RealFile(object):
    def setupUi(self, RealFile):
        RealFile.setObjectName(_fromUtf8("RealFile"))
        RealFile.resize(400, 300)
        RealFile.setWindowTitle(_fromUtf8(""))
        self.verticalLayout = QtGui.QVBoxLayout(RealFile)
        self.verticalLayout.setContentsMargins(0, -1, 0, 0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.filename = RevLibEditor(RealFile)
        self.filename.setReadOnly(True)
        self.filename.setObjectName(_fromUtf8("filename"))
        self.verticalLayout.addWidget(self.filename)

        self.retranslateUi(RealFile)
        QtCore.QMetaObject.connectSlotsByName(RealFile)

    def retranslateUi(self, RealFile):
        pass

from revkitui import RevLibEditor
