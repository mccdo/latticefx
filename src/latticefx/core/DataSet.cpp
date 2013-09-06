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
#include <latticefx/core/JsonSerializer.h>

#include <osg/Group>
#include <osg/MatrixTransform>

#include <boost/foreach.hpp>

//#include <iostream>
#include <fstream>


namespace lfx
{
namespace core
{


DataSet::DataSet( const std::string& logName)
    : ObjBase(),
      LogBase( logName.empty() ? "lfx.core.dataset" : logName ),
      _sceneGraph( new osg::Group ),
      _dirty( ALL_DIRTY )
{
}
DataSet::DataSet( const DataSet& rhs )
    : ObjBase( rhs ),
      LogBase( rhs ),
      _data( rhs._data ),
      _dataNames( rhs._dataNames ),
      _db( rhs._db ),
      _preprocess( rhs._preprocess ),
      _ops( rhs._ops ),
      _renderer( rhs._renderer ),
      _maskList( rhs._maskList ),
      _dirty( ALL_DIRTY )
{
}
DataSet::~DataSet()
{
	_dataSetUUIDMap.clear();
}

void DataSet::addChannel( const ChannelDataPtr channel, const TimeValue time )
{
    _data[ time ].push_back( channel );
    _dataNames.insert( channel->getName() );
    setDirty();
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
    setDirty();
}

ChannelDataPtr DataSet::getChannel( const std::string& name, const TimeValue time )
{
    ChannelDataPtr cdp;
    ChannelDataTimeMap::iterator cdlIt( _data.find( time ) );
    if( cdlIt != _data.end() )
    {
        cdp = cdlIt->second.findData( name );
    }

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
                {
                    break;
                }
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
void DataSet::insertPreprocess( const unsigned int index, const PreprocessPtr pre )
{
    unsigned int idx( 0 );
    PreprocessList::iterator it;
    for( it = _preprocess.begin(); it != _preprocess.end(); ++it )
    {
        if( ++idx == index )
        {
            _preprocess.insert( it, pre );
            setDirty( PREPROCESS_DIRTY );
            break;
        }
    }
}
void DataSet::insertPreprocess( const PreprocessPtr location, const PreprocessPtr pre )
{
    PreprocessList::iterator it;
    for( it = _preprocess.begin(); it != _preprocess.end(); ++it )
    {
        if( *it == location )
        {
            _preprocess.insert( it, pre );
            setDirty( PREPROCESS_DIRTY );
            break;
        }
    }
}
unsigned int DataSet::getNumPreprocess() const
{
    return( _preprocess.size() );
}
PreprocessPtr DataSet::getPreprocess( const unsigned int index )
{
    unsigned int idx( 0 );
    BOOST_FOREACH( PreprocessPtr pre, _preprocess )
    {
        if( idx++ == index )
        {
            return( pre );
        }
    }

    return( PreprocessPtr( ( Preprocess* ) NULL ) );
}
const PreprocessPtr DataSet::getPreprocess( const unsigned int index ) const
{
    DataSet* nonConstThis( const_cast< DataSet* >( this ) );
    return( nonConstThis->getPreprocess( index ) );
}
PreprocessList& DataSet::getPreprocesses()
{
    return( _preprocess );
}
const PreprocessList& DataSet::getPreprocesses() const
{
    return( _preprocess );
}



void DataSet::addOperation( const RTPOperationPtr op )
{
    _ops.push_back( op );
    setDirty( RTPOPERATION_DIRTY );
}
void DataSet::insertOperation( const unsigned int index, const RTPOperationPtr op )
{
    unsigned int idx( 0 );
    RTPOperationList::iterator it;
    for( it = _ops.begin(); it != _ops.end(); ++it )
    {
        if( ++idx == index )
        {
            _ops.insert( it, op );
            setDirty( RTPOPERATION_DIRTY );
            break;
        }
    }
}
void DataSet::insertOperation( const RTPOperationPtr location, const RTPOperationPtr op )
{
    RTPOperationList::iterator it;
    for( it = _ops.begin(); it != _ops.end(); ++it )
    {
        if( *it == location )
        {
            _ops.insert( it, op );
            setDirty( RTPOPERATION_DIRTY );
            break;
        }
    }
}
unsigned int DataSet::getNumOperations() const
{
    return( _ops.size() );
}
RTPOperationPtr DataSet::getOperation( const unsigned int index )
{
    unsigned int idx( 0 );
    BOOST_FOREACH( RTPOperationPtr op, _ops )
    {
        if( idx++ == index )
        {
            return( op );
        }
    }

    return( RTPOperationPtr( ( RTPOperation* ) NULL ) );
}
const RTPOperationPtr DataSet::getOperation( const unsigned int index ) const
{
    DataSet* nonConstThis( const_cast< DataSet* >( this ) );
    return( nonConstThis->getOperation( index ) );
}
RTPOperationList& DataSet::getOperations()
{
    return( _ops );
}
const RTPOperationList& DataSet::getOperations() const
{
    return( _ops );
}



void DataSet::setRenderer( const RendererPtr renderer )
{
    _renderer = renderer;
    setDirty( RENDERER_DIRTY );
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
    if( _sceneGraph->getNumChildren() == 0 )
    {
        setDirty();
    }

    if( getDirty() != 0 )
    {
        updateAll();
    }

    return( _sceneGraph.get() );
}

bool DataSet::updateAll()
{
    if( !_dirty )
    {
        return( true );
    }

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
    if( _dirty & PREPROCESS_DIRTY )
        updatePreprocessing();

    // Run Time Operations (if dirty)
    if( _dirty & ( PREPROCESS_DIRTY | RTPOPERATION_DIRTY ) )
        updateRunTimeProcessing();

    // Rendering Framework support
    if( _dirty & ( PREPROCESS_DIRTY | RTPOPERATION_DIRTY | RENDERER_DIRTY ) )
    {
        _sceneGraph->removeChildren( 0, _sceneGraph->getNumChildren() );
        updateRenderer();
    }

    clearDirty();
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
            {
                continue;
            }

            ChannelDataPtr newData( ( *prePtr )() );
            if( newData == NULL )
            {
                continue;
            }

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
            {
                continue;
            }

            // Assign actual / current data to the prePtr OperationBase.
            setInputs( prePtr, currentData );
            if( !prePtr->validInputs() )
            {
                continue;
            }

            ChannelDataPtr newData( ( *prePtr )() );
            if( newData == NULL )
            {
                continue;
            }

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
        mask->setAll( ( const char )1 ); // Enable all elements.
        _maskList.push_back( mask );

        // Iterate over all attached run time operations.
        BOOST_FOREACH( RTPOperationPtr opPtr, _ops )
        {
            if( !( opPtr->getEnable() ) )
            {
                continue;
            }

            // Assign actual / current data to the opPtr OperationBase.
            setInputs( opPtr, currentData );
            if( !opPtr->validInputs() )
            {
                continue;
            }

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
            default:
            case RTPOperation::Undefined:
            {
                LFX_ERROR( "Bad RTPOperation operation type." );
            }
            }
        }
    }
    return( true );
}

bool DataSet::updateRenderer()
{
    if( _renderer == NULL )
    {
        return( false );
    }

    if( _maskList.empty() )
    {
        createFallbackMaskList();
    }

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
    {
        _sceneGraph->setName( "Lfx-TimePagingRoot" );
    }
    else
    {
        _sceneGraph->setName( "Lfx-Root" );
    }


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
        for( idx = 0; idx < imageData->getNumChannels(); idx++ )
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
        for( idx = 0; idx < lodData->getNumChannels(); idx++ )
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
        if( _renderer->validInputs() )
        {
            return( _renderer->getSceneGraph( mask ) );
        }
    }

