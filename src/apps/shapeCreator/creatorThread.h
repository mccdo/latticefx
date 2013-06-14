#ifndef CREATORTHREAD_H
#define CREATORTHREAD_H

#include <QThread>
#include "CreateVolume.h"

class CreatorThread : public QThread
{
    Q_OBJECT
public:
    explicit CreatorThread(QObject *parent = 0);
    
	void run();

	void setCreateVolume(boost::shared_ptr<CreateVolume> createVolume) { _createVolume = createVolume; }

signals:
	void signalTestStart();
    void signalTestProgress(float percent);
    void signalTestEnd();
    void signalTestMsg(QString msg);
    
public slots:
	void slotOnCancel();

protected:
	bool checkCancel();

protected:
	bool _cancel;
	boost::shared_ptr<CreateVolume> _createVolume;
    
};

#endif // CREATORTHREAD_H
