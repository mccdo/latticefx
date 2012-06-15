/*************** <auto-copyright.rb BEGIN do not edit this line> *************
*
* latticeFX is (C) Copyright 2011-2012 by Ames Laboratory
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; 
* version 2.1 of the License.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*
* -----------------------------------------------------------------
* Date modified: $Date$
* Version:       $Rev$
* Author:        $Author$
* Id:            $Id$
* -----------------------------------------------------------------
*
*************** <auto-copyright.rb END do not edit this line> **************/

#include <latticefx/PagingThread.h>
#include <latticefx/LoadRequest.h>
#include <latticefx/DBUtils.h>
#include <osgDB/ReadFile>
#include <boost/foreach.hpp>

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
    boost::mutex::scoped_lock lock( _requestMutex );
    _haltRequest = true;
}
bool PagingThread::getHaltRequest() const
{
    boost::mutex::scoped_lock lock( _requestMutex );
    return( _haltRequest );
}

void PagingThread::addLoadRequest( LoadRequestPtr request )
{
    boost::mutex::scoped_lock lock( _requestMutex );
    _requestList.push_back( request );
}

LoadRequestPtr PagingThread::retrieveLoadRequest( const osg::NodePath& path )
{
    boost::mutex::scoped_lock lock( _availableMutex );

    LoadRequestList::iterator it( find( _availableList, path ) );
    if( it != _availableList.end() )
    {
        LoadRequestPtr result( *it );
        _availableList.erase( it );
        return( result );
    }
    return( LoadRequestPtr( (LoadRequest*)NULL ) );
}

void dump( const std::string& header, const LoadRequestList& requests )
{
    std::cout << header << std::endl;
    BOOST_FOREACH( const LoadRequestPtr req, requests )
    {
        std::cout << "\t" << req->_keys.size() << std::endl;
    }
}

void PagingThread::cancelLoadRequest( const osg::NodePath& path )
{
    boost::mutex::scoped_lock cancelLock( _cancelMutex );
    _cancelList.push_back( path );
}

void PagingThread::operator()()
{
    while( !getHaltRequest() )
    {
        LoadRequestPtr request;
        bool requestAvailable;
        {
            boost::mutex::scoped_lock requestLock( _requestMutex );
            //std::cout << "__thread " << _requestList.size() << " " << _completedList.size() <<
            //    " " << _availableList.size() << std::endl;

            // Process cancellations stored on the _cancelList. Must hold _requestMutex
            // before calling this function.
            processCancellations();

            requestAvailable = !( _requestList.empty() );
            if( requestAvailable )
            {
                request = *( _requestList.begin() );
                _requestList.pop_front();
                //std::cout << "____Got a request for " << request._dbKey << std::endl;
            }
        }

        if( requestAvailable )
        {
            request->load();
            //std::cout << "__    loaded: " << std::hex << request._loadedImage.get() << std::endl;
            _completedList.push_back( request );
        }
        else
        {
            // Sleep for just shy of 1 frame: 1/60th second (16.6667 milliseconds).
            boost::this_thread::sleep( boost::posix_time::milliseconds( 16 ) );
        }

        // Only allow 16 requests to be returned to the app for now,
        // this will throttle how many OpenGL objects get created per frame.
        int numReturnsAvailable;
        {
            boost::mutex::scoped_lock availableLock( _availableMutex );
            numReturnsAvailable = 16 - (int)( _availableList.size() );
        }
        if( numReturnsAvailable > 0 )
        {
            LoadRequestList tempList;
            while( ( --numReturnsAvailable >= 0 ) && ( !( _completedList.empty() ) ) )
            {
                tempList.push_back( *( _completedList.begin() ) );
                _completedList.pop_front();
            }
            if( tempList.size() > 0 )
            {
                boost::mutex::scoped_lock availableLock( _availableMutex );
                _availableList.insert( _availableList.end(), tempList.begin(), tempList.end() );
            }
        }
    }
}

void PagingThread::processCancellations()
{
    boost::mutex::scoped_lock cancelLock( _cancelMutex );
    if( _cancelList.empty() )
        return;

    // Calling code must lock the _requestMutex.
    //boost::mutex::scoped_lock requestLock( _requestMutex );

    boost::mutex::scoped_lock availableLock( _availableMutex );

    BOOST_FOREACH( const osg::NodePath& path, _cancelList )
    {
        LoadRequestList::iterator it;
        if( ( it = find( _requestList, path ) ) != _requestList.end() )
            _requestList.erase( it );
        else if( ( it = find( _completedList, path ) ) != _completedList.end() )
            _completedList.erase( it );
        else if( ( it = find( _availableList, path ) ) != _availableList.end() )
            _availableList.erase( it );
        else
            // Couldn't find the specified path. This is an error. It means the client code
            // requested cancellation for a LoadRequest that the PagingThread doesn't posess.
            std::cout << "PagingThread::processCancellations(): Couldn't cancel path." << std::endl;
    }

    _cancelList.clear();
}

void PagingThread::setModelView( const osg::Matrix& mv )
{
    boost::mutex::scoped_lock cancelLock( _transformMutex );
    _mv = mv;
}
void PagingThread::setTransforms( const osg::Camera* camera )
{
    setTransforms( camera->getViewMatrix(),
        camera->getProjectionMatrix(),
        camera->getViewport() );
}
void PagingThread::setTransforms( const osg::Matrix& mv, const osg::Matrix& proj, const osg::Viewport* vp )
{
    boost::mutex::scoped_lock cancelLock( _transformMutex );
    _mv = mv;
    _proj = proj;
    _vp = vp;
}
void PagingThread::getTransforms( osg::Matrix& mv, osg::Matrix& proj,
    osg::ref_ptr< const osg::Viewport >& vp ) const
{
    boost::mutex::scoped_lock cancelLock( _transformMutex );
    mv = _mv;
    proj = _proj;
    vp = _vp;
}



// static
LoadRequestList::iterator PagingThread::find( LoadRequestList& requestList, const osg::NodePath& path )
{
    LoadRequestList::iterator it;
    for( it = requestList.begin(); it != requestList.end(); ++it )
    {
        if( (*it)->_path == path )
            break;
    }
    return( it );
}


// lfx
}
