# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ExactSynthesis.ui'
#
# Created: Wed May 11 10:16:15 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_ExactSynthesis(object):
    def setupUi(self, ExactSynthesis):
        ExactSynthesis.setObjectName(_fromUtf8("ExactSynthesis"))
        ExactSynthesis.resize(221, 75)
        ExactSynthesis.setWindowTitle(_fromUtf8(""))
        self.formLayout = QtGui.QFormLayout(ExactSynthesis)
        self.formLayout.setObjectName(_fromUtf8("formLayout"))
        self.spec_incremental = QtGui.QCheckBox(ExactSynthesis)
        self.spec_incremental.setObjectName(_fromUtf8("spec_incremental"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.FieldRole, self.spec_incremental)
        self.label = QtGui.QLabel(ExactSynthesis)
        self.label.setObjectName(_fromUtf8("label"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.LabelRole, self.label)
        self.max_depth = QtGui.QSpinBox(ExactSynthesis)
        self.max_depth.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.max_depth.setMaximum(1000)
        self.max_depth.setProperty(_fromUtf8("value"), 20)
        self.max_depth.setObjectName(_fromUtf8("max_depth"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.FieldRole, self.max_depth)
        self.label_2 = QtGui.QLabel(ExactSynthesis)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.formLayout.setWidget(2, QtGui.QFormLayout.LabelRole, self.label_2)
        self.runtime = QtGui.QLabel(ExactSynthesis)
        self.runtime.setText(_fromUtf8(""))
        self.runtime.setObjectName(_fromUtf8("runtime"))
        self.formLayout.setWidget(2, QtGui.QFormLayout.FieldRole, self.runtime)
        self.label.setBuddy(self.max_depth)

        self.retranslateUi(ExactSynthesis)
        QtCore.QMetaObject.connectSlotsByName(ExactSynthesis)

    def retranslateUi(self, ExactSynthesis):
        self.spec_incremental.setText(QtGui.QApplication.translate("ExactSynthesis", "Incremental Encoding", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("ExactSynthesis", "Max Gates:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("ExactSynthesis", "Run-time:", None, QtGui.QApplication.UnicodeUTF8))

