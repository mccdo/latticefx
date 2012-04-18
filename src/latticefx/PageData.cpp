
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
    _rangeDataVec( rhs._rangeDataVec ),
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

void PageData::setRangeData( const unsigned int childIndex, const RangeData rangeData )
{
    if( childIndex >= _rangeDataVec.size() )
        _rangeDataVec.resize( childIndex + 1 );
    _rangeDataVec[ childIndex ] = rangeData;
}
PageData::RangeData PageData::getRangeData( const unsigned int childIndex ) const
{
    if( childIndex >= _rangeDataVec.size() )
    {
        std::cerr << "RangeData: childIndex " << childIndex << " out of range." << std::endl;
        return( RangeData() );
    }
    else
        return( _rangeDataVec[ childIndex ] );
}

void PageData::setParent( osg::Group* parent )
{
    _parent = parent;
}
osg::Group* PageData::getParent()
{
    return( _parent.get() );
}



// lfx
}
