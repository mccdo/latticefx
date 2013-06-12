#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "UtlSettings.h"
#include "shapeCreatorDefines.h"
#include "shapeCreator.h"
#include <QFileDialog>

////////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    m_pVtk = new VtkCreator("", "");

    ui->setupUi(this);

    m_settingsFile = QApplication::applicationDirPath() + "/settings.ini";

    guiSettingsInit();
    guiSettingsLoad();
    guiFeaturesInit();

	m_pThread = new CreatorThread();

}

////////////////////////////////////////////////////////////////////////////////
MainWindow::~MainWindow()
{
    guiSettingsSave();

    delete ui;
    delete m_pVtk;
	delete m_pThread;
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::guiSettingsInit()
{
    int i=0;

    //////////////////////////////////////////////////////////
    // standard options
    //
    ui->checkBoxPrune->setChecked(true);

    // init shape type
    i=0;
    ui->comboBoxShape->insertItem(i++, QString("Vtk"), QVariant(E_SHAPE_VTK));
    ui->comboBoxShape->insertItem(i++, QString("Cube"), QVariant(E_SHAPE_CUBE));
    ui->comboBoxShape->insertItem(i++, QString("Soft Cube"), QVariant(E_SHAPE_SCUBE));
    ui->comboBoxShape->insertItem(i++, QString("Cone"), QVariant(E_SHAPE_CONE));
    ui->comboBoxShape->insertItem(i++, QString("Sphere"), QVariant(E_SHAPE_SPHERE));
    ui->comboBoxShape->insertItem(i++, QString("Soft Sphere"), QVariant(E_SHAPE_SSPHERE));

    // init depths
    i=0;
    ui->comboBoxDepth->insertItem(i++, QString("4"), QVariant(4));
    ui->comboBoxDepth->insertItem(i++, QString("3"), QVariant(3));
    ui->comboBoxDepth->insertItem(i++, QString("2"), QVariant(2));
    ui->comboBoxDepth->insertItem(i++, QString("1"), QVariant(1));

    //////////////////////////////////////////////////////////
    // crunchstore options
    //
    on_radioButtonWriteToDb_clicked();


    //////////////////////////////////////////////////////////
    // vtk options
    //
    ui->checkBoxVtkCacheOn->setChecked(true);
    ui->checkBoxVtkHiresLod->setChecked(true);
    ui->listWidgetVtkScalars->setEnabled(false);
    ui->listWidgetVtkVectors->setEnabled(false);

    // init threads
    i=0;
    ui->comboBoxVtkThreads->insertItem(i++, QString("128"), QVariant(128));
    ui->comboBoxVtkThreads->insertItem(i++, QString("64"), QVariant(64));
    ui->comboBoxVtkThreads->insertItem(i++, QString("32"), QVariant(32));
    ui->comboBoxVtkThreads->insertItem(i++, QString("16"), QVariant(16));
    ui->comboBoxVtkThreads->insertItem(i++, QString("8"), QVariant(8));
    ui->comboBoxVtkThreads->insertItem(i++, QString("4"), QVariant(4));
    ui->comboBoxVtkThreads->insertItem(i++, QString("2"), QVariant(2));
    ui->comboBoxVtkThreads->insertItem(i++, QString("1"), QVariant(1));
    ui->comboBoxVtkThreads->setCurrentIndex(2);

    ui->pushButtonCancel->setEnabled(false);

}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::guiSettingsLoad()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);

    //////////////////////////////////////////////////////////
    // standard options
    //
    settings.beginGroup("standard");
    UtlSettings::initCheckBox(&settings, ui->checkBoxPrune, "prune", true);
    UtlSettings::initComboBox(&settings, ui->comboBoxShape, "shapeType", QVariant(E_SHAPE_VTK));
    UtlSettings::initComboBox(&settings, ui->comboBoxDepth, "depth", QVariant(4));
    settings.endGroup();

    //////////////////////////////////////////////////////////
    // crunchstore options
    //
    settings.beginGroup("cruchstore");

    bool b = settings.value("useCrunchstore", QVariant(true)).toBool();
    if (b)
    {
        on_radioButtonWriteToDb_clicked();
    }
    else
    {
        on_radioButtonWriteToFiles_clicked();
    }

    UtlSettings::initPlainText(&settings, ui->plainTextEditDbFile, "dbfile", QString(""));
    UtlSettings::initPlainText(&settings, ui->plainTextEditFileFolder, "fileFolder", QString(""));
    settings.endGroup();

    //////////////////////////////////////////////////////////
    // vtk options
    //
    settings.beginGroup("Vtk");

    QString s = QApplication::applicationDirPath();
    m_lastPathVtk = settings.value("vtkLastPath", QVariant(s)).toString();

    UtlSettings::initComboBox(&settings, ui->comboBoxVtkThreads, "vtkThreads", QVariant(32));
    UtlSettings::initCheckBox(&settings, ui->checkBoxVtkCacheOn, "vtkCacheOn", true);
    UtlSettings::initCheckBox(&settings, ui->checkBoxVtkHiresLod, "vtkHiResLod", true);
    settings.endGroup();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::guiSettingsSave()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);

    //////////////////////////////////////////////////////////
    // standard options
    //
    settings.beginGroup("standard");
    UtlSettings::saveCheckBox(&settings, ui->checkBoxPrune, "prune");
    UtlSettings::saveComboBox(&settings, ui->comboBoxShape, "shapeType");
    UtlSettings::saveComboBox(&settings, ui->comboBoxDepth, "depth");
    settings.endGroup();

    //////////////////////////////////////////////////////////
    // crunchstore options
    //
    settings.beginGroup("cruchstore");
    UtlSettings::saveRadioBtn(&settings, ui->radioButtonWriteToDb, "useCrunchstore");
    UtlSettings::savePlainText(&settings, ui->plainTextEditDbFile, "dbfile");
    UtlSettings::savePlainText(&settings, ui->plainTextEditFileFolder, "fileFolder");
    settings.endGroup();

    //////////////////////////////////////////////////////////
    // vtk options
    //
    settings.beginGroup("Vtk");
    settings.value("vtkLastPath", m_lastPathVtk);
    UtlSettings::saveComboBox(&settings, ui->comboBoxVtkThreads, "vtkThreads");
    UtlSettings::saveCheckBox(&settings, ui->checkBoxVtkCacheOn, "vtkCacheOn");
    UtlSettings::saveCheckBox(&settings, ui->checkBoxVtkHiresLod, "vtkHiResLod");
    settings.endGroup();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::guiFeaturesInit()
{
#ifndef CRUNCHSTORE_FOUND
    ui->groupBoxCrunchstore->setTitle("Crunchstore - Not Available");
    on_radioButtonWriteToFiles_clicked();

#endif

#ifndef VTK_FOUND
    ui->groupBoxVtk->setEnabled(false);
    ui->groupBoxVtk->setTitle("VTK - Not Available");
    ui->comboBoxShape->removeItem(0);
    /*
    ui->plainTextEditVtkFile->setEnabled(false);
    ui->pushButtonVtkBrowse->setEnabled(false);
    ui->listWidgetVtkScalars->setEnabled(false);
    ui->listWidgetVtkVectors->setEnabled(false);
    ui->comboBoxVtkThreads->setEnabled(false);
    ui->checkBoxVtkCacheOn->setEnabled(false);
    ui->checkBoxVtkHiresLod->setEnabled(false);
    */
#endif
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::loadVtkFile(QString file)
{
    //m_pVtk->init(file.toAscii());

    // get the scalars and vectors..
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButtonWriteToDb_clicked()
{
    ui->radioButtonWriteToDb->setChecked(true);
    ui->radioButtonWriteToFiles->setChecked(false);

    ui->plainTextEditDbFile->setEnabled(true);
    ui->plainTextEditFileFolder->setEnabled(false);
    ui->pushButtonBrowseFolder->setEnabled(false);
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButtonWriteToFiles_clicked()
{
    ui->radioButtonWriteToDb->setChecked(false);
    ui->radioButtonWriteToFiles->setChecked(true);

    ui->plainTextEditDbFile->setEnabled(false);
    ui->plainTextEditFileFolder->setEnabled(true);
    ui->pushButtonBrowseFolder->setEnabled(true);
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_pushButtonBrowseFolder_clicked()
{
    QStringList list;
    QString folderStart = ui->plainTextEditFileFolder->toPlainText();
    QFileDialog fd(this, QString("Browse for file output folder"), folderStart, "");
    fd.setFileMode(QFileDialog::DirectoryOnly);
    if (fd.exec() == QDialog::Rejected) { return; }

    list = fd.selectedFiles();
    if (list.count() <= 0) { return; }

    QString folder = list.at(0);
    ui->plainTextEditFileFolder->setPlainText(folder);
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_pushButtonVtkBrowse_clicked()
{
    QStringList list;
    QString filter = "Vtk Files (*.vtu *.vts);;All files (*.*)";
    QFileDialog fd(this, QString("Browse for vtk file"), m_lastPathVtk, filter);

    if (fd.exec() == QDialog::Rejected) { return; }

    list = fd.selectedFiles();
    if (list.count() <= 0) { return; }


    QDir fullPath = fd.directory();
    m_lastPathVtk = fullPath.absolutePath();
    ui->plainTextEditVtkFile->setPlainText(list.at(0));
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_comboBoxShape_currentIndexChanged(int index)
{

}

void MainWindow::on_pushButtonCreate_clicked()
{

}
 
void MainWindow::on_pushButtonCancel_clicked()
{

}


void createDataSet( const std::string& csFile, SaveHierarchy* saver )
{
}
