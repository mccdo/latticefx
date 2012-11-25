/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * Copyright 2012-2012 by Ames Laboratory
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2.1 as published by the Free Software Foundation.
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
 *************** <auto-copyright.rb END do not edit this line> ***************/

#include <latticefx/core/PagingThread.h>
#include <latticefx/core/LoadRequest.h>
#include <latticefx/core/LogMacros.h>

#include <osgDB/ReadFile>
#include <boost/foreach.hpp>

#include <iostream>
#include <sstream>


namespace lfx {
namespace core {


PagingThread::PagingThread()
  : LogBase( "lfx.core.page.thread" ),
    _haltRequest( false )
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

void PagingThread::dump( const std::string& header, const LoadRequestList& requests )
{
    if( !( LFX_LOG_CRITICAL ) )
        return;

    LFX_CRITICAL( header );
    BOOST_FOREACH( const LoadRequestPtr req, requests )
    {
        LFX_CRITICAL( "\t" + req->_keys.size() );
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
            if( LFX_LOG_TRACE )
            {
                if( !_requestList.empty() || !_completedList.empty() || !_availableList.empty() )
                {
                    std::ostringstream ostr;
                    ostr << "  queues: " << _requestList.size() << " " << _completedList.size() <<
                        " " << _availableList.size();
                    LFX_TRACE( ostr.str() );
                }
            }

            // Process cancellations stored on the _cancelList. Must hold _requestMutex
            // before calling this function.
            processCancellations();

            requestAvailable = !( _requestList.empty() );
            if( requestAvailable )
            {
                request = *( _requestList.begin() );
                _requestList.pop_front();
            }
        }

        if( requestAvailable )
        {
            request->load();
            if( LFX_LOG_TRACE )
            {
                std::ostringstream ostr;
                ostr << "      loaded " << request->_keys.size() << " images, including " << request->_keys.front();
                LFX_TRACE( ostr.str() );
            }
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
        {
            // Couldn't find the specified path. This is an error. It means the client code
            // requested cancellation for a LoadRequest that the PagingThread doesn't posess.
            LFX_ERROR( "processCancellations(): Couldn't cancel path." );
        }
    }

    _cancelList.clear();
}

void PagingThread::setTransforms( const osg::Vec3& wcEyePosition )
{
    boost::mutex::scoped_lock transformLock( _transformMutex );
    _wcEyePosition = wcEyePosition;
}
void PagingThread::setTransforms( const osg::Matrix& proj, const osg::Viewport* vp )
{
    boost::mutex::scoped_lock transformLock( _transformMutex );
    _proj = proj;
    _vp = vp;
}
void PagingThread::setTransforms( const osg::Vec3& wcEyePosition, const osg::Matrix& proj,
        const osg::Viewport* vp )
{
    boost::mutex::scoped_lock transformLock( _transformMutex );
    _wcEyePosition = wcEyePosition;
    _proj = proj;
    _vp = vp;
}
void PagingThread::getTransforms( osg::Vec3& wcEyePosition, osg::Matrix& proj,
    osg::ref_ptr< const osg::Viewport >& vp ) const
{
    boost::mutex::scoped_lock transformLock( _transformMutex );
    wcEyePosition = _wcEyePosition;
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


// core
}
// lfx
}
