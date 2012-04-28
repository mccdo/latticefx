
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

void DataSet::addChannel( const ChannelDataPtr channel )
{
    _data.push_back( channel );
    setDirty( ALL_DIRTY );
    checkAndResizeMask();
}

ChannelDataPtr DataSet::getChannel( const std::string& name )
{
    BOOST_FOREACH( ChannelDataPtr cdp, _data )
    {
        if( cdp->getName() == name )
            return( cdp );
    }

    return( ChannelDataPtr( ( ChannelData* )( NULL ) ) );
}

const ChannelDataPtr DataSet::getChannel( const std::string& name ) const
{
    DataSet* nonConstThis = const_cast< DataSet* >( this );
    return( nonConstThis->getChannel( name ) );
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
    BOOST_FOREACH( ChannelDataPtr cdp, _data )
    {
        cdp->reset();
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

            // The ChannelData attached as input to the operations might be ChannelDataComposite,
            // in which case there is no real data attached. But we now have the real data in
            // the currentData list. So, swap out the (possibly) abstart ChannelData for the
            // real data. We'll restore them after the operation executes.
            ChannelDataList saveData = swapData( opPtr, currentData );

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

            // Restore the original ChannelData.
            swapData( opPtr, saveData );
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

            ChannelDataList saveData = swapData( _renderer, currentData );
            _sceneGraph->addChild( _renderer->getSceneGraph( _mask ) );
            swapData( _renderer, saveData );
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
    BOOST_FOREACH( ChannelDataPtr cdp, _data )
    {
        const std::string& name( cdp->getName() );
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
    BOOST_FOREACH( ChannelDataPtr cdp, _data )
    {
        const std::string& name( cdp->getName() );
        ChannelDataComposite* comp( dynamic_cast< ChannelDataComposite* >( cdp.get() ) );
        if( comp != NULL )
            dataList.push_back( comp->getChannel( name, time ) );
        else
            dataList.push_back( cdp );
    }
    return( dataList );
}

ChannelDataList DataSet::swapData( OperationBasePtr opPtr, ChannelDataList& currentData )
{
    ChannelDataList newList;
    ChannelDataList returnList( opPtr->getInputs() );
    BOOST_FOREACH( ChannelDataPtr cdp, returnList )
    {
        const std::string& name( cdp->getName() );
        ChannelDataPtr data( findChannelData( name, currentData ) );
        if( data == NULL )
            OSG_WARN << "DataSet::swapData: Could not find data named \"" << name << "\"." << std::endl;
        newList.push_back( data );
    }
    opPtr->setInputs( newList );
    return( returnList );
}


bool DataSet::checkAndResizeMask()
{
    unsigned int size( 0 );

    BOOST_FOREACH( ChannelDataPtr cdp, _data )
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
