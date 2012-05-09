
#include <latticefx/MaskUtils.h>
#include <latticefx/ChannelDataOSGArray.h>

#include <osg/Notify>


namespace lfx {


ChannelDataPtr getMaskedChannel( const ChannelDataPtr source, const ChannelDataPtr mask )
{
    if( ( source == NULL ) || ( mask == NULL ) )
        return( ChannelDataPtr( ( ChannelData* )( NULL ) ) );

    const osg::ByteArray* osgMask( static_cast< osg::ByteArray* >( mask->asOSGArray() ) );
    osg::ByteArray::const_iterator maskIt;

    // Shortcut to avoid data copy. If all mask values are 1, just return
    // the input array. (Loop, looking for non-1 values.)
    bool masked( false );
    for( maskIt=osgMask->begin(); maskIt != osgMask->end(); ++maskIt )
    {
        if( *maskIt != 1 )
        {
            masked = true;
            break;
        }
    }
    if( !masked )
        // All mask values are 1. No masking. Return the input array.
        return( source );

    osg::Array* sourceArray( source->asOSGArray() );
    switch( sourceArray->getType() )
    {
    case osg::Array::Vec3ArrayType:
    {
        const osg::Vec3Array* osgSource( static_cast< osg::Vec3Array* >( sourceArray ) );
        osg::Vec3Array::const_iterator srcIt;
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
    case osg::Array::FloatArrayType:
    {
        const osg::FloatArray* osgSource( static_cast< osg::FloatArray* >( sourceArray ) );
        osg::FloatArray::const_iterator srcIt;
        osg::ref_ptr< osg::FloatArray > maskedData( new osg::FloatArray );
        maskedData->resize( osgSource->size() );
        osg::FloatArray::iterator destIt;

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
    default:
    {
        OSG_WARN << "getMaskedChannel(): Array type not supported." << std::endl;
        return( ChannelDataPtr( ( ChannelData* )( NULL ) ) );
    }
    }
}


// lfx
}