    return( NULL );
}

void DataSet::setDirty( const int dirty )
{
    _dirty |= dirty;
}
void DataSet::clearDirty( const int dirty )
{
    _dirty &= ( ~dirty & ALL_DIRTY );
}
int DataSet::getDirty() const
{
    return( _dirty );
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
    BOOST_FOREACH( const std::string & name, _dataNames )
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
    BOOST_FOREACH( const std::string & inputName, inputs )
    {
        ChannelDataPtr cdp( currentData.findData( inputName ) );
        if( cdp != NULL )
        {
            newList.push_back( cdp );
        }
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
        const unsigned int total( x * y * z );
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
    mask->setAll( ( char )1 );
    _maskList.push_back( mask );
}

bool DataSet::isTemporalData() const
{
    return( !getTimeSet().empty() );
}

////////////////////////////////////////////////////////////////////////////////
bool DataSet::loadPipeline( IObjFactory *objf, const std::string &filePath, std::string *perr )
{
    /*
	// TODO: ifstream seems to be an issue is osg, where using ifstream it creates a link problem
	// need to sort that out at some point, for now just going to use c style FILE
	//
	std::ifstream file( filePath );
	if (!file.is_open())
	{
		if (perr) *perr =  std::string( "Failed to open the file: " ) + filePath;
		return false;
	}

	std::stringstream strJson;
	strJson << file.rdbuf();
	file.close();
    */

	FILE *fp = fopen( filePath.c_str(), "rb" );
	if (!fp)
	{ 
		if (perr) *perr =  std::string( "Failed to open the file: " ) + filePath;
		return false;
	}

	fseek(fp, 0, SEEK_END);
    long int size = ftell(fp);
    rewind(fp);
	if( size <= 0 )
	{
		if (perr) *perr =  std::string( "The file is empty: " ) + filePath;
		return false;
	}

	std::vector<char> vec( size );
	fread( &vec[0], 1, size, fp);
	std::string strJson(vec.begin(), vec.end());


	try
	{
		JsonSerializer js( filePath.c_str() );
		if( !js.load( strJson ) )
		{ 
			if (perr) *perr =  std::string( "Json does not appear to be valid in file: " ) + filePath;
			return false;
		}

		
		std::string err;
		ObjBasePtr p = ObjBase::loadObj( &js, objf, &err );
		if( !p )
		{
			if (perr) *perr = std::string("Failed to load the pipeline: ") + err;
			return false;
		}

		DataSetPtr ds = boost::dynamic_pointer_cast<DataSet>( p );
		if( !ds )
		{
			if (perr) *perr = "Unexpected error: serialized root object is not of type DataSet";
			return false;
		}

		// copy over loaded data into this 
		_preprocess = ds->_preprocess;
		_ops = ds->_ops;
		_renderer = ds->_renderer;

		// initialize the database
		BOOST_FOREACH( PreprocessPtr pre, _preprocess )
		{
			if( pre ) pre->setDB( getDB() );
		}

		BOOST_FOREACH( RTPOperationPtr op, _ops )
		{
			if( op ) op->setDB( getDB() );
		}

		if( _renderer ) _renderer->setDB( getDB() );

		return true;
	}
	catch( Poco::JSON::JSONException je)
	{
		if (perr)
		{
			*perr =  std::string( "Unexpected JSON exception: " ) +  je.message();
		}

		return false;
	}
}

////////////////////////////////////////////////////////////////////////////////
bool DataSet::savePipeline( const std::string &filePath, std::string *perr )
{
	std::ofstream file;
	try
	{
		file.open( filePath.c_str(), std::ios::out );
		if (!file.is_open())
		{
			if (perr) *perr =  std::string( "Failed to create the file: " ) + filePath;
			return false;
		}

		JsonSerializer js( filePath.c_str() );
		serialize( &js );

		js.toStream( &file );
		file.close();

		std::string debug = js.toString();
		return true;
	}
	catch( Poco::JSON::JSONException je)
	{
		if (perr)
		{
			*perr =  std::string( "Unexpected JSON exception: " ) +  je.message();
		}

		file.close();
		return false;
	}
}

////////////////////////////////////////////////////////////////////////////////
void DataSet::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	ObjBase::serializeData( json );

	json->insertObj( DataSet::getClassName(), true );

	json->insertArray( "preprocessList", true );
	BOOST_FOREACH( PreprocessPtr pre, _preprocess )
    {
		if( pre ) pre->serialize( json );
    }
	json->popParent();

	json->insertArray( "rtpOperationList", true );
	BOOST_FOREACH( RTPOperationPtr op, _ops )
    {
		if( op ) op->serialize( json );
    }
	json->popParent();

	json->insertObj( "renderer", true );
	if( _renderer ) _renderer->serialize( json );
	json->popParent();

	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
bool DataSet::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if( !ObjBase::loadData( json, pfactory, perr ) ) return false;

	bool ret = false;

	json->markParentStack();

	while( true )
	{
		// get to this classes data
		if ( !json->getObj( DataSet::getClassName() ) )
		{
			if (perr) *perr = "Json: Failed to get DataSet data";
			break;
		}

		_preprocess.clear();
		_ops.clear();
		_renderer.reset();

		if( !loadArrListType<PreprocessPtr, Preprocess>( json, pfactory, "preprocessList", &_preprocess, perr) ) break;
		if( !loadArrListType<RTPOperationPtr, RTPOperation>( json, pfactory, "rtpOperationList", &_ops, perr) ) break;

		// renderer object
		if ( !json->getObj( "renderer" ) )
		{
			if (perr) *perr = "Json: Failed to get renderer";
			return false;
		} 

		// may not be a renderer
		ObjBasePtr p = loadObj( json, pfactory, perr );
		if( p ) 
		{
			_renderer = boost::dynamic_pointer_cast<Renderer>( p );
			if( !_renderer ) break;
		
			json->popParent();
		}

		ret = true;
		break;
	}
	
	json->popMark();
	return ret;
}

