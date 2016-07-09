# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'SettingsDialog.ui'
#
# Created: Fri May  6 15:18:47 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_SettingsDialog(object):
    def setupUi(self, SettingsDialog):
        SettingsDialog.setObjectName(_fromUtf8("SettingsDialog"))
        SettingsDialog.resize(494, 246)
        self.verticalLayout = QtGui.QVBoxLayout(SettingsDialog)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.tabWidget = QtGui.QTabWidget(SettingsDialog)
        self.tabWidget.setDocumentMode(True)
        self.tabWidget.setObjectName(_fromUtf8("tabWidget"))
        self.tab_appearance = QtGui.QWidget()
        self.tab_appearance.setObjectName(_fromUtf8("tab_appearance"))
        self.verticalLayout_2 = QtGui.QVBoxLayout(self.tab_appearance)
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.groupBox = QtGui.QGroupBox(self.tab_appearance)
        self.groupBox.setObjectName(_fromUtf8("groupBox"))
        self.formLayout = QtGui.QFormLayout(self.groupBox)
        self.formLayout.setObjectName(_fromUtf8("formLayout"))
        self.label = QtGui.QLabel(self.groupBox)
        self.label.setObjectName(_fromUtf8("label"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.LabelRole, self.label)
        self.setting_color_configured = ColorComboBox(self.groupBox)
        self.setting_color_configured.setMinimumSize(QtCore.QSize(200, 0))
        self.setting_color_configured.setObjectName(_fromUtf8("setting_color_configured"))
        self.formLayout.setWidget(0, QtGui.QFormLayout.FieldRole, self.setting_color_configured)
        self.label_2 = QtGui.QLabel(self.groupBox)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.LabelRole, self.label_2)
        self.setting_color_unconfigured = ColorComboBox(self.groupBox)
        self.setting_color_unconfigured.setMinimumSize(QtCore.QSize(200, 0))
        self.setting_color_unconfigured.setObjectName(_fromUtf8("setting_color_unconfigured"))
        self.formLayout.setWidget(1, QtGui.QFormLayout.FieldRole, self.setting_color_unconfigured)
        self.label_3 = QtGui.QLabel(self.groupBox)
        self.label_3.setObjectName(_fromUtf8("label_3"))
        self.formLayout.setWidget(2, QtGui.QFormLayout.LabelRole, self.label_3)
        self.setting_color_processing = ColorComboBox(self.groupBox)
        self.setting_color_processing.setMinimumSize(QtCore.QSize(200, 0))
        self.setting_color_processing.setObjectName(_fromUtf8("setting_color_processing"))
        self.formLayout.setWidget(2, QtGui.QFormLayout.FieldRole, self.setting_color_processing)
        self.label_4 = QtGui.QLabel(self.groupBox)
        self.label_4.setObjectName(_fromUtf8("label_4"))
        self.formLayout.setWidget(3, QtGui.QFormLayout.LabelRole, self.label_4)
        self.setting_color_finished = ColorComboBox(self.groupBox)
        self.setting_color_finished.setMinimumSize(QtCore.QSize(200, 0))
        self.setting_color_finished.setObjectName(_fromUtf8("setting_color_finished"))
        self.formLayout.setWidget(3, QtGui.QFormLayout.FieldRole, self.setting_color_finished)
        self.verticalLayout_2.addWidget(self.groupBox)
        spacerItem = QtGui.QSpacerItem(20, 32, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout_2.addItem(spacerItem)
        self.buttonBox = QtGui.QDialogButtonBox(self.tab_appearance)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Apply|QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok|QtGui.QDialogButtonBox.RestoreDefaults)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.verticalLayout_2.addWidget(self.buttonBox)
        self.tabWidget.addTab(self.tab_appearance, _fromUtf8(""))
        self.verticalLayout.addWidget(self.tabWidget)
        self.label.setBuddy(self.setting_color_configured)
        self.label_2.setBuddy(self.setting_color_unconfigured)
        self.label_3.setBuddy(self.setting_color_processing)
        self.label_4.setBuddy(self.setting_color_finished)

        self.retranslateUi(SettingsDialog)
        self.tabWidget.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(SettingsDialog)

    def retranslateUi(self, SettingsDialog):
        SettingsDialog.setWindowTitle(QtGui.QApplication.translate("SettingsDialog", "Settings", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox.setTitle(QtGui.QApplication.translate("SettingsDialog", "Flow Graph", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("SettingsDialog", "Configured Item:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("SettingsDialog", "Unconfigured Item:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("SettingsDialog", "Processing Item:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("SettingsDialog", "Finished Item:", None, QtGui.QApplication.UnicodeUTF8))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_appearance), QtGui.QApplication.translate("SettingsDialog", "Appearance", None, QtGui.QApplication.UnicodeUTF8))

from core.ColorComboBox import ColorComboBox
