# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ESOPSynthesis.ui'
#
# Created: Wed May 11 17:29:16 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_ESOPSynthesis(object):
    def setupUi(self, ESOPSynthesis):
        ESOPSynthesis.setObjectName(_fromUtf8("ESOPSynthesis"))
        ESOPSynthesis.resize(257, 167)
        ESOPSynthesis.setWindowTitle(_fromUtf8(""))
        self.formLayout = QtGui.QFormLayout(ESOPSynthesis)
        self.formLayout.setObjectName(_fromUtf8("formLayout"))
        self.separate_polarities = QtGui.QCheckBox(ESOPSynthesis)
        self.separate_polarities.setObjectName(_fromUtf8("separate_polarities"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.FieldRole, self.separate_polarities)
        self.label = QtGui.QLabel(ESOPSynthesis)
        self.label.setObjectName(_fromUtf8("label"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.LabelRole, self.label)
        self.reordering = QtGui.QComboBox(ESOPSynthesis)
        self.reordering.setObjectName(_fromUtf8("reordering"))
        self.reordering.addItem(_fromUtf8(""))
        self.formLayout.setWidget(1, QtGui.QFormLayout.FieldRole, self.reordering)
        self.label_2 = QtGui.QLabel(ESOPSynthesis)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.formLayout.setWidget(2, QtGui.QFormLayout.LabelRole, self.label_2)
        self.alpha = QtGui.QDoubleSpinBox(ESOPSynthesis)
        self.alpha.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.alpha.setDecimals(1)
        self.alpha.setSingleStep(0.1)
        self.alpha.setProperty(_fromUtf8("value"), 0.5)
        self.alpha.setObjectName(_fromUtf8("alpha"))
        self.formLayout.setWidget(2, QtGui.QFormLayout.FieldRole, self.alpha)
        self.label_3 = QtGui.QLabel(ESOPSynthesis)
        self.label_3.setObjectName(_fromUtf8("label_3"))
        self.formLayout.setWidget(3, QtGui.QFormLayout.LabelRole, self.label_3)
        self.beta = QtGui.QDoubleSpinBox(ESOPSynthesis)
        self.beta.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.beta.setDecimals(1)
        self.beta.setSingleStep(0.1)
        self.beta.setProperty(_fromUtf8("value"), 0.5)
        self.beta.setObjectName(_fromUtf8("beta"))
        self.formLayout.setWidget(3, QtGui.QFormLayout.FieldRole, self.beta)
        self.label_4 = QtGui.QLabel(ESOPSynthesis)
        self.label_4.setObjectName(_fromUtf8("label_4"))
        self.formLayout.setWidget(4, QtGui.QFormLayout.LabelRole, self.label_4)
        self.garbage_name = QtGui.QLineEdit(ESOPSynthesis)
        self.garbage_name.setObjectName(_fromUtf8("garbage_name"))
        self.formLayout.setWidget(4, QtGui.QFormLayout.FieldRole, self.garbage_name)
        self.label_5 = QtGui.QLabel(ESOPSynthesis)
        self.label_5.setObjectName(_fromUtf8("label_5"))
        self.formLayout.setWidget(5, QtGui.QFormLayout.LabelRole, self.label_5)
        self.runtime = QtGui.QLabel(ESOPSynthesis)
        self.runtime.setText(_fromUtf8(""))
        self.runtime.setObjectName(_fromUtf8("runtime"))
        self.formLayout.setWidget(5, QtGui.QFormLayout.FieldRole, self.runtime)
        self.label.setBuddy(self.reordering)
        self.label_2.setBuddy(self.alpha)
        self.label_3.setBuddy(self.beta)
        self.label_4.setBuddy(self.garbage_name)

        self.retranslateUi(ESOPSynthesis)
        QtCore.QMetaObject.connectSlotsByName(ESOPSynthesis)

    def retranslateUi(self, ESOPSynthesis):
        self.separate_polarities.setText(QtGui.QApplication.translate("ESOPSynthesis", "Separate Polarities", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("ESOPSynthesis", "Reordering:", None, QtGui.QApplication.UnicodeUTF8))
        self.reordering.setItemText(0, QtGui.QApplication.translate("ESOPSynthesis", "Weighted Reordering", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("ESOPSynthesis", "Alpha:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("ESOPSynthesis", "Beta:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("ESOPSynthesis", "Garbage Name:", None, QtGui.QApplication.UnicodeUTF8))
        self.garbage_name.setText(QtGui.QApplication.translate("ESOPSynthesis", "-", None, QtGui.QApplication.UnicodeUTF8))
        self.label_5.setText(QtGui.QApplication.translate("ESOPSynthesis", "Run-time:", None, QtGui.QApplication.UnicodeUTF8))

