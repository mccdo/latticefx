
#include <latticefx/DataSet.h>
#include <latticefx/ChannelDataComposite.h>
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

void DataSet::addChannel( const ChannelDataPtr channel, const double time )
{
    ChannelDataNameMap::iterator cdpIt( _data.find( channel->getName() ) );
    ChannelDataComposite* comp;
    if( cdpIt == _data.end() )
    {
        ChannelDataCompositePtr newComp( new ChannelDataComposite( channel->getName() ) );
        _data[ channel->getName() ] = newComp;
        comp = newComp.get();
    }
    else
        comp = static_cast< ChannelDataComposite* >( cdpIt->second.get() );
    comp->addChannel( channel, time );

    setDirty( ALL_DIRTY );
    checkAndResizeMask();
}

ChannelDataPtr DataSet::getChannel( const std::string& name, const double time )
{
    ChannelDataNameMap::iterator cdpIt( _data.find( name ) );
    if( cdpIt == _data.end() )
        return( ChannelDataPtr( ( ChannelData* )NULL ) );
    else
    {
        ChannelDataComposite* comp( static_cast< ChannelDataComposite* >( cdpIt->second.get() ) );
        return( comp->getChannel( time ) );
    }
}

const ChannelDataPtr DataSet::getChannel( const std::string& name, const double time ) const
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
    updateSceneGraph();

    return( _sceneGraph.get() );
}

bool DataSet::updateSceneGraph()
{
    if( _dirtyFlags == NOT_DIRTY )
        return( true );

    // Reset all attached inputs. If a ChannelData instance needs to refresh
    // a working copy of data from the original source data, it does so here.
    _mask->reset();
    BOOST_FOREACH( ChannelDataNameMap::value_type channelPair, _data )
    {
        channelPair.second->reset();
    }

    // Initially, everything is enabled.
    _mask->setAll( (const char)1 ); // Enable all elements.


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
    // Iterate over all time steps.
    TimeSet timeSet( getTimeSet() );
    BOOST_FOREACH( double time, timeSet )
    {
        // Get the data at the current time.
        ChannelDataList currentData( getDataAtTime( time ) );

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
                addChannel( cdp );
                break;
            }
            }
        }
    }
    return( true );
}
bool DataSet::updateRenderer()
{
    // TBD support for time series data.
    if( _renderer != NULL )
    {
        TimeSet timeSet( getTimeSet() );
        BOOST_FOREACH( double time, timeSet )
        {
            // Get the data at the current time.
            ChannelDataList currentData( getDataAtTime( time ) );

            // Assign actual / current data to the _renderer OperationBase.
            setInputs( _renderer, currentData );

            _sceneGraph->addChild( _renderer->getSceneGraph( _mask ) );
        }

        return( true );
    }
    return( false );
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

TimeSet DataSet::getTimeSet() const
{
    TimeSet timeSet;
    BOOST_FOREACH( ChannelDataNameMap::value_type channelPair, _data )
    {
        const std::string& name( channelPair.first );
        ChannelDataPtr cdp( channelPair.second );
        ChannelDataComposite* comp( dynamic_cast< ChannelDataComposite* >( cdp.get() ) );
        if( comp != NULL )
        {
            const TimeSet& ts( comp->getTimeSet() );
            timeSet.insert( ts.begin(), ts.end() );
        }
        else
        {
            // 'cdp' isn't a ChannelDataComposite, so it contains no
            // time series data and therefore by definition is at time 0.
            timeSet.insert( 0. );
        }
    }
    if( timeSet.size() == 0 )
        timeSet.insert( 0. );
    return( timeSet );
}

ChannelDataList DataSet::getDataAtTime( const double time )
{
    ChannelDataList dataList;
    BOOST_FOREACH( ChannelDataNameMap::value_type channelPair, _data )
    {
        const std::string& name( channelPair.first );
        ChannelDataPtr cdp( channelPair.second );
        ChannelDataComposite* comp( dynamic_cast< ChannelDataComposite* >( cdp.get() ) );
        if( comp != NULL )
            dataList.push_back( comp->getChannel( time ) );
        else
            dataList.push_back( cdp );
    }
    return( dataList );
}

void DataSet::setInputs( OperationBasePtr opPtr, ChannelDataList& currentData )
{
    ChannelDataList newList;
    const OperationBase::StringList& inputs( opPtr->getInputNames() );
    BOOST_FOREACH( const std::string& inputName, inputs )
    {
        ChannelDataPtr cdp( lfx::findChannelData( inputName, currentData ) );
        if( cdp == NULL )
            OSG_WARN << "DataSet::swapData: Could not find data named \"" << inputName << "\"." << std::endl;
        newList.push_back( cdp );
    }
    opPtr->setInputs( newList );
}


bool DataSet::checkAndResizeMask()
{
    unsigned int size( 0 );

    BOOST_FOREACH( ChannelDataNameMap::value_type channelPair, _data )
    {
        ChannelData* cdp( channelPair.second.get() );
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
