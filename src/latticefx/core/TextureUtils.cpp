/*************** <auto-copyright.rb BEGIN do not edit this line> *************
*
* latticeFX is (C) Copyright 2011-2012 by Ames Laboratory
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; 
* version 2.1 of the License.
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
* -----------------------------------------------------------------
* Date modified: $Date$
* Version:       $Rev$
* Author:        $Author$
* Id:            $Id$
* -----------------------------------------------------------------
*
*************** <auto-copyright.rb END do not edit this line> **************/

#include <latticefx/core/TextureUtils.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/LogMacros.h>

#include <osg/Texture>
#include <osg/Texture3D>
#include <osg/Vec3>

#include <cmath>


namespace lfx {


// ceilPower2 - Originally in OpenGL Distilled example source code.
//
// Return next highest power of 2 greater than x
// if x is a power of 2, return x.
static unsigned short ceilPower2( unsigned short x )
{
    if( x == 0 )
        return( 1 );

    if( (x & (x-1)) == 0 )
        // x is a power of 2.
        return( x );

    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    return( x+1 );
}

osg::Vec3 computeTexture3DDimensions( const unsigned int numElements, const int flags )
{
    double dim( pow( (double)numElements, 1./3. ) );
    if( (double)( (int)dim ) != dim )
        dim = ceil( dim );

    if( flags == LFX_TEXUTILS_FORCE_UNIFORM )
        // Not using power of 2, but want uniform dimensions.
        return( osg::Vec3( dim, dim, dim ) );

    if( flags == 0 )
    {
        // Not using power of 2, and don't require uniform dimensions.
        if( dim*(dim-1.)*(dim-1.) >= numElements )
            return( osg::Vec3( dim, dim-1., dim-1. ) );
        else if( dim*dim*(dim-1.) >= numElements )
            return( osg::Vec3( dim, dim, dim-1. ) );
        else
            return( osg::Vec3( dim, dim, dim ) );
    }

    // Must have a power of 2.

    unsigned int dimPO2( ceilPower2( (unsigned short)dim ) );
    if( ( flags & LFX_TEXUTILS_FORCE_UNIFORM ) != 0 )
        // Must have uniform dimensions.
        return( osg::Vec3( dimPO2, dimPO2, dimPO2 ) );

    // Don't require uniform dimensions.
    if( dimPO2*(dimPO2>>1)*(dimPO2>>1) >= numElements )
        return( osg::Vec3( dimPO2, dimPO2>>1, dimPO2>>1 ) );
    else if( dimPO2*dimPO2*(dimPO2>>1) >= numElements )
        return( osg::Vec3( dimPO2, dimPO2, dimPO2>>1 ) );
    else
        return( osg::Vec3( dimPO2, dimPO2, dimPO2 ) );
}


osg::Texture3D* createTexture3DForInstancedRenderer( const ChannelDataPtr source )
{
    osg::ref_ptr< osg::Image > image( createImage3DForInstancedRenderer( source ) );
    if( image == NULL )
        return( NULL );

    osg::ref_ptr< osg::Texture3D > tex( new osg::Texture3D( image.get() ) );
    tex->setResizeNonPowerOfTwoHint( false );
    tex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    tex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
    return( tex.release() );
}

osg::Image* createImage3DForInstancedRenderer( const ChannelDataPtr source )
{
    const osg::Array* sourceArray( source->asOSGArray() );
    if( sourceArray == NULL )
    {
        LFX_WARNING_STATIC( "lfx.core", "createTexture3DForInstancedRenderer(): Unsupported source data." );
        return( NULL );
    }

    void* sourceData( NULL );
    GLenum dataFormat, dataType;
    GLint intFormat;
    switch( sourceArray->getType() )
    {
    case osg::Array::Vec3ArrayType:
    {
        const osg::Vec3Array* v3Array( dynamic_cast< const osg::Vec3Array* >( sourceArray ) );
        sourceData = const_cast< osg::Vec3* >( &( (*v3Array)[ 0 ] ) );
        intFormat = GL_RGB32F_ARB;
        dataFormat = GL_RGB;
        dataType = GL_FLOAT;
        break;
    }
    case osg::Array::Vec2ArrayType:
    {
        const osg::Vec2Array* v2Array( dynamic_cast< const osg::Vec2Array* >( sourceArray ) );
        sourceData = const_cast< osg::Vec2* >( &( (*v2Array)[ 0 ] ) );
        // Shader retrieves values from the 'ba' components.
        intFormat = GL_LUMINANCE_ALPHA32F_ARB;
        dataFormat = GL_LUMINANCE_ALPHA;
        dataType = GL_FLOAT;
        break;
    }
    case osg::Array::FloatArrayType:
    {
        const osg::FloatArray* fArray( dynamic_cast< const osg::FloatArray* >( sourceArray ) );
        sourceData = const_cast< GLfloat* >( &( (*fArray)[ 0 ] ) );
        // Shader retrieves values from the 'a' component.
        intFormat = GL_ALPHA32F_ARB;
        dataFormat = GL_ALPHA;
        dataType = GL_FLOAT;
        break;
    }
    default:
    {
        LFX_WARNING_STATIC( "lfx.core", "createTexture3DForInstancedRenderer(): Unsupported source data." );
        return( NULL );
    }
    }

    osg::Vec3 texDim( computeTexture3DDimensions( sourceArray->getNumElements() ) );

    osg::ref_ptr< osg::Image > image( new osg::Image );
    image->allocateImage( texDim[ 0 ], texDim[ 1 ], texDim[ 2 ], dataFormat, dataType );
    memcpy( image->data(), sourceData, sourceArray->getTotalDataSize() );
    image->setInternalTextureFormat( intFormat );
    image->setDataType( dataType );

    return( image.release() );
}


// lfx
}
