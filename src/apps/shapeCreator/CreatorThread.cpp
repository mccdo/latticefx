#include "CreatorThread.h"


//using namespace lfx::core;

////////////////////////////////////////////////////////////////////////////////
CreatorThread::CreatorThread( QObject *parent ) :
    QThread( parent )
{
	_cancel = false;
}

////////////////////////////////////////////////////////////////////////////////
void CreatorThread::run()
{
	_cancel = false;

	if (_createVolume == NULL) return;

	_createVolume->setCallbackProgress( this );
	emit signalStart(); 

	try
	{
		_createVolume->create();
	}
	catch ( std::exception ex )
	{
		sendMsg( "Unexpected exception.. brick creation aborted." );
	}

	if (checkCancel())
	{
		sendMsg( "Brick creation canceled." );
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
void CreatorThread::updateProgress( int percent )
{
	emit signalProgress( percent );
}

////////////////////////////////////////////////////////////////////////////////
void CreatorThread::sendMsg( const char* msg )
{
	QString qmsg( msg );
	emit signalMsg( qmsg );
}