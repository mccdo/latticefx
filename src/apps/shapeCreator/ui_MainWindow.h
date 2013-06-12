/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created: Sun Jun 9 12:42:35 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QStatusBar>
#include <QtGui/QTextBrowser>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QGroupBox *groupBoxStandard;
    QLabel *label;
    QCheckBox *checkBoxPrune;
    QComboBox *comboBoxShape;
    QLabel *label_8;
    QComboBox *comboBoxDepth;
    QGroupBox *groupBoxCrunchstore;
    QLabel *label_2;
    QPlainTextEdit *plainTextEditDbFile;
    QRadioButton *radioButtonWriteToDb;
    QRadioButton *radioButtonWriteToFiles;
    QPlainTextEdit *plainTextEditFileFolder;
    QLabel *label_3;
    QPushButton *pushButtonBrowseFolder;
    QGroupBox *groupBoxVtk;
    QLabel *label_4;
    QPlainTextEdit *plainTextEditVtkFile;
    QPushButton *pushButtonVtkBrowse;
    QListWidget *listWidgetVtkScalars;
    QLabel *label_5;
    QLabel *label_6;
    QListWidget *listWidgetVtkVectors;
    QLabel *label_7;
    QCheckBox *checkBoxVtkCacheOn;
    QCheckBox *checkBoxVtkHiresLod;
    QComboBox *comboBoxVtkThreads;
    QProgressBar *progressBar;
    QTextBrowser *textBrowserOutput;
    QPushButton *pushButtonCreate;
    QPushButton *pushButtonCancel;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(640, 776);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        groupBoxStandard = new QGroupBox(centralWidget);
        groupBoxStandard->setObjectName(QString::fromUtf8("groupBoxStandard"));
        groupBoxStandard->setGeometry(QRect(20, 30, 201, 141));
        label = new QLabel(groupBoxStandard);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 40, 46, 13));
        checkBoxPrune = new QCheckBox(groupBoxStandard);
        checkBoxPrune->setObjectName(QString::fromUtf8("checkBoxPrune"));
        checkBoxPrune->setGeometry(QRect(140, 80, 51, 17));
        comboBoxShape = new QComboBox(groupBoxStandard);
        comboBoxShape->setObjectName(QString::fromUtf8("comboBoxShape"));
        comboBoxShape->setGeometry(QRect(60, 40, 111, 22));
        label_8 = new QLabel(groupBoxStandard);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(20, 80, 31, 16));
        comboBoxDepth = new QComboBox(groupBoxStandard);
        comboBoxDepth->setObjectName(QString::fromUtf8("comboBoxDepth"));
        comboBoxDepth->setGeometry(QRect(60, 80, 69, 22));
        comboBoxDepth->setEditable(false);
        groupBoxCrunchstore = new QGroupBox(centralWidget);
        groupBoxCrunchstore->setObjectName(QString::fromUtf8("groupBoxCrunchstore"));
        groupBoxCrunchstore->setGeometry(QRect(230, 30, 401, 141));
        label_2 = new QLabel(groupBoxCrunchstore);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 30, 71, 16));
        plainTextEditDbFile = new QPlainTextEdit(groupBoxCrunchstore);
        plainTextEditDbFile->setObjectName(QString::fromUtf8("plainTextEditDbFile"));
        plainTextEditDbFile->setGeometry(QRect(90, 20, 271, 31));
        radioButtonWriteToDb = new QRadioButton(groupBoxCrunchstore);
        radioButtonWriteToDb->setObjectName(QString::fromUtf8("radioButtonWriteToDb"));
        radioButtonWriteToDb->setGeometry(QRect(110, 70, 82, 17));
        radioButtonWriteToFiles = new QRadioButton(groupBoxCrunchstore);
        radioButtonWriteToFiles->setObjectName(QString::fromUtf8("radioButtonWriteToFiles"));
        radioButtonWriteToFiles->setGeometry(QRect(210, 70, 82, 17));
        plainTextEditFileFolder = new QPlainTextEdit(groupBoxCrunchstore);
        plainTextEditFileFolder->setObjectName(QString::fromUtf8("plainTextEditFileFolder"));
        plainTextEditFileFolder->setGeometry(QRect(90, 100, 271, 31));
        label_3 = new QLabel(groupBoxCrunchstore);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(20, 110, 71, 16));
        pushButtonBrowseFolder = new QPushButton(groupBoxCrunchstore);
        pushButtonBrowseFolder->setObjectName(QString::fromUtf8("pushButtonBrowseFolder"));
        pushButtonBrowseFolder->setGeometry(QRect(270, 70, 75, 23));
        groupBoxVtk = new QGroupBox(centralWidget);
        groupBoxVtk->setObjectName(QString::fromUtf8("groupBoxVtk"));
        groupBoxVtk->setGeometry(QRect(30, 180, 521, 251));
        label_4 = new QLabel(groupBoxVtk);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(20, 20, 46, 13));
        plainTextEditVtkFile = new QPlainTextEdit(groupBoxVtk);
        plainTextEditVtkFile->setObjectName(QString::fromUtf8("plainTextEditVtkFile"));
        plainTextEditVtkFile->setGeometry(QRect(50, 10, 271, 31));
        pushButtonVtkBrowse = new QPushButton(groupBoxVtk);
        pushButtonVtkBrowse->setObjectName(QString::fromUtf8("pushButtonVtkBrowse"));
        pushButtonVtkBrowse->setGeometry(QRect(330, 10, 75, 23));
        listWidgetVtkScalars = new QListWidget(groupBoxVtk);
        listWidgetVtkScalars->setObjectName(QString::fromUtf8("listWidgetVtkScalars"));
        listWidgetVtkScalars->setGeometry(QRect(50, 80, 121, 141));
        label_5 = new QLabel(groupBoxVtk);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(50, 60, 46, 13));
        label_6 = new QLabel(groupBoxVtk);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(210, 60, 46, 13));
        listWidgetVtkVectors = new QListWidget(groupBoxVtk);
        listWidgetVtkVectors->setObjectName(QString::fromUtf8("listWidgetVtkVectors"));
        listWidgetVtkVectors->setGeometry(QRect(210, 80, 121, 141));
        label_7 = new QLabel(groupBoxVtk);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(350, 90, 46, 13));
        checkBoxVtkCacheOn = new QCheckBox(groupBoxVtk);
        checkBoxVtkCacheOn->setObjectName(QString::fromUtf8("checkBoxVtkCacheOn"));
        checkBoxVtkCacheOn->setGeometry(QRect(380, 150, 70, 17));
        checkBoxVtkHiresLod = new QCheckBox(groupBoxVtk);
        checkBoxVtkHiresLod->setObjectName(QString::fromUtf8("checkBoxVtkHiresLod"));
        checkBoxVtkHiresLod->setGeometry(QRect(380, 190, 81, 17));
        comboBoxVtkThreads = new QComboBox(groupBoxVtk);
        comboBoxVtkThreads->setObjectName(QString::fromUtf8("comboBoxVtkThreads"));
        comboBoxVtkThreads->setGeometry(QRect(400, 90, 69, 22));
        comboBoxVtkThreads->setEditable(true);
        progressBar = new QProgressBar(centralWidget);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setGeometry(QRect(30, 440, 561, 23));
        progressBar->setValue(24);
        textBrowserOutput = new QTextBrowser(centralWidget);
        textBrowserOutput->setObjectName(QString::fromUtf8("textBrowserOutput"));
        textBrowserOutput->setGeometry(QRect(30, 470, 531, 192));
        pushButtonCreate = new QPushButton(centralWidget);
        pushButtonCreate->setObjectName(QString::fromUtf8("pushButtonCreate"));
        pushButtonCreate->setGeometry(QRect(150, 680, 75, 23));
        pushButtonCancel = new QPushButton(centralWidget);
        pushButtonCancel->setObjectName(QString::fromUtf8("pushButtonCancel"));
        pushButtonCancel->setGeometry(QRect(280, 680, 75, 23));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 640, 21));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        groupBoxStandard->setTitle(QApplication::translate("MainWindow", "Standard Options", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Shape:", 0, QApplication::UnicodeUTF8));
        checkBoxPrune->setText(QApplication::translate("MainWindow", "Prune", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("MainWindow", "Depth:", 0, QApplication::UnicodeUTF8));
        groupBoxCrunchstore->setTitle(QApplication::translate("MainWindow", "Crunchstore", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "Database File:", 0, QApplication::UnicodeUTF8));
        radioButtonWriteToDb->setText(QApplication::translate("MainWindow", "Database", 0, QApplication::UnicodeUTF8));
        radioButtonWriteToFiles->setText(QApplication::translate("MainWindow", "Files", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("MainWindow", "File Folder:", 0, QApplication::UnicodeUTF8));
        pushButtonBrowseFolder->setText(QApplication::translate("MainWindow", "Browse", 0, QApplication::UnicodeUTF8));
        groupBoxVtk->setTitle(QApplication::translate("MainWindow", "VTK", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("MainWindow", "File:", 0, QApplication::UnicodeUTF8));
        pushButtonVtkBrowse->setText(QApplication::translate("MainWindow", "Browse", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("MainWindow", "Scalars:", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("MainWindow", "Vectors:", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("MainWindow", "Threads:", 0, QApplication::UnicodeUTF8));
        checkBoxVtkCacheOn->setText(QApplication::translate("MainWindow", "cache on", 0, QApplication::UnicodeUTF8));
        checkBoxVtkHiresLod->setText(QApplication::translate("MainWindow", "HighResLod", 0, QApplication::UnicodeUTF8));
        pushButtonCreate->setText(QApplication::translate("MainWindow", "Create", 0, QApplication::UnicodeUTF8));
        pushButtonCancel->setText(QApplication::translate("MainWindow", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
