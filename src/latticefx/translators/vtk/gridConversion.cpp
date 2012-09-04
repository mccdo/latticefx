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
#include <latticefx/translators/vtk/gridConversion.h>

#include <iostream>

#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkIdList.h>
#include <vtkGenericCell.h>
#include <vtkPointData.h>
using namespace lfx::vtk_translator;

vtkUnstructuredGrid* lfx::vtk_translator::convertToUnstructuredGrid( vtkDataSet* rgrid )
{
    int debug = 0;

    int numCells = rgrid->GetNumberOfCells();
    if( debug )
    {
        std::cout << "\tThe number of cells in the input grid is " << numCells << std::endl;
    }

    int numPts = rgrid->GetNumberOfPoints();
    if( debug )
    {
        std::cout << "\tThe number of points in the input grid is " << numPts << std::endl;
    }

    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
    if( numCells == 0 )
    {
        return ugrid;
    }

    vtkIdType pt, npts;
    vtkIdList* pts = vtkIdList::New();
    vtkGenericCell* cell = vtkGenericCell::New();

    // attach the cell information to the unstructured grid
    for( int cellId = 0; cellId < numCells; cellId++ )
    {
        //std::cout << "cellId = " << cellId << " of " << numCells << std::endl;
        rgrid->GetCell( cellId, cell );
        npts = cell->GetNumberOfPoints();
        pts->Reset();
        for( int i = 0; i < npts; i++ )
        {
            pt = cell->GetPointId( i );
            pts->InsertId( i, pt );
        }
        ugrid->InsertNextCell( cell->GetCellType(), pts );
    }//for all cells

    vtkPoints* vertices = vtkPoints::New();
    double x[3];
    for( int ptId = 0; ptId < numPts; ptId++ )
    {
        rgrid->GetPoint( ptId, x );
        /*
                if (ptId<20 || ptId>numPts-20) std::cout << "ptId = " << ptId << " of " << numPts
                                               << "\tx:\t" << x[0] << "\t" << x[1] << "\t" << x[2] << std::endl;
        */
        vertices->InsertPoint( ptId, x );
    }//for all points

    // attach the point information to the unstructured grid
    ugrid->SetPoints( vertices );

    // if the data set has FIELD data connected to the point data, attach these to unstructured grid...
    int numPDArrays = rgrid->GetPointData()->GetNumberOfArrays();
    if( debug )
    {
        std::cout << "number of point data arrays = " << numPDArrays << std::endl;
    }
    if( numPDArrays )
    {
        ugrid->GetPointData()->AllocateArrays( numPDArrays );

        for( int i = 0; i < numPDArrays; i++ )
        {
            ugrid->GetPointData()->AddArray( rgrid->GetPointData()->GetArray( i ) );
        }
    }

    // cleanup
    pts->Delete();
    cell->Delete();
    vertices->Delete();

    return ugrid;
}

vtkStructuredGrid* lfx::vtk_translator::convertToStructuredGrid( vtkRectilinearGrid* rGrid )
{
    int debug = 0;

    int numCells = rGrid->GetNumberOfCells();
    if( debug )
    {
        std::cout << "\tThe number of cells in the input grid is " << numCells << std::endl;
    }

    int numPts = rGrid->GetNumberOfPoints();
    if( debug )
    {
        std::cout << "\tThe number of points in the input grid is " << numPts << std::endl;
    }

    if( numCells == 0 )
    {
        return NULL;
    }

    vtkStructuredGrid* sGrid = vtkStructuredGrid::New();

    /*
       int pt, npts;
       vtkIdList *pts = vtkIdList::New();
       vtkGenericCell *cell = vtkGenericCell::New();

       // attach the cell information to the structured grid
       for(int cellId = 0; cellId < numCells; cellId++)
       {
          if(debug > 1 )
             std::cout << "cellId = " << cellId << " of " << numCells << std::endl;

          rGrid->GetCell( cellId, cell );
          npts = cell->GetNumberOfPoints();
          pts->Reset();
          for(int i = 0; i < npts; i++)
          {
             pt = cell->GetPointId( i );
             pts->InsertId( i, pt );
          }
          sGrid->InsertNextCell( cell->GetCellType(), pts );
       }//for all cells
       pts->Delete();
       cell->Delete();
    */

    sGrid->SetDimensions( rGrid->GetDimensions() );

    vtkPoints* vertices = vtkPoints::New();
    double x[3];
    for( int ptId = 0; ptId < numPts; ptId++ )
    {
        rGrid->GetPoint( ptId, x );
        if( debug > 1 )
        {
            if( ptId < 20 || ptId > numPts - 20 )
                std::cout << "ptId = " << ptId << " of " << numPts
                          << "\tx:\t" << x[0] << "\t" << x[1] << "\t" << x[2] << std::endl;
        }
        vertices->InsertPoint( ptId, x );
    }//for all points

    // attach the point information to the unstructured grid
    sGrid->SetPoints( vertices );
    vertices->Delete();

    // if the input data set has FIELD data connected to the point data,
    // attach these to the structured grid...
    int numPDArrays = rGrid->GetPointData()->GetNumberOfArrays();
    if( debug )
    {
        std::cout << "number of point data arrays = " << numPDArrays << std::endl;
    }

    if( numPDArrays )
    {
        sGrid->GetPointData()->AllocateArrays( numPDArrays );

        for( int i = 0; i < numPDArrays; i++ )
        {
            sGrid->GetPointData()->AddArray( rGrid->GetPointData()->GetArray( i ) );
        }
    }

    return sGrid;
}

