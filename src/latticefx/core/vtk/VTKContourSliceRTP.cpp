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
#include <latticefx/core/vtk/VTKContourSliceRTP.h>
#include <latticefx/core/vtk/ChannelDatavtkAlgorithmOutput.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>

#include <vtkDataObject.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkCellDataToPointData.h>
#include <vtkTriangleFilter.h>
#include <vtkStripper.h>
#include <vtkPolyDataNormals.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCutter.h>
#include <vtkPlane.h>

namespace lfx {

namespace core {

namespace vtk {

////////////////////////////////////////////////////////////////////////////////
lfx::core::ChannelDataPtr VTKContourSliceRTP::channel( const lfx::core::ChannelDataPtr maskIn )
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
    
    vtkTriangleFilter* tris = vtkTriangleFilter::New();

    if( tempVtkDO->IsA( "vtkCompositeDataSet" ) )
    {
        vtkCompositeDataGeometryFilter* m_multiGroupGeomFilter = 
            vtkCompositeDataGeometryFilter::New();
        m_multiGroupGeomFilter->SetInputConnection( c2p->GetOutputPort() );
        //return m_multiGroupGeomFilter->GetOutputPort(0);
        tris->SetInputConnection( m_multiGroupGeomFilter->GetOutputPort(0) );
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
        tris->SetInputConnection( m_surfaceFilter->GetOutputPort() );
        m_surfaceFilter->Delete();
    }
    
    //mC2p->SetInputConnection( polydata );
    //mC2p->Update();
    
    //tris->SetInputConnection( polydata );//mC2p->GetOutputPort() );
    //tris->Update();
    //tris->GetOutput()->ReleaseDataFlagOn();
    
    // decimate points is used for lod control of contours
    /*deci->SetInputConnection( tris->GetOutputPort() );
     deci->PreserveTopologyOn();
     deci->BoundaryVertexDeletionOff();
     deci->Update();*/
    //deci->GetOutput()->ReleaseDataFlagOn();
    vtkStripper* strip = vtkStripper::New();
    strip->SetInputConnection( tris->GetOutputPort() );
    //strip->Update();
    //strip->GetOutput()->ReleaseDataFlagOn();
    
    vtkPolyDataNormals* normals = vtkPolyDataNormals::New();
    //if( fillType == 0 )
    {
        normals->SetInputConnection( strip->GetOutputPort() );
        normals->SetFeatureAngle( 130.0f );
        //normals->GetOutput()->ReleaseDataFlagOn();
        normals->ComputePointNormalsOn();
        //normals->ComputeCellNormalsOn();
        normals->FlipNormalsOn();
        //normals->Update();
    }
    /*else if( fillType == 1 ) // banded contours
    {
        // putting the decimation routines as inputs to the bfilter
        // cause the bfilter to crash while being updated
        bfilter->SetInputConnection( strip->GetOutputPort() );
        double range[2];
        GetActiveDataSet()->GetUserRange( range );
        bfilter->GenerateValues( 10, range[0], range[1] );
        bfilter->SetScalarModeToValue();
        bfilter->GenerateContourEdgesOn();
        bfilter->SetInputArrayToProcess( 0, 0, 0,
                                        vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                        GetActiveDataSet()->GetActiveScalarName().c_str() );
        
        //bfilter->GetOutput()->ReleaseDataFlagOn();
        normals->SetInputConnection( bfilter->GetOutputPort() );
        normals->SetFeatureAngle( 130.0f );
        //normals->GetOutput()->ReleaseDataFlagOn();
        normals->ComputePointNormalsOn();
        //normals->ComputeCellNormalsOn();
        normals->FlipNormalsOn();
    }
    else if( fillType == 2 ) // contourlines
    {
        cfilter->SetInputConnection( mC2p->GetOutputPort() );
        double range[2];
        GetActiveDataSet()->GetUserRange( range );
        cfilter->GenerateValues( 10, range[0], range[1] );
        //cfilter->UseScalarTreeOn();
        cfilter->SetInputArrayToProcess( 0, 0, 0,
                                        vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                        GetActiveDataSet()->GetActiveScalarName().c_str() );
        //cfilter->GetOutput()->ReleaseDataFlagOn();
        normals->SetInputConnection( cfilter->GetOutputPort() );
        normals->SetFeatureAngle( 130.0f );
        //normals->GetOutput()->ReleaseDataFlagOn();
        normals->ComputePointNormalsOn();
        //normals->ComputeCellNormalsOn();
        normals->FlipNormalsOn();
    }*/
    vtkAlgorithmOutput* tempPolydata =  normals->GetOutputPort();
    
    lfx::core::vtk::ChannelDatavtkAlgorithmOutputPtr cdpd( 
        new lfx::core::vtk::ChannelDatavtkAlgorithmOutput( tempPolydata, "vtkAlgorithmOutput" ) );
    
    //normals->Delete();
    c2p->Delete();
    strip->Delete();
    tris->Delete();
    
    return( cdpd );
}
////////////////////////////////////////////////////////////////////////////////
}
}
}
