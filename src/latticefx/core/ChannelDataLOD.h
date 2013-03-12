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

#ifndef __LFX_CORE_CHANNEL_DATA_LOD_H__
#define __LFX_CORE_CHANNEL_DATA_LOD_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/ChannelDataComposite.h>
#include <latticefx/core/PageData.h>

#include <boost/shared_ptr.hpp>

#include <string>


namespace lfx
{
namespace core
{


/** \class ChannelDataLOD ChannelDataLOD.h <latticefx/core/ChannelDataLOD.h>
\brief Composite pattern container for LOD data.
\details TBD */
class LATTICEFX_EXPORT ChannelDataLOD : public ChannelDataComposite
{
public:
    ChannelDataLOD( const std::string& name = std::string( "" ) );
    ChannelDataLOD( const ChannelDataLOD& rhs );
    virtual ~ChannelDataLOD();


    virtual ChannelDataLOD* getAsLOD()
    {
        return( this );
    }


    /** \brief TBD
    \details TBD */
    void setRange( const unsigned int index, const RangeValues& value );
    /** \brief TBD
    \details TBD */
    RangeValues& getRange( const unsigned int index );
    /** \brief TBD
    \details TBD */
    const RangeValues& getRange( const unsigned int index ) const;


    /** \brief Returns true if all ChannelData objects in \c data are ChannelDataLOD. */
    static bool allLODData( const ChannelDataList& data );

protected:
    RangeValueList _ranges;
};

typedef boost::shared_ptr< ChannelDataLOD > ChannelDataLODPtr;


// core
}
// lfx
}


// __LFX_CORE_CHANNEL_DATA_LOD_H__
#endif
