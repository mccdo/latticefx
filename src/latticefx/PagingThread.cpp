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
#include <latticefx/DBUtils.h>
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
    boost::mutex::scoped_lock lock( _requestMutex );
    _haltRequest = true;
}
bool PagingThread::getHaltRequest() const
{
    boost::mutex::scoped_lock lock( _requestMutex );
    return( _haltRequest );
}

void PagingThread::addLoadRequest( osg::Node* location, const DBKey& dbKey )
{
    boost::mutex::scoped_lock lock( _requestMutex );
    _loadRequestList.push_back( LoadRequest( location, dbKey ) );
}
void PagingThread::addLoadRequest( const LoadRequestList& requests )
{
    boost::mutex::scoped_lock lock( _requestMutex );
    _loadRequestList.insert( _loadRequestList.end(), requests.begin(), requests.end() );
}

osg::Node* PagingThread::retrieveRequest( const osg::Node* location )
{
    return( retrieveAndRemove( location, _returnList, _retrieveMutex ) );
}
bool PagingThread::debugChechReturnsEmpty()
{
    boost::mutex::scoped_lock lock( _retrieveMutex );
    if( _returnList.size() > 0 )
    {
        std::cout << "debugCheckReturnsEmpty(): List size: " << _returnList.size() << " should be zero." << std::endl;
        return( false );
    }
    return( true );
}

bool PagingThread::cancelLoadRequest( const osg::Node* location )
{
    if( retrieveAndRemove( location, _loadRequestList, _requestMutex ) != NULL )
        return( true );
    else if( retrieveAndRemove( location, _completedList, _completedMutex ) != NULL )
        return( true );
    else if( retrieveAndRemove( location, _returnList, _retrieveMutex ) != NULL )
        return( true );
    else
        return( false );
}

void PagingThread::operator()()
{
    while( !getHaltRequest() )
    {
        bool requestAvailable;
        {
            boost::mutex::scoped_lock( _requestMutex );
            //std::cout << "__thread " << _loadRequestList.size() << " " << _completedList.size() <<
            //    " " << _returnList.size() << std::endl;
            requestAvailable = !( _loadRequestList.empty() );
        }

        if( requestAvailable )
        {
            LoadRequest request;
            {
                boost::mutex::scoped_lock( _requestMutex );
                request = *( _loadRequestList.begin() );
                _loadRequestList.pop_front();
                //std::cout << "____Got a request for " << request._dbKey << std::endl;
            }

            if( !( request._dbKey.empty() ) )
            {
                request._loadedModel = loadSubGraph( request._dbKey );
                //std::cout << "__    loaded: " << std::hex << request._loadedModel.get() << std::endl;
                boost::mutex::scoped_lock completedLock( _completedMutex );
                _completedList.push_back( request );
            }
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
            boost::mutex::scoped_lock lock( _retrieveMutex );
            numReturnsAvailable = 16 - (int)( _returnList.size() );
        }
        if( numReturnsAvailable > 0 )
        {
            LoadRequestList tempList;
            {
                boost::mutex::scoped_lock lock( _completedMutex );
                while( ( --numReturnsAvailable >= 0 ) && ( !( _completedList.empty() ) ) )
                {
                    tempList.push_back( *( _completedList.begin() ) );
                    _completedList.pop_front();
                }
            }
            if( tempList.size() > 0 )
            {
                boost::mutex::scoped_lock lock( _retrieveMutex );
                _returnList.insert( _returnList.end(), tempList.begin(), tempList.end() );
            }
        }
    }
}


osg::Node* PagingThread::retrieveAndRemove( const osg::Node* location,
        LoadRequestList& theList, boost::mutex& theMutex )
{
    boost::mutex::scoped_lock lock( theMutex );

    LoadRequestList::iterator it;
    for( it = theList.begin(); it != theList.end(); ++it )
    {
        if( it->_location == location )
        {
            osg::ref_ptr< osg::Node > returnValue = it->_loadedModel;
            theList.erase( it );
            return( returnValue.release() );
        }
    }
    return( NULL );
}





PagingThread::LoadRequest::LoadRequest()
  : _location( NULL ),
    _dbKey( DBKey( "" ) )
{
}
PagingThread::LoadRequest::LoadRequest( osg::Node* location, const DBKey& dbKey )
  : _location( location ),
    _dbKey( dbKey )
{
}

PagingThread::LoadRequest& PagingThread::LoadRequest::operator=( const PagingThread::LoadRequest& rhs )
{
    _location = rhs._location;
    _dbKey = rhs._dbKey;

    _loadedModel = rhs._loadedModel;

    return( *this );
}


// lfx
}
