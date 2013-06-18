#include "creatorThread.h"


//using namespace lfx::core;

////////////////////////////////////////////////////////////////////////////////
CreatorThread::CreatorThread(QObject *parent) :
    QThread(parent)
{
	_cancel = false;
}

////////////////////////////////////////////////////////////////////////////////
void CreatorThread::run()
{
	_cancel = false;

	if (_createVolume == NULL) return;

	//_createVolume->setCallbackProgress(this);
	emit signalStart(); 

	try
	{
		_createVolume->create();
	}
	catch (std::exception ex)
	{
	}

	emit signalEnd();
}

/*
////////////////////////////////////////////////////////////////////////////////
void CreatorThread::slotOnCancel()
{
	_cancel = true;
}
*/

////////////////////////////////////////////////////////////////////////////////
bool CreatorThread::checkCancel()
{
	return _cancel;
}

////////////////////////////////////////////////////////////////////////////////
void CreatorThread::updateProgress(float percent)
{
	emit signalProgress(percent);
}