# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'Embedding.ui'
#
# Created: Mon May  9 08:26:25 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_Embedding(object):
    def setupUi(self, Embedding):
        Embedding.setObjectName(_fromUtf8("Embedding"))
        Embedding.resize(200, 55)
        Embedding.setWindowTitle(_fromUtf8(""))
        self.formLayout = QtGui.QFormLayout(Embedding)
        self.formLayout.setObjectName(_fromUtf8("formLayout"))
        self.label = QtGui.QLabel(Embedding)
        self.label.setObjectName(_fromUtf8("label"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.LabelRole, self.label)
        self.garbage_name = QtGui.QLineEdit(Embedding)
        self.garbage_name.setObjectName(_fromUtf8("garbage_name"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.FieldRole, self.garbage_name)
        self.label_2 = QtGui.QLabel(Embedding)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.LabelRole, self.label_2)
        self.runtime = QtGui.QLabel(Embedding)
        self.runtime.setText(_fromUtf8(""))
        self.runtime.setObjectName(_fromUtf8("runtime"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.FieldRole, self.runtime)

        self.retranslateUi(Embedding)
        QtCore.QMetaObject.connectSlotsByName(Embedding)

    def retranslateUi(self, Embedding):
        self.label.setText(QtGui.QApplication.translate("Embedding", "Garbage Name:", None, QtGui.QApplication.UnicodeUTF8))
        self.garbage_name.setText(QtGui.QApplication.translate("Embedding", "-", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("Embedding", "Run-time:", None, QtGui.QApplication.UnicodeUTF8))

