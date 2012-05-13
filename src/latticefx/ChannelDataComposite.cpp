
#include <latticefx/ChannelDataComposite.h>
#include <osg/Notify>
#include <osg/math>

#include <boost/foreach.hpp>


namespace lfx {


ChannelDataComposite::ChannelDataComposite( const std::string& name )
  : ChannelData( name )
{
}
ChannelDataComposite::ChannelDataComposite( const ChannelDataComposite& rhs )
  : ChannelData( rhs )
{
}
ChannelDataComposite::~ChannelDataComposite()
{
}

void ChannelDataComposite::addChannel( const ChannelDataPtr channel, const double time )
{
    _timeData[ time ] = channel;
}
void ChannelDataComposite::addChannel( const ChannelDataPtr channel, const unsigned int level )
{
    OSG_WARN << "Composite LOD not yet supported." << std::endl;
}

ChannelDataPtr ChannelDataComposite::getChannel( const double time )
{
    TimeDataMap::iterator dataIt( _timeData.find( time ) );
    if( dataIt == _timeData.end() )
    {
        // 'time' not in map; return data for previous time.
        // TBD Need more intelligence than this:
        return( _timeData.begin()->second );
    }
    else
        return( dataIt->second );
}
ChannelDataPtr ChannelDataComposite::getChannel( const unsigned int level )
{
    OSG_WARN << "Composite LOD not yet supported." << std::endl;
    return( ChannelDataPtr( ( ChannelData* )NULL ) );
}

const ChannelDataPtr ChannelDataComposite::getChannel( const double time ) const
{
    ChannelDataComposite* nonConstThis = const_cast< ChannelDataComposite* >( this );
    return( nonConstThis->getChannel( time ) );
}
const ChannelDataPtr ChannelDataComposite::getChannel( const unsigned int level ) const
{
    OSG_WARN << "Composite LOD not yet supported." << std::endl;
    return( ChannelDataPtr( ( ChannelData* )NULL ) );
}

const TimeSet ChannelDataComposite::getTimeSet() const
{
    TimeSet timeSet;
    BOOST_FOREACH( TimeDataMap::value_type timeData, _timeData )
    {
        timeSet.insert( timeData.first );
    }
    return( timeSet );
}

void ChannelDataComposite::getDimensions( unsigned int& x, unsigned int& y, unsigned int& z )
{
    x = y = z = 0;
    BOOST_FOREACH( TimeDataMap::value_type timeData, _timeData )
    {
        unsigned int lx, ly, lz;
        timeData.second->getDimensions( lx, ly, lz );
        x = osg::maximum( x, lx );
        y = osg::maximum( y, ly );
        z = osg::maximum( z, lz );
    }
}


// lfx
}
