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
#ifndef __LATTICEFX_CORE_VTK_VECTORFIELD_RTP_OPERATION_H__
#define __LATTICEFX_CORE_VTK_VECTORFIELD_RTP_OPERATION_H__ 1

#include <latticefx/RTPOperation.h>

#include <latticefx/core/vtk/Export.h>

namespace lfx {

namespace core {

namespace vtk {

/** \class VTKVectorFieldRTP VTKVectorFieldRTP.h <latticefx/core/vtk/VTKVectorFieldRTP.h>
 \brief Class the creates a vector field polydata from a vtk dataset.
 \details This class takes a vtkDataObject in a ChannelDatavtkDataObject with the
 name vtkDataObject and creates a vtkPolyData with the vector field. */

class LATTICEFX_CORE_VTK_EXPORT VTKVectorFieldRTP : public lfx::RTPOperation
{
public:
    ///Default constructor
    ///\note Since this is a channel operation we pass the enum for a Channel
    ///operation when we construct the VTKVectorFieldRTP.
    VTKVectorFieldRTP()
        : 
        lfx::RTPOperation( lfx::RTPOperation::Channel )
    {
        ;
    }
    
    ///Destructor
    virtual ~VTKVectorFieldRTP()
    {
        ;
    }
    
    ///We are going to be creating a ChannelDatavtkPolyData so we override the 
    ///channel method since we do not have a ChannelData already
    virtual lfx::ChannelDataPtr channel( const lfx::ChannelDataPtr maskIn );
    
protected:
};

typedef boost::shared_ptr< VTKVectorFieldRTP > VTKVectorFieldRTPPtr;

}
}
}
// __LATTICEFX_CORE_VTK_VECTORFIELD_RTP_OPERATION_H__
#endif