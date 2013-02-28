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
#include <latticefx/core/vtk/VTKIsoSurfaceRTP.h>
#include <latticefx/core/vtk/ChannelDatavtkPolyDataMapper.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>

#include <latticefx/core/LogMacros.h>

#include <vtkDataObject.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkCellDataToPointData.h>
#include <vtkContourFilter.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataMapper.h>

namespace lfx {

namespace core {

namespace vtk {

////////////////////////////////////////////////////////////////////////////////
lfx::core::ChannelDataPtr VTKIsoSurfaceRTP::channel( const lfx::core::ChannelDataPtr maskIn )
{
    if( m_activeScalar.empty() )
    {
        LFX_ERROR( "VTKIsoSurfaceRTP::channel : The scalar name for the iso-surface is empty." );
    }

    lfx::core::vtk::ChannelDatavtkDataObjectPtr cddoPtr =
        boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkDataObject >( 
        getInput( "vtkDataObject" ) );
    vtkDataObject* tempVtkDO = cddoPtr->GetDataObject();

    vtkCellDataToPointData* c2p = vtkCellDataToPointData::New();
    c2p->SetInput( tempVtkDO );
    //c2p->Update();

    vtkContourFilter* contourFilter = vtkContourFilter::New();
    contourFilter->UseScalarTreeOn();
    contourFilter->SetInputConnection( 0, c2p->GetOutputPort( 0 ) );
    contourFilter->SetValue( 0, m_requestedValue );
    contourFilter->ComputeNormalsOff();
    contourFilter->SetInputArrayToProcess( 0, 0, 0,
        vtkDataObject::FIELD_ASSOCIATION_POINTS,
        m_activeScalar.c_str() );
    //contourFilter->Update();

    vtkPolyDataNormals* normals = vtkPolyDataNormals::New();
    
    if( tempVtkDO->IsA( "vtkCompositeDataSet" ) )
    {
        vtkCompositeDataGeometryFilter* m_multiGroupGeomFilter = 
            vtkCompositeDataGeometryFilter::New();
        m_multiGroupGeomFilter->SetInputConnection( contourFilter->GetOutputPort( 0 ) );
        normals->SetInputConnection( m_multiGroupGeomFilter->GetOutputPort( 0 ) );
        m_multiGroupGeomFilter->Delete();
    }
    else
    {
        vtkDataSetSurfaceFilter* m_surfaceFilter = 
            vtkDataSetSurfaceFilter::New();
        m_surfaceFilter->SetInputConnection( contourFilter->GetOutputPort( 0 ) );

        normals->SetInputConnection( m_surfaceFilter->GetOutputPort() );

        m_surfaceFilter->Delete();
    }
    
    normals->Update();

    lfx::core::vtk::ChannelDatavtkPolyDataMapperPtr cdpd(
        new lfx::core::vtk::ChannelDatavtkPolyDataMapper( normals->GetOutputPort(), "vtkPolyDataMapper" ) );
    //cdpd->GetPolyDataMapper()->SetScalarModeToUsePointFieldData();
    //cdpd->GetPolyDataMapper()->UseLookupTableScalarRangeOn();
    //cdpd->GetPolyDataMapper()->SelectColorArray( m_colorByScalar.c_str() );

    normals->Delete();
    c2p->Delete();
    contourFilter->Delete();
    
    return( cdpd );
}
////////////////////////////////////////////////////////////////////////////////
}
}
}
