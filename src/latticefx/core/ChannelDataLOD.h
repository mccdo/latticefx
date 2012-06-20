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

#ifndef __LATTICEFX_CHANNEL_LOD_H__
#define __LATTICEFX_CHANNEL_LOD_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/ChannelDataComposite.h>
#include <latticefx/core/PageData.h>

#include <boost/shared_ptr.hpp>

#include <string>


namespace lfx {



/** \class ChannelDataLOD ChannelDataLOD.h <latticefx/core/ChannelDataLOD.h>
\brief Composite pattern container for LOD data.
\details TBD */
class LATTICEFX_EXPORT ChannelDataLOD : public lfx::ChannelDataComposite
{
public:
    ChannelDataLOD( const std::string& name=std::string( "" ) );
    ChannelDataLOD( const ChannelDataLOD& rhs );
    virtual ~ChannelDataLOD();


    virtual ChannelDataLOD* getAsLOD() { return( this ); }


    /** \brief TBD
    \details TBD */
    void setRange( const unsigned int index, const RangeValues& value );
    /** \brief TBD
    \details TBD */
    RangeValues& getRange( const unsigned int index );
    /** \brief TBD
    \details TBD */
    const RangeValues& getRange( const unsigned int index ) const;


    /** \brief Prepare the ChannelData for processing by the DataSet.
    \details Prior to processing ChannelData in the LatticeFX data pipeline,
    the DataSet calls ChannelData::reset() on all attached ChannelData
    instances. This is useful in situations where data processing modifies
    ChannelData (such as a convolution filter or Gaussion blur). Dereived
    ChannelData classes must store a copy of the original data to protect
    against such destruction. reset() allows the ChannelData to refresh the
    working copy of the data from the original. */
    virtual void reset() {}

    /** \brief TBD
    \details TBD */
    static bool allLODData( const ChannelDataList& data );

protected:
    RangeValueList _ranges;
};

typedef boost::shared_ptr< ChannelDataLOD > ChannelDataLODPtr;


// lfx
}


// __LATTICEFX_CHANNEL_LOD_H__
#endif
