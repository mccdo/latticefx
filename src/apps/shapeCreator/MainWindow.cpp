#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "UtlSettings.h"
#include "ShapeVolumes.h"
#include <QFileDialog>
#include <QMessageBox>

const std::string logstr( "lfx.demo" );
const std::string loginfo( logstr+".info" );

////////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    _pVtk.reset(new VtkCreator(logstr.c_str(), loginfo.c_str()));

    ui->setupUi(this);

    _settingsFile = QApplication::applicationDirPath() + "/settings.ini";

    guiSettingsInit();
    guiSettingsLoad();
    guiFeaturesInit();

	_pThread = new CreatorThread();

	QObject::connect( _pThread, SIGNAL(signalStart()), this, SLOT(slotStart()));
	QObject::connect( _pThread, SIGNAL(signalProgress(float)), this, SLOT(slotProgress(float)));
	QObject::connect( _pThread, SIGNAL(signalEnd()), this, SLOT(slotEnd()));
	QObject::connect( _pThread, SIGNAL(signalMsg(std::string)), this, SLOT(slotMsg(std::string)));

	ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);

	setShapeType(UtlSettings::getSelectedValueInt(ui->comboBoxShape));
}

////////////////////////////////////////////////////////////////////////////////
MainWindow::~MainWindow()
{
    guiSettingsSave();

    delete ui;
	delete _pThread;
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
    QSettings settings(_settingsFile, QSettings::IniFormat);

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
    _lastPathVtk = settings.value("vtkLastPath", QVariant(s)).toString();

    UtlSettings::initComboBox(&settings, ui->comboBoxVtkThreads, "vtkThreads", QVariant(32));
    UtlSettings::initCheckBox(&settings, ui->checkBoxVtkCacheOn, "vtkCacheOn", true);
    UtlSettings::initCheckBox(&settings, ui->checkBoxVtkHiresLod, "vtkHiResLod", true);
    settings.endGroup();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::guiSettingsSave()
{
    QSettings settings(_settingsFile, QSettings::IniFormat);

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
    settings.value("vtkLastPath", _lastPathVtk);
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
	ui->radioButtonWriteToDb->setEnabled(false);
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
void MainWindow::setShapeType(int shapeType)
{
	bool enable = false;
	if (shapeType == E_SHAPE_VTK)
	{
		enable = true;
	}
	
	ui->plainTextEditVtkFile->setEnabled(enable);
	ui->pushButtonVtkBrowse->setEnabled(enable);
	ui->comboBoxVtkThreads->setEnabled(enable);
	ui->checkBoxVtkCacheOn->setEnabled(enable);
	ui->checkBoxVtkHiresLod->setEnabled(enable);
	ui->listWidgetVtkScalars->setEnabled(enable);
	ui->listWidgetVtkVectors->setEnabled(enable);
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::loadVtkFile(QString file)
{
    vtkCreator()->init(file.toAscii());

    // get the scalars and vectors..
    QString str;
    int num;

    num = vtkCreator()->getNumScalars();
    ui->listWidgetVtkScalars->clear();
    for (int i=0; i<num; i++)
    {
        str = vtkCreator()->getScalarName(i).c_str();
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(str);
        item->setCheckState(Qt::Checked);
        ui->listWidgetVtkScalars->addItem(item);
    }

	if (num > 0)
	{
		ui->listWidgetVtkScalars->setEnabled(true);
	}
	else
	{
		ui->listWidgetVtkScalars->setEnabled(false);
	}

    num = vtkCreator()->getNumVectors();
    ui->listWidgetVtkVectors->clear();
    for (int i=0; i<num; i++)
    {
        str = vtkCreator()->getVectorName(i).c_str();
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(str);
        item->setCheckState(Qt::Checked);
        ui->listWidgetVtkVectors->addItem(item);
    }

	if (num > 0)
	{
		ui->listWidgetVtkVectors->setEnabled(true);
	}
	else
	{
		ui->listWidgetVtkVectors->setEnabled(false);
	}
}

////////////////////////////////////////////////////////////////////////////////
VtkCreator* MainWindow::vtkCreator()
{
	return (VtkCreator *)_pVtk.get();
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
    QFileDialog fd(this, QString("Browse for vtk file"), _lastPathVtk, filter);

    if (fd.exec() == QDialog::Rejected) { return; }

    list = fd.selectedFiles();
    if (list.count() <= 0) { return; }


    QDir fullPath = fd.directory();
    _lastPathVtk = fullPath.absolutePath();
    ui->plainTextEditVtkFile->setPlainText(list.at(0));

	loadVtkFile(list.at(0));
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_comboBoxShape_currentIndexChanged(int index)
{
	setShapeType(ui->comboBoxShape->itemData(index).toInt());
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_pushButtonCreate_clicked()
{
	if ( _pThread->isRunning())
    {
        QMessageBox::information(NULL, "Volume Creation Running", "Volume creation is currently running.\nPlease wait until it finishes.");
        return;
    }

	int shapeType = UtlSettings::getSelectedValueInt(ui->comboBoxShape);
	boost::shared_ptr<CreateVolume> createVolume(new CreateVolume(logstr.c_str(), loginfo.c_str()));
	VolumeBrickDataPtr vbd;
	bool prune = ui->checkBoxPrune->isChecked();

	switch (shapeType)
	{
	case E_SHAPE_VTK:
	{
		if (!vtkCreator()->haveFile())
		{
			QMessageBox::information(NULL, "No Vtk File", "Please open a vtk file first.");
			return;
		}

		std::vector<int> scalars, vectors;
		UtlSettings::getCheckedItems(ui->listWidgetVtkScalars, &scalars);
		UtlSettings::getCheckedItems(ui->listWidgetVtkVectors, &vectors);

		int threads = UtlSettings::getSelectedValueInt(ui->comboBoxVtkThreads);
		bool cacheon = ui->checkBoxVtkCacheOn->isChecked();
		bool hireslod = ui->checkBoxVtkHiresLod->isChecked();

		vtkCreator()->setScalarsToProcess(scalars);
		vtkCreator()->setVectorsToProcess(vectors);
		vtkCreator()->setThreads(threads);
		vtkCreator()->setCache(cacheon);
		vtkCreator()->setHiresLod(hireslod);

		createVolume = _pVtk;
		break;
	}
	case E_SHAPE_CUBE:
		vbd.reset(new CubeVolumeBrickData(prune, false));
		createVolume->setVolumeObj(vbd);
		break;
	case E_SHAPE_SCUBE:
		vbd.reset(new CubeVolumeBrickData(prune, true));
		createVolume->setVolumeObj(vbd);
		break;
	case E_SHAPE_CONE:
		vbd.reset(new ConeVolumeBrickData(prune));
		createVolume->setVolumeObj(vbd);
		break;
	case E_SHAPE_SPHERE:
		vbd.reset(new SphereVolumeBrickData(prune, false));
		createVolume->setVolumeObj(vbd);
		break;
	case E_SHAPE_SSPHERE:
		vbd.reset(new SphereVolumeBrickData(prune, true));
		break;
	default:
		QMessageBox::information(NULL, "UnRecognized Shape Type", "UnRecognized shape type.");
		return;
	}


	int depth = UtlSettings::getSelectedValueInt(ui->comboBoxDepth);
	bool usedb = ui->radioButtonWriteToDb->isChecked();

	createVolume->setPrune(prune);
	createVolume->setDepth(depth);
	createVolume->setUseCrunchStore(usedb);

	QString file = ui->plainTextEditDbFile->toPlainText();
	if (!usedb)
	{
		file = ui->plainTextEditFileFolder->toPlainText();
	}

	std::string strFileOrFolder = file.toStdString();
	createVolume->setCsFileOrFolder(strFileOrFolder.c_str());

	_pThread->setCreateVolume(createVolume);
	_pThread->start();
}
 
////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_pushButtonCancel_clicked()
{
	if ( _pThread->isRunning())
    {
        _pThread->cancel();
    }
}


////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotStart()
{
	ui->pushButtonCreate->setEnabled(false);
	ui->pushButtonCancel->setEnabled(true);
	ui->progressBar->setValue(0);
	msgClearAll();
	msgOut(QString("Creating bricks..."));
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotProgress(float percent)
{
	ui->progressBar->setValue((int)(100.*percent));
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotEnd()
{
	ui->pushButtonCreate->setEnabled(true);
	ui->pushButtonCancel->setEnabled(false);
	msgOut(QString("Finished..."));
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotMsg(std::string msg)
{
	QString m(msg.c_str());
	msgOut(m);
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::msgOut(const QString &msg)
{
    //QString shortTime = QLocale::system().toString(QTime::currentTime(), QLocale::ShortFormat);
    //shortTime += " : ";
    //QString out = shortTime + msg;
    //out += "\n";

	QString out = msg + "\n";
    out = ui->textBrowserOutput->toPlainText() + out;

    //QDateTime current = QDateTime::currentDateTime();
    //current.toString()
	ui->textBrowserOutput->setText(out);
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::msgClearAll()
{
    ui->textBrowserOutput->setText("");
}