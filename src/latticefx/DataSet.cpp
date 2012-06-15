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
#include <latticefx/ChannelDataImageSet.h>
#include <latticefx/ChannelDataLOD.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/RootCallback.h>
#include <latticefx/DBUtils.h>

#include <osg/Group>
#include <osg/Notify>

#include <boost/foreach.hpp>


namespace lfx {


DataSet::DataSet()
  : _sceneGraph( new osg::Group ),
    _dirtyFlags( ALL_DIRTY ),
    _sceneGraphPagesTexturesOnly( false )
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
    _dirtyFlags( ALL_DIRTY ),
    _sceneGraphPagesTexturesOnly( rhs._sceneGraphPagesTexturesOnly )
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
void DataSet::replaceChannel( const ChannelDataPtr channel, const double time )
{
    ChannelDataTimeMap::iterator cdlIt( _data.find( time ) );
    if( cdlIt == _data.end() )
    {
        // No data for this time at all. Just add 'channel' at this 'time'.
        addChannel( channel, time );
        return;
    }
    // Found a ChannelDataList for the specified 'time'. Now replace any
    // existing ChannelData with 'channel' (or append 'channel' to the end).
    cdlIt->second.replaceData( channel );
    setDirty( ALL_DIRTY );
}

ChannelDataPtr DataSet::getChannel( const std::string& name, const double time )
{
    ChannelDataPtr cdp;
    ChannelDataTimeMap::iterator cdlIt( _data.find( time ) );
    if( cdlIt != _data.end() )
        cdp = cdlIt->second.findData( name );

    if( cdp == NULL )
    {
        // No data found for this exact time. Search data prior to the specified
        // time by starting at the end of time and searching backwards.
        ChannelDataTimeMap::reverse_iterator crt( _data.rbegin() );
        while( crt != _data.rend() )
        {
            if( crt->first < time )
            {
                // Found some data just before time.
                cdp = crt->second.findData( name );
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



void DataSet::addPreprocess( const PreprocessPtr pre )
{
    _preprocess.push_back( pre );
    setDirty( PREPROCESS_DIRTY );
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
    updateAll();

    return( _sceneGraph.get() );
}

bool DataSet::updateAll()
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
        if( _sceneGraphPagesTexturesOnly )
            updateRendererPagingTexturesOnly();
        else
            updateRenderer();
    }

    _dirtyFlags = NOT_DIRTY;
    return( true );
}

bool DataSet::updatePreprocessing()
{
    TimeSet timeSet( getTimeSet() );
    if( timeSet.empty() )
        return( true );

    // Iterate over all time steps.
    BOOST_FOREACH( double time, timeSet )
    {
        // Get the data at the current time.
        ChannelDataList currentData( getDataAtTime( time ) );

        // Iterate over all attached preprocessing & caching objects.
        BOOST_FOREACH( PreprocessPtr prePtr, _preprocess )
        {
            if( !( prePtr->getEnable() ) )
                continue;

            // Assign actual / current data to the prePtr OperationBase.
            setInputs( prePtr, currentData );

            ChannelDataPtr newData( (*prePtr)() );

            // The Proprocessor object tells us how to handle the new data
            // we just got back. This is so that apps can configure the Preprocessor
            // directly. In the future, we might have to support adding the returned
            // data to the DB.
            switch( prePtr->getActionType() )
            {
            case Preprocess::ADD_DATA:
                addChannel( newData, time );
                break;
            case Preprocess::REPLACE_DATA:
                replaceChannel( newData, time );
                break;
            default:
            case Preprocess::IGNORE_DATA:
                // No-op. Do nothing.
                break;
            }
        }
    }

    return( true );
}
bool DataSet::updateRunTimeProcessing()
{
    // Create a new list of masks (one for each time step).
    _maskList.clear();

    TimeSet timeSet( getTimeSet() );
    if( timeSet.empty() )
    {
        // Not the typical case.
        return( true );
    }

    // Iterate over all time steps.
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

bool DataSet::updateRendererPagingTexturesOnly()
{
    if( _renderer == NULL )
        return( false );

    if( _maskList.empty() )
        createFallbackMaskList();

    TimeSet timeSet( getTimeSet() );
    if( timeSet.size() == 1 )
    {
        // Simplified scene graph creation when not using time series.
        const double time( timeSet.empty() ? 0. : *( timeSet.begin() ) );
        ChannelDataList currentData( getDataAtTime( time ) );
        ChannelDataList::iterator maskIt = _maskList.begin();

        osg::ref_ptr< osg::Node > newChild( recurseGetSceneGraphPagingTexturesOnly( currentData, *maskIt ) );

        if( newChild != NULL )
            _sceneGraph->addChild( newChild.get() );
    }
    else
    {
        OSG_WARN << "DataSet: TimeSet size > 1 not yet supported in 'page textures only' mode." << std::endl;
    }

    return( false );
}
bool DataSet::updateRenderer()
{
    if( _renderer != NULL )
    {
        if( _maskList.empty() )
            createFallbackMaskList();

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
            const double time( timeSet.empty() ? 0. : *( timeSet.begin() ) );
            ChannelDataList currentData( getDataAtTime( time ) );

            ChannelDataList::iterator maskIt = _maskList.begin();

            osg::Node* newChild( recurseGetSceneGraph( currentData, *maskIt ) );

            if( newChild != NULL )
                _sceneGraph->addChild( newChild );
        }

        _sceneGraph->setStateSet( _renderer->getRootState() );

        return( true );
    }
    return( false );
}

osg::Node* DataSet::recurseGetSceneGraphPagingTexturesOnly( ChannelDataList& data, ChannelDataPtr mask )
{
    ChannelDataImageSet* imageData( NULL );
    ChannelDataLOD* lodData( NULL );
    if( !( data.empty() ) )
    {
        ChannelDataComposite* comp( dynamic_cast< ChannelDataComposite* >( data[ 0 ].get() ) );
        if( comp != NULL )
        {
            imageData = comp->getAsSet();
            lodData = comp->getAsLOD();
        }
    }

    if( imageData != NULL )
    {
        if( !( ChannelDataImageSet::allImageSetData( data ) ) )
        {
            OSG_WARN << "recurseGetSceneGraphPagingTexturesOnly: All data must be ChannelDataImageSet." << std::endl;
            return( NULL );
        }

        OSG_NOTICE << "ImageSet only partially supported." << std::endl;

        osg::ref_ptr< osg::Group > parent( new osg::Group );
        unsigned int idx;
        for( idx=0; idx < lodData->getNumChannels(); idx++ )
        {
            ChannelDataList currentData( getCompositeChannels( data, idx ) );
            parent->addChild( recurseGetSceneGraphPagingTexturesOnly( currentData, mask ) );
        }
        return( parent.release() );
    }
    else if( lodData != NULL )
    {
        if( !( ChannelDataLOD::allLODData( data ) ) )
        {
            OSG_WARN << "recurseGetSceneGraphPagingTexturesOnly: All data must be ChannelDataLOD." << std::endl;
            return( NULL );
        }

        osg::ref_ptr< osg::Group > parent( new osg::Group );
        osg::ref_ptr< PageData > pageData( new PageData( lfx::PageData::PIXEL_SIZE_RANGE ) );
        parent->setUserData( pageData.get() );
        pageData->setParent( parent.get() );

        // TBD kind of ugly.
        parent->setUpdateCallback( _rootcb->create() );

        unsigned int childIndex( 0 );
        unsigned int idx;
        for( idx=0; idx < lodData->getNumChannels(); idx++ )
        {
            ChannelDataList currentData( getCompositeChannels( data, idx ) );
            osg::Node* child( recurseGetSceneGraphPagingTexturesOnly( currentData, mask ) );

            if( child != NULL )
            {
                PageData::RangeData rangeData( lodData->getRange( idx ) );
                rangeData._status = PageData::RangeData::UNLOADED;
                rangeData._dbKey = generateDBKey();
                pageData->setRangeData( childIndex++, rangeData );

                child->setNodeMask( 0u );
                parent->addChild( child );
            }
        }
        return( parent.release() );
    }
    else
    {
        setInputs( _renderer, data );
        return( _renderer->getSceneGraph( mask ) );
    }

    return( NULL );
}
osg::Node* DataSet::recurseGetSceneGraph( ChannelDataList& data, ChannelDataPtr mask )
{
    ChannelDataLOD* cdLOD( dynamic_cast< ChannelDataLOD* >( data[ 0 ].get() ) );
    if( cdLOD == NULL )
    {
        setInputs( _renderer, data );
        return( _renderer->getSceneGraph( mask ) );
    }

    else
    {
        osg::ref_ptr< osg::Group > parent( new osg::Group );
        RootCallback* rootcb( static_cast< RootCallback* >( _sceneGraph->getUpdateCallback() ) );
        rootcb->addPageParent( parent.get() );
        osg::ref_ptr< PageData > pageData( new PageData( lfx::PageData::PIXEL_SIZE_RANGE ) );
        parent->setUserData( pageData.get() );
        pageData->setParent( parent.get() );
        unsigned int childIndex( 0 );
        osg::ref_ptr< osg::Group > stubGroup( new osg::Group );

        unsigned int idx;
        for( idx=0; idx < cdLOD->getNumChannels(); idx++ )
        {
            ChannelDataList newDataList;
            newDataList.push_back( cdLOD->getChannel( idx ) );
            osg::Node* child( recurseGetSceneGraph( newDataList, mask ) );

            if( child != NULL )
            {
                PageData::RangeData rangeData( cdLOD->getRange( idx ) );
                rangeData._status = PageData::RangeData::UNLOADED;
                rangeData._dbKey = generateDBKey();
                pageData->setRangeData( childIndex++, rangeData );

                parent->addChild( stubGroup.get() );
                osg::BoundingSphere bs( parent->getInitialBound() );
                bs.expandBy( child->getBound() );
                parent->setInitialBound( bs );

                storeSubGraph( child, rangeData._dbKey );
            }
        }
        return( parent.release() );
    }
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
ChannelDataList DataSet::getCompositeChannels( ChannelDataList data, const unsigned int index )
{
    ChannelDataList cdl;
    BOOST_FOREACH( ChannelDataPtr cdp, data )
    {
        ChannelDataComposite* comp( static_cast< ChannelDataComposite* >( cdp.get() ) );
        cdl.push_back( comp->getChannel( index ) );
    }
    return( cdl );
}


void DataSet::setInputs( OperationBasePtr opPtr, ChannelDataList& currentData )
{
    ChannelDataList newList;
    const OperationBase::StringList& inputs( opPtr->getInputNames() );
    BOOST_FOREACH( const std::string& inputName, inputs )
    {
        ChannelDataPtr cdp( currentData.findData( inputName ) );
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

void DataSet::createFallbackMaskList()
{
    osg::ByteArray* osgMask( new osg::ByteArray );
    osgMask->resize( 1 );
    ChannelDataOSGArrayPtr mask( new ChannelDataOSGArray( osgMask ) );
    mask->setAll( (char)1 );
    _maskList.push_back( mask );
}

void DataSet::setSceneGraphPagesTexturesOnly()
{
    _sceneGraphPagesTexturesOnly = true;
}

void DataSet::useCustomRootCallback( lfx::RootCallback* rootcb )
{
    _rootcb = rootcb;
    _sceneGraph->setUpdateCallback( _rootcb.get() );
}


// lfx
}
