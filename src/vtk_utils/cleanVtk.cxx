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
#include <ves/xplorer/util/cleanVtk.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkGenericCell.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

using namespace ves::xplorer::util;

void ves::xplorer::util::dumpVerticesNotUsedByCells( vtkPointSet * grid )
{
    if( grid == NULL )
    {
        std::cerr << "ERROR: In dumpVerticesNotUsedByCells, grid = NULL" << std::endl;
        return;
    }

    // 0=no debug output, 1=some debug output, 2=more debug output
    int debug = 0;

    int j;
    int numVertices = grid->GetNumberOfPoints();
    if( debug ) std::cout << "numVertices = " << numVertices << std::endl;

    // create index of needed points and initialize to "not needed"
    int * isNeededPoint = new int [ numVertices ];
    for( j = 0; j < numVertices; j++ )
        isNeededPoint[j] = 0;

    if( grid->IsA( "vtkUnstructuredGrid" ) )
    {
        if( debug ) std::cout << "working on vtkUnstructuredGrid" << std::endl;

        // if points are referenced by cells, then they are "needed"
        vtkIdType *pts, npts;
        for ((( vtkUnstructuredGrid * )grid )->GetCells()->InitTraversal();
                (( vtkUnstructuredGrid * )grid )->GetCells()->GetNextCell( npts, pts ); )
        {
            for( j = 0; j < npts; j++ )
                isNeededPoint[( int )pts[j] ] = 1;
        }
    }
    else if( grid->IsA( "vtkPolyData" ) )
    {
        if( debug ) std::cout << "working on a vtkPolyData" << std::endl;

        // if points are referenced by polygons, then they are "needed"
        vtkIdType *pts, npts;
        for ((( vtkPolyData* )grid )->GetPolys()->InitTraversal();
                (( vtkPolyData* )grid )->GetPolys()->GetNextCell( npts, pts ); )
        {
            for( j = 0; j < npts; j++ )
                isNeededPoint[( int )pts[j] ] = 1;
        }
    }
    else
    {
        std::cout << "NOTE: currently can only use 'dumpVerticesNotUsedByCells' "
        << "on vtkUnstructuredGrid or vtkPolyData" << std::endl;
        delete [] isNeededPoint;
        return;
    }

    // count the number of "needed points"
    int numNeededVertices = 0;
    for( j = 0; j < numVertices; j++ )
    {
        if( debug > 1 && isNeededPoint[j] )
            std::cout << "\tisNeededPoint[" << j << "] = " << isNeededPoint[j] << std::endl;
        numNeededVertices += isNeededPoint[j];
    }
    if( debug ) std::cout << "numNeededVertices = " << numNeededVertices << std::endl;

    int * old2NewVertexMapper = new int [ numVertices ];
    for( j = 0; j < numVertices; j++ ) old2NewVertexMapper[j] = -1;

    if( debug > 1 ) std::cout << "Vertex relationship..." << std::endl;
    vtkPoints *smallVertices = vtkPoints::New();
    double vertex [3];
    int k = 0;
    for( j = 0; j < numVertices; j++ )
    {
        if( isNeededPoint[j] )
        {
            // get coordinates of the vertex and insert vertex in new mesh

            grid->GetPoints()->GetPoint( j, vertex );
            smallVertices->InsertPoint( k, vertex );
            old2NewVertexMapper[j] = k;
            if( debug > 1 ) std::cout << "\told\t" << j << "\trevised\t" << k << std::endl;
            k++;
        }
    }

    // replace the vertex array if vertex array contains some extraneous data
    if( numNeededVertices < numVertices )
    {
        vtkPointSet * smallGrid = NULL;

        if( grid->IsA( "vtkUnstructuredGrid" ) )
        {
            int numCells = (( vtkUnstructuredGrid * )grid )->GetNumberOfCells();
            if( debug ) std::cout << "The number of cells is " << numCells << std::endl;

            smallGrid = vtkUnstructuredGrid::New();
            (( vtkUnstructuredGrid * )smallGrid )->Allocate( numCells, numCells );

            smallGrid->SetPoints( smallVertices );
            smallVertices->Delete();

            // Loop over the original cells and store revised definitions in smallGrid...
            int npts, ptId;
            vtkIdList *ptIdList = vtkIdList::New();
            vtkGenericCell *cell = vtkGenericCell::New();

            for( int cellId = 0; cellId < numCells; cellId++ )
            {
                (( vtkUnstructuredGrid * )grid )->GetCell( cellId, cell );
                if( debug > 1 )
                    std::cout << "\tcellType = " << cell->GetCellType() << std::endl;

                npts = cell->GetNumberOfPoints();
                ptIdList->Reset();
                for( int i = 0; i < npts; i++ )
                {
                    ptId = cell->GetPointId( i );
                    ptIdList->InsertId( i, old2NewVertexMapper[ptId] );
                }
                (( vtkUnstructuredGrid * )smallGrid )->InsertNextCell(
                    cell->GetCellType(), ptIdList );
            }//for all cells
            ptIdList->Delete();
            cell->Delete();
        }
        else if( grid->IsA( "vtkPolyData" ) )
        {
            int numCells = (( vtkPolyData* )grid )->GetNumberOfCells();
            if( debug ) std::cout << "The number of cells is " << numCells << std::endl;

            smallGrid = vtkPolyData::New();
            (( vtkPolyData* )smallGrid )->Allocate( numCells, numCells );

            smallGrid->SetPoints( smallVertices );
            smallVertices->Delete();

            // Loop over the original cells and store revised definitions in smallGrid...
            int npts, ptId;
            vtkIdList *ptIdList = vtkIdList::New();
            vtkGenericCell *cell = vtkGenericCell::New();

            for( int cellId = 0; cellId < numCells; cellId++ )
            {
                (( vtkPolyData* )grid )->GetCell( cellId, cell );
                if( debug > 1 )
                    std::cout << "\tcellType = " << cell->GetCellType() << std::endl;

                npts = cell->GetNumberOfPoints();
                ptIdList->Reset();
                for( int i = 0; i < npts; i++ )
                {
                    ptId = cell->GetPointId( i );
                    ptIdList->InsertId( i, old2NewVertexMapper[ptId] );
                }
                (( vtkPolyData* )smallGrid )->InsertNextCell(
                    cell->GetCellType(), ptIdList );
            }//for all cells
            ptIdList->Delete();
            cell->Delete();
        }

        //std::cout << "grid->GetPointData()->GetVectors() = " << grid->GetPointData()->GetVectors() << std::endl;
        // if the data set has VECTORS data, then move over appropriate data...
        if( grid->GetPointData()->GetVectors() )
        {
            vtkFloatArray *smallVectorSet = vtkFloatArray::New();
            smallVectorSet->SetNumberOfComponents( 3 );
            smallVectorSet->SetName( grid->GetPointData()->GetVectors()->GetName() );

            double vectorTemp[3];
            for( j = 0; j < numVertices; j++ )
            {
                if( isNeededPoint[j] )
                {
                    grid->GetPointData()->GetVectors()->GetTuple( j, vectorTemp );
                    smallVectorSet->InsertTuple( old2NewVertexMapper[j], vectorTemp );
                }
            }
            smallGrid->GetPointData()->SetVectors( smallVectorSet );
            smallVectorSet->Delete();
        }

        /*
              std::cout << "grid->GetPointData()->GetScalars() = "
                   << grid->GetPointData()->GetScalars() << std::endl;
        */
        // if the data set has SCALARS data, then move over appropriate data...
        if( grid->GetPointData()->GetScalars() )
        {
            vtkFloatArray *smallScalarSet = vtkFloatArray::New();
            smallScalarSet->SetNumberOfComponents( 1 );
            smallScalarSet->SetName( grid->GetPointData()->GetScalars()->GetName() );

            double scalarTemp[1];
            for( j = 0; j < numVertices; j++ )
            {
                if( isNeededPoint[j] )
                {
                    grid->GetPointData()->GetScalars()->GetTuple( j, scalarTemp );
                    smallScalarSet->InsertTuple( old2NewVertexMapper[j], scalarTemp );
                }
            }
            smallGrid->GetPointData()->SetScalars( smallScalarSet );
            smallScalarSet->Delete();
        }

        /*
              std::cout << "grid->GetPointData()->GetNormals() = "
                   << grid->GetPointData()->GetNormals() << std::endl;
        */
        // if the data set has Normals data, then move over appropriate data...
        if( grid->GetPointData()->GetNormals() )
        {
            vtkFloatArray *smallVectorSet = vtkFloatArray::New();
            smallVectorSet->SetNumberOfComponents( 3 );
            smallVectorSet->SetName( grid->GetPointData()->GetNormals()->GetName() );

            double vectorTemp[3];
            for( j = 0; j < numVertices; j++ )
            {
                if( isNeededPoint[j] )
                {
                    grid->GetPointData()->GetNormals()->GetTuple( j, vectorTemp );
                    smallVectorSet->InsertTuple( old2NewVertexMapper[j], vectorTemp );
                }
            }
            smallGrid->GetPointData()->SetNormals( smallVectorSet );
            smallVectorSet->Delete();
        }

        // if the data set has FIELD data, then move over all data...
        int numFieldArrays = grid->GetFieldData()->GetNumberOfArrays();
        if( debug ) std::cout << "numFieldArrays = " << numFieldArrays << std::endl;
        if( numFieldArrays )
        {
            //smallGrid->SetFieldData( grid->GetFieldData() ); //does not work: vtkFloatArray memory leak
            for( int i = 0; i < numFieldArrays; i++ )
            {
                smallGrid->GetFieldData()->AddArray(
                    grid->GetFieldData()->GetArray( i ) );
            }
        }

        // if the data set has FIELD data connected to the point data,
        // then move over appropriate data...
        int numPointDataFieldArrays = grid->GetPointData()->GetNumberOfArrays();
        if( debug )
        {
            std::cout << "numPointDataFieldArrays connected to the point data = "
            << numPointDataFieldArrays << std::endl;
        }

        if( numPointDataFieldArrays )
        {
            smallGrid->GetPointData()->AllocateArrays( numPointDataFieldArrays );

            for( int i = 0; i < numPointDataFieldArrays; i++ )
            {
                vtkDataArray * dataArray = grid->GetPointData()->GetArray( i );
                int numComponents = dataArray->GetNumberOfComponents();
                vtkFloatArray * tempArray = vtkFloatArray::New();
                tempArray->SetNumberOfComponents( numComponents );
                tempArray->SetName( dataArray->GetName() );
                double * tuple = new double [ numComponents ];
                for( j = 0; j < dataArray->GetNumberOfTuples(); j++ )
                {
                    if( isNeededPoint[j] )
                    {
                        dataArray->GetTuple( j, tuple );
                        tempArray->InsertTuple( old2NewVertexMapper[j], tuple );
                    }
                }
                smallGrid->GetPointData()->AddArray( tempArray );
                tempArray->Delete();
                delete [] tuple;
                tuple = NULL;
            }
        }

        delete [] old2NewVertexMapper;

        smallGrid->Update();

        if( grid->IsA( "vtkUnstructuredGrid" ) )
        {
            //grid->Delete();
            grid->ShallowCopy(( vtkUnstructuredGrid* )smallGrid );
            //grid->DeepCopy( (vtkUnstructuredGrid*)smallGrid );
        }
        else if( grid->IsA( "vtkPolyData" ) )
        {
            //grid->Delete();
            grid->ShallowCopy(( vtkPolyData* )smallGrid );
            //grid->DeepCopy( (vtkPolyData*)smallGrid );
        }

        grid->Squeeze();
        smallGrid->Delete();
    }
    else
    {
        if( debug ) std::cout << "Will NOT try to collapse the file" << std::endl;
        smallVertices->Delete();
    }

    delete [] isNeededPoint;
}