////////////////////////////////////////////////////////////////////////////////
void DataSet::dumpState( std::ostream &os )
{
	ObjBase::dumpState( os );

	dumpStateStart( DataSet::getClassName(), os );

	// TODO:
	//typedef std::map< TimeValue, ChannelDataList > ChannelDataTimeMap;
    //ChannelDataTimeMap _data;

	os << "_dataNames count: " << _dataNames.size() << std::endl;
	BOOST_FOREACH( std::string s, _dataNames )
	{
		os << "data name: " << s << std::endl;
	}
		
	// TODO:
    // DBBasePtr _db;

	os << "_preprocess count: " << _preprocess.size() << std::endl;
	BOOST_FOREACH( PreprocessPtr p, _preprocess )
	{
		p->dumpState( os );
	}

	os << "_ops count: " << _ops.size() << std::endl;
	BOOST_FOREACH( RTPOperationPtr p, _ops )
	{
		p->dumpState( os );
	}

	if( _renderer )
	{
			_renderer->dumpState( os );
	}
	else 
	{
		os << "_renderer: NULL" << std::endl;
	}

    // TODO:
    //ChannelDataList _maskList;
    //osg::ref_ptr< osg::Group > _sceneGraph;

	os << "_dirty: " << _dirty << std::endl;

	dumpStateEnd( DataSet::getClassName(), os );
}

////////////////////////////////////////////////////////////////////////////////
void DataSet::setUUID( const std::string& attribute, const std::string& uuid )
{
    _dataSetUUIDMap[ attribute ] = uuid;
}

////////////////////////////////////////////////////////////////////////////////
const std::string DataSet::getUUID( const std::string& attribute )
{
    std::map< std::string, std::string >::iterator iter;
    iter = _dataSetUUIDMap.find( attribute );
    if( iter == _dataSetUUIDMap.end() )
    {
        return std::string( "" );
    }
    else
    {
        return iter->second;
    }
}

// core
}
// lfx
}
