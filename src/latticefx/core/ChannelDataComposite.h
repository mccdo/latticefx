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

#ifndef __LFX_CORE_CHANNEL_DATA_COMPOSITE_H__
#define __LFX_CORE_CHANNEL_DATA_COMPOSITE_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/ChannelData.h>

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>
#include <set>
#include <map>


namespace lfx {
namespace core {


//forwards
class ChannelDataImageSet;
class ChannelDataLOD;


/** \class ChannelDataComposite ChannelDataComposite.h <latticefx/core/ChannelDataComposite.h>
\brief Composite pattern container for ChannelData objects
\details This class allows multiple ChannelData objects and is used for level of detail
and texture bricking / subtextures.

In typical usage, a Preprocess operation stores one or more concrete ChannelData objects
in a ChannelDataComposite, one for each level of detail or texture octant. Concrete ChannelData
classes are ChannelData objects that store actual data, such as ChannelDataOSGArray,
ChannelDataOSGImage, or (TBD not yet implemented) ChannelDataDBKey.

ChannelDataComposite is a base class. Preprocess operations will deal directly with the
derived classes, ChannelDataLOD and ChannelDataImageSet.

RTPOperation has no special handling for ChannelDataComposite and should never
encounter one in practice. The DataSet will invoke an RTPOperation only with concrete
ChannelData. */
class LATTICEFX_EXPORT ChannelDataComposite : public ChannelData
{
public:
    typedef enum {
        UNSPECIFIED,
        COMPOSITE_LOD,
        COMPOSITE_SET
    } CompositeType;

    ChannelDataComposite( const CompositeType compositeType, const std::string& name=std::string( "" ) );
    ChannelDataComposite( const ChannelDataComposite& rhs );
    virtual ~ChannelDataComposite();

    CompositeType getCompositeType() const;
    virtual ChannelDataImageSet* getAsSet() { return( NULL ); }
    virtual ChannelDataLOD* getAsLOD() { return( NULL ); }


    /** \brief Add a data channel to the ChannelDataList.
    \details Returns the index of \channel. */
    unsigned int addChannel( const ChannelDataPtr channel );

    /** \brief Sets the ChannelData at the specified index. */
    void setChannel( const unsigned int index, const ChannelDataPtr channel );

    /** \brief Resizes the ChannelDataList of child ChannelData objects. */
    virtual void reserveChannels( const unsigned int count );

    /** \brief Get the total number of ChannelData inside this composite. */
    unsigned int getNumChannels() const;

    /** \brief Get a channel at a specific \c index.
    \returns ChannelData for the specified \c index. If the index is out
    of renge, this function returns NULL. */
    ChannelDataPtr getChannel( const unsigned int index );
    /** \overload */
    const ChannelDataPtr getChannel( const unsigned int index ) const;


    /** \brief
    \details */
    virtual void getDimensions( unsigned int& x, unsigned int& y, unsigned int& z );

    /** \brief Set all elements of the data to the same value.
    \details */
    virtual void setAll( const char value ) {}
    virtual void setAll( const float value ) {}

    /** \brief Prepare the ChannelData for processing by the DataSet.
    \details Prior to processing ChannelData in the LatticeFX data pipeline,
    the DataSet calls ChannelData::reset() on all attached ChannelData
    instances. This is useful in situations where data processing modifies
    ChannelData (such as a convolution filter or Gaussion blur). Derived
    ChannelData classes must store a copy of the original data to protect
    against such destruction. reset() allows the ChannelData to refresh the
    working copy of the data from the original. */
    virtual void reset();

    /** \brief Specify the size of the ChannelData.
    \details Used by derived classes to resize internal arrays. */
    virtual void resize( size_t size ) {}

protected:
    CompositeType _compositeType;

    ChannelDataList _data;
};

typedef boost::shared_ptr< ChannelDataComposite > ChannelDataCompositePtr;


// core
}
// lfx
}


// __LFX_CORE_CHANNEL_DATA_COMPOSITE_H__
#endif
