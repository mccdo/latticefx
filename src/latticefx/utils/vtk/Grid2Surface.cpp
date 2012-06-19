/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * VE-Suite is (C) Copyright 1998-2012 by Iowa State University
 *
 * Original Development Team:
 *   - ISU's Thermal Systems Virtual Engineering Group,
 *     Headed by Kenneth Mark Bryden, Ph.D., www.vrac.iastate.edu/~kmbryden
 *   - Reaction Engineering International, www.reaction-eng.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
 *************** <auto-copyright.rb END do not edit this line> ***************/
#include <iostream>

#include <latticefx/utils/vtk/Grid2Surface.h>
#include <vtkDataObject.h>
#include <vtkPointData.h>
#include <vtkContourFilter.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkGeometryFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkDecimatePro.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkAlgorithmOutput.h>

using namespace lfx::vtk_utils;

vtkPolyData* lfx::vtk_utils::Grid2Surface( vtkDataObject *dataSet, float deciVal )
{
    vtkPolyDataAlgorithm* geometryFilter = 0; 

    if( dataSet->IsA( "vtkCompositeDataSet" ) )
    {
        // we have to use a compsite pipeline
        geometryFilter = vtkCompositeDataGeometryFilter::New();
    }
    else
    {
        geometryFilter = vtkGeometryFilter::New();
        dynamic_cast<vtkGeometryFilter*>( geometryFilter )->MergingOff();
    }

    //convert vtkDataSet to polydata
    //vtkGeometryFilter *cFilter = vtkGeometryFilter::New();
    //cFilter->SetInput( dataSet );
    geometryFilter->SetInput( dataSet );
    // Turn off merging of coincident points.
    // Note that if merging is on, points with different point attributes
    // (e.g., normals) are merged, which may cause rendering artifacts.
    //cFilter->MergingOff();

    // generate triangles from input polygons
    vtkTriangleFilter *tFilter = vtkTriangleFilter::New();
    //tFilter->SetInput( cFilter->GetOutput() );
    tFilter->SetInput( geometryFilter->GetOutput() );

    // reduce the number of triangles...
    vtkDecimatePro *deci = vtkDecimatePro::New();
    deci->SetInput( tFilter->GetOutput() );
    deci->SetTargetReduction( deciVal );
    deci->PreserveTopologyOn();

    // smooth the vertex coordinates
    vtkSmoothPolyDataFilter *smoother = vtkSmoothPolyDataFilter::New();
    smoother->SetInput( deci->GetOutput() );
    smoother->SetNumberOfIterations( 0 );   //was set to one

    // vtkPolyDataNormals will add normals and vertices (and corresponding
    // scalar/vector data) However it will not add corresponding field data,
    // causing vertices and field length to be out of sequence
    // So delete the field at this point...
    smoother->GetOutput()->Update();
    int numberOfArrays = smoother->GetOutput()->GetFieldData()->GetNumberOfArrays();
    //std::cout << "GetNumberOfArrays = " << numberOfArrays << std::endl;

    // remove the first array numberOfArrays times...
    for( int i = 0; i < numberOfArrays; i++ )
    {
        //std::cout << i << "  " << smoother->GetOutput()->GetFieldData()->GetArrayName(0) << std::endl;
        smoother->GetOutput()->GetFieldData()->RemoveArray(
            smoother->GetOutput()->GetFieldData()->GetArrayName( 0 ) );
    }
    smoother->GetOutput()->Update();

    // add normals and vertices (and corresponding scalar/vector data)
    vtkPolyDataNormals *cNormal = vtkPolyDataNormals::New();
    cNormal->SetInput( smoother->GetOutput() );
    cNormal->ConsistencyOn();  //Enforce consistent polygon ordering

    vtkPolyData * uGrid = vtkPolyData::New();
    cNormal->GetOutput()->Update();
    uGrid->ShallowCopy( cNormal->GetOutput() );
    /*
       int numCells = uGrid->GetNumberOfCells();
       std::cout << "\tThe number of cells in the input grid is " << numCells << std::endl;
       std::cout << "\tactive scalar name is " << uGrid->GetPointData()->GetScalars()->GetName();
    */

    //cFilter->Delete();
    geometryFilter->Delete();
    tFilter->Delete();
    deci->Delete();
    smoother->Delete();
    cNormal->Delete();

    return uGrid;
}
