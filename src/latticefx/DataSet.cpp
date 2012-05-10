
#include <latticefx/DataSet.h>
#include <latticefx/ChannelDataComposite.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/RootCallback.h>

#include <osg/Group>
#include <osg/Notify>

#include <boost/foreach.hpp>


namespace lfx {


DataSet::DataSet()
  : _sceneGraph( new osg::Group ),
    _dirtyFlags( ALL_DIRTY )
{
    RootCallback* rootcb( new RootCallback() );
    rootcb->addTimeSeriesParent( _sceneGraph.get() );
    _sceneGraph->setUpdateCallback( rootcb );

    PageData* pageData( new PageData );
    pageData->setRangeMode( PageData::TIME_RANGE );
    pageData->setParent( _sceneGraph.get() );
    _sceneGraph->setUserData( pageData );
}
DataSet::DataSet( const DataSet& rhs )
  : _data( rhs._data ),
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
    BOOST_FOREACH( ChannelDataNameMap::value_type channelPair, _data )
    {
        channelPair.second->reset();
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
        RootCallback* rootcb( static_cast< RootCallback* >( _sceneGraph->getUpdateCallback() ) );
        PageData* pageData( static_cast< PageData* >( _sceneGraph->getUserData() ) );
        unsigned int childIndex( 0 );
        double minTime( FLT_MAX ), maxTime( -FLT_MAX );

        ChannelDataList::iterator maskIt = _maskList.begin();
        TimeSet timeSet( getTimeSet() );
        BOOST_FOREACH( double time, timeSet )
        {
            // Get the data at the current time and assign as inputs to the Renderer.
            ChannelDataList currentData( getDataAtTime( time ) );
            setInputs( _renderer, currentData );

            PageData::RangeData rangeData( time, 0. );
            rangeData._status = PageData::RangeData::ACTIVE;
            pageData->setRangeData( childIndex, rangeData );
            ++childIndex;

            _sceneGraph->addChild( _renderer->getSceneGraph( *maskIt ) );
            ++maskIt;

            minTime = osg::minimum( minTime, time );
            maxTime = osg::maximum( maxTime, time );
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
