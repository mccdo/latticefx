
#include <latticefx/PageData.h>

#include <iostream>


namespace lfx {


PageData::PageData()
  : osg::Object(),
    _rangeMode( UNSPECIFIED_RANGE ),
    _parent( NULL )
{
}
PageData::PageData( const PageData& rhs, const osg::CopyOp& copyOp )
  : osg::Object( rhs, copyOp ),
    _rangeMode( rhs._rangeMode ),
    _rangeDataMap( rhs._rangeDataMap ),
    _parent( rhs._parent )
{
}
PageData::~PageData()
{
}

void PageData::setRangeMode( const RangeMode rangeMode )
{
    _rangeMode = rangeMode;
}
PageData::RangeMode PageData::getRangeMode() const
{
    return( _rangeMode );
}


void PageData::setRangeData( const unsigned int childIndex, const RangeData& rangeData )
{
    _rangeDataMap[ childIndex ] = rangeData;
}
PageData::RangeData* PageData::getRangeData( const unsigned int childIndex )
{
    RangeDataMap::iterator it( _rangeDataMap.find( childIndex ) );
    if( it != _rangeDataMap.end() )
        return( &( it->second ) );
    return( NULL );
}
const PageData::RangeData* PageData::getRangeData( const unsigned int childIndex ) const
{
    RangeDataMap::const_iterator it( _rangeDataMap.find( childIndex ) );
    if( it != _rangeDataMap.end() )
        return( &( it->second ) );
    return( NULL );
}
PageData::RangeDataMap& PageData::getRangeDataMap()
{
    return( _rangeDataMap );
}

void PageData::setParent( osg::Group* parent )
{
    _parent = parent;
}
osg::Group* PageData::getParent()
{
    return( _parent.get() );
}



PageData::RangeData::RangeData()
  : _childIndex( 0 ),
    _rangeValues( RangeValues( 0., FLT_MAX ) ),
    _status( UNLOADED )
{
}
PageData::RangeData::RangeData( double minVal, double maxVal, const std::string& fileName )
  : _childIndex( 0 ),
    _rangeValues( RangeValues( minVal, maxVal ) ),
    _fileName( fileName ),
    _status( UNLOADED )
{
}



// lfx
}
