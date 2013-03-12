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
#include <latticefx/core/vtk/VTKVectorSliceRTP.h>
#include <latticefx/core/vtk/ChannelDatavtkPolyData.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>

#include <vtkDataObject.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkCellDataToPointData.h>
#include <vtkMaskPoints.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCutter.h>
#include <vtkPlane.h>

namespace lfx
{

namespace core
{

namespace vtk
{

////////////////////////////////////////////////////////////////////////////////
lfx::core::ChannelDataPtr VTKVectorSliceRTP::channel( const lfx::core::ChannelDataPtr maskIn )
{

    lfx::core::vtk::ChannelDatavtkDataObjectPtr cddoPtr =
        boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkDataObject >(
            getInput( "vtkDataObject" ) );
    vtkDataObject* tempVtkDO = cddoPtr->GetDataObject();

    double* bounds = cddoPtr->GetBounds();

    lfx::core::vtk::CuttingPlane* cuttingPlane =
        new lfx::core::vtk::CuttingPlane( bounds, m_planeDirection, 1 );
    // insure that we are using correct bounds for the given data set...
    cuttingPlane->Advance( m_requestedValue );

    vtkCutter* cutter = vtkCutter::New();
    cutter->SetInput( tempVtkDO );
    cutter->SetCutFunction( cuttingPlane->GetPlane() );

    //cutter->Update();
    delete cuttingPlane;
    cuttingPlane = NULL;

    vtkCellDataToPointData* c2p = vtkCellDataToPointData::New();
    c2p->SetInputConnection( cutter->GetOutputPort() );
    //c2p->Update();
    cutter->Delete();

    vtkMaskPoints* ptmask = vtkMaskPoints::New();
    //This is required for use with VTK 5.10
    ptmask->SetMaximumNumberOfPoints( cddoPtr->GetNumberOfPoints() );
#if ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION >= 10 )
    //New feature for selecting points at random in VTK 5.10
    ptmask->SetRandomModeType( 0 );
#else
    ptmask->RandomModeOn();
#endif
    // get every nth point from the dataSet data
    ptmask->SetOnRatio( m_mask );

    if( tempVtkDO->IsA( "vtkCompositeDataSet" ) )
    {
        vtkCompositeDataGeometryFilter* m_multiGroupGeomFilter =
            vtkCompositeDataGeometryFilter::New();
        m_multiGroupGeomFilter->SetInputConnection( c2p->GetOutputPort() );
        //return m_multiGroupGeomFilter->GetOutputPort(0);
        ptmask->SetInputConnection( m_multiGroupGeomFilter->GetOutputPort( 0 ) );
        m_multiGroupGeomFilter->Delete();
    }
    else
    {
        //m_geometryFilter->SetInputConnection( input );
        //return m_geometryFilter->GetOutputPort();
        vtkDataSetSurfaceFilter* m_surfaceFilter =
            vtkDataSetSurfaceFilter::New();
        m_surfaceFilter->SetInputConnection( c2p->GetOutputPort() );
        //return m_surfaceFilter->GetOutputPort();
        ptmask->SetInputConnection( m_surfaceFilter->GetOutputPort() );
        m_surfaceFilter->Delete();
    }

    ptmask->Update();

    lfx::core::vtk::ChannelDatavtkPolyDataPtr cdpd(
        new lfx::core::vtk::ChannelDatavtkPolyData( ptmask->GetOutput(), "vtkPolyData" ) );

    ptmask->Delete();
    c2p->Delete();

    return( cdpd );
}
////////////////////////////////////////////////////////////////////////////////
}
}
}
