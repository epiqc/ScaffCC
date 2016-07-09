# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'PathBenchmarks.ui'
#
# Created: Thu Jun  9 13:03:04 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_PathBenchmarks(object):
    def setupUi(self, PathBenchmarks):
        PathBenchmarks.setObjectName(_fromUtf8("PathBenchmarks"))
        PathBenchmarks.resize(400, 300)
        PathBenchmarks.setWindowTitle(_fromUtf8(""))
        self.horizontalLayout_2 = QtGui.QHBoxLayout(PathBenchmarks)
        self.horizontalLayout_2.setObjectName(_fromUtf8("horizontalLayout_2"))
        spacerItem = QtGui.QSpacerItem(91, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem)
        self.widget = QtGui.QWidget(PathBenchmarks)
        self.widget.setObjectName(_fromUtf8("widget"))
        self.formLayout = QtGui.QFormLayout(self.widget)
        self.formLayout.setMargin(0)
        self.formLayout.setMargin(0)
        self.formLayout.setObjectName(_fromUtf8("formLayout"))
        self.label = QtGui.QLabel(self.widget)
        self.label.setObjectName(_fromUtf8("label"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.LabelRole, self.label)
        self.label_2 = QtGui.QLabel(self.widget)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.LabelRole, self.label_2)
        self.filetype = QtGui.QComboBox(self.widget)
        self.filetype.setObjectName(_fromUtf8("filetype"))
        self.filetype.addItem(_fromUtf8(""))
        self.filetype.addItem(_fromUtf8(""))
        self.filetype.addItem(_fromUtf8(""))
        self.formLayout.setWidget(1, QtGui.QFormLayout.FieldRole, self.filetype)
        self.info = QtGui.QLabel(self.widget)
        self.info.setObjectName(_fromUtf8("info"))
        self.formLayout.setWidget(2, QtGui.QFormLayout.FieldRole, self.info)
        self.path = PathSelector(self.widget)
        self.path.setMinimumSize(QtCore.QSize(200, 0))
        self.path.setObjectName(_fromUtf8("path"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.FieldRole, self.path)
        self.horizontalLayout_2.addWidget(self.widget)
        spacerItem1 = QtGui.QSpacerItem(90, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem1)

        self.retranslateUi(PathBenchmarks)
        QtCore.QMetaObject.connectSlotsByName(PathBenchmarks)

    def retranslateUi(self, PathBenchmarks):
        self.label.setText(QtGui.QApplication.translate("PathBenchmarks", "Location:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("PathBenchmarks", "Type:", None, QtGui.QApplication.UnicodeUTF8))
        self.filetype.setItemText(0, QtGui.QApplication.translate("PathBenchmarks", "Functions", None, QtGui.QApplication.UnicodeUTF8))
        self.filetype.setItemText(1, QtGui.QApplication.translate("PathBenchmarks", "Circuits", None, QtGui.QApplication.UnicodeUTF8))
        self.filetype.setItemText(2, QtGui.QApplication.translate("PathBenchmarks", "Truth Tables", None, QtGui.QApplication.UnicodeUTF8))
        self.info.setText(QtGui.QApplication.translate("PathBenchmarks", "Please choose a location", None, QtGui.QApplication.UnicodeUTF8))

from core.PathSelector import PathSelector
