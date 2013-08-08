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

#include <latticefx/core/RawImg.h>

namespace lfx
{
namespace core
{

RawImg::RawImg() :
	_name( "" ),
	_internalFrmt( 0 ),
	_pixelFrmt( 0 ),
	_dataType( 0 ),
	_width( 0 ),
	_height( 0 ),
	_depth( 0 ),
	_totalSize( 0 ),
	_imgSize( 0 ),
	_rowSize( 0 ),
	_packing( 0 ),
	_numMipLevels( 0 )

{
}

RawImg::RawImg( const osg::Image *img )
{
	set( img );
}

void RawImg::set( const osg::Image *img )
{
	if( img == NULL ) return;

	_name = img->getName();
	_internalFrmt = img->getInternalTextureFormat();
	_pixelFrmt = img->getPixelFormat();
	_dataType = img->getDataType();
	_width = img->s();
	_height = img->t();
	_depth = img->r();
	_totalSize = img->getTotalSizeInBytes();
	_imgSize = img->getImageSizeInBytes();
	_rowSize = img->getRowSizeInBytes();
	_packing = img->getPacking();
	_numMipLevels = img->getNumMipmapLevels();

	_data.reserve( _totalSize );
	for( unsigned int i=0; i<_totalSize; i++ )
	{
		_data.push_back( img->data()[i] );
	}
}

osg::ref_ptr< osg::Image > RawImg::get()
{
	if( _data.size() <= 0 ) return osg::ref_ptr< osg::Image >();

	// create pixel buffer
	unsigned char *data = new unsigned char[_data.size()];
	for( unsigned int i=0; i<_data.size(); i++ )
	{
		data[i] = _data[i];
	}

	osg::ref_ptr< osg::Image > img(new osg::Image() );
	img->setImage( _width, _height, _depth, _internalFrmt, _pixelFrmt, _dataType, data, osg::Image::AllocationMode::USE_NEW_DELETE, _packing );
	return img;
}

// core
}
// lfx
}