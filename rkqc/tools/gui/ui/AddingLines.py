# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'AddingLines.ui'
#
# Created: Wed May  4 11:00:30 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_AddingLines(object):
    def setupUi(self, AddingLines):
        AddingLines.setObjectName(_fromUtf8("AddingLines"))
        AddingLines.resize(400, 300)
        AddingLines.setWindowTitle(_fromUtf8(""))
        self.horizontalLayout = QtGui.QHBoxLayout(AddingLines)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        spacerItem = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.widget = QtGui.QWidget(AddingLines)
        self.widget.setObjectName(_fromUtf8("widget"))
        self.formLayout = QtGui.QFormLayout(self.widget)
        self.formLayout.setMargin(0)
        self.formLayout.setObjectName(_fromUtf8("formLayout"))
        self.label = QtGui.QLabel(self.widget)
        self.label.setObjectName(_fromUtf8("label"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.LabelRole, self.label)
        self.additional_lines = QtGui.QSpinBox(self.widget)
        self.additional_lines.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.additional_lines.setSuffix(_fromUtf8(""))
        self.additional_lines.setMinimum(1)
        self.additional_lines.setMaximum(10)
        self.additional_lines.setObjectName(_fromUtf8("additional_lines"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.FieldRole, self.additional_lines)
        self.label_2 = QtGui.QLabel(self.widget)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.LabelRole, self.label_2)
        self.runtime = QtGui.QLabel(self.widget)
        self.runtime.setText(_fromUtf8(""))
        self.runtime.setObjectName(_fromUtf8("runtime"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.FieldRole, self.runtime)
        self.horizontalLayout.addWidget(self.widget)
        spacerItem1 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem1)

        self.retranslateUi(AddingLines)
        QtCore.QMetaObject.connectSlotsByName(AddingLines)

    def retranslateUi(self, AddingLines):
        self.label.setText(QtGui.QApplication.translate("AddingLines", "Additional Lines:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("AddingLines", "Run-time:", None, QtGui.QApplication.UnicodeUTF8))

