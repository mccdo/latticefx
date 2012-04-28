
#include <latticefx/ChannelData.h>

#include <boost/foreach.hpp>


namespace lfx {


ChannelData::ChannelData( const std::string& name )
  : _name( name )
{
}
ChannelData::ChannelData( const ChannelData& rhs )
  : _name( rhs._name )
{
}
ChannelData::~ChannelData()
{
}


void ChannelData::setName( const std::string& name )
{
    _name = name;
}
const std::string& ChannelData::getName() const
{
    return( _name );
}



ChannelDataPtr findChannelData( const std::string& name, const ChannelDataList& dataList )
{
    BOOST_FOREACH( ChannelDataPtr cdp, dataList )
    {
        if( cdp->getName() == name )
            return( cdp );
    }
    return( ChannelDataPtr( ( ChannelData* )NULL ) );
}


// lfx
}
