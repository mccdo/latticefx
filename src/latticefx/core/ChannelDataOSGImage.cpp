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

#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/LogMacros.h>
#include <latticefx/core/DBUtils.h>

#include <boost/foreach.hpp>


namespace lfx {
namespace core {


ChannelDataOSGImage::ChannelDataOSGImage( const std::string& name, osg::Image* image )
  : ChannelData( name )
{
    if( image != NULL )
        setImage( image );
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


void ChannelDataOSGImage::setStorageModeHint( const StorageModeHint& storageMode )
{
    ChannelData::setStorageModeHint( storageMode );

    if( ( getStorageModeHint() == STORE_IN_DB ) &&
        ( _image != NULL ) && !( getDBKey().empty() ) )
        setImage( _image.get() );
}
void ChannelDataOSGImage::setDBKey( const DBKey dbKey )
{
    ChannelData::setDBKey( dbKey );

    if( ( getStorageModeHint() == STORE_IN_DB ) &&
        ( _image != NULL ) && !( getDBKey().empty() ) )
        setImage( _image.get() );
}
void ChannelDataOSGImage::flushToDB()
{
    storeImage( _workingImage.get(), getDBKey() );
    _workingImage = NULL;
}


char* ChannelDataOSGImage::asCharPtr()
{
    if( getStorageModeHint() == STORE_IN_DB )
        _workingImage = loadImage( getDBKey() );

    if( _workingImage != NULL )
        return( const_cast< char* >( (const char*)( _workingImage->data() ) ) );
    else
        return( NULL );
}
const char* ChannelDataOSGImage::asCharPtr() const
{
    ChannelDataOSGImage* nonConstThis( const_cast< ChannelDataOSGImage* >( this ) );
    return( nonConstThis->asCharPtr() );
}

void ChannelDataOSGImage::setImage( osg::Image* image )
{
    if( ( getStorageModeHint() == STORE_IN_DB ) && !( getDBKey().empty() ) )
    {
        storeImage( image, getDBKey() + DBKey( "-base" ) );
        _image = NULL;
        _workingImage = NULL;
    }
    else // STORE_IN_RAM or there is no DB key yet.
    {
        _image = image;
    }

    if( image != NULL )
        setDimensions( image->s(), image->t(), image->r() );
    else
        setDimensions( 0, 0, 0 );
}
osg::Image* ChannelDataOSGImage::getImage()
{
    if( getStorageModeHint() == STORE_IN_DB )
        _workingImage = loadImage( getDBKey() );
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
    if( getStorageModeHint() == STORE_IN_DB )
    {
        osg::Image* image( loadImage( getDBKey() + DBKey( "-base" ) ) );
        storeImage( image, getDBKey() );
    }
    else // STORE_IN_RAM
    {
        if( _workingImage == NULL )
        {
            _workingImage = new osg::Image( *_image, osg::CopyOp::DEEP_COPY_ALL );
        }
        else if( _workingImage->data() != NULL )
        {
            // Only do the copy if _workingImage has data. It will not have any data
            // if the image has not yet been paged in.
            memcpy( _workingImage->data(), _image->data(),
                _image->getTotalSizeInBytes() );
        }
    }
}


// core
}
// lfx
}
