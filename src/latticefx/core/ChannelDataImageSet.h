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

#ifndef __LFX_CORE_CHANNEL_DATA_IMAGE_SET_H__
#define __LFX_CORE_CHANNEL_DATA_IMAGE_SET_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/ChannelDataComposite.h>

#include <osg/Vec3>
#include <osg/Array>

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>
#include <set>
#include <map>


namespace lfx
{
namespace core
{


/** \class ChannelDataImageSet ChannelDataImageSet.h <latticefx/core/ChannelDataImageSet.h>
\brief Composite pattern container for sets of image data, such as texture octants.
\details TBD */
class LATTICEFX_EXPORT ChannelDataImageSet : public ChannelDataComposite
{
public:
    ChannelDataImageSet( const std::string& name = std::string( "" ) );
    ChannelDataImageSet( const ChannelDataImageSet& rhs );
    virtual ~ChannelDataImageSet();


    virtual ChannelDataImageSet* getAsSet()
    {
        return( this );
    }


    /** \brief Override base class, to support resizing of the offsets vector.
    \details TBD */
    virtual void removeChannel( const unsigned int index );
    /** \brief Override base class, to support resizing of the offsets vector.
    \details TBD */
    virtual void reserveChannels( const unsigned int count );

    /** \brief TBD
    \details TBD */
    void setOffset( const unsigned int index, const osg::Vec3& offset );
    /** \brief TBD
    \details TBD */
    osg::Vec3& getOffset( const unsigned int index );
    /** \brief TBD
    \details TBD */
    const osg::Vec3& getOffset( const unsigned int index ) const;


    /** \brief TBD
    \details TBD */
    static bool allImageSetData( const ChannelDataList& data );

protected:
    osg::ref_ptr< osg::Vec3Array > _offsets;
};

typedef boost::shared_ptr< ChannelDataImageSet > ChannelDataImageSetPtr;


// core
}
// lfx
}


// __LFX_CORE_CHANNEL_DATA_IMAGE_SET_H__
#endif
