#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include "VtkCreator.h"
#include "CreatorThread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
	void loadVtkFile( QString file );

	virtual void closeEvent(QCloseEvent * event);

protected:
    void guiSettingsInit();
    void guiSettingsLoad();
    void guiSettingsSave();
    void guiFeaturesInit();

	void setShapeType( int type );

	VtkCreator* vtkCreator();

	void enableCreateButton();
	bool validDbFile( const QString &path, const QString &file, QString *pfullpath=NULL, QString *pmsg=NULL );

	void msgOut( const QString &msg );
	void msgClearAll();

	void finishProgress();

private:
    Ui::MainWindow *ui;
    QString _settingsFile;
    QString _lastPathVtk;
	boost::shared_ptr<CreateVolume> _pVtk;
	CreatorThread *_pThread;
	boost::shared_ptr<QDialog> _pLoadingDlg;


protected:
    enum ShapeType
    {
        E_SHAPE_VTK = 1,
        E_SHAPE_CUBE,
        E_SHAPE_SCUBE,
        E_SHAPE_CONE,
        E_SHAPE_SPHERE,
        E_SHAPE_SSPHERE,
    };

signals:
	void signalVtkLoadDone();

private slots:
    void on_radioButtonWriteToDb_clicked();
    void on_radioButtonWriteToFiles_clicked();
    void on_pushButtonBrowseFolder_clicked();
    void on_pushButtonVtkBrowse_clicked();
    void on_comboBoxShape_currentIndexChanged(int index);
    void on_pushButtonCreate_clicked();
    void on_pushButtonCancel_clicked();
	void on_plainTextEditFileFolder_textChanged();
	void on_plainTextEditDbFile_textChanged();

	void on_pushButtonVtkClearAll_clicked();
	void on_pushButtonVtkSelectAll_clicked();


	void slotStart();
    void slotProgress(int percent);
    void slotEnd();
    void slotMsg(QString msg);

	void slotVtkLoadDone();
};

#endif // MAINWINDOW_H
