
#include <latticefx/DataSet.h>
#include <latticefx/ChannelDataOSGArray.h>

#include <osg/Group>
#include <osg/Notify>

#include <boost/foreach.hpp>


namespace lfx {


DataSet::DataSet()
  : _sceneGraph( new osg::Group ),
    _dirtyFlags( ALL_DIRTY )
{
}

DataSet::~DataSet()
{
}

void DataSet::addChannel( const ChannelDataPtr channel, const TimeValue& time )
{
    _data[ time ].push_back( channel );
    setDirty( ALL_DIRTY );
    checkAndResizeMask();
}

ChannelDataPtr DataSet::getChannel( const std::string& name, const TimeValue& time )
{
    TimeSeriesData::iterator tsdIt( _data.find( time ) );
    if( tsdIt == _data.end() )
        return( ChannelDataPtr( ( ChannelData* )( NULL ) ) );

    ChannelDataList& cdl = tsdIt->second;
    ChannelDataPtr cdp;
    BOOST_FOREACH( cdp, cdl )
    {
        if( cdp->getName() == name )
            return( cdp );
    }

    return( ChannelDataPtr( ( ChannelData* )( NULL ) ) );
}

const ChannelDataPtr DataSet::getChannel( const std::string& name, const TimeValue& time ) const
{
    DataSet* nonConstThis = const_cast< DataSet* >( this );
    return( nonConstThis->getChannel( name, time ) );
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
    processChanges();

    return( _sceneGraph.get() );
}

bool DataSet::processChanges()
{
    if( _dirtyFlags == NOT_DIRTY )
        return( true );

    // Reset all attached inputs. If a ChannelData instance needs to refresh
    // a working copy of data from the original source data, it does so here.
    _mask->reset();
    BOOST_FOREACH( TimeSeriesData::value_type timeData, _data )
    {
        BOOST_FOREACH( ChannelDataPtr cdp, timeData.second )
        {
            cdp->reset();
        }
    }

    // Initially, everything is enabled.
    _mask->setAll( (const char)1 ); // Enable all elements.


    //
    // Preprocess & Cache (if dirty)
    //
    if( _dirtyFlags & PREPROCESS_DIRTY )
    {
        // Not yet implemented.
    }


    //
    // Run Time Operations (if dirty)
    //
    if( _dirtyFlags & RTP_DIRTY )
    {
        // Iterate over all time steps.
        BOOST_FOREACH( TimeSeriesData::value_type timeData, _data )
        {
            // Iterate over all attached run time operations.
            BOOST_FOREACH( RTPOperationPtr opPtr, _ops )
            {
                if( opPtr->getEnable() )
                {
                    switch( opPtr->getRTPOpType() )
                    {
                    case RTPOperation::Mask:
                    {
                        const ChannelDataPtr mask = opPtr->mask( _mask );
                        _mask->andValues( mask.get() );
                        break;
                    }
                    case RTPOperation::Filter:
                    {
                        opPtr->filter( _mask );
                        break;
                    }
                    case RTPOperation::Channel:
                    {
                        ChannelDataPtr cdp = opPtr->channel( _mask );
                        addChannel( cdp, timeData.first );
                        break;
                    }
                    }
                }
            }
        }
    }


    //
    // Rendering Framework support
    //

    if( _dirtyFlags & RTP_DIRTY )
    {
        _sceneGraph->removeChildren( 0, _sceneGraph->getNumChildren() );

        // TBD support for time series data.
        if( _renderer != NULL )
            _sceneGraph->addChild( _renderer->getSceneGraph( _mask ) );
    }


    _dirtyFlags = NOT_DIRTY;

    return( true );
}

const ChannelDataPtr DataSet::getMask() const
{
    return( _mask );
}

void DataSet::setDirty( const DirtyFlags flags )
{
    _dirtyFlags = flags;
}
DataSet::DirtyFlags DataSet::getDirty() const
{
    return( _dirtyFlags );
}


bool DataSet::checkAndResizeMask()
{
    unsigned int size( 0 );

    std::pair< TimeValue, ChannelDataList > timeData;
    BOOST_FOREACH( timeData, _data )
    {
        ChannelDataPtr cdp;
        BOOST_FOREACH( cdp, timeData.second )
        {
            unsigned int x, y, z;
            cdp->getDimensions( x, y, z );
            const unsigned int total( x * y * z );
            if( total != size )
            {
                if( size != 0 )
                {
                    osg::notify( osg::WARN ) << "DataSet::checkAndResizeMask: Size inconsistency." << std::endl;
                    return( false );
                }
                size = total;
            }
        }
    }

    osg::ByteArray* byteArray( NULL );
    if( _mask == NULL )
    {
        byteArray = new osg::ByteArray;
        _mask = ChannelDataOSGArrayPtr( new ChannelDataOSGArray( byteArray ) );
    }
    else
        byteArray = static_cast< osg::ByteArray* >( _mask->asOSGArray() );

    if( byteArray->getNumElements() != size )
        _mask->resize( size );

    return( true );
}


// lfx
}