void ves::xplorer::util::dumpVerticesNotUsedByCells( vtkPointSet * grid, std::string vtkFileName )
{
    if( grid == NULL )
    {
        std::cerr << "ERROR: In dumpVerticesNotUsedByCells, grid = NULL" << std::endl;
        return;
    }

    int debug = 0;   // 0=no debug output, 1=some debug output, 2=more debug output
    std::fstream rpt;
    rpt.open( vtkFileName.c_str(), std::ios::out );

    std::cout << "Writing to " << vtkFileName << "..." << std::endl;

    int j;
    int numVertices = grid->GetNumberOfPoints();
    if( debug ) std::cout << "numVertices = " << numVertices << std::endl;

    // create index of needed points and initialize to "not needed"
    int * isNeededPoint = new int [ numVertices ];
    for( j = 0; j < numVertices; j++ )
        isNeededPoint[j] = 0;

    if( grid->IsA( "vtkUnstructuredGrid" ) )
    {
        if( debug ) std::cout << "working on vtkUnstructuredGrid" << std::endl;

//      vtkUnstructuredGrid * uGrid = vtkUnstructuredGrid::SafeDownCast( grid );
//      if(uGrid == NULL ) std::cout << "SafeDownCast to a vtkUnstructuredGrid failed";

        // if points are referenced by cells, then they are "needed"
        vtkIdType *pts, npts;
//      for(uGrid->GetCells()->InitTraversal(); uGrid->GetCells()->GetNextCell(npts,pts); )
        for ((( vtkUnstructuredGrid * )grid )->GetCells()->InitTraversal();
                (( vtkUnstructuredGrid * )grid )->GetCells()->GetNextCell( npts, pts ); )
            for( j = 0; j < npts; j++ ) isNeededPoint[( int )pts[j] ] = 1;
    }
    /*
       else if(grid->IsA("vtkPolyData") )
       {
          if(debug ) std::cout << "working on a vtkPolyData" << std::endl;

          // if points are referenced by polygons, then they are "needed"
          vtkIdType *pts, npts;
          for(((vtkPolyData*)grid)->GetPolys()->InitTraversal();
                ((vtkPolyData*)grid)->GetPolys()->GetNextCell(npts,pts); )
             for (j=0; j<npts; j++) isNeededPoint[ (int)pts[j] ] = 1;
       }
    */
    else
    {
        std::cout << "NOTE: currently can only use 'dumpVerticesNotUsedByCells' "
        << "on vtkUnstructuredGrids" << std::endl;
        delete [] isNeededPoint;
        return;
    }

    // count the number of "needed points"
    int numNeededVertices = 0;
    for( j = 0; j < numVertices; j++ )
    {
        if( debug > 1 && isNeededPoint[j] )
            std::cout << "\tisNeededPoint[" << j << "] = " << isNeededPoint[j] << std::endl;
        numNeededVertices += isNeededPoint[j];
    }
    if( debug ) std::cout << "numNeededVertices = " << numNeededVertices << std::endl;

    int * old2NewVertexMapper = new int [ numVertices ];
    for( j = 0; j < numVertices; j++ ) old2NewVertexMapper[j] = -1;

    if( debug > 1 ) std::cout << "Vertex relationship..." << std::endl;
    int k = 0;
    for( j = 0; j < numVertices; j++ )
    {
        if( isNeededPoint[j] )
        {
            old2NewVertexMapper[j] = k;
            if( debug > 1 ) std::cout << "\told\t" << j << "\trevised\t" << k << std::endl;
            k++;
        }
    }

    rpt << "# vtk DataFile Version 3.0" << std::endl;
    rpt << "vtk output" << std::endl;
    rpt << "ASCII" << std::endl;
    rpt << "DATASET UNSTRUCTURED_GRID" << std::endl;

    //char *str;

    // if the data set has FIELD data, then output that...
    int numFieldArrays = grid->GetFieldData()->GetNumberOfArrays();
    if( debug ) std::cout << "numFieldArrays = " << numFieldArrays << std::endl;
    if( numFieldArrays )
    {
        rpt << "FIELD FieldData " << numFieldArrays << std::endl;

        for( int i = 0; i < numFieldArrays; i++ )
        {
            vtkDataArray * dataArray = grid->GetFieldData()->GetArray( i );
            int numTuples = dataArray->GetNumberOfTuples();
            int numComponents = dataArray->GetNumberOfComponents();
            rpt << dataArray->GetName() << " " << numTuples
            << " " << numComponents << " float" << std::endl;
            int counter = 0;
            for( int j = 0; j < numTuples; j++ )
            {
                for( int k = 0; k < numComponents; k++ )
                {
                    double value = dataArray->GetComponent( j, k );
                    std::ostringstream dirStringStream;
                    dirStringStream << value << " ";
                    rpt << dirStringStream.str();
                    counter++;
                    if( counter % 9 == 0 )
                        rpt << "\n";
                }
                rpt << "\n";
            }
        }
    }

    rpt << "POINTS " << numNeededVertices << " float" << std::endl;
    int counter = 0;
    for( j = 0; j < numVertices; j++ )
    {
        if( ! isNeededPoint[j] )
            continue;

        double vertex [3];
        grid->GetPoints()->GetPoint( j, vertex ); // get vertex coordinates

        for( int k = 0; k < 3; k++ )
        {
            std::ostringstream dirStringStream;
            dirStringStream << vertex[k] << " ";
            rpt << dirStringStream.str();
            counter++;
            if( counter % 9 == 0 )
                rpt << "\n";
        }
    }
    rpt << "\n";

    int numCells = (( vtkUnstructuredGrid * )grid )->GetCells()
                   ->GetNumberOfCells();

    int size = (( vtkUnstructuredGrid * )grid )->GetCells()
               ->GetNumberOfConnectivityEntries();

    rpt << "CELLS " << numCells << " " << size << std::endl;

    vtkIdType *pts = 0;
    vtkIdType npts = 0;
    for ((( vtkUnstructuredGrid * )grid )->GetCells()->InitTraversal();
            (( vtkUnstructuredGrid * )grid )->GetCells()->GetNextCell( npts, pts ); )
    {
        rpt << ( int )npts << " ";
        for( j = 0; j < npts; j++ )
            rpt << old2NewVertexMapper[( int )pts[j] ] << " ";
        rpt << "\n";
    }
    rpt << "\n";

    rpt << "CELL_TYPES " << numCells << "\n";
    int * types = new int[numCells];
    for( int cellId = 0; cellId < numCells; cellId++ )
    {
        types[cellId] = (( vtkUnstructuredGrid * )grid )->GetCellType( cellId );
        rpt << types[cellId] << "\n";
    }
    rpt << "\n";

    rpt << "CELL_DATA " << numCells << std::endl;
    rpt << "POINT_DATA " << numNeededVertices << std::endl;

    int numPointDataFieldArrays = grid->GetPointData()->GetNumberOfArrays();

    if( numPointDataFieldArrays )
    {
        for( int i = 0; i < numPointDataFieldArrays; i++ )
        {
            vtkDataArray * dataArray = grid->GetPointData()->GetArray( i );
            int numComponents = dataArray->GetNumberOfComponents();
            if( numComponents > 1 )
            {
                std::string name = dataArray->GetName();

                // only NORMALS won't be written in field data
                if( name.compare( "NORMALS" ) == 0 )// strcmp(name,"NORMALS") )
                    continue;
                /*
                         //in vtk, transforming can cause loss of name...
                         const char * name = dataArray->GetName();
                         if(! strcmp(name,"") )
                            rpt << "VECTORS vectors float" << std::endl;
                         else
                            rpt << "VECTORS " << name << " float" << std::endl;
                */
                rpt << "VECTORS " << name << " float" << std::endl;
                double * tuple = new double [ numComponents ];
                counter = 0;
                for( j = 0; j < dataArray->GetNumberOfTuples(); j++ )
                {
                    if( isNeededPoint[j] )
                    {
                        dataArray->GetTuple( j, tuple );
                        for( int k = 0; k < numComponents; k++ )
                        {
                            rpt << tuple[ k ] << " ";
                            counter++;
                            if( counter % 9 == 0 )
                                rpt << "\n";
                        }
                    }
                }
                delete [] tuple;
                rpt << "\n";
                numPointDataFieldArrays--;
            }
        }

        rpt << "FIELD FieldData " << numPointDataFieldArrays << std::endl;
        for( int i = 0; i < numPointDataFieldArrays; i++ )
        {
            counter = 0;
            vtkDataArray * dataArray = grid->GetPointData()->GetArray( i );
            std::string name = dataArray->GetName();
            int numComponents = dataArray->GetNumberOfComponents();

            // NORMALS won't be written in field data
            if( name.compare( "NORMALS" ) != 0 )// !strcmp(name,"NORMALS") )
                continue;

            //in vtk, transforming can cause loss of name...
            if( name.compare( "" ) != 0 ) //! strcmp(name,"") )
            {
                rpt << "lost_name "  << numComponents
                << " " << numNeededVertices << " float" << std::endl;
            }
            else
            {
                rpt << name << " "  << numComponents << " "
                << numNeededVertices << " float" << std::endl;
            }

            double * tuple = new double [numComponents];
            for( j = 0; j < dataArray->GetNumberOfTuples(); j++ )
            {
                if( isNeededPoint[j] )
                {
                    dataArray->GetTuple( j, tuple );
                    for( int k = 0; k < numComponents; k++ )
                    {
                        rpt << tuple[ k ] << " ";
                        counter++;
                        if( counter % 9 == 0 )
                            rpt << "\n";
                    }
                }
            }
            delete [] tuple;
            rpt << "\n";
        }
    }
    rpt.close();

    delete [] types;
    delete [] isNeededPoint;
    delete [] old2NewVertexMapper;
    std::cout << "done" << std::endl;
    return;
}

