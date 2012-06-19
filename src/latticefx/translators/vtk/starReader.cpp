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
#include <latticefx/translators/vtk/starReader.h>

#include <iostream>
#include <fstream>
#include <cassert>

#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkCellType.h>
#include <vtkIdList.h>

#include <latticefx/translators/vtk/converter.h>     // for "letUsersAddParamsToField>
#include <latticefx/utils/vtk/fileIO.h>       // for "getTagAndValue>
#include <latticefx/utils/vtk/readWriteVtkThings.h>

using namespace lfx::vtk_translator;
using namespace lfx::vtk_utils;

starReader::starReader( std::string paramFile )
{
    this->paramFileName.assign( paramFile );//strcpy( this->paramFileName, paramFile );

    this->debug = 0;

    for( int i = 0; i < 3; i++ )
    {
        this->rotations[ i ] = 0.0f;
        this->translations[ i ] = 0.0f;
    }
    this->scaleIndex = 0;      // scale index default is no scaling
    this->scaleFactor = 1.0f;  // custom scale default is no scaling
    this->numScalars = 0;
    this->numVectors = 0;
    this->writeOption = 0;  // default is to let vtk write the file
    this->vtkFileName.assign( "starFlowdata.vtk" );
}

starReader::~starReader( void )
{
    if( this->numScalars > 0 )
    {
        //for ( int i = 0; i < this->numScalars; i++ )
        //   delete [] this->scalarName[ i ];

        this->scalarName.clear();
    }

    if( this->numVectors > 0 )
    {
        //for ( int i = 0; i < this->numVectors; i++ )
        //   delete [] this->vectorName[ i ];

        this->vectorName.clear();
    }
}

void starReader::SetDebugLevel( int _debug )
{
    this->debug = _debug;
}

