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

#include <latticefx/ChannelDataOSGImage.h>

#include <osg/Notify>

#include <boost/foreach.hpp>


namespace lfx {


ChannelDataOSGImage::ChannelDataOSGImage( const std::string& name, osg::Image* image )
  : ChannelData( name ),
    _image( image )
{
    reset();
}
ChannelDataOSGImage::ChannelDataOSGImage( const ChannelDataOSGImage& rhs )
  : ChannelData( rhs ),
    _image( rhs._image )
{
    reset();
}
ChannelDataOSGImage::~ChannelDataOSGImage()
{
}


char* ChannelDataOSGImage::asCharPtr()
{
    if( _image != NULL )
        return( const_cast< char* >( (const char*)( _image->data() ) ) );
    else
        return( NULL );
}
const char* ChannelDataOSGImage::asCharPtr() const
{
    if( _image != NULL )
        return( (char*) _image->data() );
    else
        return( NULL );
}

void ChannelDataOSGImage::getDimensions( unsigned int& x, unsigned int& y, unsigned int& z )
{
    if( _image != NULL )
    {
        x = _image->s();
        y = _image->t();
        z = _image->r();
    }
    else
        z = y = z = 0;
}

void ChannelDataOSGImage::setImage( osg::Image* image )
{
    _image = image;
}
osg::Image* ChannelDataOSGImage::getImage()
{
    return( _image.get() );
}
const osg::Image* ChannelDataOSGImage::getImage() const
{
    ChannelDataOSGImage* nonConstThis( const_cast<
        ChannelDataOSGImage*>( this ) );
    return( nonConstThis->getImage() );
}

ChannelDataPtr ChannelDataOSGImage::getMaskedChannel( const ChannelDataPtr maskIn )
{
    OSG_WARN << "ChannelDataOSGImage::getMaskedChannel(): Host mask not yet implemented." << std::endl;
    return( shared_from_this() );
}

void ChannelDataOSGImage::reset()
{
    if( _workingImage == NULL )
    {
        _workingImage = new osg::Image( *_image, osg::CopyOp::DEEP_COPY_ALL );
    }
    else
    {
        memcpy( _workingImage->data(), _image->data(),
            _image->getTotalSizeInBytes() );
    }
}


// lfx
}
