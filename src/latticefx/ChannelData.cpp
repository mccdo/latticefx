#include <latticefx/ChannelData.h>


namespace lfx {


ChannelData::ChannelData( const std::string& name )
  : _name( name )
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


// lfx
}
