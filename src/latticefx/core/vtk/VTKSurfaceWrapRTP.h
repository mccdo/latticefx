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
#ifndef __LATTICEFX_CORE_VTK_SURFACEWRAP_RTP_OPERATION_H__
#define __LATTICEFX_CORE_VTK_SURFACEWRAP_RTP_OPERATION_H__ 1

#include <latticefx/core/vtk/VTKBaseRTP.h>

#include <latticefx/core/vtk/Export.h>

namespace lfx
{

namespace core
{

namespace vtk
{

/** \class VTKSurfaceWrapRTP VTKSurfaceWrapRTP.h <latticefx/core/vtk/VTKSurfaceWrapRTP.h>
 \brief Class the creates a vector field polydata from a vtk dataset.
 \details This class takes a vtkDataObject in a ChannelDatavtkDataObject with the
 name vtkDataObject and creates a vtkPolyData with the vector field. */

class LATTICEFX_CORE_VTK_EXPORT VTKSurfaceWrapRTP : public VTKBaseRTP
{
public:
    ///Default constructor
    ///\note Since this is a channel operation we pass the enum for a Channel
    ///operation when we construct the VTKSurfaceWrapRTP.
    VTKSurfaceWrapRTP()
        :
        VTKBaseRTP( lfx::core::RTPOperation::Channel )
    {
        ;
    }

    ///Destructor
    virtual ~VTKSurfaceWrapRTP()
    {
        ;
    }

	///Get a string name for this class
	virtual std::string getClassName() const { return std::string( "VTKSurfaceWrapRTP" ); }

    ///We are going to be creating a ChannelDatavtkPolyData so we override the
    ///channel method since we do not have a ChannelData already
    virtual lfx::core::ChannelDataPtr channel( const lfx::core::ChannelDataPtr maskIn );

	virtual void dumpState( std::ostream &os );

protected:

	virtual void serializeData( JsonSerializer *json ) const;
	virtual bool loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr=NULL );

};

typedef boost::shared_ptr< VTKSurfaceWrapRTP > VTKSurfaceWrapRTPPtr;

}
}
}
// __LATTICEFX_CORE_VTK_VECTORFIELD_RTP_OPERATION_H__
#endif