vtkUnstructuredGrid* starReader::GetUnsGrid( void )
{
    vtkUnstructuredGrid* uGrid = NULL;

    std::ifstream fvertices( this->starVertFileName.c_str() );

    if( fvertices == NULL )
    {
        std::cerr << "\nError - Cannot open the designated vertex file: "
                  << this->starVertFileName << std::endl;
        return uGrid;
    }

    int starcdVersion = 0;

    //Read the header information again from the star files to get version
    int tempGet = 0;
    tempGet = fvertices.get();

    if( tempGet == 'P' )
    {
        starcdVersion = 4;
    }
    else
    {
        starcdVersion = 324;
    }

    std::cout << "StarCD Version " << starcdVersion << std::endl;
    std::cout << "\nReading vertex data from " << this->starVertFileName << std::endl;

    //Read the header information again from the star 4 files
    if( starcdVersion == 4 )
    {
        char* tempBuffer = new char[512];
        fvertices.getline( tempBuffer, 512 );
        fvertices.getline( tempBuffer, 512 );
        delete [] tempBuffer;
    }

    int charSize = 512;
    char* tempLine = new char[ charSize ];

    // The first thing we do is count the vertices and compute the minimum
    // and maximum vertex IDs

    // Read the first vertex line and assign the first vertexId to vShift,
    // the offset of the vertex numbering
    int vertexId;
    float vx, vy, vz;

    fvertices >> vertexId >> vx >> vy >> vz;
    fvertices.getline( tempLine, charSize );

    int vShift = vertexId;
    int maxOrigVertexId = vertexId;
    int numStarVertices = 1;

    // Read the remainder of the vertex lines and continue to count the vertices
    // and keep track of the minimum and maximum vertex IDs
    while( !fvertices.eof() )
    {
        fvertices >> vertexId >> vx >> vy >> vz;
        fvertices.getline( tempLine, charSize );

        if( vShift > vertexId )
        {
            vShift = vertexId;
        }
        if( maxOrigVertexId < vertexId )
        {
            maxOrigVertexId = vertexId;
        }
        numStarVertices++;
    }

    std::cout << "\tTotal no. of points in vertex file = "
              << numStarVertices << std::endl;

    if( this->debug )
    {
        std::cout << "\tvShift = " << vShift << std::endl;
    }

    // Because we know the number of vertices, use vtk's SetNumberOfPoints
    // and SetPoint functions for efficiency
    vtkPoints* vertices = vtkPoints::New();
    vertices->SetNumberOfPoints( maxOrigVertexId - vShift + 1 );

    // Reread all of the vertex lines and store the vertex
    // info in the vertices array using shifted vertex numbering
    fvertices.clear(); // needed to make seekg work because it sets fail
    // It seems that when you finished the last read it read the end of
    // file which set the fail bit on the next read.
    // When there are fail flags set the seekg will not work.
    // Once you clear the flags , then your seekg should work.
    fvertices.seekg( 0, std::ios_base::beg );

    if( starcdVersion == 4 )
    {
        char* tempBuffer = new char[512];
        fvertices.getline( tempBuffer, 512 );
        fvertices.getline( tempBuffer, 512 );
        delete [] tempBuffer;
    }

    while( !fvertices.eof() )
    {
        fvertices >> vertexId >> vx >> vy >> vz;
        fvertices.getline( tempLine, charSize );

        vertices->SetPoint( vertexId - vShift, vx, vy, vz );
        if( this->debug > 1 )
            std::cout << "Inserted point # " << vertexId - vShift << ": "
                      << "\t" << vx << "\t" << vy << "\t" << vz << std::endl;
    }
    fvertices.close();

    uGrid = vtkUnstructuredGrid::New();
    uGrid->SetPoints( vertices );
    vertices->Delete();

    // now for the cell data...
    std::cout << "\nReading cell data from " << this->starCellFileName << std::endl;

    std::ifstream fcells( this->starCellFileName.c_str() );
    if( fcells == NULL )
    {
        std::cerr << "\nError - Cannot open the designated cell file: "
                  << this->starCellFileName << std::endl;
        uGrid->Delete();
        uGrid = NULL;
        return uGrid;
    }

    vtkIdType cId, cPt[8], cGroup, cType;
    int sId, sPt[8], sGroup, sType;  // used for samm cells - second record line
    int tPt[8];                      // used for samm cells - third record line
    int resolutionType, REG;         // used for samm cells
    //int PERM;                      // used for samm cells
    vtkIdType temp[8];                     // used for samm cells
    int numVtkCells = 0;
    int numStarCells = 0;
    int i;
    /*
       // First compute the number of cells needed for the data set...
       if(starcdVersion == 324 )
       {
          while( !feof(fcells) )
          {
             fscanf(fcells,"%d %d %d %d %d %d %d %d %d %d %d\n",
                    &cId,
                    &cPt[0], &cPt[1], &cPt[2], &cPt[3], &cPt[4], &cPt[5], &cPt[6], &cPt[7],
                    &cGroup, &cType);

             // reject all types except for fluid cells or samm cells
             if(cType != 1 && cType != -1) continue;

             if(cType == -1 )                // star-cd samm cells
             {
                numStarCells++;
                // read second line of this record...
                fscanf(fcells,"%d %d %d %d %d %d %d %d %d %d %d\n",
                       &sId,
                       &sPt[0], &sPt[1], &sPt[2], &sPt[3], &sPt[4], &sPt[5], &sPt[6], &sPt[7],
                       &sGroup, &sType);
                if(sId != cId ) std::cerr << "ERROR: On reading line 2 of samm cell "
                   << cId << ", sId=" << sId << std::endl;
                if(sType != 0 ) std::cerr << "ERROR: For cell " << sId
                   << ", record line 2, sType=" << sType
                   << ".  Expected sType != 0" << std::endl;

                // read third line of this record...
                fscanf(fcells,"%d %d %d %d %d %d %d %d %d %d %d\n",
                       &sId,
                       &tPt[0], &tPt[1], &tPt[2], &tPt[3], &tPt[4], &tPt[5], &tPt[6], &tPt[7],
                       &sGroup, &sType);
                if(sId != cId ) std::cerr << "ERROR: On reading line 3 of samm cell "
                   << cId << ", sId=" << sId << std::endl;
                if(sType != 0 ) std::cerr << "ERROR: For cell " << sId
                   << ", record line 3, sType=" << sType
                   << ".  Expected sType != 0" << std::endl;
                resolutionType = tPt[5];
                REG = tPt[6];

                if(resolutionType == 1 )                   // hex with one corner cut away
                {
                   if      ( REG == 0 ) numVtkCells += 1;    // the corner (a tetrahedron)
                   else if(REG == 1 ) numVtkCells += 3;    // remainder to be a hexahedron and 2 tets
                   else
                      std::cerr << "ERROR: samm cell resolutionType " << resolutionType
                         << ", REG=" << REG << " is not handled" << std::endl;
                }

                else if(resolutionType == 2 )              // hex with two corners (wedge) cut away
                {
                   if      ( REG == 0 ) numVtkCells += 1;    // the wedge
                   else if(REG == 1 ) numVtkCells += 2;    // hex with wedge cut away
                   else
                      std::cerr << "ERROR: samm cell resolutionType " << resolutionType
                         << ", REG=" << REG << " is not handled" << std::endl;
                }

                else if(resolutionType == 3 )              // samm hexahedron
                {
                   if      ( REG == 0 ) numVtkCells += 1;
                   else
                      std::cerr << "ERROR: samm cell resolutionType " << resolutionType
                         << ", REG=" << REG << " is not handled" << std::endl;
                }

                else if(resolutionType == 7 )              // hex with plane cutting three corners
                {
                   if      ( REG == 0 ) numVtkCells += 4;    //
                   else if(REG == 1 ) numVtkCells += 3;    //
                   else
                      std::cerr << "ERROR: samm cell resolutionType " << resolutionType
                         << ", REG=" << REG << " is not handled" << std::endl;
                }

                else if(resolutionType == 8 ) numVtkCells += 5;    // hex with plane cutting four corners

                else if(resolutionType == 85) numVtkCells += 2;    // hex with two opposing edges removed

                else if(resolutionType == 275 && REG == 0 ) numVtkCells += 1;    // samm pyramid

                else
                   std::cerr << "ERROR: samm cell resolutionType=" << resolutionType
                      << " with REG=" << REG << " is not yet handled" << std::endl;
             }
             else
             {
                numStarCells++;
                numVtkCells++;
             }
          }
       }*/
    if( starcdVersion == 4 )
    {
        std::ifstream cellFile( this->starCellFileName.c_str() );
        int charSize = 512;
        char* tempLine = new char[ charSize ];
        cellFile.getline( tempLine, charSize );
        int firstInt = 0;
        int secondInt = 0;
        cellFile >> firstInt >> secondInt;
        cellFile.getline( tempLine, charSize );
        int cellNumber = 0;
        int cellType = 0;
        int numberOfVertsPerCell = 0;
        int tempData;
        int numberOfFaces = 0;
        int currentPos = 0;
        vtkIdList* vertList = vtkIdList::New();

        while( !cellFile.eof() )
        {
            cellFile >> cellNumber >> cellType >> numberOfVertsPerCell >> tempData >> tempData;
            cellFile.getline( tempLine, charSize );
            if( cellType == 255 )
            {
                cellFile >> tempData;
                cellFile >> numberOfFaces;
                currentPos = 1;
                //std::cout << "cellnumber " << cellNumber << " " << numberOfVertsPerCell << " " << numberOfFaces<<  std::endl;

                //Read the start positions for the faces for this particular
                //polyhedral cell
                for( int i = 0; i < numberOfFaces - 1; ++i )
                {
                    cellFile >> tempData;
                    //std::cout << tempData << " ";
                    currentPos += 1;
                    if( currentPos % 8 == 0 )
                    {
                        //reset counter
                        currentPos = 0;
                        //get the end of line and go to the next one
                        cellFile.getline( tempLine, charSize );
                        //ignore the cell number
                        cellFile >> tempData;
                    }
                }
                //now read the vertices for all the faces
                size_t numPolyPoints = numberOfVertsPerCell - numberOfFaces;
                for( size_t i = 0; i < numPolyPoints - 1; ++i )
                {
                    cellFile >> tempData;
                    currentPos += 1;
                    vertList->InsertUniqueId( tempData - vShift );
                    if( currentPos % 8 == 0 && ( ( i + 1 ) < numPolyPoints ) )
                    {
                        //reset counter
                        currentPos = 0;
                        //get the end of line and go to the next one
                        cellFile.getline( tempLine, charSize );
                        //ignore the cell number
                        cellFile >> tempData;
                    }
                }
                cellFile.getline( tempLine, charSize );
                uGrid->InsertNextCell( VTK_CONVEX_POINT_SET, vertList );
                vertList->Reset();
                numStarCells++;
                numVtkCells++;
            }

            // cell type 11 (8 vertex cell hexahedron, aka brick)
            else if( cellType == 11 )
            {
                cellFile >> tempData;  //first entry on the second line

                for( size_t i = 0; i < 8; ++i )
                {
                    cellFile >> tempData;
                    vertList->InsertUniqueId( tempData - vShift );
                }

                cellFile.getline( tempLine, charSize );
                // vertList->InsertUniqueId( tempData - vShift );
                uGrid->InsertNextCell( VTK_CONVEX_POINT_SET, vertList );
                vertList->Reset();
                numStarCells++;
                numVtkCells++;
            }
            // end cell type 11

            // cell type 12 (6 vertex cell prism)
            else if( cellType == 12 )
            {
                cellFile >> tempData;  //first entry on the second line

                for( size_t i = 0; i < 6; ++i )
                {
                    cellFile >> tempData;
                    vertList->InsertUniqueId( tempData - vShift );
                }

                cellFile.getline( tempLine, charSize );
                // vertList->InsertUniqueId( tempData - vShift );
                uGrid->InsertNextCell( VTK_CONVEX_POINT_SET, vertList );
                vertList->Reset();
                numStarCells++;
                numVtkCells++;
            }
            // end cell type 12

            // cell type 3 baffle/shell (not needed for translation)
            // prevents screen output for this cell type
            else if( cellType == 3 )
            {
                cellFile.getline( tempLine, charSize );
                if( numberOfVertsPerCell > 8 )
                {
                    cellFile.getline( tempLine, charSize );
                }

                if( cellFile.peek() == '\n' )
                {
                    cellFile.getline( tempLine, charSize );
                }

            }
            // end cell type 3

            else
            {
                std::cout << "Unsupported cell type " << cellType << " " <<
                          numberOfVertsPerCell << " " << cellNumber << std::endl;

                cellFile.getline( tempLine, charSize );
                if( numberOfVertsPerCell > 8 )
                {
                    cellFile.getline( tempLine, charSize );
                }

                if( cellFile.peek() == '\n' )
                {
                    cellFile.getline( tempLine, charSize );
                }
            }
        }
        vertList->Delete();
        delete [] tempLine;
    }
    // std::cout << "\tTotal no. of cells in star-cd model  = " << numStarCells << std::endl;
    // std::cout << "\tTotal no. of vtk cells to be created = " << numVtkCells << std::endl;

    // Read the cell vertex connectivity and write vtk cells...
    fcells.clear(); // needed to make seekg work because it sets fail
    // It seems that when you finished the last read it read the end of
    // file which set the fail bit on the next read.
    // When there are fail flags set the seekg will not work.
    // Once you clear the flags , then your seekg should work.
    fcells.seekg( 0, std::ios_base::beg );

    if( starcdVersion == 324 )
    {
        while( !fcells.eof() )
        {
            /*
            fscanf(fcells,"%d %d %d %d %d %d %d %d %d %d %d\n",
            &cId,
            &cPt[0], &cPt[1], &cPt[2], &cPt[3], &cPt[4], &cPt[5], &cPt[6], &cPt[7],
            &cGroup, &cType);
            */
            fcells >> cId >> cPt[0] >> cPt[1] >> cPt[2] >> cPt[3] >> cPt[4] >>
                   cPt[5] >> cPt[6] >> cPt[7] >> cGroup >> cType;

            fcells.getline( tempLine , charSize );
            // reject all types except for cells or samm cells
            if( cType != 1 && cType != -1 )
            {
                continue;
            }

            for( i = 0; i < 8; i++ )
            {
                cPt[i] -= vShift;
            }

            if( this->debug > 1 )
                std::cout << "After vShift adjustment, just read cell " << cId
                          << " with vertices: "
                          << "\t" << cPt[0] << "\t" << cPt[1] << "\t" << cPt[2]
                          << "\t" << cPt[3] << "\t" << cPt[4] << "\t" << cPt[5]
                          << "\t" << cPt[6] << "\t" << cPt[7] << std::endl;

            // Skip the cell if all vertices are zero...
            if( cPt[0] == 0 && cPt[1] == 0 && cPt[2] == 0 && cPt[3] == 0 &&
                    cPt[4] == 0 && cPt[5] == 0 && cPt[6] == 0 && cPt[7] == 0 )
            {
                if( this->debug )
                    std::cout << "***Skipping cell " << cId << " with vertices: "
                              << "\t" << cPt[0] << "\t" << cPt[1] << "\t" << cPt[2]
                              << "\t" << cPt[3] << "\t" << cPt[4] << "\t" << cPt[5]
                              << "\t" << cPt[6] << "\t" << cPt[7] << std::endl;
                continue;    // to next cell record
            }

            if( cType == -1 )          // star-cd samm cells
            {
                // read second line of this record...
                /*
                fscanf(fcells,"%d %d %d %d %d %d %d %d %d %d %d\n",
                &sId,
                &sPt[0], &sPt[1], &sPt[2], &sPt[3], &sPt[4], &sPt[5], &sPt[6], &sPt[7],
                &sGroup, &sType);
                */

                fcells >> sId >> sPt[0] >> sPt[1] >> sPt[2] >> sPt[3] >> sPt[4] >>
                       sPt[5] >> sPt[6] >> sPt[7] >> sGroup >> sType;

                fcells.getline( tempLine, charSize );
                for( i = 0; i < 8; i++ )
                {
                    sPt[i] -= vShift;
                }

                if( this->debug )
                    std::cout << "               After vShift adjustment, line 2 vertices: "
                              << "\t" << sPt[0] << "\t" << sPt[1] << "\t" << sPt[2]
                              << "\t" << sPt[3] << "\t" << sPt[4] << "\t" << sPt[5]
                              << "\t" << sPt[6] << "\t" << sPt[7] << std::endl;

                // read third line of this record...
                /*
                fscanf(fcells,"%d %d %d %d %d %d %d %d %d %d %d\n",
                       &sId, &tPt[0], &tPt[1], &tPt[2], &tPt[3],
                       &tPt[4], &tPt[5], &tPt[6], &tPt[7],
                       &sGroup, &sType);
                */

                fcells >> sId >> tPt[0] >> tPt[1] >> tPt[2] >> tPt[3] >> tPt[4] >>
                       tPt[5] >> tPt[6] >> tPt[7] >> sGroup >> sType;

                fcells.getline( tempLine , charSize );

                if( this->debug )
                    std::cout << "                                    On line 3, vertices: "
                              << "\t" << tPt[0] << "\t" << tPt[1] << "\t" << tPt[2]
                              << "\t" << tPt[3] << "\t" << tPt[4] << "\t" << tPt[5]
                              << "\t" << tPt[6] << "\t" << tPt[7] << std::endl;

                resolutionType = tPt[5];
                REG = tPt[6];
                //PERM = tPt[7];                 // not used at this point

                if( resolutionType == 1 ) // hex with one corner cut away
                {
                    if( REG == 0 )         // the corner (a tetrahedron)
                    {
                        temp[0] = cPt[0];
                        temp[1] = sPt[3];
                        temp[2] = sPt[4];
                        temp[3] = sPt[1];
                        uGrid->InsertNextCell( VTK_TETRA, 4, temp );
                        if( this->debug )
                            std::cout << "For samm cell 1, inserted tetrahedron cell w/ vertices:  "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << std::endl;
                    }
                    else if( REG == 1 ) // write the [hex with corner cut away] as a hexahedron and two tetrahedrons
                    {
                        temp[0] = sPt[1];
                        temp[1] = cPt[1];
                        temp[2] = cPt[2];
                        temp[3] = cPt[3];
                        temp[4] = cPt[4];
                        temp[5] = cPt[5];
                        temp[6] = cPt[6];
                        temp[7] = cPt[7];
                        uGrid->InsertNextCell( VTK_HEXAHEDRON, 8, temp );
                        if( this->debug )
                            std::cout << "For samm cell 1, inserted hexahedron cell w/ vertices:  "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << "\t" << temp[5]
                                      << "\t" << temp[6] << "\t" << temp[7] << std::endl;
                        temp[0] = sPt[1];
                        temp[1] = sPt[3];
                        temp[2] = sPt[4];
                        temp[3] = cPt[3];
                        uGrid->InsertNextCell( VTK_TETRA, 4, temp );
                        if( this->debug )
                            std::cout << "For samm cell 1, inserted tetrahedron cell w/ vertices: "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << std::endl;
                        temp[0] = sPt[1];
                        temp[1] = cPt[3];
                        temp[2] = cPt[4];
                        temp[3] = sPt[4];
                        uGrid->InsertNextCell( VTK_TETRA, 4, temp );
                        if( this->debug )
                            std::cout << "For samm cell 1, inserted tetrahedron cell w/ vertices: "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << std::endl;
                    }
                    else
                        std::cerr << "ERROR: For samm cell cId=" << cId << ", with resolutionType="
                                  << resolutionType << ", invalid REG=" << REG << std::endl;
                }

                else if( resolutionType == 2 ) // hex with wedge cut away
                {
                    if( REG == 0 )         // the wedge
                    {
                        temp[0] = cPt[0];
                        temp[1] = sPt[3];
                        temp[2] = sPt[4];
                        temp[3] = cPt[1];
                        temp[4] = sPt[2];
                        temp[5] = sPt[5];
                        uGrid->InsertNextCell( VTK_WEDGE, 6, temp );
                        if( this->debug )
                            std::cout << "For samm cell resolutionType 2, inserted wedge cell with vertices: "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << "\t" << temp[5]
                                      << std::endl;
                    }
                    else if( REG == 1 ) // write the [hex with wedge cut away] as a hexahedron and a wedge
                    {
                        temp[0] = cPt[3];
                        temp[1] = cPt[7];
                        temp[2] = sPt[3];
                        temp[3] = cPt[2];
                        temp[4] = cPt[6];
                        temp[5] = sPt[2];
                        uGrid->InsertNextCell( VTK_WEDGE, 6, temp );
                        if( this->debug )
                            std::cout << "For samm cell resolutionType 2, inserted wedge cell with vertices: "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << "\t" << temp[5]
                                      << std::endl;
                        temp[0] = sPt[3];
                        temp[1] = cPt[7];
                        temp[2] = cPt[4];
                        temp[3] = sPt[4];
                        temp[4] = sPt[2];
                        temp[5] = cPt[6];
                        temp[6] = cPt[5];
                        temp[7] = sPt[5];
                        uGrid->InsertNextCell( VTK_HEXAHEDRON, 8, temp );
                        if( this->debug )
                            std::cout << "For samm cell resolutionType 2, inserted hex cell with vertices: "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << "\t" << temp[5]
                                      << "\t" << temp[6] << "\t" << temp[7] << std::endl;
                    }
                    else std::cerr << "ERROR: For samm cell cId=" << cId << ", with resolutionType=" << resolutionType
                                       << ", invalid REG=" << REG << std::endl;
                }

                else if( resolutionType == 3 ) // samm hexahedron
                {
                    if( REG == 0 )      // convert samm hexahedron into vtk hexahedron
                    {
                        temp[0] = cPt[0];
                        temp[1] = cPt[1];
                        temp[2] = cPt[2];
                        temp[3] = cPt[3];
                        temp[4] = sPt[4];
                        temp[5] = sPt[5];
                        temp[6] = sPt[6];
                        temp[7] = sPt[7];
                        uGrid->InsertNextCell( VTK_HEXAHEDRON, 8, temp );
                        if( this->debug )
                            std::cout << "For samm cell resolutionType 3, inserted hex cell with vertices: "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << "\t" << temp[5]
                                      << "\t" << temp[6] << "\t" << temp[7] << std::endl;
                    }
                    else                    // samm hexahedron will only be "inside"
                    {
                        std::cerr << "ERROR: For samm cell cId=" << cId << ", with resolutionType="
                                  << resolutionType << ", invalid REG=" << REG << std::endl;
                    }
                }

                else if( resolutionType == 7 ) // hex with plane cutting three corners
                {
                    if( REG == 0 )   // divide the piece with the five original corners into two wedges, a pyr, and a tet
                    {
                        temp[0] = cPt[0];
                        temp[1] = cPt[1];
                        temp[2] = cPt[2];
                        temp[3] = sPt[0];
                        temp[4] = sPt[1];
                        temp[5] = sPt[2];
                        uGrid->InsertNextCell( VTK_WEDGE, 6, temp );
                        if( this->debug )
                            std::cout << "For samm cell 7, inserted 1st wedge cell with vertices:  "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << "\t" << temp[5]
                                      << std::endl;
                        temp[0] = cPt[0];
                        temp[1] = cPt[2];
                        temp[2] = cPt[3];
                        temp[3] = sPt[4];
                        temp[4] = sPt[2];
                        temp[5] = sPt[3];
                        uGrid->InsertNextCell( VTK_WEDGE, 6, temp );
                        if( this->debug )
                            std::cout << "For samm cell 7, inserted 2nd wedge cell with vertices:  "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << "\t" << temp[5]
                                      << std::endl;
                        temp[0] = cPt[0];
                        temp[1] = cPt[2];
                        temp[2] = sPt[2];
                        temp[3] = sPt[0];
                        temp[4] = sPt[4];
                        uGrid->InsertNextCell( VTK_PYRAMID, 5, temp );
                        if( this->debug )
                            std::cout << "For samm cell 7, inserted pyramid cell with vertices:    "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << std::endl;
                        temp[0] = cPt[0];
                        temp[1] = sPt[0];
                        temp[2] = sPt[4];
                        temp[3] = cPt[4];
                        uGrid->InsertNextCell( VTK_TETRA, 4, temp );
                        if( this->debug )
                            std::cout << "For samm cell 7, inserted tetrahedron cell w/ vertices:  "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << std::endl;
                    }
                    else if( REG == 1 ) // divide the piece with the three original corners into two pyramids and a tet
                    {
                        temp[0] = sPt[1];
                        temp[1] = sPt[2];
                        temp[2] = cPt[6];
                        temp[3] = cPt[5];
                        temp[4] = sPt[0];
                        uGrid->InsertNextCell( VTK_PYRAMID, 5, temp );
                        if( this->debug )
                            std::cout << "For samm cell 7, inserted 1st pyramid cell w/ vertices:  "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << std::endl;
                        temp[0] = sPt[2];
                        temp[1] = sPt[3];
                        temp[2] = cPt[7];
                        temp[3] = cPt[6];
                        temp[4] = sPt[4];
                        uGrid->InsertNextCell( VTK_PYRAMID, 5, temp );
                        if( this->debug )
                            std::cout << "For samm cell 7, inserted 2nd pyramid cell w/ vertices:  "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << std::endl;
                        temp[0] = sPt[0];
                        temp[1] = cPt[6];
                        temp[2] = sPt[2];
                        temp[3] = sPt[4];
                        uGrid->InsertNextCell( VTK_TETRA, 4, temp );
                        if( this->debug )
                            std::cout << "For samm cell 7, inserted tetrahedron cell w/ vertices:  "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << std::endl;
                    }
                }

                else if( resolutionType == 8 )// hex with plane cutting four corners
                {
                    if( REG == 0 )   // divide the piece containing the original 0th vertex into 2 pyrs, and 3 tets
                    {
                        temp[0] = sPt[0];
                        temp[1] = sPt[1];
                        temp[2] = sPt[4];
                        temp[3] = sPt[5];
                        temp[4] = cPt[0];
                        uGrid->InsertNextCell( VTK_PYRAMID, 5, temp );
                        if( this->debug )
                            std::cout << "For samm cell 8, inserted 1st pyramid cell with vertices:"
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << std::endl;
                        temp[0] = sPt[1];
                        temp[1] = sPt[2];
                        temp[2] = sPt[3];
                        temp[3] = sPt[4];
                        temp[4] = cPt[0];
                        uGrid->InsertNextCell( VTK_PYRAMID, 5, temp );
                        if( this->debug )
                            std::cout << "For samm cell 8, inserted 2nd pyramid cell with vertices:"
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << std::endl;
                        temp[0] = cPt[1];
                        temp[1] = sPt[0];
                        temp[2] = sPt[5];
                        temp[3] = cPt[0];
                        uGrid->InsertNextCell( VTK_TETRA, 4, temp );
                        if( this->debug )
                            std::cout << "For samm cell 8, inserted 1st tetra cell with vertices: "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << std::endl;
                        temp[0] = cPt[3];
                        temp[1] = sPt[2];
                        temp[2] = sPt[1];
                        temp[3] = cPt[0];
                        uGrid->InsertNextCell( VTK_TETRA, 4, temp );
                        if( this->debug )
                            std::cout << "For samm cell 8, inserted 2nd tetra cell with vertices: "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << std::endl;
                        temp[0] = cPt[4];
                        temp[1] = sPt[4];
                        temp[2] = sPt[3];
                        temp[3] = cPt[0];
                        uGrid->InsertNextCell( VTK_TETRA, 4, temp );
                        if( this->debug )
                            std::cout << "For samm cell 8, inserted 3rd tetra cell with vertices: "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << std::endl;
                    }
                    else                     // Wayne Oaks of adapco says resolution type 8 cells will only be "inside"
                    {
                        std::cerr << "ERROR: For samm cell cId=" << cId << ", with resolutionType="
                                  << resolutionType << ", invalid REG=" << REG << std::endl;
                    }
                }

                else if( resolutionType == 85 ) // hex with two opposing edges removed can be a six-sided cylinder or 2 wedges
                {
                    if( REG == 0 )       // specify two wedges
                    {
                        temp[0] = cPt[0];
                        temp[1] = sPt[3];
                        temp[2] = sPt[0];
                        temp[3] = cPt[1];
                        temp[4] = sPt[2];
                        temp[5] = sPt[1];
                        uGrid->InsertNextCell( VTK_WEDGE, 6, temp );
                        if( this->debug )
                            std::cout << "For samm cell resolutionType 85, inserted first wedge cell with vertices: "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << "\t" << temp[5]
                                      << std::endl;
                        temp[0] = cPt[7];
                        temp[1] = sPt[4];
                        temp[2] = sPt[7];
                        temp[3] = cPt[6];
                        temp[4] = sPt[5];
                        temp[5] = sPt[6];
                        uGrid->InsertNextCell( VTK_WEDGE, 6, temp );
                        if( this->debug )
                            std::cout << "For samm cell resolutionType 85, inserted second wedge cell with vertices: "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << "\t" << temp[5]
                                      << std::endl;
                    }
                    else if( REG == 1 )  // write the six-sided cylinder as two hexahedrons
                    {
                        temp[0] = sPt[0];
                        temp[1] = sPt[3];
                        temp[2] = cPt[3];
                        temp[3] = sPt[7];
                        temp[4] = sPt[1];
                        temp[5] = sPt[2];
                        temp[6] = cPt[2];
                        temp[7] = sPt[6];
                        uGrid->InsertNextCell( VTK_HEXAHEDRON, 8, temp );
                        if( this->debug )
                            std::cout << "For samm cell resolutionType 85, inserted first hex cell with vertices: "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << "\t" << temp[5]
                                      << "\t" << temp[6] << "\t" << temp[7] << std::endl;
                        temp[0] = sPt[0];
                        temp[1] = sPt[7];
                        temp[2] = sPt[4];
                        temp[3] = cPt[4];
                        temp[4] = sPt[1];
                        temp[5] = sPt[6];
                        temp[6] = sPt[5];
                        temp[7] = cPt[5];
                        uGrid->InsertNextCell( VTK_HEXAHEDRON, 8, temp );
                        if( this->debug )
                            std::cout << "For samm cell resolutionType 85, inserted second hex cell with vertices: "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << "\t" << temp[5]
                                      << "\t" << temp[6] << "\t" << temp[7] << std::endl;
                    }
                    else
                        std::cerr << "ERROR: For samm cell cId=" << cId << ", with resolutionType="
                                  << resolutionType << ", invalid REG=" << REG << std::endl;
                }
                else if( resolutionType == 275 ) // samm pyramid
                {
                    if( REG == 0 )   // convert samm pyramid into vtk pyramid
                    {
                        temp[0] = cPt[0];
                        temp[1] = cPt[1];
                        temp[2] = cPt[2];
                        temp[3] = cPt[3];
                        temp[4] = sPt[0];
                        uGrid->InsertNextCell( VTK_PYRAMID, 5, temp );

                        if( this->debug )
                            std::cout << "For samm cell 275, inserted a pyramid cell with vertices: "
                                      << "\t" << temp[0] << "\t" << temp[1] << "\t" << temp[2]
                                      << "\t" << temp[3] << "\t" << temp[4] << std::endl;
                    }
                    else                    // samm pyramid will only be "inside"
                    {
                        std::cerr << "ERROR: For samm cell cId=" << cId << ", with resolutionType="
                                  << resolutionType << ", invalid REG=" << REG << std::endl;
                    }
                }
            }

            // StarCD tetrahedron cell vertex connectivity:      12334444
            // use a tet if third and fourth are same and the last four are equal and non-zero
            else if( cPt[2] == cPt[3] && ( cPt[4] == cPt[5] && cPt[5] == cPt[6] && cPt[6] == cPt[7] && cPt[4] != 0 ) )
            {
                cPt[3] = cPt[4];
                uGrid->InsertNextCell( VTK_TETRA, 4, cPt );
                if( this->debug > 1 )
                    std::cout << "Inserted tetrahedron cell with vertices: " << "\t\t\t"
                              << "\t" << cPt[0] << "\t" << cPt[1] << "\t" << cPt[2]
                              << "\t" << cPt[3] << std::endl;
            }
            // use a tet if third and fourth are unique and the last four are zero
            else if( cPt[2] != cPt[3] && ( cPt[4] == cPt[5] && cPt[5] == cPt[6] && cPt[6] == cPt[7] && cPt[7] == 0 ) )
            {
                uGrid->InsertNextCell( VTK_TETRA, 4, cPt );
                if( this->debug > 1 )
                    std::cout << "Inserted tetrahed. cell with vertices: " << "\t\t\t"
                              << "\t" << cPt[0] << "\t" << cPt[1] << "\t" << cPt[2]
                              << "\t" << cPt[3] << std::endl;
            }

            // StarCD pyramid cell vertex connectivity:  12345555
            else if( cPt[4] == cPt[5] && cPt[5] == cPt[6] && cPt[6] == cPt[7] && cPt[7] != 0 )
            {
                uGrid->InsertNextCell( VTK_PYRAMID, 5, cPt );
                if( this->debug > 1 )
                    std::cout << "Inserted pyramid cell with vertices: " << "\t\t\t"
                              << "\t" << cPt[0] << "\t" << cPt[1] << "\t" << cPt[2]
                              << "\t" << cPt[3] << "\t" << cPt[4] << std::endl;
            }

            // StarCD prism (wedge) cell vertex connectivity:  12334566
            else if( cPt[2] == cPt[3] && cPt[2] != 0 && cPt[6] == cPt[7]  && cPt[6] != 0 )
            {
                cPt[3] = cPt[4];
                cPt[4] = cPt[5];
                cPt[5] = cPt[6];
                uGrid->InsertNextCell( VTK_WEDGE, 6, cPt );
                if( this->debug > 1 )
                    std::cout << "Inserted wedge cell with vertices: "
                              << "\t" << cPt[0] << "\t" << cPt[1] << "\t" << cPt[2]
                              << "\t" << cPt[3] << "\t" << cPt[4] << "\t" << cPt[5]
                              << std::endl;
            }

            // StarCD hexahedron cell vertex connectivity:    12345678
            else
            {
                uGrid->InsertNextCell( VTK_HEXAHEDRON, 8, cPt );
                if( this->debug > 1 )
                    std::cout << "Inserted hex cell with vertices: " << "\t\t\t"
                              << "\t" << cPt[0] << "\t" << cPt[1] << "\t" << cPt[2]
                              << "\t" << cPt[3] << "\t" << cPt[4] << "\t" << cPt[5]
                              << "\t" << cPt[6] << "\t" << cPt[7] << std::endl;
            }
        }
    }
    fcells.close();

    // now for the solution data
    std::cout << "\nReading solution data from " << this->starUsrFileName << std::endl;

    if( this->debug )
    {
        std::cout << "numScalars = " << this->numScalars << std::endl;
        std::cout << "numVectors = " << this->numVectors << std::endl;
    }

    int numColumns = 3 * this->numVectors + this->numScalars;
    if( this->debug )
    {
        std::cout << "numColumns = " << numColumns << std::endl;
    }

    float* data = new float [ numColumns ];

    std::ifstream fsolns( this->starUsrFileName.c_str(), std::ios::in );
    if( fsolns == NULL )
    {
        std::cerr << "\nError - Cannot open the designated solution file: "
                  << this->starUsrFileName << std::endl;
        uGrid->Delete();
        uGrid = NULL;
        return uGrid;
    }

    // there can be at most one vector in starCD
    vtkFloatArray* vec = NULL;
    if( this->numVectors )
    {
        if( this->debug )
        {
            std::cout << "\tcreating vector for " << this->vectorName[ 0 ] << std::endl;
        }

        vec = vtkFloatArray::New();
        vec->SetNumberOfComponents( 3 );
        vec->SetName( this->vectorName[ 0 ].c_str() );
        vec->SetNumberOfTuples( maxOrigVertexId - vShift + 1 );
    }

    // set up arrays to store scalar data...
    vtkFloatArray** scalarData = NULL;
    if( vec == NULL )
    {
        scalarData = new vtkFloatArray * [ this->numScalars ];
    }
    else
    {
        // This is to calculate vmag from vector data
        scalarData = new vtkFloatArray * [ this->numScalars + 1 ];
    }

    for( i = 0; i < this->numScalars; i++ )
    {
        scalarData[ i ] = vtkFloatArray::New();
        scalarData[ i ]->SetNumberOfComponents( 1 );
        scalarData[ i ]->SetName( this->scalarName[ i ].c_str() );
        scalarData[ i ]->SetNumberOfTuples( maxOrigVertexId - vShift + 1 );
    }

    if( this->numVectors > 0 )
    {
        // This is to calculate vmag from vector data
        // NOTE arrays are zero based therfore numScalars is the last index for vmag
        scalarData[ this->numScalars ] = vtkFloatArray::New();
        scalarData[ this->numScalars ]->SetNumberOfComponents( 1 );
        scalarData[ this->numScalars ]->SetName( "Velocity_Magnitude" );
        scalarData[ this->numScalars ]->SetNumberOfTuples( maxOrigVertexId - vShift + 1 );
    }

    int numSolns = 0;
    // read the first term in the first line of data...
    fsolns >> sId;
    while( ! fsolns.eof() )
    {
        // read all of the data columns in that line...
        for( i = 0; i < numColumns; i++ )
        {
            fsolns >> data[ i ];
        }
        fsolns.getline( textline, 256 );   //skip past remainder of line

        if( this->debug > 1 )
        {
            std::cout << "raw data: " << sId << "\t";
            for( i = 0; i < numColumns; i++ )
            {
                std::cout <<  data[ i ] << "\t";
            }
            std::cout << std::endl;
        }

        // if solution pertains to a vertex higher than the range already defined,
        // then skip...
        if( sId > maxOrigVertexId )
        {
            if( this->debug )
            {
                std::cout << "skipping solution " << sId << std::endl;
            }

            // try to read the first term in the next line of data
            // (failure will get us out of loop)...
            fsolns >> sId;
            continue;
        }

        sId -= vShift;

        if( sId < 0 )
        {
            if( this->debug )
            {
                std::cout << "Invalid sId = " << sId << std::endl;
            }

            // try to read the first term in the next line of data
            // (failure will get us out of loop)...
            fsolns >> sId;
            continue;
        }

        // assumes that the vector is in first three columns
        int scalarStartIndex = 0;
        if( this->numVectors )
        {
            if( this->debug > 1 )
            {
                std::cout << "VECTOR: " << sId;
                for( i = 0; i < 3; i++ )
                {
                    std::cout << "\t" << data[ i ];
                }
                std::cout << std::endl;
            }

            vec->SetTuple( sId, data );
            scalarStartIndex = 3;
        }

        // add in all of the scalar data...
        for( i = 0; i < this->numScalars; i++ )
        {
            scalarData[ i ]->SetTuple1( sId, data[ scalarStartIndex + i ] );
        }

        // if there is a vector, create an extra scalar term consisting of the
        // vector magnitude
        if( vec != NULL )
        {
            // NOTE: scalarData array is zero-based therefore numScalars is
            // the last index for vmag
            float inputVmag = sqrt( ( data[ 0 ] * data[ 0 ] ) +
                                    ( data[ 1 ] * data[ 1 ] ) +
                                    ( data[ 2 ] * data[ 2 ] ) );
            scalarData[ this->numScalars ]->SetTuple1( sId, inputVmag );
        }
        numSolns++;

        // try to read the first term in the next line of data
        // (failure will get us out of loop)...
        fsolns >> sId;
    }

    delete [] data;

    std::cout << "\tTotal no. of solutions = " << numSolns << std::endl;

    assert( ( numSolns <= numStarVertices ) && "Please export data per vertex rather than per cell." );
    std::cout << std::endl;

    // If vector data exists, then vector magnitude scalar will exist as well.
    // Let the user select the scalar and vector quantities to be written
    // to the pointdata array
    vtkPointData* uGridPointData = uGrid->GetPointData();
    if( vec != NULL )
    {
        uGridPointData->SetVectors( vec );
        //uGridPointData->AddArray( vec );//vectors stored as point data arrays don't get rotated
        vec->Delete();

        letUsersAddParamsToField( this->numScalars + 1, scalarData, uGridPointData, 0 );
        for( i = 0; i < this->numScalars + 1; i++ )
        {
            scalarData[ i ]->Delete();
        }
    }
    else
    {
        letUsersAddParamsToField( this->numScalars, scalarData, uGridPointData, 0 );
        for( i = 0; i < this->numScalars; i++ )
        {
            scalarData[ i ]->Delete();
        }
    }

    delete [] scalarData;

    return uGrid;
}

