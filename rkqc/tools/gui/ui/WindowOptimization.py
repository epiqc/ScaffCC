# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'WindowOptimization.ui'
#
# Created: Sat May  7 14:41:23 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_WindowOptimization(object):
    def setupUi(self, WindowOptimization):
        WindowOptimization.setObjectName(_fromUtf8("WindowOptimization"))
        WindowOptimization.resize(335, 147)
        WindowOptimization.setWindowTitle(_fromUtf8(""))
        self.formLayout = QtGui.QFormLayout(WindowOptimization)
        self.formLayout.setObjectName(_fromUtf8("formLayout"))
        self.label = QtGui.QLabel(WindowOptimization)
        self.label.setObjectName(_fromUtf8("label"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.LabelRole, self.label)
        self.window_selection = QtGui.QComboBox(WindowOptimization)
        self.window_selection.setObjectName(_fromUtf8("window_selection"))
        self.window_selection.addItem(_fromUtf8(""))
        self.window_selection.addItem(_fromUtf8(""))
        self.formLayout.setWidget(0, QtGui.QFormLayout.FieldRole, self.window_selection)
        self.window_length_label = QtGui.QLabel(WindowOptimization)
        self.window_length_label.setObjectName(_fromUtf8("window_length_label"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.LabelRole, self.window_length_label)
        self.window_length = QtGui.QSpinBox(WindowOptimization)
        self.window_length.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.window_length.setProperty(_fromUtf8("value"), 10)
        self.window_length.setObjectName(_fromUtf8("window_length"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.FieldRole, self.window_length)
        self.offset_label = QtGui.QLabel(WindowOptimization)
        self.offset_label.setObjectName(_fromUtf8("offset_label"))
        self.formLayout.setWidget(2, QtGui.QFormLayout.LabelRole, self.offset_label)
        self.offset = QtGui.QSpinBox(WindowOptimization)
        self.offset.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.offset.setProperty(_fromUtf8("value"), 1)
        self.offset.setObjectName(_fromUtf8("offset"))
        self.formLayout.setWidget(2, QtGui.QFormLayout.FieldRole, self.offset)
        self.label_4 = QtGui.QLabel(WindowOptimization)
        self.label_4.setObjectName(_fromUtf8("label_4"))
        self.formLayout.setWidget(3, QtGui.QFormLayout.LabelRole, self.label_4)
        self.synthesis_method = QtGui.QComboBox(WindowOptimization)
        self.synthesis_method.setObjectName(_fromUtf8("synthesis_method"))
        self.synthesis_method.addItem(_fromUtf8(""))
        self.synthesis_method.addItem(_fromUtf8(""))
        self.formLayout.setWidget(3, QtGui.QFormLayout.FieldRole, self.synthesis_method)
        self.label_5 = QtGui.QLabel(WindowOptimization)
        self.label_5.setObjectName(_fromUtf8("label_5"))
        self.formLayout.setWidget(4, QtGui.QFormLayout.LabelRole, self.label_5)
        self.costs_function = QtGui.QComboBox(WindowOptimization)
        self.costs_function.setObjectName(_fromUtf8("costs_function"))
        self.costs_function.addItem(_fromUtf8(""))
        self.costs_function.addItem(_fromUtf8(""))
        self.costs_function.addItem(_fromUtf8(""))
        self.formLayout.setWidget(4, QtGui.QFormLayout.FieldRole, self.costs_function)
        self.label_2 = QtGui.QLabel(WindowOptimization)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.formLayout.setWidget(5, QtGui.QFormLayout.LabelRole, self.label_2)
        self.runtime = QtGui.QLabel(WindowOptimization)
        self.runtime.setText(_fromUtf8(""))
        self.runtime.setObjectName(_fromUtf8("runtime"))
        self.formLayout.setWidget(5, QtGui.QFormLayout.FieldRole, self.runtime)
        self.label.setBuddy(self.window_selection)
        self.window_length_label.setBuddy(self.window_length)
        self.offset_label.setBuddy(self.offset)
        self.label_4.setBuddy(self.synthesis_method)
        self.label_5.setBuddy(self.costs_function)

        self.retranslateUi(WindowOptimization)
        QtCore.QMetaObject.connectSlotsByName(WindowOptimization)

    def retranslateUi(self, WindowOptimization):
        self.label.setText(QtGui.QApplication.translate("WindowOptimization", "Window Selection:", None, QtGui.QApplication.UnicodeUTF8))
        self.window_selection.setItemText(0, QtGui.QApplication.translate("WindowOptimization", "Shift Window Optimization", None, QtGui.QApplication.UnicodeUTF8))
        self.window_selection.setItemText(1, QtGui.QApplication.translate("WindowOptimization", "Line Window Optimization", None, QtGui.QApplication.UnicodeUTF8))
        self.window_length_label.setText(QtGui.QApplication.translate("WindowOptimization", "Window Length:", None, QtGui.QApplication.UnicodeUTF8))
        self.offset_label.setText(QtGui.QApplication.translate("WindowOptimization", "Offset:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("WindowOptimization", "Synthesis Method:", None, QtGui.QApplication.UnicodeUTF8))
        self.synthesis_method.setItemText(0, QtGui.QApplication.translate("WindowOptimization", "Transformation Based Synthesis", None, QtGui.QApplication.UnicodeUTF8))
        self.synthesis_method.setItemText(1, QtGui.QApplication.translate("WindowOptimization", "Exact Synthesis", None, QtGui.QApplication.UnicodeUTF8))
        self.label_5.setText(QtGui.QApplication.translate("WindowOptimization", "Cost Function:", None, QtGui.QApplication.UnicodeUTF8))
        self.costs_function.setItemText(0, QtGui.QApplication.translate("WindowOptimization", "Gates", None, QtGui.QApplication.UnicodeUTF8))
        self.costs_function.setItemText(1, QtGui.QApplication.translate("WindowOptimization", "Quantum Costs", None, QtGui.QApplication.UnicodeUTF8))
        self.costs_function.setItemText(2, QtGui.QApplication.translate("WindowOptimization", "Transistor Costs", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("WindowOptimization", "Run-time:", None, QtGui.QApplication.UnicodeUTF8))

