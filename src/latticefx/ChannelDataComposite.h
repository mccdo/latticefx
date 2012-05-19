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

#ifndef __LATTICEFX_CHANNEL_DATA_COMPOSITE_H__
#define __LATTICEFX_CHANNEL_DATA_COMPOSITE_H__ 1


#include <latticefx/Export.h>
#include <latticefx/ChannelData.h>

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>
#include <set>
#include <map>


namespace lfx {



/** \class ChannelDataComposite ChannelDataComposite.h <latticefx/ChannelDataComposite.h>
\brief Composite pattern container for ChannelData objects
\details This class allows ChannelData per level of detail.

In typical usage, a Preprocess operation stores one or more concrete ChannelData objects
in a ChannelDataComposite, one for each level of details. Concrete ChannelData
classes are ChannelData objects that store actual data, such as ChannelDataOSGArray, or
(TBD not yet implemented) ChannelDataDBKey.

RTPOperation has no special handling for ChannelDataComposite and should never
encounter one in practice. The DataSet will invoke an RTPOperation only with concrete
ChannelData.

Renderer operations need to support ChannelDataComposite LOD data during scene graph
creation. */
class LATTICEFX_EXPORT ChannelDataComposite : public lfx::ChannelData
{
public:
    ChannelDataComposite( const std::string& name=std::string( "" ) );
    ChannelDataComposite( const ChannelDataComposite& rhs );
    virtual ~ChannelDataComposite();


    /** \brief Add a data channel to the ChannelDataList for a specific \c level of detail. */
    void addChannel( const ChannelDataPtr channel, const unsigned int level=0 );

    /** \brief Get a channel for a specific \c level of detail.
    \returns ChannelData for the specified \c level. If the exact level doesn't
    have a ChannelData, this function returns NULL. */
    ChannelDataPtr getChannel( const unsigned int level=0 );
    /** \overload */
    const ChannelDataPtr getChannel( const unsigned int level=0 ) const;


    /** \brief
    \details */
    virtual void getDimensions( unsigned int& x, unsigned int& y, unsigned int& z );

    /** \brief Set all elements of the data to the same value.
    \details */
    virtual void setAll( const char value ) {}
    virtual void setAll( const float value ) {}

    /** \brief Boolean AND \c rhs with the existing data.
    \details Assumes both the existing data and \c rhs are osg::ByteArray type, zero
    representing false and non-zero representing true. */
    virtual void andValues( const ChannelData* rhs ) {}

    /** \brief Prepare the ChannelData for processing by the DataSet.
    \details Prior to processing ChannelData in the LatticeFX data pipeline,
    the DataSet calls ChannelData::reset() on all attached ChannelData
    instances. This is useful in situations where data processing modifies
    ChannelData (such as a convolution filter or Gaussion blur). Dereived
    ChannelData classes must store a copy of the original data to protect
    against such destruction. reset() allows the ChannelData to refresh the
    working copy of the data from the original. */
    virtual void reset() {}

    /** \brief Specify the size of the ChannelData.
    \details Used by derived classes to resize internal arrays. */
    virtual void resize( size_t size ) {}

protected:
    // TBD will need to change to store data by LOD level.
    ChannelDataList _data;
};

typedef boost::shared_ptr< ChannelDataComposite > ChannelDataCompositePtr;


// lfx
}


// __LATTICEFX_CHANNEL_DATA_COMPOSITE_H__
#endif
