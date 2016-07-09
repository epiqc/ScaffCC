# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'TransformationBasedSynthesis.ui'
#
# Created: Fri Jun 24 14:06:08 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_TransformationBasedSynthesis(object):
    def setupUi(self, TransformationBasedSynthesis):
        TransformationBasedSynthesis.setObjectName(_fromUtf8("TransformationBasedSynthesis"))
        TransformationBasedSynthesis.resize(282, 132)
        TransformationBasedSynthesis.setWindowTitle(_fromUtf8(""))
        self.formLayout = QtGui.QFormLayout(TransformationBasedSynthesis)
        self.formLayout.setObjectName(_fromUtf8("formLayout"))
        self.bidi_synthesis = QtGui.QCheckBox(TransformationBasedSynthesis)
        self.bidi_synthesis.setChecked(True)
        self.bidi_synthesis.setObjectName(_fromUtf8("bidi_synthesis"))
        self.formLayout.setWidget(3, QtGui.QFormLayout.FieldRole, self.bidi_synthesis)
        self.label = QtGui.QLabel(TransformationBasedSynthesis)
        self.label.setObjectName(_fromUtf8("label"))
        self.formLayout.setWidget(4, QtGui.QFormLayout.LabelRole, self.label)
        self.runtime = QtGui.QLabel(TransformationBasedSynthesis)
        self.runtime.setText(_fromUtf8(""))
        self.runtime.setObjectName(_fromUtf8("runtime"))
        self.formLayout.setWidget(4, QtGui.QFormLayout.FieldRole, self.runtime)
        self.swop = QtGui.QComboBox(TransformationBasedSynthesis)
        self.swop.setObjectName(_fromUtf8("swop"))
        self.swop.addItem(_fromUtf8(""))
        self.swop.addItem(_fromUtf8(""))
        self.swop.addItem(_fromUtf8(""))
        self.formLayout.setWidget(1, QtGui.QFormLayout.FieldRole, self.swop)
        self.label_2 = QtGui.QLabel(TransformationBasedSynthesis)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.LabelRole, self.label_2)
        self.cost_function = QtGui.QComboBox(TransformationBasedSynthesis)
        self.cost_function.setEnabled(False)
        self.cost_function.setObjectName(_fromUtf8("cost_function"))
        self.cost_function.addItem(_fromUtf8(""))
        self.cost_function.addItem(_fromUtf8(""))
        self.cost_function.addItem(_fromUtf8(""))
        self.formLayout.setWidget(2, QtGui.QFormLayout.FieldRole, self.cost_function)
        self.cost_function_label = QtGui.QLabel(TransformationBasedSynthesis)
        self.cost_function_label.setEnabled(False)
        self.cost_function_label.setObjectName(_fromUtf8("cost_function_label"))
        self.formLayout.setWidget(2, QtGui.QFormLayout.LabelRole, self.cost_function_label)
        self.label_3 = QtGui.QLabel(TransformationBasedSynthesis)
        self.label_3.setObjectName(_fromUtf8("label_3"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.LabelRole, self.label_3)
        self.variant = QtGui.QComboBox(TransformationBasedSynthesis)
        self.variant.setObjectName(_fromUtf8("variant"))
        self.variant.addItem(_fromUtf8(""))
        self.variant.addItem(_fromUtf8(""))
        self.formLayout.setWidget(0, QtGui.QFormLayout.FieldRole, self.variant)
        self.label_2.setBuddy(self.swop)

        self.retranslateUi(TransformationBasedSynthesis)
        QtCore.QMetaObject.connectSlotsByName(TransformationBasedSynthesis)
        TransformationBasedSynthesis.setTabOrder(self.swop, self.bidi_synthesis)

    def retranslateUi(self, TransformationBasedSynthesis):
        self.bidi_synthesis.setText(QtGui.QApplication.translate("TransformationBasedSynthesis", "Bidirectional", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("TransformationBasedSynthesis", "Run-time:", None, QtGui.QApplication.UnicodeUTF8))
        self.swop.setItemText(0, QtGui.QApplication.translate("TransformationBasedSynthesis", "None", None, QtGui.QApplication.UnicodeUTF8))
        self.swop.setItemText(1, QtGui.QApplication.translate("TransformationBasedSynthesis", "Exhaustive", None, QtGui.QApplication.UnicodeUTF8))
        self.swop.setItemText(2, QtGui.QApplication.translate("TransformationBasedSynthesis", "Heuristic", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("TransformationBasedSynthesis", "Output Permutation:", None, QtGui.QApplication.UnicodeUTF8))
        self.cost_function.setItemText(0, QtGui.QApplication.translate("TransformationBasedSynthesis", "Gates", None, QtGui.QApplication.UnicodeUTF8))
        self.cost_function.setItemText(1, QtGui.QApplication.translate("TransformationBasedSynthesis", "Quantum Costs", None, QtGui.QApplication.UnicodeUTF8))
        self.cost_function.setItemText(2, QtGui.QApplication.translate("TransformationBasedSynthesis", "Transistor Costs", None, QtGui.QApplication.UnicodeUTF8))
        self.cost_function_label.setText(QtGui.QApplication.translate("TransformationBasedSynthesis", "Cost Function:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("TransformationBasedSynthesis", "Variant:", None, QtGui.QApplication.UnicodeUTF8))
        self.variant.setItemText(0, QtGui.QApplication.translate("TransformationBasedSynthesis", "Truth Table", None, QtGui.QApplication.UnicodeUTF8))
        self.variant.setItemText(1, QtGui.QApplication.translate("TransformationBasedSynthesis", "Reed Muller Spectra", None, QtGui.QApplication.UnicodeUTF8))

