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

#ifndef __LATTICEFX_CHANNEL_DATA_VTKPOLYDATA_H__
#define __LATTICEFX_CHANNEL_DATA_VTKPOLYDATA_H__ 1


#include <latticefx/Export.h>
#include <latticefx/ChannelData.h>

#include <boost/shared_ptr.hpp>

class vtkPolyData;

namespace lfx {


/** \class ChannelDataComposite ChannelDataComposite.h <latticefx/ChannelDataComposite.h>
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
class LATTICEFX_EXPORT ChannelDatavtkPolyData : public lfx::ChannelData
{
public:

    ChannelDatavtkPolyData( vtkPolyData* const pd, const std::string& name=std::string( "" ) );
    ChannelDatavtkPolyData( const ChannelDatavtkPolyData& rhs );
    virtual ~ChannelDatavtkPolyData();

    vtkPolyData* GetPolyData();
    
protected:
    vtkPolyData* m_pd;
};

typedef boost::shared_ptr< ChannelDatavtkPolyData > ChannelDatavtkPolyDataPtr;


// lfx
}


// __LATTICEFX_CHANNEL_DATA_VTKPOLYDATA_H__
#endif
