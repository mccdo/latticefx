/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * Copyright 2012-2012 by Ames Laboratory
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *************** <auto-copyright.rb END do not edit this line> ***************/

#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/LogMacros.h>



namespace lfx
{
namespace core
{


/** \def AS_CHAR_PTR
\param __T is the name of a class derived from osg::Array (e.g., Vec3Array).
\param __A is an osg::Array*, but the instance tupe must match __T.
*/
#define AS_CHAR_PTR(__T,__A) \
        (char*)( &( (*( static_cast< __T* >( __A ) ))[ 0 ] ) )
#define AS_CONST_CHAR_PTR(__T,__A) \
        (const char*)( &( (*( static_cast< const __T* >( __A ) ))[ 0 ] ) )



ChannelDataOSGArray::ChannelDataOSGArray( const std::string& name, osg::Array* data )
    : ChannelData( name )
{
    if( data != NULL )
    {
        setOSGArray( data );
        reset();
    }
}
ChannelDataOSGArray::ChannelDataOSGArray( const ChannelDataOSGArray& rhs )
    : ChannelData( rhs ),
      _data( rhs._data )
{
    if( _data != NULL )
    {
        reset();
    }
}
ChannelDataOSGArray::~ChannelDataOSGArray()
{
}


void ChannelDataOSGArray::setOSGArray( osg::Array* data )
{
    _data = data;

    if( ( data != NULL ) && ( data->getNumElements() > 0 ) )
    {
        setDimensions( data->getNumElements(), 1, 1 );
    }
    else
    {
        setDimensions( 0, 0, 0 );
    }
}


char* ChannelDataOSGArray::asCharPtr()
{
    switch( _workingData->getType() )
    {
    case osg::Array::ByteArrayType:
        return( AS_CHAR_PTR( osg::ByteArray, _workingData.get() ) );
        break;
    case osg::Array::IntArrayType:
        return( AS_CHAR_PTR( osg::IntArray, _workingData.get() ) );
        break;
    case osg::Array::FloatArrayType:
        return( AS_CHAR_PTR( osg::FloatArray, _workingData.get() ) );
        break;
    case osg::Array::DoubleArrayType:
        return( AS_CHAR_PTR( osg::DoubleArray, _workingData.get() ) );
        break;
    case osg::Array::Vec2ArrayType:
        return( AS_CHAR_PTR( osg::Vec2Array, _workingData.get() ) );
        break;
    case osg::Array::Vec3ArrayType:
        return( AS_CHAR_PTR( osg::Vec3Array, _workingData.get() ) );
        break;
    case osg::Array::Vec4ArrayType:
        return( AS_CHAR_PTR( osg::Vec4Array, _workingData.get() ) );
        break;
    default:
    {
        LFX_WARNING( "OSGArray::asCharPtr(): Unsupported array type." );
        return( NULL );
        break;
    }
    }
}
const char* ChannelDataOSGArray::asCharPtr() const
{
    ChannelDataOSGArray* nonConstThis( const_cast< ChannelDataOSGArray* >( this ) );
    return( nonConstThis->asCharPtr() );
}

osg::Array* ChannelDataOSGArray::asOSGArray()
{
    return( _workingData.get() );
}
const osg::Array* ChannelDataOSGArray::asOSGArray() const
{
    ChannelDataOSGArray* nonConstThis( const_cast< ChannelDataOSGArray* >( this ) );
    return( nonConstThis->asOSGArray() );
}

ChannelDataPtr ChannelDataOSGArray::getMaskedChannel( const ChannelDataPtr maskIn )
{
#define MASK_LOOP( _sIt, _sArr, _mIt, _mArr, _dIt, _dArr ) \
    { \
        unsigned int count( 0 ); \
        _dArr->resize( _sArr->size() ); \
        for( _sIt=_sArr->begin(), _mIt=_mArr->begin(), _dIt=_dArr->begin(); \
            _sIt != _sArr->end(); ++_sIt, ++_mIt ) \
        { \
            if( *_mIt != 0 ) \
            { \
                *_dIt  = *_sIt; \
                count++; \
                ++_dIt; \
            } \
        } \
        _dArr->resize( count ); \
    }

    if( maskIn == NULL )
    {
        return( ChannelDataPtr( ( ChannelData* )( NULL ) ) );
    }

    const osg::ByteArray* osgMask( static_cast< osg::ByteArray* >( maskIn->asOSGArray() ) );
    osg::ByteArray::const_iterator maskIt;

    // Shortcut to avoid data copy. If all mask values are 1, just return
    // the input array. (Loop, looking for non-1 values.)
    bool masked( false );
    for( maskIt = osgMask->begin(); maskIt != osgMask->end(); ++maskIt )
    {
        if( *maskIt != 1 )
        {
            masked = true;
            break;
        }
    }
    if( !masked )
        // All mask values are 1. No masking. Return the input array.
    {
        return( shared_from_this() );
    }

    osg::Array* sourceArray( asOSGArray() );
    switch( sourceArray->getType() )
    {
    case osg::Array::FloatArrayType:
    {
        const osg::FloatArray* osgSource( static_cast< osg::FloatArray* >( sourceArray ) );
        osg::FloatArray::const_iterator srcIt;
        osg::ref_ptr< osg::FloatArray > maskedData( new osg::FloatArray );
        osg::FloatArray::iterator destIt;

        MASK_LOOP( srcIt, osgSource, maskIt, osgMask, destIt, maskedData );

        ChannelDataOSGArrayPtr newData( new ChannelDataOSGArray( getName(), maskedData.get() ) );
        return( newData );
    }
    case osg::Array::Vec2ArrayType:
    {
        const osg::Vec2Array* osgSource( static_cast< osg::Vec2Array* >( sourceArray ) );
        osg::Vec2Array::const_iterator srcIt;
        osg::ref_ptr< osg::Vec2Array > maskedData( new osg::Vec2Array );
        osg::Vec2Array::iterator destIt;

        MASK_LOOP( srcIt, osgSource, maskIt, osgMask, destIt, maskedData );

        ChannelDataOSGArrayPtr newData( new ChannelDataOSGArray( getName(), maskedData.get() ) );
        return( newData );
    }
    case osg::Array::Vec3ArrayType:
    {
        const osg::Vec3Array* osgSource( static_cast< osg::Vec3Array* >( sourceArray ) );
        osg::Vec3Array::const_iterator srcIt;
        osg::ref_ptr< osg::Vec3Array > maskedData( new osg::Vec3Array );
        osg::Vec3Array::iterator destIt;

        MASK_LOOP( srcIt, osgSource, maskIt, osgMask, destIt, maskedData );

        ChannelDataOSGArrayPtr newData( new ChannelDataOSGArray( getName(), maskedData.get() ) );
        return( newData );
    }
    default:
    {
        LFX_WARNING( "getMaskedChannel(): Array type not supported." );
        return( ChannelDataPtr( ( ChannelData* )( NULL ) ) );
    }
    }
#undef MASK_LOOP
}


void ChannelDataOSGArray::setAll( const char value )
{
    switch( _workingData->getType() )
    {
    case osg::Array::ByteArrayType:
    {
        osg::ByteArray* array( static_cast< osg::ByteArray* >( _workingData.get() ) );
        osg::ByteArray::iterator it;
        for( it = array->begin(); it != array->end(); ++it )
        {
            *it = value;
        }
        break;
    }
    default:
    {
        LFX_WARNING( "OSGArray::setAll(const char): Unsupported array type." );
        break;
    }
    }
}
void ChannelDataOSGArray::setAll( const float value )
{
    switch( _workingData->getType() )
    {
    case osg::Array::FloatArrayType:
    {
        osg::FloatArray* array( static_cast< osg::FloatArray* >( _workingData.get() ) );
        osg::FloatArray::iterator it;
        for( it = array->begin(); it != array->end(); ++it )
        {
            *it = value;
        }
        break;
    }
    default:
    {
        LFX_WARNING( "OSGArray::setAll(const float): Unsupported array type." );
        break;
    }
    }
}

void ChannelDataOSGArray::andValues( const ChannelData* rhs )
{
    const char* inPtr( rhs->asCharPtr() );
    if( inPtr == NULL )
    {
        return;
    }

    switch( _workingData->getType() )
    {
    case osg::Array::ByteArrayType:
    {
        osg::ByteArray* array( static_cast< osg::ByteArray* >( _workingData.get() ) );
        osg::ByteArray::iterator it;
        for( it = array->begin(); it != array->end(); ++it )
        {
            *it = ( ( *it != 0 ) && ( *inPtr++ != 0 ) )  ?  1 : 0;
        }
        break;
    }
    default:
    {
        LFX_WARNING( "OSGArray::andValues(): Unsupported array type." );
        break;
    }
    }
}

void ChannelDataOSGArray::reset()
{
    if( _workingData == NULL )
        _workingData = dynamic_cast< osg::Array* >(
                           _data->clone( osg::CopyOp::DEEP_COPY_ALL ) );
    else if( _data->getNumElements() > 0 )
        // Only copy if there is data in the array.
    {
        copyArray( _workingData.get(), _data.get() );
    }
}

void ChannelDataOSGArray::resize( size_t size )
{
    if( _workingData == NULL )
    {
        reset();
    }

    switch( _data->getType() )
    {
    case osg::Array::ByteArrayType:
        ( dynamic_cast< osg::ByteArray* >( _data.get() ) )->resize( size );
        ( dynamic_cast< osg::ByteArray* >( _workingData.get() ) )->resize( size );
        break;
    case osg::Array::FloatArrayType:
        ( dynamic_cast< osg::FloatArray* >( _data.get() ) )->resize( size );
        ( dynamic_cast< osg::FloatArray* >( _workingData.get() ) )->resize( size );
        break;
    case osg::Array::Vec2ArrayType:
        ( dynamic_cast< osg::Vec2Array* >( _data.get() ) )->resize( size );
        ( dynamic_cast< osg::Vec2Array* >( _workingData.get() ) )->resize( size );
        break;
    case osg::Array::Vec3ArrayType:
        ( dynamic_cast< osg::Vec3Array* >( _data.get() ) )->resize( size );
        ( dynamic_cast< osg::Vec3Array* >( _workingData.get() ) )->resize( size );
        break;
    default:
    {
        LFX_WARNING( "OSGArray::resize(): Unsupported array type." );
        break;
    }
    }
}

void ChannelDataOSGArray::copyArray( osg::Array* lhs, const osg::Array* rhs )
{
    if( ( lhs->getType() != rhs->getType() ) ||
            ( lhs->getTotalDataSize() < rhs->getTotalDataSize() ) )
    {
        LFX_WARNING( "OSGArray::copyArray(): Array mismatch." );
        return;
    }

    const size_t sizeBytes( lhs->getTotalDataSize() );

    switch( lhs->getType() )
    {
    case osg::Array::ByteArrayType:
    {
        memcpy( AS_CHAR_PTR( osg::ByteArray, lhs ),
                AS_CONST_CHAR_PTR( osg::ByteArray, rhs ), sizeBytes );
        break;
    }
    case osg::Array::FloatArrayType:
    {
        memcpy( AS_CHAR_PTR( osg::FloatArray, lhs ),
                AS_CONST_CHAR_PTR( osg::FloatArray, rhs ), sizeBytes );
        break;
    }
    case osg::Array::Vec2ArrayType:
    {
        memcpy( AS_CHAR_PTR( osg::Vec2Array, lhs ),
                AS_CONST_CHAR_PTR( osg::Vec2Array, rhs ), sizeBytes );
        break;
    }
    case osg::Array::Vec3ArrayType:
    {
        memcpy( AS_CHAR_PTR( osg::Vec3Array, lhs ),
                AS_CONST_CHAR_PTR( osg::Vec3Array, rhs ), sizeBytes );
        break;
    }
    default:
    {
        LFX_WARNING( "OSGArray::copyArray(): Unsupported array type." );
        break;
    }
    }
}

osg::Vec3Array* ChannelDataOSGArray::convertToVec3Array( osg::Array* source )
{
    if( source->getType() == osg::Array::Vec3ArrayType )
    {
        return( static_cast< osg::Vec3Array* >( source ) );
    }

    osg::Vec3Array* result( new osg::Vec3Array );
    result->resize( source->getNumElements() );
    unsigned int idx;
    switch( source->getType() )
    {
    case osg::Array::ByteArrayType:
    {
        osg::ByteArray* byteSource( static_cast< osg::ByteArray* >( source ) );
        for( idx = 0; idx < source->getNumElements(); ++idx )
        {
            ( *result )[ idx ].set( ( float )( ( *byteSource )[ idx ] ), 0., 0. );
        }
        break;
    }
    case osg::Array::FloatArrayType:
    {
        osg::FloatArray* floatSource( static_cast< osg::FloatArray* >( source ) );
        for( idx = 0; idx < source->getNumElements(); ++idx )
        {
            ( *result )[ idx ].set( ( *floatSource )[ idx ], 0., 0. );
        }
        break;
    }
    case osg::Array::Vec2ArrayType:
    {
        osg::Vec2Array* vec2Source( static_cast< osg::Vec2Array* >( source ) );
        for( idx = 0; idx < source->getNumElements(); ++idx )
        {
            ( *result )[ idx ].set( ( *vec2Source )[ idx ].x(), ( *vec2Source )[ idx ].y(), 0. );
        }
        break;
    }
    default:
    {
        LFX_WARNING_STATIC( "lfx.core.channel", "OSGArray::convertToVec3Array(): Unsupported array type." );
        break;
    }
    }
    return( result );
}


// core
}
// lfx
}
