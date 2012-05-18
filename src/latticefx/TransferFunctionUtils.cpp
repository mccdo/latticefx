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

#include <latticefx/TransferFunctionUtils.h>
#include <osg/Image>
#include <osg/Notify>
#include <osgDB/FileUtils>

#include <osg/io_utils>
#include <iostream>


namespace lfx {


osg::Image* loadImageFromDat( const std::string& fileName,
    const unsigned int alphaPolicy, float alpha )
{
    std::string fullName( osgDB::findDataFile( fileName ) );
    if( fullName.empty() )
    {
        OSG_WARN << "loadImageFromDat(): Could not find \"" << fileName << "\"." << std::endl;
        return( NULL );
    }

    osg::Vec4Array* array( new osg::Vec4Array );
    array->resize( 256 );
    std::ifstream istr( fullName.c_str() );
    while( !( istr.eof() ) )
    {
        unsigned int index;
        osg::Vec3 rgb;
        istr >> index;
        if( istr.fail() )
            break;
        istr >> rgb[0] >> rgb[1] >> rgb[2];
        rgb *= 1.f/255.f;
        
        if( index+1 > array->size() )
            array->resize( index+1 );
        (*array)[ index ] = osg::Vec4( rgb, alpha );
    }
    istr.close();

    if( alphaPolicy != LFX_ALPHA_CONSTANT )
    {
        unsigned int idx, numIndices( array->size() );
        for( idx=0; idx<numIndices; ++idx )
        {
            float rampAlpha;
            if( alphaPolicy == LFX_ALPHA_RAMP_0_TO_1 )
                rampAlpha = (float)idx / (float)( numIndices-1 );
            else // LFX_ALPHA_RAMP_1_TO_0
                rampAlpha = 1.f - (float)idx / (float)( numIndices-1 );
            (*array)[ idx ][ 3 ] = rampAlpha;
        }
    }

    osg::ref_ptr< osg::Image > image( new osg::Image );
    image->setImage( array->size(), 1, 1, GL_RGBA, GL_RGBA, GL_FLOAT,
        (unsigned char*)( &((*array)[0]) ), osg::Image::USE_NEW_DELETE );


    return( image.release() );
}


// lfx
}
