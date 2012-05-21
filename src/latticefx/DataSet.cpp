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

#include <latticefx/DataSet.h>
#include <latticefx/ChannelDataComposite.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/RootCallback.h>
#include <latticefx/DBUtils.h>

#include <osg/Group>
#include <osg/Notify>

#include <boost/foreach.hpp>


namespace lfx {


DataSet::DataSet()
  : _sceneGraph( new osg::Group ),
    _dirtyFlags( ALL_DIRTY )
{
    RootCallback* rootcb( new RootCallback() );
    _sceneGraph->setUpdateCallback( rootcb );

    PageData* pageData( new PageData );
    pageData->setRangeMode( PageData::TIME_RANGE );
    pageData->setParent( _sceneGraph.get() );
    _sceneGraph->setUserData( pageData );
}
DataSet::DataSet( const DataSet& rhs )
  : _data( rhs._data ),
    _dataNames( rhs._dataNames ),
    _preprocess( rhs._preprocess ),
    _ops( rhs._ops ),
    _renderer( rhs._renderer ),
    _maskList( rhs._maskList ),
    _dirtyFlags( ALL_DIRTY )
{
}
DataSet::~DataSet()
{
}

void DataSet::addChannel( const ChannelDataPtr channel, const double time )
{
    _data[ time ].push_back( channel );
    _dataNames.insert( channel->getName() );
    setDirty( ALL_DIRTY );
}
ChannelDataPtr DataSet::getChannel( const std::string& name, const double time )
{
    ChannelDataPtr cdp;
    ChannelDataTimeMap::const_iterator cdlIt( _data.find( time ) );
    if( cdlIt != _data.end() )
        cdp = findChannelData( name, cdlIt->second );

    if( cdp == NULL )
    {
        // No data found for this exact time. Search data prior to the specified
        // time by starting at the end of time and searching backwards.
        ChannelDataTimeMap::const_reverse_iterator crt( _data.rbegin() );
        while( crt != _data.rend() )
        {
            if( crt->first < time )
            {
                // Found some data just before time.
                cdp = findChannelData( name, crt->second );
                if( cdp != NULL )
                    break;
            }
            ++crt;
        }
        // If we got here, we never found any data with the given name
        // and cdp is NULL, which is what we will return.
    }
    return( cdp );
}
const ChannelDataPtr DataSet::getChannel( const std::string& name, const double time ) const
{
    DataSet* nonConstThis = const_cast< DataSet* >( this );
    return( nonConstThis->getChannel( name, time ) );
}

osg::Vec2d DataSet::getTimeRange() const
{
    double minTime, maxTime;
    getTimeRange( minTime, maxTime );
    return( osg::Vec2d( minTime, maxTime ) );
}
void DataSet::getTimeRange( double& minTime, double& maxTime ) const 
{
    TimeSet timeSet( getTimeSet() );
    if( timeSet.empty() )
    {
        minTime = maxTime = 0.;
        return;
    }
    minTime = *( timeSet.begin() );
    maxTime = *( timeSet.rbegin() );
}



void DataSet::addOperation( const RTPOperationPtr op )
{
    _ops.push_back( op );
    setDirty( RTP_DIRTY );
}



void DataSet::setRenderer( const RendererPtr renderer )
{
    _renderer = renderer;
}
RendererPtr DataSet::getRenderer()
{
    return( _renderer );
}
const RendererPtr DataSet::getRenderer() const
{
    return( _renderer );
}

osg::Node* DataSet::getSceneData()
{
    updateSceneGraph();

    return( _sceneGraph.get() );
}

bool DataSet::updateSceneGraph()
{
    if( _dirtyFlags == NOT_DIRTY )
        return( true );

    // Reset all attached inputs. If a ChannelData instance needs to refresh
    // a working copy of data from the original source data, it does so here.
    BOOST_FOREACH( ChannelDataTimeMap::value_type channelTimePair, _data )
    {
        BOOST_FOREACH( ChannelDataPtr cdp, channelTimePair.second )
        {
            cdp->reset();
        }
    }


    // Preprocess & Cache (if dirty)
    if( _dirtyFlags & PREPROCESS_DIRTY )
    {
        updatePreprocessing();
    }

    // Run Time Operations (if dirty)
    if( _dirtyFlags & RTP_DIRTY )
    {
        updateRunTimeProcessing();
    }

    // Rendering Framework support
    if( ( _dirtyFlags & ALL_DIRTY ) != 0 )
    {
        _sceneGraph->removeChildren( 0, _sceneGraph->getNumChildren() );
        updateRenderer();
    }

    _dirtyFlags = NOT_DIRTY;
    return( true );
}

bool DataSet::updatePreprocessing()
{
    // Not yet implemented.
    return( true );
}
bool DataSet::updateRunTimeProcessing()
{
    // Create a new list of masks (one for each time step).
    _maskList.clear();

    // Iterate over all time steps.
    TimeSet timeSet( getTimeSet() );
    BOOST_FOREACH( double time, timeSet )
    {
        // Get the data at the current time.
        ChannelDataList currentData( getDataAtTime( time ) );

        // Allocate a mask and initialize to all zeros.
        ChannelDataOSGArrayPtr mask( createSizedMask( currentData ) );
        mask->setAll( (const char)1 ); // Enable all elements.
        _maskList.push_back( mask );

        // Iterate over all attached run time operations.
        BOOST_FOREACH( RTPOperationPtr opPtr, _ops )
        {
            if( !( opPtr->getEnable() ) )
                continue;

            // Assign actual / current data to the opPtr OperationBase.
            setInputs( opPtr, currentData );

            switch( opPtr->getRTPOpType() )
            {
            case RTPOperation::Mask:
            {
                const ChannelDataPtr newMask = opPtr->mask( mask );
                mask->andValues( newMask.get() );
                break;
            }
            case RTPOperation::Filter:
            {
                opPtr->filter( mask );
                break;
            }
            case RTPOperation::Channel:
            {
                ChannelDataPtr cdp = opPtr->channel( mask );
                addChannel( cdp, time );
                break;
            }
            }
        }
    }
    return( true );
}
bool DataSet::updateRenderer()
{
    if( _renderer != NULL )
    {
        TimeSet timeSet( getTimeSet() );
        if( timeSet.size() > 1 )
        {
            RootCallback* rootcb( static_cast< RootCallback* >( _sceneGraph->getUpdateCallback() ) );
            rootcb->addTimeSeriesParent( _sceneGraph.get() );
            PageData* pageData( static_cast< PageData* >( _sceneGraph->getUserData() ) );
            pageData->setRangeMode( lfx::PageData::TIME_RANGE );
            unsigned int childIndex( 0 );
            osg::ref_ptr< osg::Group > stubGroup( new osg::Group );

            ChannelDataList::iterator maskIt = _maskList.begin();
            BOOST_FOREACH( double time, timeSet )
            {
                // Get the data at the current time and assign as inputs to the Renderer.
                ChannelDataList currentData( getDataAtTime( time ) );
                setInputs( _renderer, currentData );

                osg::Node* newChild( _renderer->getSceneGraph( *maskIt ) );
                if( newChild != NULL )
                {
                    PageData::RangeData rangeData( time, 0. );
                    rangeData._status = PageData::RangeData::UNLOADED;
                    rangeData._dbKey = generateDBKey();
                    pageData->setRangeData( childIndex++, rangeData );

                    if( _sceneGraph->getNumChildren() == 0 )
                    {
                        // Always add the first child
                        _sceneGraph->addChild( newChild );
                        rangeData._status = PageData::RangeData::ACTIVE;
                    }
                    else
                    {
                        // Add stub Group node for paging.
                        _sceneGraph->addChild( stubGroup.get() );
                    }

                    storeSubGraph( newChild, rangeData._dbKey );
                }
                ++maskIt;
            }
        }
        else
        {
            // Simplified scene graph creation when not using time series.
            ChannelDataList currentData( getDataAtTime( 0. ) );
            setInputs( _renderer, currentData );

            ChannelDataList::iterator maskIt = _maskList.begin();
            const double time( *( timeSet.begin() ) );
            osg::Node* newChild( _renderer->getSceneGraph( *maskIt ) );
            if( newChild != NULL )
                _sceneGraph->addChild( newChild );
        }

        _sceneGraph->setStateSet( _renderer->getRootState() );

        return( true );
    }
    return( false );
}


void DataSet::setDirty( const DirtyFlags flags )
{
    _dirtyFlags = flags;
}
DataSet::DirtyFlags DataSet::getDirty() const
{
    // TBD query the Renderer to see if it's dirty.
    return( _dirtyFlags );
}

TimeSet DataSet::getTimeSet() const
{
    TimeSet timeSet;
    BOOST_FOREACH( ChannelDataTimeMap::value_type channelTimePair, _data )
    {
        timeSet.insert( channelTimePair.first );
    }
    return( timeSet );
}

ChannelDataList DataSet::getDataAtTime( const double time )
{
    ChannelDataList cdl;
    BOOST_FOREACH( const std::string& name, _dataNames )
    {
        /*
        ChannelDataPtr cdp( getChannel( name, time ) );
        if( cdp != NULL )
            // It might be NULL if the RTP operations contain a channel creator.
            cdl.push_back( cdp );
            */
        cdl.push_back( getChannel( name, time ) );
    }
    return( cdl );
}

void DataSet::setInputs( OperationBasePtr opPtr, ChannelDataList& currentData )
{
    ChannelDataList newList;
    const OperationBase::StringList& inputs( opPtr->getInputNames() );
    BOOST_FOREACH( const std::string& inputName, inputs )
    {
        ChannelDataPtr cdp( lfx::findChannelData( inputName, currentData ) );
        if( cdp == NULL )
            OSG_WARN << "DataSet::setInputs(): Could not find data named \"" << inputName << "\"." << std::endl;
        newList.push_back( cdp );
    }
    opPtr->setInputs( newList );
}


ChannelDataOSGArrayPtr DataSet::createSizedMask( const ChannelDataList& dataList )
{
    unsigned int size( 0 );

    BOOST_FOREACH( const ChannelDataPtr cdp, dataList )
    {
        unsigned int x, y, z;
        cdp->getDimensions( x, y, z );
        const unsigned int total( x*y*z );
        size = osg::maximum( size, total );
    }

    osg::ByteArray* byteArray( new osg::ByteArray );
    byteArray->resize( size );
    return( ChannelDataOSGArrayPtr( new ChannelDataOSGArray( byteArray ) ) );
}


// lfx
}
