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

#include <latticefx/core/DataSet.h>
#include <latticefx/core/ChannelDataImageSet.h>
#include <latticefx/core/ChannelDataLOD.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/VolumeRenderer.h>
#include <latticefx/core/OctreeGroup.h>
#include <latticefx/core/PagingCallback.h>
#include <latticefx/core/LogMacros.h>

#include <osg/Group>
#include <osg/MatrixTransform>

#include <boost/foreach.hpp>


namespace lfx {
namespace core {


DataSet::DataSet()
  : LogBase( "lfx.core.dataset" ),
    _sceneGraph( new osg::Group ),
    _dirtyFlags( ALL_DIRTY )
{
}
DataSet::DataSet( const DataSet& rhs )
  : LogBase( rhs ),
    _data( rhs._data ),
    _dataNames( rhs._dataNames ),
    _db( rhs._db ),
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

void DataSet::addChannel( const ChannelDataPtr channel, const TimeValue time )
{
    _data[ time ].push_back( channel );
    _dataNames.insert( channel->getName() );
    setDirty( ALL_DIRTY );
}
void DataSet::replaceChannel( const ChannelDataPtr channel, const TimeValue time )
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

ChannelDataPtr DataSet::getChannel( const std::string& name, const TimeValue time )
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
const ChannelDataPtr DataSet::getChannel( const std::string& name, const TimeValue time ) const
{
    DataSet* nonConstThis = const_cast< DataSet* >( this );
    return( nonConstThis->getChannel( name, time ) );
}

osg::Vec2d DataSet::getTimeRange() const
{
    TimeValue minTime, maxTime;
    getTimeRange( minTime, maxTime );
    return( osg::Vec2d( minTime, maxTime ) );
}
void DataSet::getTimeRange( TimeValue& minTime, TimeValue& maxTime ) const 
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
        updateRenderer();
    }

    _dirtyFlags = NOT_DIRTY;
    return( true );
}

bool DataSet::updatePreprocessing()
{
    TimeSet timeSet( getTimeSet() );
    if( timeSet.empty() )
    {
        // Execute Preprocess ops with no input. A Preprocess op could generate
        // a new ChannelData procedurally.

        BOOST_FOREACH( PreprocessPtr prePtr, _preprocess )
        {
            if( !( prePtr->getEnable() ) )
                continue;

            ChannelDataPtr newData( (*prePtr)() );
            if( newData == NULL )
                continue;

            // The Proprocessor object tells us how to handle the new data
            // we just got back. This is so that apps can configure the Preprocessor
            // directly. In the future, we might have to support adding the returned
            // data to the DB.
            switch( prePtr->getActionType() )
            {
            case Preprocess::ADD_DATA:
                addChannel( newData );
                break;
            case Preprocess::REPLACE_DATA:
                LFX_WARNING( "Preprocess op with REPLACE_DATA action, not currently supported with empty TimeSet." );
                break;
            default:
            case Preprocess::IGNORE_DATA:
                // No-op. Do nothing.
                break;
            }
        }
        return( true );
    }

    // Iterate over all time steps.
    BOOST_FOREACH( TimeValue time, timeSet )
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
            if( newData == NULL )
                continue;

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
        // This might have a valid use, as in a channel creation op
        // that generates the new ChannelData procedurally.
        // Not yet supported.
        LFX_WARNING( "updateRunTimeProcessing: timeSet.size() == 0." );
        return( false );
    }

    // Iterate over all time steps.
    BOOST_FOREACH( TimeValue time, timeSet )
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
    if( _renderer == NULL )
        return( false );

    if( _maskList.empty() )
        createFallbackMaskList();

    TimeSet timeSet( getTimeSet() );
    if( timeSet.size() == 0 )
    {
        // No ChannelData was added to the DataSet.
        // It might be useful for a Renderer to generate the scene graph procedurally
        // from other input data, but for now we require that this be done as a
        // Preprocess op, and possibly as an RTP channel creation op in the future.
        LFX_WARNING( "updateRenderer: timeSet.size() == 0. No scene graph generated." );
        return( false );
    }


    PagingCallback* rootcb( new PagingCallback() );
    rootcb->setDB( _db );
    _sceneGraph->setUpdateCallback( rootcb );

    PageData* pageData( new PageData );
    pageData->setRangeMode( PageData::TIME_RANGE );
    pageData->setMinMaxTime( *( timeSet.begin() ), *( timeSet.rbegin() ) );
    pageData->setParent( _sceneGraph.get() );
    _sceneGraph->setUserData( pageData );

    unsigned int childIndex( 0 );
    ChannelDataList::iterator maskIt( _maskList.begin() );
    BOOST_FOREACH( TimeValue time, timeSet )
    {
        // Get the data at the current time and assign as inputs to the Renderer.
        ChannelDataList currentData( getDataAtTime( time ) );

        osg::ref_ptr< osg::Node > newChild( recurseGetSceneGraph( currentData, *maskIt ) );
        if( newChild != NULL )
        {
            PageData::RangeData rangeData( time, time );
            rangeData._status = PageData::RangeData::UNLOADED;
            pageData->setRangeData( childIndex++, rangeData );

            newChild->setNodeMask( 0u );
            _sceneGraph->addChild( newChild.get() );
        }
        ++maskIt;
    }
    if( timeSet.size() > 1 )
        _sceneGraph->setName( "Lfx-TimePagingRoot" );
    else
        _sceneGraph->setName( "Lfx-Root" );


    _sceneGraph->setStateSet( _renderer->getRootState() );
    return( true );
}

