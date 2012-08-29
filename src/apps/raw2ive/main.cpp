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

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable: 4275) //non-dll interface
#include <boost/program_options.hpp>
#pragma warning(pop)
#else
#include <boost/program_options.hpp>
#endif
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/assert.hpp>

#include <osg/Image>
#include <osg/Texture3D>
#include <osg/Texture2D>
#include <osg/io_utils>

#include <osgDB/WriteFile>
#include <osgDB/ReadFile>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main( int argc, char** argv )
{
    int xSize = 0, ySize = 0, zSize = 0, numComponents = 0;
    std::string raw_filename;
    po::options_description desc( "Allowed options" );
    desc.add_options()
    ( "help", "Produce help message" )
    ( "xsize,x", po::value< int >( &xSize )->default_value( 256 ), "Set the x size of the texture" )
    ( "ysize,y", po::value< int >( &ySize )->default_value( 256 ), "Set the y size of the texture" )
    ( "zsize,z", po::value< int >( &zSize )->default_value( 256 ), "Set the z size of the texture" )
    ( "components,c", po::value< int >( &numComponents )->default_value( 1 ), "The number of components in the image" )
    ( "filename,f", po::value< std::string >( &raw_filename ), "The raw filename" );

    po::positional_options_description p;
    p.add("filename", -1);
    
    po::variables_map vm;
    //po::store( po::parse_command_line( argc, argv, desc ), vm );
    po::store( po::command_line_parser( argc, argv ).options(desc).positional(p).run(), vm );
    po::notify( vm );
    
    if( vm.count( "help" ) || !vm.count( "filename" ) )
    {
        std::cout << "To us this tool: " << argv[ 0 ] << " -x <x res> -y <y res> -z <z res> <filename>.raw" << std::endl;
        std::cout << desc << std::endl;
        return 1;
    }

    fs::path pathName( raw_filename );
    if( !fs::exists( pathName ) )
    {
        std::cout << "Please specify a valid file." << std::endl;
        return 1;
    }

    std::ifstream rawStream(raw_filename.c_str(), std::ifstream::binary);    
    
    // get length of file:
    rawStream.seekg(0, std::ios::end);
    size_t length = rawStream.tellg();
    rawStream.seekg(0, std::ios::beg);
    
    char* buffer = new char[ length ];
    
    // read data as a block:
    rawStream.read(buffer,length);
    rawStream.close();
    
    size_t numPixels = xSize * ySize * zSize;
    unsigned char* pixels( new unsigned char[ numPixels ] );
    unsigned char* pixelPtr( pixels );
    
    BOOST_ASSERT_MSG( length == numPixels, "The length of the file is different than the size specified on the command line." );
    for( size_t i=0; i < length; ++i ) 
    { 
        *pixelPtr++ = buffer[ i ];
    } 
    delete [] buffer;
    buffer = 0;

    osg::ref_ptr< osg::Image > image = new osg::Image();
    //We will let osg manage the raw image data
    image->setImage( xSize, ySize, zSize, 
                    GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                    pixels, osg::Image::USE_NEW_DELETE );
    
    pathName.replace_extension( ".ive" );
    osgDB::writeImageFile( *image, pathName.string() );
    
    return 0;
}
