
#include <latticefx/ChannelDataComposite.h>

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
}

ChannelDataPtr ChannelDataComposite::getChannel( const std::string& name, const double time )
{
    return( _timeData[ time ] );
}
ChannelDataPtr ChannelDataComposite::getChannel( const std::string& name, const unsigned int level )
{
    return( ChannelDataPtr( ( ChannelData* )NULL ) );
}

const ChannelDataPtr ChannelDataComposite::getChannel( const std::string& name, const double time ) const
{
    return( ChannelDataPtr( ( ChannelData* )NULL ) );
}
const ChannelDataPtr ChannelDataComposite::getChannel( const std::string& name, const unsigned int level ) const
{
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