void starReader::ReadParameterFile( void )
{
    this->numScalars = 0;

    std::fstream StarParamFile;
    StarParamFile.open( this->paramFileName.c_str(), std::ios::in );

    std::string tagName;//char tagName[ 50 ];//
    std::string tagValue;//char tagValue[ 50 ];//
    int  scaleIndexSpecified = 0;
    int  scaleFactorSpecified = 0;

    while( 1 )
    {
        StarParamFile.getline( textline, 256 );

        if( StarParamFile.eof() )
        {
            break;
        }

        if( StarParamFile.peek() == '\n' )
        {
            break;
        }

        tagName.clear();
        tagValue.clear();

        fileIO::getTagAndValue( textline, tagName, tagValue );

        if( this->debug )
        {
            std::cout << "textline = " << textline << std::endl;
            std::cout << "tagValue = " << tagValue << std::endl;
        }
        if( tagName.compare( "STARVRT" ) == 0 )
        {
            this->starVertFileName.assign( tagValue );
        }
        else if( tagName.compare( "STARCEL" ) == 0 )
        {
            this->starCellFileName.assign( tagValue );
        }
        else if( tagName.compare( "STARUSR" ) == 0 )
        {
            this->starUsrFileName.assign( tagValue );
        }
        else if( tagName.compare( "VECTORNAME" ) == 0 )
        {
            std::string newSpace;// = new char[ strlen(tagValue)+1 ];
            newSpace.assign( tagValue );//strcpy(newSpace,tagValue);
            this->vectorName.push_back( newSpace );
            if( this->debug )
                std::cout << "vectorName[" << this->numVectors << "] = "
                          << this->vectorName[ this->numVectors ] << std::endl;
            this->numVectors++;
            if( this->numVectors > 1 )
            {
                std::cerr << "\nError - Star-CD format should have no more than 1 vector"
                          << std::endl;
                exit( 1 );
            }
        }
        else if( tagName.compare( "SCALARNAME" ) == 0 )
        {
            std::string newSpace;// = new char[ strlen(tagValue)+1 ];
            newSpace.assign( tagValue );//strcpy(newSpace,tagValue);
            this->scalarName.push_back( newSpace );
            if( this->debug )
                std::cout << "scalarName[" << this->numScalars << "] = "
                          << this->scalarName[ this->numScalars ] << std::endl;
            this->numScalars++;
        }
        else if( tagName.compare( "SCALEINDEX" ) == 0 )
        {
            scaleIndexSpecified = 1;
            // uses the integer indices defined in translateToVtk.cpp
            // 0 = No scaling, 1 = Custom scaling, 2 = meters to feet, etc.
            // The use of SCALEINDEX=0 is not necessary as it is the default
            // The use of SCALEINDEX=1 needs SCALEFACTOR to be set (or
            // scale factor defaults to 1)
            //       this->scaleIndex = atoi( tagValue );
            if( this->debug )
            {
                std::cout << "scale selection = " << this->scaleIndex << std::endl;
            }
        }
        else if( tagName.compare( "SCALEFACTOR" ) == 0 )
        {
            scaleFactorSpecified = 1;
            // the use of this option implies that SCALEINDEX=1 (Custom scaling)
            this->scaleFactor = atof( tagValue.c_str() );
            if( this->debug )
            {
                std::cout << "scale factor = " << this->scaleFactor << std::endl;
            }
        }
        else if( tagName.compare( "OUTPUTFILENAME" ) == 0 )
        {
            this->vtkFileName.assign( tagValue );
        }
        else if( tagName.compare( "ROTATEX" ) == 0 )
        {
            this->rotations[ 0 ] = atof( tagValue.c_str() );
        }
        else if( tagName.compare( "ROTATEY" ) == 0 )
        {
            this->rotations[ 1 ] = atof( tagValue.c_str() );
        }
        else if( tagName.compare( "ROTATEZ" ) == 0 )
        {
            this->rotations[ 2 ] = atof( tagValue.c_str() );
        }
        else if( tagName.compare( "TRANSLATEX" ) == 0 )
        {
            this->translations[ 0 ] = atof( tagValue.c_str() );
        }
        else if( tagName.compare( "TRANSLATEY" ) == 0 )
        {
            this->translations[ 1 ] = atof( tagValue.c_str() );
        }
        else if( tagName.compare( "TRANSLATEZ" ) == 0 )
        {
            this->translations[ 2 ] = atof( tagValue.c_str() );
        }
        else if( tagName.compare( "WRITEOPTION" ) == 0 )
        {
            this->writeOption = atoi( tagValue.c_str() );
        }
        else
        {
            std::cerr << "Parameter " << tagName << " not found!" << std::endl;
        }
    }
    StarParamFile.close();

    // if user failed to set the scaleIndex, but provided a scalefactor
    // reset scaleIndex to custom scale
    if( !scaleIndexSpecified && this->scaleFactor != 1.0 )
    {
        this->scaleIndex = 1;
    }
    else if( this->scaleIndex == 1 && !scaleFactorSpecified )
    {
        std::cout << "\n!!! Custom scale factor requested but not provided!\n"
                  << "Using the default values.\n"
                  << std::endl;

        this->scaleIndex = 0;
        this->scaleFactor = 1.0;
        //exit(1);
    }

    // check for indeterminate case -- not custom, but scale factor specified
    else if( ( this->scaleIndex != 1 || this->scaleIndex != 0 ) && scaleFactorSpecified )
    {
        std::cout << "\n!!! Indeterminate case -- "
                  << "not custom, but scale factor specified!\n"
                  << "Using the default values.\n"
                  << std::endl;
        //exit(1);
        this->scaleIndex = 0;
        this->scaleFactor = 1.0;
    }
    else
    {
        //this is do nothing
    }
}

float* starReader::GetRotations( void )
{
    return this->rotations;
}

float* starReader::GetTranslations( void )
{
    return this->translations;
}

int starReader::GetWriteOption( void )
{
    return this->writeOption;
}

int starReader::GetScaleIndex( void )
{
    return this->scaleIndex;
}

float starReader::GetScaleFactor( void )
{
    return this->scaleFactor;
}

std::string starReader::GetVTKFileName( void )
{
    return this->vtkFileName;
}

