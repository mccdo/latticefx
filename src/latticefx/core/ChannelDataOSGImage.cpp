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

#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/LogMacros.h>

#include <boost/foreach.hpp>
#include <cstring>


namespace lfx
{
namespace core
{


ChannelDataOSGImage::ChannelDataOSGImage( const std::string& name, osg::Image* image )
    : ChannelData( name )
{
    if( image != NULL )
    {
        setImage( image );
        reset();
    }
}
ChannelDataOSGImage::ChannelDataOSGImage( const ChannelDataOSGImage& rhs )
    : ChannelData( rhs ),
      _image( rhs._image )
{
    if( _image != NULL )
    {
        reset();
    }
}
ChannelDataOSGImage::~ChannelDataOSGImage()
{
}


void ChannelDataOSGImage::setDBKey( const DBKey dbKey )
{
    ChannelData::setDBKey( dbKey );
}


char* ChannelDataOSGImage::asCharPtr()
{
    if( _workingImage != NULL )
    {
        return( const_cast< char* >( ( const char* )( _workingImage->data() ) ) );
    }
    else
    {
        return( NULL );
    }
}
const char* ChannelDataOSGImage::asCharPtr() const
{
    ChannelDataOSGImage* nonConstThis( const_cast< ChannelDataOSGImage* >( this ) );
    return( nonConstThis->asCharPtr() );
}

void ChannelDataOSGImage::setImage( osg::Image* image )
{
    _image = image;

    if( image != NULL )
    {
        setDimensions( image->s(), image->t(), image->r() );
    }
    else
    {
        setDimensions( 0, 0, 0 );
    }
}
osg::Image* ChannelDataOSGImage::getImage()
{
    return( _workingImage.get() );
}
const osg::Image* ChannelDataOSGImage::getImage() const
{
    ChannelDataOSGImage* nonConstThis( const_cast< ChannelDataOSGImage*>( this ) );
    return( nonConstThis->getImage() );
}

ChannelDataPtr ChannelDataOSGImage::getMaskedChannel( const ChannelDataPtr maskIn )
{
    LFX_WARNING( "OSGImage::getMaskedChannel(): Host mask not yet implemented." );
    return( shared_from_this() );
}

void ChannelDataOSGImage::reset()
{
    if( _workingImage == NULL )
    {
        if( _image == NULL )
        {
            LFX_WARNING( "OSGImage::reset(): _image == NULL." );
        }
        else
            _workingImage = new osg::Image( *_image, osg::CopyOp::DEEP_COPY_ALL );
    }
    else if( _workingImage->data() != NULL )
    {
        // Only do the copy if _workingImage has data. It will not have any data
        // if the image has not yet been paged in.
        std::memcpy( _workingImage->data(), _image->data(),
                _image->getTotalSizeInBytes() );
    }
}


// core
}
// lfx
}