osg::Node* DataSet::recurseGetSceneGraph( ChannelDataList& data, ChannelDataPtr mask )
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
            LFX_WARNING( "recurseGetSceneGraph: All data must be ChannelDataImageSet." );
            return( NULL );
        }

        SpatialVolumePtr spatial( boost::dynamic_pointer_cast< SpatialVolume >( _renderer ) );
        osg::ref_ptr< OctreeGroup > parent( new OctreeGroup( spatial->getVolumeOrigin() ) );
        parent->setName( "Lfx-ImageSet" );
        unsigned int idx;
        for( idx=0; idx < imageData->getNumChannels(); idx++ )
        {
            ChannelDataList currentData( getCompositeChannels( data, idx ) );
            osg::Node* child( recurseGetSceneGraph( currentData, mask ) );

            if( child != NULL )
            {
                osg::Vec3 offset( imageData->getOffset( idx ) * .5 );
                osg::Matrix negateOrigin;
                if( spatial != NULL )
                {
                    // If the Renderer is a SpatialVolume, scale the offset by the volume dimensions.
                    offset[ 0 ] *= spatial->getVolumeDims()[ 0 ];
                    offset[ 1 ] *= spatial->getVolumeDims()[ 1 ];
                    offset[ 2 ] *= spatial->getVolumeDims()[ 2 ];
                    // Negate the origin offset from the Renderer in order to place
                    // the geometry at the correct location.
                    negateOrigin = osg::Matrix::translate( spatial->getVolumeOrigin() );
                }
                const osg::Matrix trans( negateOrigin *
                    osg::Matrix::translate( offset ) *
                    osg::Matrix::scale( .5, .5, .5 ) );

                osg::MatrixTransform* mt( new osg::MatrixTransform( trans ) );
                mt->addChild( child );
                parent->addChild( mt, imageData->getOffset( idx ) );
            }
        }
        return( parent.release() );
    }
    else if( lodData != NULL )
    {
        if( !( ChannelDataLOD::allLODData( data ) ) )
        {
            LFX_WARNING( "recurseGetSceneGraph: All data must be ChannelDataLOD." );
            return( NULL );
        }

        osg::ref_ptr< osg::Group > parent( new osg::Group );
        parent->setName( "Lfx-LODPaging" );
        osg::ref_ptr< PageData > pageData( new PageData( PageData::PIXEL_SIZE_RANGE ) );
        parent->setUserData( pageData.get() );
        pageData->setParent( parent.get() );

        PagingCallback* cb( new PagingCallback() );
        cb->setDB( _db );
        parent->setUpdateCallback( cb );

        unsigned int childIndex( 0 );
        unsigned int idx;
        for( idx=0; idx < lodData->getNumChannels(); idx++ )
        {
            ChannelDataList currentData( getCompositeChannels( data, idx ) );
            osg::Node* child( recurseGetSceneGraph( currentData, mask ) );

            if( child != NULL )
            {
                PageData::RangeData rangeData( lodData->getRange( idx ) );
                rangeData._status = PageData::RangeData::UNLOADED;
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

ChannelDataList DataSet::getDataAtTime( const TimeValue time )
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
        {
            LFX_WARNING_STATIC( "lfx.core.dataset", "setInputs(): Could not find data named \"" + inputName + "\"." );
        }
        newList.push_back( cdp );
    }
    opPtr->setInputs( newList );
}


ChannelDataOSGArrayPtr DataSet::createSizedMask( const ChannelDataList& dataList )
{
    unsigned int size( 0 );

    BOOST_FOREACH( const ChannelDataPtr cdp, dataList )
    {
        unsigned int x = 0, y = 0, z = 0;
        //The default implementation of ChannelData::getDimensions does not
        //return a default value therefore we must set our x,y,z to 0.
        cdp->getDimensions( x, y, z );
        const unsigned int total( x*y*z );
        size = osg::maximum( size, total );
    }

    osg::ByteArray* byteArray( new osg::ByteArray );
    byteArray->resize( size );
    return( ChannelDataOSGArrayPtr( new ChannelDataOSGArray( "DataSet-SizedMask", byteArray ) ) );
}

void DataSet::createFallbackMaskList()
{
    osg::ByteArray* osgMask( new osg::ByteArray );
    osgMask->resize( 1 );
    ChannelDataOSGArrayPtr mask( new ChannelDataOSGArray( "DataSet-FallbackMask", osgMask ) );
    mask->setAll( (char)1 );
    _maskList.push_back( mask );
}


// core
}
// lfx
}
