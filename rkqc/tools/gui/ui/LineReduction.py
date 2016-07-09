# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'LineReduction.ui'
#
# Created: Wed May 18 09:58:38 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_LineReduction(object):
    def setupUi(self, LineReduction):
        LineReduction.setObjectName(_fromUtf8("LineReduction"))
        LineReduction.resize(227, 90)
        LineReduction.setWindowTitle(_fromUtf8(""))
        self.formLayout = QtGui.QFormLayout(LineReduction)
        self.formLayout.setObjectName(_fromUtf8("formLayout"))
        self.label = QtGui.QLabel(LineReduction)
        self.label.setObjectName(_fromUtf8("label"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.LabelRole, self.label)
        self.max_window_lines = QtGui.QSpinBox(LineReduction)
        self.max_window_lines.setMinimumSize(QtCore.QSize(50, 0))
        self.max_window_lines.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.max_window_lines.setProperty(_fromUtf8("value"), 6)
        self.max_window_lines.setObjectName(_fromUtf8("max_window_lines"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.FieldRole, self.max_window_lines)
        self.label_2 = QtGui.QLabel(LineReduction)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.LabelRole, self.label_2)
        self.max_grow_up_window_lines = QtGui.QSpinBox(LineReduction)
        self.max_grow_up_window_lines.setMinimumSize(QtCore.QSize(50, 0))
        self.max_grow_up_window_lines.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.max_grow_up_window_lines.setProperty(_fromUtf8("value"), 9)
        self.max_grow_up_window_lines.setObjectName(_fromUtf8("max_grow_up_window_lines"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.FieldRole, self.max_grow_up_window_lines)
        self.label_3 = QtGui.QLabel(LineReduction)
        self.label_3.setObjectName(_fromUtf8("label_3"))
        self.formLayout.setWidget(2, QtGui.QFormLayout.LabelRole, self.label_3)
        self.window_variables_threshold = QtGui.QSpinBox(LineReduction)
        self.window_variables_threshold.setMinimumSize(QtCore.QSize(50, 0))
        self.window_variables_threshold.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.window_variables_threshold.setProperty(_fromUtf8("value"), 17)
        self.window_variables_threshold.setObjectName(_fromUtf8("window_variables_threshold"))
        self.formLayout.setWidget(2, QtGui.QFormLayout.FieldRole, self.window_variables_threshold)
        self.label_4 = QtGui.QLabel(LineReduction)
        self.label_4.setObjectName(_fromUtf8("label_4"))
        self.formLayout.setWidget(3, QtGui.QFormLayout.LabelRole, self.label_4)
        self.runtime = QtGui.QLabel(LineReduction)
        self.runtime.setText(_fromUtf8(""))
        self.runtime.setObjectName(_fromUtf8("runtime"))
        self.formLayout.setWidget(3, QtGui.QFormLayout.FieldRole, self.runtime)
        self.label.setBuddy(self.max_window_lines)
        self.label_2.setBuddy(self.max_grow_up_window_lines)
        self.label_3.setBuddy(self.window_variables_threshold)

        self.retranslateUi(LineReduction)
        QtCore.QMetaObject.connectSlotsByName(LineReduction)

    def retranslateUi(self, LineReduction):
        self.label.setToolTip(QtGui.QApplication.translate("LineReduction", "Number of lines the selected windows can have initially.", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("LineReduction", "Maximal Window Lines:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setToolTip(QtGui.QApplication.translate("LineReduction", "When the truth table is not reversible, obtained by a window with maximal window lines, then the number of lines can be increased up at most this value.", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("LineReduction", "Maximal Grow Up:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setToolTip(QtGui.QApplication.translate("LineReduction", "The possible window inputs are obtained by simulating its cone of influence. It is only simulated if the number of its primary inputs is less or equal to this value.", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("LineReduction", "Window Variables Threshold:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("LineReduction", "Run-time:", None, QtGui.QApplication.UnicodeUTF8))

