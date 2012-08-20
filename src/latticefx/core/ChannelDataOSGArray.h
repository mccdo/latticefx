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

#ifndef __LFX_CORE_CHANNEL_DATA_OSG_ARRAY_H__
#define __LFX_CORE_CHANNEL_DATA_OSG_ARRAY_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/ChannelData.h>

#include <osg/Array>
#include <osg/ref_ptr>
#include <boost/shared_ptr.hpp>

#include <string>
#include <list>


namespace lfx {
namespace core {


/** \class ChannelDataOSGArray ChannelDataOSGArray.h <latticefx/core/ChannelDataOSGArray.h>
\brief Scalar data container class.
\details Create one instance for each scalar array in
the data set, and add it to the DataSet class. All ChannelData
objects within a single DataSet must have the same data size
(number of elements in the array).

Currently, ChannelData is just a wrapper around an osg::Array,
and only osg::ByteArray (for masking) and osg::Vec3Array (for
vertices) are supported. */
class LATTICEFX_EXPORT ChannelDataOSGArray : public ChannelData
{
public:
    ChannelDataOSGArray( const std::string& name=std::string( "" ) );
    ChannelDataOSGArray( osg::Array* data, const std::string& name=std::string( "" ) );
    ChannelDataOSGArray( const ChannelDataOSGArray& rhs );
    virtual ~ChannelDataOSGArray();


    /** \brief
    \details */
    void setOSGArray( osg::Array* array );

    /** \brief
    \details */
    virtual void getDimensions( unsigned int& x, unsigned int& y, unsigned int& z );

    /** \brief
    \details */
    virtual char* asCharPtr();
    /** \overload char* ChannelDataOSGArray::asCharPtr(); */
    virtual const char* asCharPtr() const;

    /** \brief
    \details */
    virtual osg::Array* asOSGArray();
    /** \overload osg::Array* ChannelDataOSGArray::asOSGArray(); */
    virtual const osg::Array* asOSGArray() const;

    /** \brief Return this ChannelData with the specified mask applied.
    \detailt If \maskIn indicates no masking (no zero values), getMaskedChannel()
    returns a pointer to the original ChannelData. Otherwise, getMaskedChannel()
    allocates a new ChannelDataOSGArray containing only the unmasked values.
    \param maskIn Must be a ChannelDataOSGArray. */
    virtual ChannelDataPtr getMaskedChannel( const ChannelDataPtr maskIn );

    /** \brief Set all elements of the data to the same value.
    \details */
    virtual void setAll( const char value );
    virtual void setAll( const float value );

    /** \brief Boolean AND \c rhs with the existing data.
    \details Assumes both the existing data and \c rhs are osg::ByteArray type. */
    virtual void andValues( const ChannelData* rhs );

    /**
    */
    virtual void reset();

    /**
    */
    virtual void resize( size_t size );

    /** \brief Convert the array to Vec3 data.
    \details If the array is already a Vec3Array, return it.
    Otherwise, copy data from the parameter array into a new Vec3Array.
    */
    static osg::Vec3Array* convertToVec3Array( osg::Array* source );

protected:
    /** \brief _data is the original data, as specified by the calling code. */
    osg::ref_ptr< osg::Array > _data;
    /** \brief _workingData is a copy og the original data.
    \details _workingData could be modified by RTPOperation filters such as
    Gaussian blur or convolution filters. The reset() member function refreshes
    _workingData from _data prior to re-executing the data processing pipeline. */
    osg::ref_ptr< osg::Array > _workingData;

    /** \brief Copy all elements in rhs to lhs.
    \details Displays an error and returns if the array types don't match
    or \c lhs::getTotalDatSize() < \c rhs::getTotalDatSize(). */
    void copyArray( osg::Array* lhs, const osg::Array* rhs );
};

typedef boost::shared_ptr< ChannelDataOSGArray > ChannelDataOSGArrayPtr;


// core
}
// lfx
}


// __LFX_CORE_CHANNEL_DATA_OSG_ARRAY_H__
#endif
