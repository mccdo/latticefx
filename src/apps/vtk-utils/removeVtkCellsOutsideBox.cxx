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

#include <ves/xplorer/util/fileIO.h>
#include <ves/xplorer/util/readWriteVtkThings.h>
#include <ves/xplorer/util/cleanVtk.h>

#include <vtkGeometryFilter.h>
#include <vtkAppendFilter.h>
#include <vtkFloatArray.h>
#include <vtkDataSet.h>
#include <vtkPointSet.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkSetGet.h>  // defines data object types
#include <vtkIdList.h>
#include <vtkGenericCell.h>
#include <vtkPointData.h>
using namespace ves::xplorer::util;

int debug = 0;

void removeCellsOutsideBox( vtkPointSet*& pointset, double xmin, double xmax,
                            double ymin, double ymax, double zmin, double zmax )
{
    int numVertices = pointset->GetNumberOfPoints();
    if( debug )
    {
        std::cout << "numVertices = " << numVertices << std::endl;
    }

    int j;

    // if points are in volume, then they are "needed"
    double vertex [ 3 ];
    int* isNeededPoint = new int [ numVertices ];
    for( j = 0; j < numVertices; j++ )
    {
        // get coordinates of the vertex
        pointset->GetPoints()->GetPoint( j, vertex );
        if( vertex[0] >= xmin && vertex[0] <= xmax &&
                vertex[1] >= ymin && vertex[1] <= ymax &&
                vertex[2] >= zmin && vertex[2] <= zmax )
        {
            isNeededPoint[ j ] = 1;
        }
        else
        {
            isNeededPoint[ j ] = 0;
        }
    }

    // count the number of "needed points"
    int numNeededVertices = 0;
    for( j = 0; j < numVertices; j++ )
    {
        if( debug > 1 && isNeededPoint[j] )
        {
            std::cout << "\tisNeededPoint[" << j << "] = " << isNeededPoint[j] << std::endl;
        }
        numNeededVertices += isNeededPoint[j];
    }
    if( debug )
    {
        std::cout << "numNeededVertices = " << numNeededVertices << std::endl;
    }
    if( numNeededVertices  == 0 )
    {
        delete [] isNeededPoint;
        return;
    }
    else if( numNeededVertices == numVertices )
    {
        std::cout << "Will NOT try to collapse the file" << std::endl;
        delete [] isNeededPoint;
        return;
    }

    vtkPointSet* smallGrid;
    if( pointset->GetDataObjectType() == VTK_UNSTRUCTURED_GRID )
    {
        smallGrid = vtkUnstructuredGrid::New();
    }
    else if( pointset->GetDataObjectType() == VTK_POLY_DATA )
    {
        smallGrid = vtkPolyData::New();
    }
    else
    {
        std::cout << "removeCellsOutsideBox can not handle this data object type" << std::endl;
        delete [] isNeededPoint;
        exit( 1 );
    }

    smallGrid->SetPoints( pointset->GetPoints() );

    int numCells = pointset->GetNumberOfCells();
    if( debug )
    {
        std::cout << "The original number of cells is " << numCells << std::endl;
    }

    if( pointset->GetDataObjectType() == VTK_UNSTRUCTURED_GRID )
    {
        ( ( vtkUnstructuredGrid* )smallGrid )->Allocate( numCells, numCells );
    }
    else if( pointset->GetDataObjectType() == VTK_POLY_DATA )
    {
        ( ( vtkPolyData* )smallGrid )->Allocate( numCells, numCells );
    }

    // Loop over original cells and store revised definitions in smallGrid...
    int npts, ptId, cellId;
    vtkIdList* ptIdList = vtkIdList::New();
    vtkGenericCell* cell = vtkGenericCell::New();

    for( cellId = 0; cellId < numCells; cellId++ )
    {
        int useThisCell = 0; // initially mark this cell to NOT be retained
        pointset->GetCell( cellId, cell );
        if( debug > 1 )
        {
            std::cout << "\tcellType = " << cell->GetCellType() << std::endl;
        }

        npts = cell->GetNumberOfPoints();
        ptIdList->Reset();
        for( int i = 0; i < npts; i++ )
        {
            ptId = cell->GetPointId( i );
            // mark this cell to be retained if ANY of its points are "needed"
            if( isNeededPoint[ptId] )
            {
                if( debug > 1 )
                {
                    std::cout << "\t\tNEED ptId= " << ptId << std::endl;
                }
                useThisCell = 1;
            }
            ptIdList->InsertId( i, ptId );
        }

        if( debug > 1 )
            std::cout << "\tcellId = " << cellId
                      << ",\tuseThisCell = " << useThisCell << std::endl;

        if( useThisCell )

        {
            if( pointset->GetDataObjectType() == VTK_UNSTRUCTURED_GRID )
            {
                ( ( vtkUnstructuredGrid* )smallGrid )->InsertNextCell( cell->GetCellType(), ptIdList );
            }
            else if( pointset->GetDataObjectType() == VTK_POLY_DATA )
            {
                ( ( vtkPolyData* )smallGrid )->InsertNextCell( cell->GetCellType(), ptIdList );
            }
        }
    }//for all cells
    ptIdList->Delete();
    cell->Delete();

    if( debug ) std::cout << "\tsmallGrid->GetNumberOfCells() = "
                              << smallGrid->GetNumberOfCells() << std::endl;

    if( pointset->GetPointData()->GetScalars() )
        smallGrid->GetPointData()->SetScalars(
            pointset->GetPointData()->GetScalars() );

    if( pointset->GetPointData()->GetVectors() )
        smallGrid->GetPointData()->SetVectors(
            pointset->GetPointData()->GetVectors() );

    if( pointset->GetPointData()->GetNormals() )
        smallGrid->GetPointData()->SetNormals(
            pointset->GetPointData()->GetNormals() );

    int numPdArrays = pointset->GetPointData()->GetNumberOfArrays();
    if( debug )
    {
        std::cout << "numPdArrays = " << numPdArrays << std::endl;
    }

    for( int i = 0; i < numPdArrays; i++ )
    {
        if(
            ( pointset->GetPointData()->GetScalars() &&
              ! strcmp( pointset->GetPointData()->GetScalars()->GetName(),
                        pointset->GetPointData()->GetArray( i )->GetName() ) )
            ||
            ( pointset->GetPointData()->GetVectors() &&
              ! strcmp( pointset->GetPointData()->GetVectors()->GetName(),
                        pointset->GetPointData()->GetArray( i )->GetName() ) )
            ||
            ( pointset->GetPointData()->GetNormals() &&
              ! strcmp( pointset->GetPointData()->GetNormals()->GetName(),
                        pointset->GetPointData()->GetArray( i )->GetName() ) )
        )
        {
            if( debug ) std::cout << "will not add "
                                      << pointset->GetPointData()->GetArray( i )->GetName()
                                      << " to point data field" << std::endl;
        }
        else
        {
            if( debug ) std::cout << "will add "
                                      << pointset->GetPointData()->GetArray( i )->GetName()
                                      << " to point data field" << std::endl;
            smallGrid->GetPointData()->AddArray(
                pointset->GetPointData()->GetArray( i ) );
        }
    }

    if( pointset->GetDataObjectType() == VTK_UNSTRUCTURED_GRID )
    {
        pointset->Delete();
        pointset = vtkUnstructuredGrid::New();
    }
    else if( pointset->GetDataObjectType() == VTK_POLY_DATA )
    {
        pointset->Delete();
        pointset = vtkPolyData::New();
    }
    else
    {
        std::cerr << "removeCellsOutsideBox can not handle this data object type"
                  << std::endl;
        exit( 1 );
    }

    pointset->DeepCopy( smallGrid );
    smallGrid->Delete();
    delete [] isNeededPoint;
}

