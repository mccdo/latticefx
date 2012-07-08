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
#include <iostream>

#include <osg/Image>
#include <osg/Texture3D>
#include <osg/Texture2D>
#include <osgDB/WriteFile>

#define CONE_HEIGHT 250
#define CONE_RADIUS 125

#define TEXTURE_X 256
#define TEXTURE_Y 256
#define TEXTURE_Z 256

#define TEXTURE_HALF_X 128
#define TEXTURE_HALF_Y 128

bool testVoxel( const int x, const int y, const int z )
{
    if( z >= CONE_HEIGHT )
        return false;

    const double radiusTest = double( (x - TEXTURE_HALF_X) * (x - TEXTURE_HALF_X) +
        (y - TEXTURE_HALF_Y) * (y - TEXTURE_HALF_Y) );
    
    double heightRadius =
        ( double( CONE_RADIUS ) * double( z ) / double( CONE_HEIGHT ) );
    heightRadius *= heightRadius;
    
    return( heightRadius >= radiusTest );
}

void writeVoxel( const size_t numPixels, unsigned char* pixels )
{
    osg::ref_ptr< osg::Image > image = new osg::Image();
    //We will let osg manage the raw image data
    image->setImage( TEXTURE_X, TEXTURE_Y, TEXTURE_Z, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                pixels, osg::Image::USE_NEW_DELETE );

    //osg::Texture3D* texture = new osg::Texture3D( image );
    //texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture2D::NEAREST );
    //texture->setFilter( osg::Texture::MAG_FILTER, osg::Texture2D::NEAREST );
    
    osgDB::writeImageFile( *image, "cone_texture.ive" );
    
    //delete texture;
    //delete image;
}

int main( int argc, char** argv )
{
    size_t numPixels = TEXTURE_X * TEXTURE_Y * TEXTURE_Z;
    unsigned char* pixels( new unsigned char[ numPixels ] );
    unsigned char* pixelPtr( pixels );
    for( size_t k = 0; k < TEXTURE_Z; ++k )
    {
        for( size_t j = 0; j < TEXTURE_Y; ++j )
        {
            for( size_t i = 0; i < TEXTURE_X; ++i )
            {
                *pixelPtr++ = ( testVoxel( i, j, k ) ? 255 : 0 );
            }
        }
    }

    writeVoxel( numPixels, pixels );

    //OSG handles the memory
    //delete [] pixels;
    return 0;
}
