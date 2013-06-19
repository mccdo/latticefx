#ifndef CREATORTHREAD_H
#define CREATORTHREAD_H

#include <QThread>
#include "CreateVolume.h"

#include <latticefx/core/HierarchyUtils.h>

class CreatorThread : public QThread, public lfx::core::ICallbackProgress
{
    Q_OBJECT
public:
    explicit CreatorThread(QObject *parent = 0);
    
	void setCreateVolume(boost::shared_ptr<CreateVolume> createVolume) { _createVolume = createVolume; }
	void cancel() { _cancel = true; }

	virtual bool checkCancel();
	virtual void updateProgress(int percent);
	virtual void sendMsg(const char* msg);

signals:
	void signalStart();
    void signalProgress(int percent);
    void signalEnd();
    void signalMsg(QString msg);
    
public slots:
	//void slotOnCancel();

protected:
	virtual void run();

protected:
	bool _cancel;
	boost::shared_ptr<CreateVolume> _createVolume;
    
};

#endif // CREATORTHREAD_H
