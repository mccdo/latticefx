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

#ifndef __LFX_CORE_RAW_IMG_H__
#define __LFX_CORE_RAW_IMG_H__ 1

#include <string>
#include <vector>

#include <boost/serialization/access.hpp>
//#include <boost/serialization/array.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/nvp.hpp>

#include <osg/Image>

namespace lfx
{
namespace core
{

class RawImg
{
public:
	RawImg();
	RawImg( const osg::Image *img );

	void set( const osg::Image *img );
	osg::ref_ptr< osg::Image > get();

	std::string _name;
	GLint _internalFrmt;
	GLenum _pixelFrmt;
	GLenum _dataType;
	int _width;
	int _height;
	int _depth;
	unsigned int _totalSize;
	unsigned int _imgSize;
	unsigned int _rowSize;
	unsigned int _packing;
	unsigned int _numMipLevels;

	std::vector<unsigned char> _data;

private:

	friend class boost::serialization::access;

	template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        // note, version is always the latest when saving
		ar << BOOST_SERIALIZATION_NVP( _name );
		ar << BOOST_SERIALIZATION_NVP( _internalFrmt );
		ar << BOOST_SERIALIZATION_NVP( _pixelFrmt );
		ar << BOOST_SERIALIZATION_NVP( _dataType );
		ar << BOOST_SERIALIZATION_NVP( _width );
		ar << BOOST_SERIALIZATION_NVP( _height );
		ar << BOOST_SERIALIZATION_NVP( _depth );
		ar << BOOST_SERIALIZATION_NVP( _totalSize );
		ar << BOOST_SERIALIZATION_NVP( _imgSize );
		ar << BOOST_SERIALIZATION_NVP( _rowSize );
		ar << BOOST_SERIALIZATION_NVP( _packing );
		ar << BOOST_SERIALIZATION_NVP( _numMipLevels );
		ar << BOOST_SERIALIZATION_NVP( _data );
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        //if(version > 0)
          //  ar & driver_name;

		ar >> BOOST_SERIALIZATION_NVP( _name );
		ar >> BOOST_SERIALIZATION_NVP( _internalFrmt );
		ar >> BOOST_SERIALIZATION_NVP( _pixelFrmt );
		ar >> BOOST_SERIALIZATION_NVP( _dataType );
		ar >> BOOST_SERIALIZATION_NVP( _width );
		ar >> BOOST_SERIALIZATION_NVP( _height );
		ar >> BOOST_SERIALIZATION_NVP( _depth );
		ar >> BOOST_SERIALIZATION_NVP( _totalSize );
		ar >> BOOST_SERIALIZATION_NVP( _imgSize );
		ar >> BOOST_SERIALIZATION_NVP( _rowSize );
		ar >> BOOST_SERIALIZATION_NVP( _packing );
		ar >> BOOST_SERIALIZATION_NVP( _numMipLevels );
		ar >> BOOST_SERIALIZATION_NVP( _data );
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

// core
}
// lfx
}

 BOOST_CLASS_VERSION(lfx::core::RawImg, 0)


// __LFX_CORE_RAW_IMG_H__
#endif
