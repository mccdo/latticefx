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
#include <latticefx/translators/vtk/cfdREITranslator.h>
#include <vtkDataSet.h>
#include <vtkImageData.h>
#include <vtkDICOMImageReader.h>
#include <vtkStructuredGrid.h>
#include <vtkProbeFilter.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>   // s-grids allow blanking - rect grids don't
#include <vtkFloatArray.h>
#include <vtkPointData.h>

#include <latticefx/utils/vtk/fileIO.h>
#include <latticefx/translators/vtk/converter.h>      // for "letUsersAddParamsToField>
#include <latticefx/translators/vtk/gridConversion.h>

#include <iostream>

using namespace lfx::vtk_utils;
using namespace lfx::vtk_translator;
////////////////////////////////////////
//Constructors                        //
////////////////////////////////////////
cfdREITranslator::cfdREITranslator()
{
    SetTranslateCallback( &_reiTranslator );
    SetPreTranslateCallback( &_cmdParser );
}
/////////////////////////////////////////
cfdREITranslator::~cfdREITranslator()
{}
/////////////////////////////////////////
void cfdREITranslator::REITranslatorCbk::Translate( vtkDataObject*& outputDataset,
        cfdTranslatorToVTK* toVTK,
        vtkAlgorithm*& )
{
    cfdREITranslator* reiTranslator =
        dynamic_cast<cfdREITranslator*>( toVTK );

    //   reiTranslator->SetNumberOfFoundFiles(1);
    if( reiTranslator )
    {
        debug = 2;
        vtkStructuredGrid* sGrid = NULL;

        FILE* s1;
        // open db file
        if( ( s1 = fopen( reiTranslator->GetFile( 0 ).c_str(), "rb" ) ) == NULL )
        {
            std::cerr << "ERROR: can't open file \"" << reiTranslator->GetFile( 0 ).c_str() << "\", so exiting" << std::endl;
            exit( 0 );
        }
        //debug = 1;

        //create and null terminate end of 80 character buffer
        char header[81];
        header[80] = '\0';

        std::cout << "\nReading the REI data file..." << std::endl;
        // read first line
        fseek( s1, 4L, SEEK_SET );
        fread( header, sizeof( char ), 80, s1 );
        std::cout << "\theader: \"" << header << "\"" << std::endl;

        // second line is grid type: "cartesian_rectangular", "body_fitted_structured_grid", "cartesian_cylindrical"
        fseek( s1, 8L, SEEK_CUR );
        fread( header, sizeof( char ), 80, s1 );
        std::cout << "\theader: \"" << header << "\"" << std::endl;

        // read third line
        fseek( s1, 8L, SEEK_CUR );
        fread( header, sizeof( char ), 80, s1 );
        std::cout << "\theader: \"" << header << "\"" << std::endl;

        //**************************************MESH GEOMETRY***********************
        // create some loop counters
        //int i,j;
        bool endian_flip = 0;

        int ndim;
        int nx, ny, nz;

        // skip forward to next place
        fseek( s1, 8L, SEEK_CUR );

        // record the position in case we have to reread
        long position = ftell( s1 );

        // try to read data with both endian types...
        for( int i = 0; i < 2; i++ )
        {
            fseek( s1, position, SEEK_SET );
            // read dimensions
            if( fileIO::readNByteBlockFromFile( &ndim, sizeof( int ), 1, s1, endian_flip ) )
            {
                std::cerr << "ERROR: bad read in fileIO::readNByteBlockFromFile, so exiting" << std::endl;
                exit( 0 );
            }
            if( debug )
            {
                std::cout << "ndim = " << ndim << std::endl;
            }

            // read nx, ny, & nz
            fseek( s1, 8L, SEEK_CUR );
            if( fileIO::readNByteBlockFromFile( &nx, sizeof( int ), 1, s1, endian_flip ) ||
                    fileIO::readNByteBlockFromFile( &ny, sizeof( int ), 1, s1, endian_flip ) ||
                    fileIO::readNByteBlockFromFile( &nz, sizeof( int ), 1, s1, endian_flip ) )
            {
                std::cerr << "ERROR: bad read in fileIO::readNByteBlockFromFile, so exiting" << std::endl;
                exit( 0 );
            }

            if( debug )
            {
                std::cout << "nx = " << nx << std::endl;
                std::cout << "ny = " << ny << std::endl;
                std::cout << "nz = " << nz << std::endl;
            }

            if( ( nx < 0 || nx > 500 ) ||
                    ( ny < 0 || ny > 500 ) ||
                    ( nz < 0 || nz > 500 ) )
            {
                if( debug )
                {
                    std::cout << "NOTE flipping endian flag in attempt to read data "
                              << "that makes sense" << std::endl;
                }
                endian_flip = 0;
                continue;
            }
            std::cout << "\tndim = " << ndim << std::endl;
            std::cout << "\tnx = " << nx << std::endl;
            std::cout << "\tny = " << ny << std::endl;
            std::cout << "\tnz = " << nz << std::endl;
            break;
        }
        int num_verts = nx * ny * nz;

        //**************************************SOLUTIONS***************************
        // read numScalars & numVectors
        int numScalars = 0;
        int numVectors = 0;
        fseek( s1, 8L, SEEK_CUR );
        if( fileIO::readNByteBlockFromFile( &numScalars, sizeof( int ), 1, s1, endian_flip ) ||
                fileIO::readNByteBlockFromFile( &numVectors, sizeof( int ), 1, s1, endian_flip ) )
        {
            std::cerr << "ERROR: bad read in fileIO::readNByteBlockFromFile, so exiting" << std::endl;
            exit( 0 );
        }
        if( debug )
        {
            std::cout << "numScalars: \"" << numScalars << "\"" << std::endl;
        }
        if( debug )
        {
            std::cout << "numVectors: \"" << numVectors << "\"" << std::endl;
        }
        int numParameters = numScalars + numVectors;
        if( debug )
        {
            std::cout << "numParameters: \"" << numParameters << "\"" << std::endl;
        }

        // make space for scalar and vector names
        char** parameterNames = new char * [numParameters];
        for( int i = 0; i < numParameters; i++ )
        {
            parameterNames[i] = new char [9];
        }

        // read and NULL terminate scalar names
        fseek( s1, 8L, SEEK_CUR );
        for( int i = 0; i < numScalars; i++ )
        {
            if( parameterNames[i] == NULL )//.empty())//==NULL)
            {
                std::cerr << "ERROR: can't get memory for parameterNames, so exiting" << std::endl;
                exit( 0 );
            }
            parameterNames[i][8] = '\0';
            fread( parameterNames[i], sizeof( char ), 8, s1 );
            fileIO::StripTrailingSpaces( parameterNames[i] );
        }

        // read and NULL terminate vector names
        fseek( s1, 8L, SEEK_CUR );
        for( int i = 0; i < numVectors; i++ )
        {
            if( parameterNames[numScalars + i] == NULL ) //.empty())//==NULL)
            {
                std::cerr << "ERROR: can't get memory for parameterNames, so exiting" << std::endl;
                exit( 0 );
            }
            parameterNames[numScalars + i][8] = '\0';
            fread( parameterNames[numScalars + i], sizeof( char ), 8, s1 );
            fileIO::StripTrailingSpaces( parameterNames[numScalars + i] );
        }

        if( debug )
        {
            for( int i = 0; i < numParameters; i++ )
            {
                std::cout << "parameterNames[" << i << "]: \t\"" << parameterNames[i] << "\"" << std::endl;
            }
        }

        // make room for xCenters, yCenters, zCenters
        float* xCenters = NULL, *yCenters = NULL, *zCenters = NULL;
        xCenters = new float [nx];
        yCenters = new float [ny];
        zCenters = new float [nz];
        if( xCenters == NULL || yCenters == NULL || zCenters == NULL )
        {
            std::cerr << "ERROR: can't get memory for centers, so exiting" << std::endl;
            exit( 0 );
        }

        // get xCenters
        fseek( s1, 8L, SEEK_CUR );
        if( fileIO::readNByteBlockFromFile( xCenters, sizeof( float ), nx, s1, endian_flip ) )
        {
            std::cerr << "ERROR: bad read in fileIO::readNByteBlockFromFile, so exiting" << std::endl;
            exit( 0 );
        }

        if( debug > 1 )
        {
            for( size_t i = 0; i < static_cast<size_t>( nx ); ++i )
            {
                std::cout << "xCenters[" << i << "] = " << xCenters[i] << std::endl;
            }
        }
        // get yCenters
        fseek( s1, 8L, SEEK_CUR );
        if( fileIO::readNByteBlockFromFile( yCenters, sizeof( float ), ny, s1, endian_flip ) )
        {
            std::cerr << "ERROR: bad read in fileIO::readNByteBlockFromFile, so exiting" << std::endl;
            exit( 0 );
        }

        if( debug > 1 )
        {
            for( size_t i = 0; i < static_cast<size_t>( ny ); i++ )
            {
                std::cout << "yCenters[" << i << "] = " << yCenters[i] << std::endl;
            }
        }
        // get zCenters
        fseek( s1, 8L, SEEK_CUR );
        if( fileIO::readNByteBlockFromFile( zCenters, sizeof( float ), nz, s1, endian_flip ) )
        {
            std::cerr << "ERROR: bad read in fileIO::readNByteBlockFromFile, so exiting" << std::endl;
            exit( 0 );
        }

        if( debug > 1 )
        {
            for( size_t i = 0; i < static_cast<size_t>( nz ); i++ )
            {
                std::cout << "zCenters[" << i << "] = " << zCenters[i] << std::endl;
            }
        }

        // populate the vertex coordinate floatArrays with the center values
        // SetArray uses the actual array provided; it does not copy the data from
        // the suppled array.
        // Set save to 1 to keep the class from deleting the array when it cleans
        // up or reallocates memory.

        if( debug )
        {
            std::cout << "DEBUG :: Allocating X_Centers float arrays " << std::endl;
        }

        vtkFloatArray* xCoords = vtkFloatArray::New();
        xCoords->SetArray( xCenters, nx, 0 );

        if( debug )
        {
            std::cout << "DEBUG :: Allocating Y_Centers float arrays " << std::endl;
        }

        vtkFloatArray* yCoords = vtkFloatArray::New();
        yCoords->SetArray( yCenters, ny, 0 );

        if( debug )
        {
            std::cout << "DEBUG :: Allocating Z_Centers float arrays " << std::endl;
        }

        vtkFloatArray* zCoords = vtkFloatArray::New();
        zCoords->SetArray( zCenters, nz, 0 );

        if( debug )
        {
            std::cout << "DEBUG :: Allocating vtkRectilinearGrid " << std::endl;
        }

        vtkRectilinearGrid* rGrid = vtkRectilinearGrid::New();
        rGrid->SetDimensions( nx, ny, nz );
        rGrid->SetXCoordinates( xCoords );
        rGrid->SetYCoordinates( yCoords );
        rGrid->SetZCoordinates( zCoords );

        xCoords->Delete();
        yCoords->Delete();
        zCoords->Delete();

        if( debug )
        {
            std::cout << "DEBUG :: Finished allocating float arrays " << std::endl;
        }

        // convertToStructuredGrid will supply a new structured grid
        sGrid = convertToStructuredGrid( rGrid );
        rGrid->Delete();


        // set up arrays to store scalar and vector data over entire mesh...
        vtkFloatArray** parameterData = NULL;
        parameterData = new vtkFloatArray * [numParameters];
        for( int i = 0; i < numParameters; i++ )
        {
            parameterData[i] = vtkFloatArray::New();
            if( parameterData[i] == NULL )
            {
                std::cerr << "ERROR: can't get memory for parameterData, so exiting" << std::endl;
                sGrid->Delete();
                sGrid = NULL;
                exit( 0 );
            }
        }

        // make room for each scalar data, we will use pointers to quickly create parameterData objects
        float** scalarData = new float * [numScalars];
        for( int i = 0; i < numScalars; i++ )
        {
            scalarData[i] = new float [num_verts];
            if( scalarData[i] == NULL )
            {
                std::cerr << "ERROR: can't get memory for scalar data, so exiting" << std::endl;
                exit( 0 );
            }
        }

        // load ALL of the scalar data into individual vtkFloatarrays
        for( int i = 0; i < numScalars; i++ )
        {
            fseek( s1, 8L, SEEK_CUR );
            if( fileIO::readNByteBlockFromFile( scalarData[i], sizeof( float ), num_verts, s1, endian_flip ) )
            {
                std::cerr << "ERROR: bad read in fileIO::readNByteBlockFromFile for scalar data, so exiting" << std::endl;
                exit( 0 );
            }

            // SetArray uses the actual array provided; it does not copy the data
            // from the suppled array.
            // Set save to 1 to keep the class from deleting the array when it
            // cleans up or reallocates memory.
            parameterData[ i ]->SetName( parameterNames[ i ] );
            parameterData[ i ]->SetNumberOfComponents( 1 );
            parameterData[ i ]->SetNumberOfTuples( num_verts );
            parameterData[ i ]->SetArray( scalarData[ i ], num_verts, 0 );

            if( debug )
                std::cout << "Reading scalarData[" << i + 1 << " of "
                          << numScalars << "]:\t\""
                          << parameterNames[i] << "\"" << std::endl;

            if( debug > 1 )
            {
                for( int j = 0; j < 4; j++ )
                    std::cout << "scalarData[" << i << "][" << j << "] = "
                              << scalarData[i][j] << std::endl;

                std::cout << "                ..." << std::endl;

                for( int j = num_verts - 4; j < num_verts; j++ )
                    std::cout << "scalarData[" << i << "][" << j << "] = "
                              << scalarData[i][j] << std::endl;

                std::cout << std::endl;
            }
        }

        // make room for vector data
        //      int xyz;
        float** vectorData = new float * [3];
        for( int xyz = 0; xyz < 3; xyz++ )
        {
            vectorData[xyz] = new float [num_verts];
            if( vectorData[xyz] == NULL )
            {
                std::cerr << "ERROR: can't get memory for vector data, so exiting" << std::endl;
                sGrid->Delete();
                sGrid = NULL;
                exit( 0 );
            }
        }

        // get vector data
        for( int i = 0; i < numVectors; i++ )
        {
            if( debug )
                std::cout << "Reading vectorData[" << i + 1 << " of "
                          << numVectors << "]:\t\""
                          << parameterNames[numScalars + i] << "\"" << std::endl;

            for( int xyz = 0; xyz < 3; xyz++ )
            {
                fseek( s1, 8L, SEEK_CUR );
                if( fileIO::readNByteBlockFromFile( vectorData[xyz], sizeof( float ),
                                                    num_verts, s1, endian_flip ) )
                {
                    std::cerr << "ERROR: bad read in fileIO::readNByteBlockFromFile, so exiting" << std::endl;
                    sGrid->Delete();
                    sGrid = NULL;
                    exit( 0 );
                }

                parameterData[numScalars + i]->SetName( parameterNames[numScalars + i] );
                parameterData[numScalars + i]->SetNumberOfComponents( 3 );
                parameterData[numScalars + i]->SetNumberOfTuples( num_verts );
                for( int tuple = 0; tuple < num_verts; tuple++ )
                {
                    parameterData[numScalars + i]->SetComponent( tuple, xyz, vectorData[xyz][tuple] );
                }

                // print begining and ending of vector data to screen...
                if( debug )
                {
                    for( int j = 0; j < 20; j++ )
                        std::cout << "vectorData[" << i << "][" << xyz << "][" << j << "] = "
                                  << vectorData[xyz][j] << std::endl;
                    std::cout << "                ..." << std::endl;
                    for( int j = num_verts - 20; j < num_verts; j++ )
                        std::cout << "vectorData[" << i << "][" << xyz << "][" << j << "] = "
                                  << vectorData[xyz][j] << std::endl;
                    std::cout << std::endl;
                }
            }
        }

        // doublecheck that you are finished: should see the message ...
        // "end of file found after reading 1 more floats"
        if( debug )
        {
            fileIO::readToFileEnd( s1 );
        }
        fclose( s1 );
        std::cout << std::endl;

        //delete parameterNames
        //for (i=0; i < numParameters; i++) delete parameterNames[i];
        delete [] parameterNames;
        parameterNames = 0;//parameterNames.clear();//delete [] parameterNames;      parameterNames = 0;

        // Add selected scalar and vector quantities to the pointdata array
        letUsersAddParamsToField( numParameters, parameterData, sGrid->GetPointData() );

        /*
           // don't need to delete this one vtk will take care of it when parameterData is deleted
           //delete scalarData
           for(i=0; i<numScalars; i++)
              delete [] scalarData[i];   scalarData[i] = NULL;
           delete [] scalarData;         scalarData = NULL;
        */

        //delete vectorData
        for( int i = 0; i < 3; i++ )
        {
            delete [] vectorData[ i ];
            vectorData[ i ] = NULL;
        }

        delete [] vectorData;
        vectorData = NULL;

        for( int i = 0; i < num_verts; i++ )
        {
            if( parameterData[ 0 ]->GetComponent( i, 0 ) == 8 )
            {
                sGrid->BlankPoint( i );
            }
        }

        //delete parameterData
        for( int i = 0; i < numParameters; i++ )
        {
            parameterData[ i ]->Delete();
        }

        delete [] parameterData;
        parameterData = NULL;

        if( !outputDataset )
        {
            outputDataset = vtkStructuredGrid::New();
        }
        outputDataset->ShallowCopy( sGrid );
        outputDataset->Update();
    }
}
////////////////////////////////////////////////////////////////////////////////
void cfdREITranslator::DisplayHelp( void )
{
    std::cout << "|\tREI Translator Usage:" << std::endl
              << "\t -singleFile <filename_to_load> -o <output_dir> "
              << "-outFileName <output_filename> -loader BANFDB -w file" << std::endl;
}
