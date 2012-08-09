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
#ifndef __LATTICEFX_CORE_VTK_BASE_RTP_OPERATION_H__
#define __LATTICEFX_CORE_VTK_BASE_RTP_OPERATION_H__ 1

#include <latticefx/core/RTPOperation.h>

#include <latticefx/core/vtk/Export.h>

namespace lfx {

namespace core {

namespace vtk {

/** \class VTKBaseRTP VTKBaseRTP.h <latticefx/core/vtk/VTKBaseRTP.h>
 \brief Base class for VTK based RTP operations.
 \details This class holds common functions that most VTK RTP operations utilized
 in creating vtkPolyData. */

class LATTICEFX_CORE_VTK_EXPORT VTKBaseRTP : public lfx::core::RTPOperation
{
public:
    
    ///Default constructor
    VTKBaseRTP()
        : 
        lfx::core::RTPOperation( lfx::core::RTPOperation::Channel ),
        m_requestedValue( 0.2 ),
        m_minScalarValue( 0.0 ),
        m_maxScalarValue( 100.0 )
    {
        ;
    }
    
    ///Destructor
    virtual ~VTKBaseRTP()
    {
        ;
    }
        
    ///Set the value for a plane location or scalar value for an isosurface
    void SetRequestedValue( double value );

    ///Set the min max values for scalar ranges for coloring objects
    void SetMinMaxScalarRangeValue( double minVal, double maxVal );

protected:
    ///Value for setting the position or value for an iso surface
    double m_requestedValue;
    
    ///Values for setting color ranges on full VTK pipelines
    double m_minScalarValue;
    double m_maxScalarValue;
};

typedef boost::shared_ptr< VTKBaseRTP > VTKBaseRTPPtr;

}
}
}
// __LATTICEFX_CORE_VTK_ISOSURFACE_RTP_OPERATION_H__
#endif