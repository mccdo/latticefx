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
#ifndef __LATTICEFX_CORE_VTK_VECTORSLICE_RTP_OPERATION_H__
#define __LATTICEFX_CORE_VTK_VECTORSLICE_RTP_OPERATION_H__ 1

#include <latticefx/core/RTPOperation.h>

#include <latticefx/core/vtk/Export.h>

#include <latticefx/core/vtk/CuttingPlane.h>

namespace lfx {

namespace core {

namespace vtk {

/** \class VTKVectorSliceRTP VTKVectorSliceRTP.h <latticefx/core/vtk/VTKVectorSliceRTP.h>
 \brief Class the creates a vector field polydata from a vtk dataset.
 \details This class takes a vtkDataObject in a ChannelDatavtkDataObject with the
 name vtkDataObject and creates a vtkPolyData with the vector field. */

class LATTICEFX_CORE_VTK_EXPORT VTKVectorSliceRTP : public lfx::core::RTPOperation
{
public:
    
    ///Default constructor
    ///\note Since this is a channel operation we pass the enum for a Channel
    ///operation when we construct the VTKVectorFieldRTP.
    VTKVectorSliceRTP( CuttingPlane::SliceDirection slice = CuttingPlane::X_PLANE )
        : 
        lfx::core::RTPOperation( lfx::core::RTPOperation::Channel ),
        m_planeDirection( slice ),
        m_requestedValue( 0.2 ),
        m_mask( 1.0 )
    {
        ;
    }
    
    ///Destructor
    virtual ~VTKVectorSliceRTP()
    {
        ;
    }
    
    ///We are going to be creating a ChannelDatavtkPolyData so we override the 
    ///channel method since we do not have a ChannelData already
    virtual lfx::core::ChannelDataPtr channel( const lfx::core::ChannelDataPtr maskIn );
    
    void SetRequestedValue( double value );
    
    void SetMaskValue( double value );

protected:
    CuttingPlane::SliceDirection m_planeDirection;
    double m_requestedValue;
    double m_mask;
};

typedef boost::shared_ptr< VTKVectorSliceRTP > VTKVectorSliceRTPPtr;

}
}
}
// __LATTICEFX_CORE_VTK_VECTORSLICE_RTP_OPERATION_H__
#endif