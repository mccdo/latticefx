
#include <latticefx/MaskUtils.h>
#include <latticefx/ChannelDataOSGArray.h>


namespace lfx {


ChannelDataPtr getMaskedChannel( const ChannelDataPtr source, const ChannelDataPtr mask )
{
    if( ( source == NULL ) || ( mask == NULL ) )
        return( ChannelDataPtr( ( ChannelData* )( NULL ) ) );

    // Super hack: assume Vec3Array for now.
    const osg::Vec3Array* osgSource( static_cast< osg::Vec3Array* >( source->asOSGArray() ) );
    osg::Vec3Array::const_iterator srcIt;
    const osg::ByteArray* osgMask( static_cast< osg::ByteArray* >( mask->asOSGArray() ) );
    osg::ByteArray::const_iterator maskIt;
    osg::ref_ptr< osg::Vec3Array > maskedData( new osg::Vec3Array );
    maskedData->resize( osgSource->size() );
    osg::Vec3Array::iterator destIt;

    unsigned int count( 0 );
    for( srcIt=osgSource->begin(), maskIt=osgMask->begin(), destIt=maskedData->begin();
        srcIt != osgSource->end(); ++srcIt, ++maskIt )
    {
        if( *maskIt != 0 )
        {
            *destIt = *srcIt;
            count++;
            ++destIt;
        }
    }
    maskedData->resize( count );

    ChannelDataOSGArrayPtr newData( new ChannelDataOSGArray( maskedData.get(), source->getName() ) );
    return( newData );
}


// lfx
}
