
#include <latticefx/PagingThread.h>
#include <osgDB/ReadFile>

#include <iostream>


namespace lfx {


PagingThread::PagingThread()
  : _haltRequest( false )
{
    _thread = new boost::thread( boost::ref( *this ) );
}
PagingThread* PagingThread::instance()
{
    static PagingThread* s_pagingThreadInstance( new PagingThread );
    return( s_pagingThreadInstance );
}
PagingThread::~PagingThread()
{
}

void PagingThread::halt()
{
    boost::mutex::scoped_lock( _requestMutex );
    _haltRequest = true;
}
bool PagingThread::getHaltRequest() const
{
    boost::mutex::scoped_lock( _requestMutex );
    return( _haltRequest );
}

void PagingThread::addLoadRequest( osg::Node* location, const std::string& fileName )
{
    boost::mutex::scoped_lock( _requestMutex );
    _loadRequestList.push_back( LoadRequest( location, fileName ) );
}

osg::Node* PagingThread::retrieveRequest( const osg::Node* location )
{
    boost::mutex::scoped_lock( _retrieveMutex );

    LoadRequestList::iterator it;
    for( it = _returnList.begin(); it != _returnList.end(); ++it )
    {
        if( it->_location == location )
        {
            osg::ref_ptr< osg::Node > returnValue = it->_loadedModel;
            _returnList.erase( it );
            std::cout << "__  Retreiving: " << std::hex << returnValue.get() << std::endl;
            return( returnValue.release() );
        }
    }
    return( NULL );
}

void PagingThread::operator()()
{
    while( !getHaltRequest() )
    {
        bool requestAvailable;
        {
            boost::mutex::scoped_lock( _requestMutex );
            std::cout << "__thread " << _loadRequestList.size() << " " << _completedList.size() <<
                " " << _returnList.size() << std::endl;
            requestAvailable = !( _loadRequestList.empty() );
        }

        if( requestAvailable )
        {
            LoadRequest request;
            {
                boost::mutex::scoped_lock( _requestMutex );
                request = *( _loadRequestList.begin() );
                _loadRequestList.pop_front();
                std::cout << "____Got a request for " << request._fileName << std::endl;
            }

            if( !( request._fileName.empty() ) )
            {
                request._loadedModel = osgDB::readNodeFile( request._fileName );
                std::cout << "__    loaded: " << std::hex << request._loadedModel.get() << std::endl;
                _completedList.push_back( request );
            }
            // else if dbKey valid
            // TBD
        }
        else
        {
            // Sleep for just shy of 1 frame: 1/60th second (16.6667 milliseconds).
            boost::this_thread::sleep( boost::posix_time::milliseconds( 16 ) );
        }

        // Only allow one request to be returned to the app for now,
        // this will throttle how many OpenGL objects get created per frame.
        if( !( _completedList.empty() ) && _returnList.empty() )
        {
            LoadRequest completed = *( _completedList.begin() );
            _completedList.pop_front();
            boost::mutex::scoped_lock( _retrieveMutex );
            _returnList.push_back( completed );
        }
    }
}




PagingThread::LoadRequest::LoadRequest()
  : _location( NULL ),
    _fileName( std::string( "" ) )
    // _dbKey( 0 )
{
}
PagingThread::LoadRequest::LoadRequest( osg::Node* location, const std::string& fileName )
  : _location( location ),
    _fileName( fileName )
    // _dbKey( 0 )
{
}
PagingThread::LoadRequest::LoadRequest( osg::Node* location, const int dbKey )
  : _location( location ),
    _fileName( std::string( "" ) )
    // dbKey is TBD
{
}

PagingThread::LoadRequest& PagingThread::LoadRequest::operator=( const PagingThread::LoadRequest& rhs )
{
    _location = rhs._location;
    _fileName = rhs._fileName;
    // dbKey is TBD.

    return( *this );
}


// lfx
}