int main( int argc, char* argv[] )
{
    // Possibly read in an input vtk file name and an output file...
    std::string inFileName;// = NULL;
    std::string outFileName;// = NULL;
    fileIO::processCommandLineArgs( argc, argv, "cut external cells from",
                                    inFileName, outFileName );
    if( ! inFileName.c_str() )
    {
        return 1;
    }

    //This will need to be changed to handle both vtkDataset and vtkMultigroupDataSet
    vtkDataSet* dataset = dynamic_cast<vtkDataSet*>( readVtkThing( inFileName, 1 ) );

    int limitOption;
    double x_min, x_max, y_min, y_max, z_min, z_max;
    // if there are lots of items on command line, assume one was executable,
    // two were filenames, and six were extents, and one was limit option
    int arg = 3;
    if( argc > 3 )
    {
        limitOption = atoi( argv[ arg++ ] );
        x_min = atof( argv[ arg++ ] );
        x_max = atof( argv[ arg++ ] );
        y_min = atof( argv[ arg++ ] );
        y_max = atof( argv[ arg++ ] );
        z_min = atof( argv[ arg++ ] );
        z_max = atof( argv[ arg++ ] );
        std::cout << "Using commandline-set extents..." << std::endl;
        std::cout << "\tlimitOption: " << limitOption << std::endl;
        std::cout << "\tx_min: " << x_min << std::endl;
        std::cout << "\tx_max: " << x_max << std::endl;
        std::cout << "\ty_min: " << y_min << std::endl;
        std::cout << "\ty_max: " << y_max << std::endl;
        std::cout << "\tz_min: " << z_min << std::endl;
        std::cout << "\tz_max: " << z_max << std::endl;
    }
    else
    {
        std::cout << "\nYou have a choice on how you specify the limits of the box."
                  << "\n\t(0) All cells with any vertices in the box will be retained"
                  << "\n\t(1) Only those cells totally inside the box will be retained"
                  << std::endl;
        limitOption = fileIO::getIntegerBetween( 0, 1 );

        std::cout << "\nNow specify the limits of the box -- ";
        if( limitOption == 0 )
        {
            std::cout << "All cells with any vertices in the box will be retained" << std::endl;
        }
        else if( limitOption == 1 )
        {
            std::cout << "Only those cells totally inside the box will be retained" << std::endl;
        }

        std::cout << "\tinput x_min: ";
        std::cin >> x_min;
        std::cout << "\tinput x_max: ";
        std::cin >> x_max;
        std::cout << "\tinput y_min: ";
        std::cin >> y_min;
        std::cout << "\tinput y_max: ";
        std::cin >> y_max;
        std::cout << "\tinput z_min: ";
        std::cin >> z_min;
        std::cout << "\tinput z_max: ";
        std::cin >> z_max;
    }

    if( limitOption == 0 )
    {
        vtkPointSet* pointset = vtkPointSet::SafeDownCast( dataset );
        if( pointset == NULL )
        {
            std::cerr << "SafeDownCast to a pointset failed";
            exit( 1 );
        }

        removeCellsOutsideBox( pointset, x_min, x_max,
                               y_min, y_max, z_min, z_max );

        if( debug )
        {
            std::cout << "now to dumpVerticesNotUsedByCells..." << std::endl;
        }
        dumpVerticesNotUsedByCells( pointset );

        if( debug )
        {
            writeVtkThing( pointset, outFileName, 0 );    // one is for binary
        }
        else
        {
            writeVtkThing( pointset, outFileName, 1 );    // one is for binary
        }
    }
    else if( limitOption == 1 )
    {
        //convert vtkDataSet to polydata
        vtkGeometryFilter* gFilter = vtkGeometryFilter::New();
        gFilter->SetInput( dataset );
        // Turn off merging of coincident points.
        // Note that if merging is on, points with different point attributes
        // (e.g., normals) are merged, which may cause rendering artifacts.
        gFilter->MergingOff();
        gFilter->SetExtent( x_min, x_max, y_min, y_max, z_min, z_max );
        gFilter->ExtentClippingOn();
        //gFilter->PointClippingOn();
        gFilter->Update();

        vtkPointSet* pointset;
        if( dataset->GetDataObjectType() == VTK_UNSTRUCTURED_GRID )
        {
            // convert the polydata to an unstructured dataset...
            vtkAppendFilter* aFilter = vtkAppendFilter::New();
            aFilter->SetInput( gFilter->GetOutput() );
            aFilter->GetOutput()->Update();
            //int numCells = aFilter->GetOutput()->GetNumberOfCells();

            pointset = vtkUnstructuredGrid::New();
            pointset->DeepCopy( aFilter->GetOutput() );
            aFilter->Delete();
        }
        else if( dataset->GetDataObjectType() == VTK_POLY_DATA )
        {
            pointset = vtkPolyData::New();
            pointset->DeepCopy( gFilter->GetOutput() );
        }
        else
        {
            std::cerr << "\nERROR - can only currently delete cells from "
                      << "vtkUnstructuredGrids or vtkPolyData" << std::endl;
            inFileName.erase();//delete [] inFileName;   inFileName = NULL;
            outFileName.erase();//delete [] outFileName;  outFileName = NULL;
            dataset->Delete();
            exit( 1 );
        }

        std::cout << "now to dumpVerticesNotUsedByCells..." << std::endl;
        dumpVerticesNotUsedByCells( pointset );
        if( debug )
        {
            writeVtkThing( pointset, outFileName, 0 );    // one is for binary
        }
        else
        {
            writeVtkThing( pointset, outFileName, 1 );    // one is for binary
        }

        pointset->Delete();
        gFilter->Delete();
    }

    inFileName.erase();//delete [] inFileName;   inFileName = NULL;
    outFileName.erase();//delete [] outFileName;  outFileName = NULL;
    //std::cout << "now to delete dataset..." << std::endl;
    dataset->Delete();
    return 0;
}


