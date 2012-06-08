/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * VE-Suite is (C) Copyright 1998-2012 by Iowa State University
 *
 * Original Development Team:
 *   - ISU's Thermal Systems Virtual Engineering Group,
 *     Headed by Kenneth Mark Bryden, Ph.D., www.vrac.iastate.edu/~kmbryden
 *   - Reaction Engineering International, www.reaction-eng.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
 *************** <auto-copyright.rb END do not edit this line> ***************/
#include "VectorFieldData.h"

#include <osg/Texture2D>
using namespace ves::xplorer::scenegraph;

////////////////////////////////////////////////////////////////////////////////
VectorFieldData::VectorFieldData()
    : 
    _pos( 0 ),
    _dir( 0 ),
    _scalar( 0 ),
    _dia( 0 )
{
    ;
}
////////////////////////////////////////////////////////////////////////////////
void VectorFieldData::loadData()
{
    internalLoad();
}
////////////////////////////////////////////////////////////////////////////////
osg::Texture3D* VectorFieldData::getPositionTexture()
{
    return( _texPos.get() );
}
////////////////////////////////////////////////////////////////////////////////
osg::Texture3D* VectorFieldData::getDirectionTexture()
{
    return( _texDir.get() );
}
////////////////////////////////////////////////////////////////////////////////
osg::Texture3D* VectorFieldData::getScalarTexture()
{
    return( _texScalar.get() );
}
////////////////////////////////////////////////////////////////////////////////
osg::Texture3D* VectorFieldData::getDiameterTexture()
{
    return( _diaScalar.get() );
}
////////////////////////////////////////////////////////////////////////////////
osg::Vec3s VectorFieldData::getTextureSizes()
{
    return( _texSizes );
}
////////////////////////////////////////////////////////////////////////////////
unsigned int VectorFieldData::getDataCount()
{
    return( _dataSize );
}
////////////////////////////////////////////////////////////////////////////////
VectorFieldData::~VectorFieldData()
{
    ;
}
////////////////////////////////////////////////////////////////////////////////
void VectorFieldData::internalLoad()
{
    // TBD Actual data size would come from file.
    _dataSize = 0;
    
    // Determine optimal 3D texture dimensions.
    int s, t, p;
    compute3DTextureSize( getDataCount(), s, t, p );
    _texSizes = osg::Vec3s( s, t, p );
    
    // Allocate memory for data.
    unsigned int size( getDataCount() );
    _pos = new float[ size * 3 ];
    _dir = new float[ size * 3 ];
    _scalar = new float[ size ];
    
    // TBD You would replace this line with code to load the data from file.
    // In this example, we just generate test data.
    
    _texPos = makeFloatTexture( (unsigned char*)_pos, 3, osg::Texture2D::NEAREST );
    _texDir = makeFloatTexture( (unsigned char*)_dir, 3, osg::Texture2D::NEAREST );
    _texScalar = makeFloatTexture( (unsigned char*)_scalar, 1, osg::Texture2D::NEAREST );
}
////////////////////////////////////////////////////////////////////////////////
unsigned short VectorFieldData::ceilPower2( unsigned short x )
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
////////////////////////////////////////////////////////////////////////////////
void VectorFieldData::compute3DTextureSize( unsigned int dataCount, int& s, int& t, int& p )
{
    // GL_MAX_3D_TEXTURE_SIZE min value is 16.
    // NVIDIA seems to support 2048.
    s = 256;
    while( dataCount / s == 0 )
        s >>= 1;
    
    float sliceSize( (float) dataCount / (float) s );
    float tDim = sqrtf( sliceSize );
    if( tDim > 65535.f )
        osg::notify( osg::FATAL ) << "compute3DTextureSize: Value too large " << tDim << std::endl;
    if( tDim == (int)tDim )
        t = ceilPower2( (unsigned short)( tDim ) );
    else
        t = ceilPower2( (unsigned short)( tDim ) + 1 );
    
    float pDim = sliceSize / ((float)t);
    if( pDim == ((int)sliceSize) / t)
        p = ceilPower2( (unsigned short)( pDim ) );
    else
        p = ceilPower2( (unsigned short)( pDim ) + 1 );
    osg::notify( osg::DEBUG_INFO ) << "dataCount " << dataCount <<
    " produces tex size (" << s << "," << t << "," << p <<
    "), total storage: " << s*t*p << std::endl;
}
////////////////////////////////////////////////////////////////////////////////
osg::Texture3D* VectorFieldData::makeFloatTexture( unsigned char* data, int numComponents, osg::Texture::FilterMode filter )
{
    int s( _texSizes.x() );
    int t( _texSizes.y() );
    int p( _texSizes.z() );
    
    GLenum intFormat, pixFormat;
    if( numComponents == 1 )
    {
        intFormat = GL_ALPHA32F_ARB;
        pixFormat = GL_ALPHA;
    }
    else
    {
        // Must be 3 for now.
        intFormat = GL_RGB32F_ARB;
        pixFormat = GL_RGB;
    }
    osg::Image* image = new osg::Image();
    //We will let osg manage the raw image data
    image->setImage( s, t, p, intFormat, pixFormat, GL_FLOAT,
        data, osg::Image::USE_NEW_DELETE );
    osg::Texture3D* texture = new osg::Texture3D( image );
    texture->setFilter( osg::Texture::MIN_FILTER, filter );
    texture->setFilter( osg::Texture::MAG_FILTER, filter );
    return( texture );
}
////////////////////////////////////////////////////////////////////////////////
