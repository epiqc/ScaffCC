# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ResultsTable.ui'
#
# Created: Thu May  5 12:33:14 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_ResultsTable(object):
    def setupUi(self, ResultsTable):
        ResultsTable.setObjectName(_fromUtf8("ResultsTable"))
        ResultsTable.resize(510, 499)
        ResultsTable.setWindowTitle(_fromUtf8(""))
        self.horizontalLayout = QtGui.QHBoxLayout(ResultsTable)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.splitter = QtGui.QSplitter(ResultsTable)
        self.splitter.setOrientation(QtCore.Qt.Horizontal)
        self.splitter.setObjectName(_fromUtf8("splitter"))
        self.tabWidget = RenamableTabWidget(self.splitter)
        self.tabWidget.setObjectName(_fromUtf8("tabWidget"))
        self.widget = QtGui.QWidget(self.splitter)
        self.widget.setObjectName(_fromUtf8("widget"))
        self.formLayout = QtGui.QFormLayout(self.widget)
        self.formLayout.setMargin(0)
        self.formLayout.setObjectName(_fromUtf8("formLayout"))
        self.label_3 = QtGui.QLabel(self.widget)
        self.label_3.setObjectName(_fromUtf8("label_3"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.LabelRole, self.label_3)
        self.widget_3 = QtGui.QWidget(self.widget)
        self.widget_3.setObjectName(_fromUtf8("widget_3"))
        self.verticalLayout_2 = QtGui.QVBoxLayout(self.widget_3)
        self.verticalLayout_2.setMargin(0)
        self.verticalLayout_2.setMargin(0)
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.global_lines = QtGui.QCheckBox(self.widget_3)
        self.global_lines.setObjectName(_fromUtf8("global_lines"))
        self.verticalLayout_2.addWidget(self.global_lines)
        self.global_gates = QtGui.QCheckBox(self.widget_3)
        self.global_gates.setObjectName(_fromUtf8("global_gates"))
        self.verticalLayout_2.addWidget(self.global_gates)
        self.global_qc = QtGui.QCheckBox(self.widget_3)
        self.global_qc.setObjectName(_fromUtf8("global_qc"))
        self.verticalLayout_2.addWidget(self.global_qc)
        self.global_tc = QtGui.QCheckBox(self.widget_3)
        self.global_tc.setObjectName(_fromUtf8("global_tc"))
        self.verticalLayout_2.addWidget(self.global_tc)
        self.formLayout.setWidget(0, QtGui.QFormLayout.FieldRole, self.widget_3)
        self.label_4 = QtGui.QLabel(self.widget)
        self.label_4.setObjectName(_fromUtf8("label_4"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.LabelRole, self.label_4)
        self.widget_4 = QtGui.QWidget(self.widget)
        self.widget_4.setObjectName(_fromUtf8("widget_4"))
        self.verticalLayout_3 = QtGui.QVBoxLayout(self.widget_4)
        self.verticalLayout_3.setMargin(0)
        self.verticalLayout_3.setMargin(0)
        self.verticalLayout_3.setObjectName(_fromUtf8("verticalLayout_3"))
        self.configure_extra_columns = QtGui.QToolButton(self.widget_4)
        self.configure_extra_columns.setToolButtonStyle(QtCore.Qt.ToolButtonTextBesideIcon)
        self.configure_extra_columns.setObjectName(_fromUtf8("configure_extra_columns"))
        self.verticalLayout_3.addWidget(self.configure_extra_columns)
        self.extra_columns = QtGui.QLabel(self.widget_4)
        self.extra_columns.setObjectName(_fromUtf8("extra_columns"))
        self.verticalLayout_3.addWidget(self.extra_columns)
        self.formLayout.setWidget(1, QtGui.QFormLayout.FieldRole, self.widget_4)
        self.label = QtGui.QLabel(self.widget)
        self.label.setObjectName(_fromUtf8("label"))
        self.formLayout.setWidget(2, QtGui.QFormLayout.LabelRole, self.label)
        self.widget_2 = QtGui.QWidget(self.widget)
        self.widget_2.setObjectName(_fromUtf8("widget_2"))
        self.verticalLayout = QtGui.QVBoxLayout(self.widget_2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.export_pdf = QtGui.QToolButton(self.widget_2)
        self.export_pdf.setToolButtonStyle(QtCore.Qt.ToolButtonTextBesideIcon)
        self.export_pdf.setObjectName(_fromUtf8("export_pdf"))
        self.verticalLayout.addWidget(self.export_pdf)
        self.export_latex = QtGui.QToolButton(self.widget_2)
        self.export_latex.setToolButtonStyle(QtCore.Qt.ToolButtonTextBesideIcon)
        self.export_latex.setObjectName(_fromUtf8("export_latex"))
        self.verticalLayout.addWidget(self.export_latex)
        self.formLayout.setWidget(2, QtGui.QFormLayout.FieldRole, self.widget_2)
        self.horizontalLayout.addWidget(self.splitter)

        self.retranslateUi(ResultsTable)
        self.tabWidget.setCurrentIndex(-1)
        QtCore.QMetaObject.connectSlotsByName(ResultsTable)
        ResultsTable.setTabOrder(self.tabWidget, self.global_lines)
        ResultsTable.setTabOrder(self.global_lines, self.global_gates)
        ResultsTable.setTabOrder(self.global_gates, self.global_qc)
        ResultsTable.setTabOrder(self.global_qc, self.global_tc)
        ResultsTable.setTabOrder(self.global_tc, self.configure_extra_columns)
        ResultsTable.setTabOrder(self.configure_extra_columns, self.export_pdf)
        ResultsTable.setTabOrder(self.export_pdf, self.export_latex)

    def retranslateUi(self, ResultsTable):
        self.label_3.setText(QtGui.QApplication.translate("ResultsTable", "Global Columns:", None, QtGui.QApplication.UnicodeUTF8))
        self.global_lines.setText(QtGui.QApplication.translate("ResultsTable", "Lines", None, QtGui.QApplication.UnicodeUTF8))
        self.global_gates.setText(QtGui.QApplication.translate("ResultsTable", "Gates", None, QtGui.QApplication.UnicodeUTF8))
        self.global_qc.setText(QtGui.QApplication.translate("ResultsTable", "Quantum Costs", None, QtGui.QApplication.UnicodeUTF8))
        self.global_tc.setText(QtGui.QApplication.translate("ResultsTable", "Transistor Costs", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("ResultsTable", "Extra Columns:", None, QtGui.QApplication.UnicodeUTF8))
        self.configure_extra_columns.setText(QtGui.QApplication.translate("ResultsTable", "Configure", None, QtGui.QApplication.UnicodeUTF8))
        self.extra_columns.setText(QtGui.QApplication.translate("ResultsTable", "No extra columns", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("ResultsTable", "Export:", None, QtGui.QApplication.UnicodeUTF8))
        self.export_pdf.setText(QtGui.QApplication.translate("ResultsTable", "Preview PDF", None, QtGui.QApplication.UnicodeUTF8))
        self.export_latex.setText(QtGui.QApplication.translate("ResultsTable", "As LaTeX Table", None, QtGui.QApplication.UnicodeUTF8))

from core.RenamableTabWidget import RenamableTabWidget
