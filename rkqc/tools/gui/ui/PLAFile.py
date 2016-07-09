# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'PLAFile.ui'
#
# Created: Fri Jun 10 09:11:23 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_PLAFile(object):
    def setupUi(self, PLAFile):
        PLAFile.setObjectName(_fromUtf8("PLAFile"))
        PLAFile.resize(549, 469)
        PLAFile.setWindowTitle(_fromUtf8(""))
        self.horizontalLayout = QtGui.QHBoxLayout(PLAFile)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.splitter = QtGui.QSplitter(PLAFile)
        self.splitter.setOrientation(QtCore.Qt.Horizontal)
        self.splitter.setObjectName(_fromUtf8("splitter"))
        self.filename = SpecificationTable(self.splitter)
        self.filename.setObjectName(_fromUtf8("filename"))
        self.filename.setColumnCount(0)
        self.filename.setRowCount(0)
        self.widget = QtGui.QWidget(self.splitter)
        self.widget.setObjectName(_fromUtf8("widget"))
        self.verticalLayout = QtGui.QVBoxLayout(self.widget)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.groupBox = QtGui.QGroupBox(self.widget)
        self.groupBox.setObjectName(_fromUtf8("groupBox"))
        self.formLayout = QtGui.QFormLayout(self.groupBox)
        self.formLayout.setObjectName(_fromUtf8("formLayout"))
        self.label = QtGui.QLabel(self.groupBox)
        self.label.setObjectName(_fromUtf8("label"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.LabelRole, self.label)
        self.numInputs = QtGui.QLabel(self.groupBox)
        self.numInputs.setText(_fromUtf8(""))
        self.numInputs.setObjectName(_fromUtf8("numInputs"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.FieldRole, self.numInputs)
        self.label_2 = QtGui.QLabel(self.groupBox)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.LabelRole, self.label_2)
        self.numOutputs = QtGui.QLabel(self.groupBox)
        self.numOutputs.setText(_fromUtf8(""))
        self.numOutputs.setObjectName(_fromUtf8("numOutputs"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.FieldRole, self.numOutputs)
        self.label_3 = QtGui.QLabel(self.groupBox)
        self.label_3.setObjectName(_fromUtf8("label_3"))
        self.formLayout.setWidget(2, QtGui.QFormLayout.LabelRole, self.label_3)
        self.numCubes = QtGui.QLabel(self.groupBox)
        self.numCubes.setText(_fromUtf8(""))
        self.numCubes.setObjectName(_fromUtf8("numCubes"))
        self.formLayout.setWidget(2, QtGui.QFormLayout.FieldRole, self.numCubes)
        self.verticalLayout.addWidget(self.groupBox)
        spacerItem = QtGui.QSpacerItem(20, 356, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout.addItem(spacerItem)
        self.horizontalLayout.addWidget(self.splitter)

        self.retranslateUi(PLAFile)
        QtCore.QMetaObject.connectSlotsByName(PLAFile)

    def retranslateUi(self, PLAFile):
        self.groupBox.setTitle(QtGui.QApplication.translate("PLAFile", "Information", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("PLAFile", "Number of Inputs:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("PLAFile", "Number of Outputs:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("PLAFile", "Number of Cubes:", None, QtGui.QApplication.UnicodeUTF8))

from core.SpecificationTable import SpecificationTable
