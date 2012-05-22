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

void PagingThread::addLoadRequest( const LoadRequestList& requests )
{
    boost::mutex::scoped_lock lock( _requestMutex );
    _requestList.insert( _requestList.end(), requests.begin(), requests.end() );
}

PagingThread::LoadRequestList PagingThread::retrieveLoadRequests( const DBKeyList& keyList )
{
    boost::mutex::scoped_lock lock( _availableMutex );

    LoadRequestList returns;
    BOOST_FOREACH( const DBKey& key, keyList )
    {
        LoadRequestList::iterator it( find( _availableList, key ) );
        if( it != _availableList.end() )
        {
            returns.push_back( *it );
            _availableList.erase( it );
        }
    }
    return( returns );
}
bool PagingThread::debugChechAvailableEmpty()
{
    boost::mutex::scoped_lock lock( _availableMutex );
    if( _availableList.size() > 0 )
    {
        std::cout << "debugCheckAvailableEmpty(): List size: " << _availableList.size() << " should be zero." << std::endl;
        return( false );
    }
    return( true );
}

bool PagingThread::cancelLoadRequest( const DBKey& dbKey )
{
    LoadRequestList::iterator it;
    boost::mutex::scoped_lock lock( _requestMutex );
    {
        boost::mutex::scoped_lock lock( _completedMutex );
        {
            boost::mutex::scoped_lock lock( _availableMutex );
            if( ( it = find( _availableList, dbKey ) ) != _availableList.end() )
            {
                _availableList.erase( it );
                return( true );
            }
        }

        if( ( it = find( _completedList, dbKey ) ) != _completedList.end() )
        {
            _completedList.erase( it );
            return( true );
        }
    }

    if( ( it = find( _requestList, dbKey ) ) != _requestList.end() )
    {
        _requestList.erase( it );
        return( true );
    }
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
            //std::cout << "__thread " << _requestList.size() << " " << _completedList.size() <<
            //    " " << _availableList.size() << std::endl;
            requestAvailable = !( _requestList.empty() );
        }

        if( requestAvailable )
        {
            LoadRequest request;
            {
                boost::mutex::scoped_lock( _requestMutex );
                request = *( _requestList.begin() );
                _requestList.pop_front();
                //std::cout << "____Got a request for " << request._dbKey << std::endl;
            }

            request._loadedModel = loadSubGraph( request._dbKey );
            //std::cout << "__    loaded: " << std::hex << request._loadedModel.get() << std::endl;
            {
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
            boost::mutex::scoped_lock lock( _availableMutex );
            numReturnsAvailable = 16 - (int)( _availableList.size() );
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
                boost::mutex::scoped_lock lock( _availableMutex );
                _availableList.insert( _availableList.end(), tempList.begin(), tempList.end() );
            }
        }
    }
}




PagingThread::LoadRequest::LoadRequest()
  : _childIndex( 0 ),
    _dbKey( DBKey( "" ) )
{
}
PagingThread::LoadRequest::LoadRequest( const unsigned int childIndex, const DBKey& dbKey )
  : _childIndex( childIndex ),
    _dbKey( dbKey )
{
}


PagingThread::LoadRequest& PagingThread::LoadRequest::operator=( const PagingThread::LoadRequest& rhs )
{
    _childIndex = rhs._childIndex;
    _dbKey = rhs._dbKey;

    _loadedModel = rhs._loadedModel;

    return( *this );
}

PagingThread::LoadRequestList::iterator PagingThread::find( LoadRequestList& requestList, const DBKey& dbKey )
{
    LoadRequestList::iterator it;
    for( it = requestList.begin(); it != requestList.end(); ++it )
    {
        if( it->_dbKey == dbKey )
            break;
    }
    return( it );
}
PagingThread::LoadRequestList::iterator PagingThread::find( LoadRequestList& requestList, const unsigned int childIndex )
{
    LoadRequestList::iterator it;
    for( it = requestList.begin(); it != requestList.end(); ++it )
    {
        if( it->_childIndex == childIndex )
            break;
    }
    return( it );
}


// lfx
}
