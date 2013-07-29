#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "UtlSettings.h"
#include "ShapeVolumes.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QtCore>
#include <QVBoxLayout>
#include <QLabel>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>
//#include <latticefx/core/JsonSerializer.h>

const std::string logstr( "lfx.demo" );
const std::string loginfo( logstr+".info" );

using namespace lfx::core;

////////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::MainWindow )
{

	Log::instance()->setPriority( Log::PrioFatal, Log::Console );
	Log::instance()->setPriority( Log::PrioInfo, Log::Console );

    _pVtk.reset( new VtkCreator( logstr.c_str(), loginfo.c_str() ) );
	_pThread = new CreatorThread();

    ui->setupUi(this);

    _settingsFile = QApplication::applicationDirPath() + "/shapecreator_settings.ini";

    guiSettingsInit();
    guiSettingsLoad();
    guiFeaturesInit();

	QObject::connect( _pThread, SIGNAL( signalStart() ), this, SLOT( slotStart() ) );
	QObject::connect( _pThread, SIGNAL( signalProgress( int ) ), this, SLOT( slotProgress( int ) ) );
	QObject::connect( _pThread, SIGNAL( signalEnd() ), this, SLOT(slotEnd() ) );
	QObject::connect( _pThread, SIGNAL( signalMsg( QString ) ), this, SLOT( slotMsg( QString ) ) );
	QObject::connect( this, SIGNAL( signalVtkLoadDone() ), this, SLOT( slotVtkLoadDone() ) );

	ui->progressBar->setRange( 0, 100 );
    ui->progressBar->setValue( 0 );

	setShapeType( UtlSettings::getSelectedValueInt( ui->comboBoxShape ) );

	/*
	JsonSerializer js;
	js.insertObjValue( "key1", "value1" );
	js.insertObjValue( "key2", "value2" );
	js.insertObj( "obj1", true );
	js.insertObjValue( "sub1", "subval1");
	js.popParent();
	js.insertArray("vector");
	js.insertArrValue(1);
	js.insertArrValue(2);
	js.insertArrValue(3);
	js.popParent();
	std::string str = js.toString();
	QMessageBox::information( NULL, "Json", QString(str.c_str()) );
	*/
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
    ui->checkBoxPrune->setChecked( true );
	ui->checkBoxResPrune->setChecked( true );

    // init shape type
    i=0;
    ui->comboBoxShape->insertItem( i++, QString("Vtk"), QVariant(E_SHAPE_VTK) );
    ui->comboBoxShape->insertItem( i++, QString("Cube"), QVariant(E_SHAPE_CUBE) );
    ui->comboBoxShape->insertItem( i++, QString("Soft Cube"), QVariant(E_SHAPE_SCUBE) );
    ui->comboBoxShape->insertItem( i++, QString("Cone"), QVariant(E_SHAPE_CONE) );
    ui->comboBoxShape->insertItem( i++, QString("Sphere"), QVariant(E_SHAPE_SPHERE) );
    ui->comboBoxShape->insertItem( i++, QString("Soft Sphere"), QVariant(E_SHAPE_SSPHERE) );

    // init depths
    i=0;
    ui->comboBoxDepth->insertItem( i++, QString("4"), QVariant(4) );
    ui->comboBoxDepth->insertItem( i++, QString("3"), QVariant(3) );
    ui->comboBoxDepth->insertItem( i++, QString("2"), QVariant(2) );
    ui->comboBoxDepth->insertItem( i++, QString("1"), QVariant(1) );

    //////////////////////////////////////////////////////////
    // crunchstore options
    //
    on_radioButtonWriteToDb_clicked();


    //////////////////////////////////////////////////////////
    // vtk options
    //
    ui->checkBoxVtkCacheOn->setChecked( true );
    ui->checkBoxVtkHiresLod->setChecked( true );
    ui->listWidgetVtkScalars->setEnabled( false );
    ui->listWidgetVtkVectors->setEnabled( false );

    // init threads
    i=0;
    ui->comboBoxVtkThreads->insertItem( i++, QString("128"), QVariant(128) );
    ui->comboBoxVtkThreads->insertItem( i++, QString("64"), QVariant(64) );
    ui->comboBoxVtkThreads->insertItem( i++, QString("32"), QVariant(32) );
    ui->comboBoxVtkThreads->insertItem( i++, QString("16"), QVariant(16) );
    ui->comboBoxVtkThreads->insertItem( i++, QString("8"), QVariant(8) );
    ui->comboBoxVtkThreads->insertItem( i++, QString("4"), QVariant(4) );
    ui->comboBoxVtkThreads->insertItem( i++, QString("2"), QVariant(2) );
    ui->comboBoxVtkThreads->insertItem( i++, QString("1"), QVariant(1) );
    ui->comboBoxVtkThreads->setCurrentIndex( 2 );

    ui->pushButtonCancel->setEnabled( false );

}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::guiSettingsLoad()
{
    QSettings settings( _settingsFile, QSettings::IniFormat );

    //////////////////////////////////////////////////////////
    // standard options
    //
    settings.beginGroup( "standard" );
    UtlSettings::initCheckBox( &settings, ui->checkBoxPrune, "prune", true );
	UtlSettings::initCheckBox( &settings, ui->checkBoxResPrune, "resPrune", true );
    UtlSettings::initComboBox( &settings, ui->comboBoxShape, "shapeType", QVariant(E_SHAPE_VTK) );
    UtlSettings::initComboBox( &settings, ui->comboBoxDepth, "depth", QVariant(4) );
    settings.endGroup();

    //////////////////////////////////////////////////////////
    // crunchstore options
    //
    settings.beginGroup( "cruchstore" );

    bool b = settings.value( "useCrunchstore", QVariant(true) ).toBool();
    if (b)
    {
        on_radioButtonWriteToDb_clicked();
    }
    else
    {
        on_radioButtonWriteToFiles_clicked();
    }

    UtlSettings::initPlainText( &settings, ui->plainTextEditDbFile, "dbfile", QString("") );
    UtlSettings::initPlainText( &settings, ui->plainTextEditFileFolder, "fileFolder", QApplication::applicationDirPath() );
    settings.endGroup();

    //////////////////////////////////////////////////////////
    // vtk options
    //
    settings.beginGroup( "Vtk" );

    QString s = QApplication::applicationDirPath();
    _lastPathVtk = settings.value( "vtkLastPath", QVariant(s) ).toString();

    UtlSettings::initComboBox( &settings, ui->comboBoxVtkThreads, "vtkThreads", QVariant(32) );
    UtlSettings::initCheckBox( &settings, ui->checkBoxVtkCacheOn, "vtkCacheOn", true );
    UtlSettings::initCheckBox( &settings, ui->checkBoxVtkHiresLod, "vtkHiResLod", true );
    settings.endGroup();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::guiSettingsSave()
{
    QSettings settings( _settingsFile, QSettings::IniFormat );

    //////////////////////////////////////////////////////////
    // standard options
    //
    settings.beginGroup( "standard");
    UtlSettings::saveCheckBox( &settings, ui->checkBoxPrune, "prune" );
	UtlSettings::saveCheckBox( &settings, ui->checkBoxResPrune, "resPrune" );
    UtlSettings::saveComboBox( &settings, ui->comboBoxShape, "shapeType" );
    UtlSettings::saveComboBox( &settings, ui->comboBoxDepth, "depth" );
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
    settings.beginGroup( "Vtk" );
    settings.setValue( "vtkLastPath", QVariant(_lastPathVtk) );
    UtlSettings::saveComboBox( &settings, ui->comboBoxVtkThreads, "vtkThreads" );
    UtlSettings::saveCheckBox( &settings, ui->checkBoxVtkCacheOn, "vtkCacheOn" );
    UtlSettings::saveCheckBox( &settings, ui->checkBoxVtkHiresLod, "vtkHiResLod" );
    settings.endGroup();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::guiFeaturesInit()
{
#ifndef LFX_USE_CRUNCHSTORE
    ui->groupBoxCrunchstore->setTitle( "Crunchstore - Not Available" );
    on_radioButtonWriteToFiles_clicked();
	ui->radioButtonWriteToDb->setEnabled( false );
#endif

#ifndef VTK_FOUND
    ui->groupBoxVtk->setEnabled( false );
    ui->groupBoxVtk->setTitle( "VTK - Not Available" );
    ui->comboBoxShape->removeItem( 0 );
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
	
	ui->plainTextEditVtkFile->setEnabled( enable );
	ui->pushButtonVtkBrowse->setEnabled( enable );
	ui->comboBoxVtkThreads->setEnabled( enable );
	ui->checkBoxVtkCacheOn->setEnabled( enable );
	ui->checkBoxVtkHiresLod->setEnabled( enable );
	//ui->checkBoxResPrune->setEnabled( enable );
	ui->listWidgetVtkScalars->setEnabled( enable );
	ui->listWidgetVtkVectors->setEnabled( enable );

	enableCreateButton();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::loadVtkFile(QString file)
{
    vtkCreator()->init( file.toAscii() );
    emit signalVtkLoadDone();
}


////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotVtkLoadDone()
{
	// get the scalars and vectors..
    QString str;
    int num;

    num = vtkCreator()->getNumScalars();
    ui->listWidgetVtkScalars->clear();
    for ( int i=0; i<num; i++ )
    {
        str = vtkCreator()->getScalarName(i).c_str();
        QListWidgetItem *item = new QListWidgetItem();
        item->setText( str );
        item->setCheckState( Qt::Checked );
        ui->listWidgetVtkScalars->addItem( item );
    }

	if (num > 0)
	{
		ui->listWidgetVtkScalars->setEnabled( true );
	}
	else
	{
		ui->listWidgetVtkScalars->setEnabled( false );
	}

    num = vtkCreator()->getNumVectors();
    ui->listWidgetVtkVectors->clear();
    for ( int i=0; i<num; i++ )
    {
        str = vtkCreator()->getVectorName(i).c_str();
        QListWidgetItem *item = new QListWidgetItem();
        item->setText( str );
        item->setCheckState( Qt::Checked );
        ui->listWidgetVtkVectors->addItem( item );
    }

	if (num > 0)
	{
		ui->listWidgetVtkVectors->setEnabled( true );
	}
	else
	{
		ui->listWidgetVtkVectors->setEnabled( false );
	}

	enableCreateButton();
	ui->pushButtonVtkBrowse->setEnabled( true );

	_pLoadingDlg.reset();
	this->setCursor(Qt::ArrowCursor);
}

////////////////////////////////////////////////////////////////////////////////
VtkCreator* MainWindow::vtkCreator()
{
	return (VtkCreator *)_pVtk.get();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::enableCreateButton()
{
	if ( _pThread->isRunning() )
    {
		return;
	}

	int shapeType = UtlSettings::getSelectedValueInt( ui->comboBoxShape );
	if ( shapeType == E_SHAPE_VTK)
	{
		if ( !vtkCreator()->haveFile() )
		{
			ui->pushButtonCreate->setEnabled( false );
			return;
		}
	}

	// check data folder
	QString folder = ui->plainTextEditFileFolder->toPlainText();
	if ( folder.length() <= 0 || !QDir( folder ).exists() )
	{
		ui->pushButtonCreate->setEnabled( false );
		return;
	}

	// check db file and full path
	if ( ui->radioButtonWriteToDb->isChecked() )
	{
		QString dbfile = ui->plainTextEditDbFile->toPlainText();
		if ( dbfile.length() <= 0 )
		{
			ui->pushButtonCreate->setEnabled( false );
			return;
		}

		if ( !validDbFile( folder, dbfile ) )
		{
			ui->pushButtonCreate->setEnabled( false );
			return;
		}
	}

	ui->pushButtonCreate->setEnabled( true );
}

////////////////////////////////////////////////////////////////////////////////
bool MainWindow::validDbFile( const QString &path, const QString &file, QString *pfullpath, QString *pmsg )
{
	QString fullpath = QDir::cleanPath( path + QDir::separator() + file );

	// make sure the directory of the file exists
	QDir dir = QFileInfo(fullpath).absoluteDir();
	if ( !dir.exists() )
	{
		if ( pmsg ) *pmsg = QString( "The directory for the database file does not exist: " ) + dir.absolutePath();
		return false;
	}

	if (pfullpath) *pfullpath = fullpath;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButtonWriteToDb_clicked()
{
    ui->radioButtonWriteToDb->setChecked( true );
    ui->radioButtonWriteToFiles->setChecked( false );
	enableCreateButton();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButtonWriteToFiles_clicked()
{
    ui->radioButtonWriteToDb->setChecked(false);
    ui->radioButtonWriteToFiles->setChecked(true);
	enableCreateButton();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_pushButtonBrowseFolder_clicked()
{
    QStringList list;
    QString folderStart = ui->plainTextEditFileFolder->toPlainText();
    QFileDialog fd( this, QString( "Browse for file output folder" ), folderStart, "" );
    fd.setFileMode( QFileDialog::DirectoryOnly );
    if ( fd.exec() == QDialog::Rejected ) { return; }

    list = fd.selectedFiles();
    if ( list.count() <= 0 ) { return; }

    QString folder = list.at(0);
    ui->plainTextEditFileFolder->setPlainText( folder );
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_pushButtonVtkBrowse_clicked()
{
	if ( _pThread->isRunning() )
    {
        QMessageBox::information( NULL, "Volume Creation Running", "Volume creation is currently running.\nPlease wait until it finishes." );
        return;
    }

	QStringList list;
    QString filter = "Vtk Files (*.vtm *.vtu *.vts);;All files (*.*)";
    QFileDialog fd( this, QString("Browse for vtk file"), _lastPathVtk, filter );

    if ( fd.exec() == QDialog::Rejected ) { return; }

    list = fd.selectedFiles();
    if ( list.count() <= 0 ) { return; }


	QDir fullPath = fd.directory();
    _lastPathVtk = fullPath.absolutePath();
    ui->plainTextEditVtkFile->setPlainText( list.at(0) );

	_pLoadingDlg.reset ( new QDialog(this, Qt::FramelessWindowHint) );

	QVBoxLayout *layout = new QVBoxLayout();
	QLabel *label = new QLabel(QString("Loading vtk file. Please wait..."));
	label->setFixedSize(400, 200);

	//QFont f( "Arial", 20, QFont::Bold);
	QFont f( "Arial", 20);
	label->setFont(f);

	layout->addWidget(label, 0, Qt::AlignCenter);
	_pLoadingDlg->setLayout(layout);

	_pLoadingDlg->setAttribute(Qt::WA_TranslucentBackground);
	_pLoadingDlg->show();
    _pLoadingDlg->raise();
    _pLoadingDlg->activateWindow();

	ui->pushButtonCreate->setEnabled( false );
	ui->pushButtonVtkBrowse->setEnabled( false );

	this->setCursor(Qt::WaitCursor);
	QtConcurrent::run(this, &MainWindow::loadVtkFile, list.at(0));
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_comboBoxShape_currentIndexChanged(int index)
{
	setShapeType( ui->comboBoxShape->itemData(index).toInt() );
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_pushButtonCreate_clicked()
{
	if ( _pThread->isRunning() )
    {
        QMessageBox::information( NULL, "Volume Creation Running", "Volume creation is currently running.\nPlease wait until it finishes." );
        return;
    }

	int shapeType = UtlSettings::getSelectedValueInt (ui->comboBoxShape );
	boost::shared_ptr<CreateVolume> createVolume( new CreateVolume( logstr.c_str(), loginfo.c_str() ) );
	VolumeBrickDataPtr vbd;
	bool prune = ui->checkBoxPrune->isChecked();
	bool resPrune = ui->checkBoxResPrune->isChecked();

	switch (shapeType)
	{
	case E_SHAPE_VTK:
	{
		if ( !vtkCreator()->haveFile() )
		{
			QMessageBox::information( NULL, "No Vtk File", "Please open a vtk file first." );
			return;
		}

		std::vector<int> scalars, vectors;
		UtlSettings::getCheckedItems( ui->listWidgetVtkScalars, &scalars );
		UtlSettings::getCheckedItems( ui->listWidgetVtkVectors, &vectors );

		int threads = UtlSettings::getSelectedValueInt( ui->comboBoxVtkThreads );
		bool cacheon = ui->checkBoxVtkCacheOn->isChecked();
		bool hireslod = ui->checkBoxVtkHiresLod->isChecked();

		vtkCreator()->setScalarsToProcess( scalars );
		vtkCreator()->setVectorsToProcess( vectors );
		vtkCreator()->setThreads( threads );
		vtkCreator()->setCache( cacheon );
		vtkCreator()->setHiresLod( hireslod );
		vtkCreator()->setResPrune( resPrune );

		createVolume = _pVtk;
		break;
	}
	case E_SHAPE_CUBE:
		vbd.reset( new CubeVolumeBrickData( prune, false, resPrune ) );
		createVolume->setVolumeObj(vbd);
		break;
	case E_SHAPE_SCUBE:
		vbd.reset( new CubeVolumeBrickData( prune, true, resPrune ) );
		createVolume->setVolumeObj(vbd);
		break;
	case E_SHAPE_CONE:
		vbd.reset( new ConeVolumeBrickData (prune, resPrune ) );
		createVolume->setVolumeObj(vbd);
		break;
	case E_SHAPE_SPHERE:
		vbd.reset( new SphereVolumeBrickData( prune, false, resPrune ) );
		createVolume->setVolumeObj( vbd );
		break;
	case E_SHAPE_SSPHERE:
		vbd.reset( new SphereVolumeBrickData( prune, true, resPrune ) );
		createVolume->setVolumeObj(vbd);
		break;
	default:
		QMessageBox::information( NULL, "UnRecognized Shape Type", "UnRecognized shape type." );
		return;
	}

	int depth = UtlSettings::getSelectedValueInt( ui->comboBoxDepth );
	bool usedb = ui->radioButtonWriteToDb->isChecked();

	createVolume->setPrune( prune );
	createVolume->setResPrune( resPrune );
	createVolume->setDepth( depth );
	createVolume->setUseCrunchStore( usedb );

	QString path = ui->plainTextEditFileFolder->toPlainText();

	if ( !QDir(path).exists() ) 
	{
		QMessageBox::information( NULL, "Output Folder", "Please specify and output folder first." );
		return;
	}

	if ( usedb )
	{
		QString file = ui->plainTextEditDbFile->toPlainText();
		
		if ( file.length() <= 0 )
		{
			QMessageBox::information( NULL, "Output Database File", "You must specify a database file first." );
			return;
		}

		// make sure the directory of the file exists
		QString msg;
		if ( !validDbFile( path, file, &path, &msg ) )
		{
			QMessageBox::information( NULL, "Output Database Directory", msg );
			return;
		}
	}

	std::string strFileOrFolder = path.toStdString();
	createVolume->setCsFileOrFolder( strFileOrFolder.c_str() );

	_pThread->setCreateVolume( createVolume );
	_pThread->start();
	_pThread->setPriority( QThread::TimeCriticalPriority );
}
 
////////////////////////////////////////////////////////////////////////////////
void MainWindow::closeEvent(QCloseEvent * event)
{
	if (_pLoadingDlg != NULL)
	{
		event->ignore();

		QMessageBox::information( NULL, "Vtk File Loading", "Vtk File is currently loading.\nPlease wait until it finishes." );
	}
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_pushButtonVtkClearAll_clicked()
{
    for ( int i=0; i<ui->listWidgetVtkScalars->count(); i++ )
    {
        QListWidgetItem *item = ui->listWidgetVtkScalars->item(i);
        item->setCheckState( Qt::Unchecked );
    }

	for ( int i=0; i<ui->listWidgetVtkVectors->count(); i++ )
    {
        QListWidgetItem *item = ui->listWidgetVtkVectors->item(i);
        item->setCheckState( Qt::Unchecked );
    }
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_pushButtonVtkSelectAll_clicked()
{
	for ( int i=0; i<ui->listWidgetVtkScalars->count(); i++ )
    {
        QListWidgetItem *item = ui->listWidgetVtkScalars->item(i);
        item->setCheckState( Qt::Checked );
    }

	for ( int i=0; i<ui->listWidgetVtkVectors->count(); i++ )
    {
        QListWidgetItem *item = ui->listWidgetVtkVectors->item(i);
        item->setCheckState( Qt::Checked );
    }
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_pushButtonCancel_clicked()
{
	if ( _pThread->isRunning() )
    {
		msgOut( QString("Canceling...") );
        _pThread->cancel();
    }
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_plainTextEditFileFolder_textChanged()
{
	enableCreateButton();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_plainTextEditDbFile_textChanged()
{
	enableCreateButton();
}


////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotStart()
{
	ui->pushButtonCreate->setEnabled( false );
	ui->pushButtonCancel->setEnabled( true );
	ui->progressBar->setValue( 0 );
	msgClearAll();
	msgOut( QString("Creating bricks...") );
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotProgress(int percent )
{
	ui->progressBar->setValue( percent );
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotEnd()
{
	ui->pushButtonCreate->setEnabled( true );
	ui->pushButtonCancel->setEnabled( false );
	finishProgress();
	msgOut( QString("Finished...") );
} 

////////////////////////////////////////////////////////////////////////////////
void MainWindow::slotMsg( QString msg )
{
	msgOut(msg);
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::msgOut( const QString &msg )
{
	QString out = msg + "\n";
    out = ui->textBrowserOutput->toPlainText() + out;

    //QDateTime current = QDateTime::currentDateTime(); 
    //current.toString()
	ui->textBrowserOutput->setText( out );
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::msgClearAll()
{
    ui->textBrowserOutput->setText("");
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::finishProgress()
{
	int v = ui->progressBar->value();
	if (v < 0) v = 0;
	while ( v < 100 )
	{
		v++;
		ui->progressBar->setValue (v );
	}
}