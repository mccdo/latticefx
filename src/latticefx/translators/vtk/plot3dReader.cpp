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
#include <latticefx/translators/vtk/plot3dReader.h>

#include <vtkStructuredGridWriter.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkPLOT3DReader.h>
#include <vtkAppendFilter.h>
#include <vtkExtractUnstructuredGrid.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkDataSetAttributes.h>
#include <vtkStructuredGridGeometryFilter.h>
#include <vtkAppendPolyData.h>

#include <string>
#include <sstream>
#include <iostream>
#include <latticefx/utils/vtk/fileIO.h>
#include <latticefx/utils/vtk/Grid2Surface.h>
#include <latticefx/utils/vtk/readWriteVtkThings.h>

using namespace lfx::vtk_utils;
using namespace lfx::vtk_translator;

plot3dReader::plot3dReader( void )
{

    SetTranslateCallback( &plot3dToVTK );
    SetPreTranslateCallback( &cmdParser );
}

plot3dReader::~plot3dReader( void )
{}
////////////////////////////////////////////////////////////////////////////////
plot3dReader::plot3dReader( plot3dReader* )
{
    ;
}
//////////////////////////////////////////////////////////////////////////
void plot3dReader::Plot3DPreTranslateCbk::Preprocess( int argc, char** argv,
        cfdTranslatorToVTK* toVTK )
{
    plot3dReader* plot3DToVTK =
        dynamic_cast< plot3dReader* >( toVTK );

    if( plot3DToVTK )
    {
        toVTK->_extractOptionFromCmdLine( argc, argv, std::string( "-geometryFileXYZ" ), xyzFilename );
        toVTK->_extractOptionFromCmdLine( argc, argv, std::string( "-dataFileQ" ), qFilename );
        toVTK->AddFoundFile( qFilename );
        toVTK->ExtractBaseName( qFilename );
        toVTK->_extractOptionFromCmdLine( argc, argv, std::string( "-multiGridFlag" ), multiGridFlag );
        toVTK->_extractOptionFromCmdLine( argc, argv, std::string( "-iblankFlag" ), iblankFlag );
        toVTK->_extractOptionFromCmdLine( argc, argv, std::string( "-bigEndian" ), byteFlag );
        toVTK->_extractOptionFromCmdLine( argc, argv, std::string( "-binaryFile" ), binaryFlag );
        toVTK->_extractOptionFromCmdLine( argc, argv, std::string( "-bytePad" ), byteCountFlag );
        toVTK->_extractOptionFromCmdLine( argc, argv, std::string( "-forceRead" ), forceReadFlag );
        toVTK->_extractOptionFromCmdLine( argc, argv, std::string( "-twoDimensions" ), numberOfDimensions );
        toVTK->SetNumberOfFoundFiles( 1 );
    }

    PreTranslateCallback::Preprocess( argc, argv, toVTK );
}
////////////////////////////////////////////////////////////////////////////////
std::string plot3dReader::Plot3DPreTranslateCbk::GetIBlankFlag( void )
{
    return iblankFlag;
}
////////////////////////////////////////////////////////////////////////////////
std::string plot3dReader::Plot3DPreTranslateCbk::GetByteFlag( void )
{
    return byteFlag;
}
////////////////////////////////////////////////////////////////////////////////
std::string plot3dReader::Plot3DPreTranslateCbk::GetBinaryFlag( void )
{
    return binaryFlag;
}
////////////////////////////////////////////////////////////////////////////////
std::string plot3dReader::Plot3DPreTranslateCbk::GetByteCountFlag( void )
{
    return byteCountFlag;
}
////////////////////////////////////////////////////////////////////////////////
std::string plot3dReader::Plot3DPreTranslateCbk::GetForceReadFlag( void )
{
    return forceReadFlag;
}
////////////////////////////////////////////////////////////////////////////////
std::string plot3dReader::Plot3DPreTranslateCbk::GetNumberOfDimensions( void )
{
    return numberOfDimensions;
}
////////////////////////////////////////////////////////////////////////////////
std::string plot3dReader::Plot3DPreTranslateCbk::GetMultigridFlag( void )
{
    return multiGridFlag;
}
//////////////////////////////////////////////////////////////////////////
std::string plot3dReader::Plot3DPreTranslateCbk::GetXYZFilename( void )
{
    return xyzFilename;
}
//////////////////////////////////////////////////////////////////////////
std::string plot3dReader::Plot3DPreTranslateCbk::GetQFilename( void )
{
    return qFilename;
}
////////////////////////////////////////////////////////////////////////////////
void plot3dReader::Plot3DTranslateCbk::Translate( vtkDataObject*& outputDataset,
        cfdTranslatorToVTK* toVTK,
        vtkAlgorithm*& dataReader )
{
    plot3dReader* plot3DToVTK =
        dynamic_cast< plot3dReader* >( toVTK );

    if( plot3DToVTK )
    {
        writer      = vtkStructuredGridWriter::New();
        reader      = vtkPLOT3DReader::New();
        dataReader = reader;
        //reader->DebugOn();
        //reader->ReleaseDataFlagOn();
        unswriter   = vtkUnstructuredGridWriter::New();
        unsgrid     = vtkUnstructuredGrid::New();
        //unsgrid->ReleaseDataFlagOn();
        filter      = vtkAppendFilter::New();
        //filter->ReleaseDataFlagOn();
        numOfSurfaceGrids = 0;

        std::string byteOrder =
            dynamic_cast<  plot3dReader::Plot3DPreTranslateCbk* >( plot3DToVTK->GetPreTranslateCallback() )->GetByteFlag();
        if( byteOrder == "1" )
        {
            reader->SetByteOrderToBigEndian();
        }
        else
        {
            reader->SetByteOrderToLittleEndian();
        }

        std::string binary =
            dynamic_cast<  plot3dReader::Plot3DPreTranslateCbk* >( plot3DToVTK->GetPreTranslateCallback() )->GetBinaryFlag();
        if( binary == "1" )
        {
            reader->BinaryFileOn();
        }
        else
        {
            reader->BinaryFileOff();
        }

        std::string multiGrid =
            dynamic_cast<  plot3dReader::Plot3DPreTranslateCbk* >( plot3DToVTK->GetPreTranslateCallback() )->GetMultigridFlag();
        if( multiGrid == "1" )
        {
            reader->MultiGridOn();
        }
        else
        {
            reader->MultiGridOff();
        }

        std::string iBlank =
            dynamic_cast<  plot3dReader::Plot3DPreTranslateCbk* >( plot3DToVTK->GetPreTranslateCallback() )->GetIBlankFlag();
        if( iBlank == "1" )
        {
            reader->IBlankingOn();
        }
        else
        {
            reader->IBlankingOff();
        }

        std::string byteCount =
            dynamic_cast<  plot3dReader::Plot3DPreTranslateCbk* >( plot3DToVTK->GetPreTranslateCallback() )->GetByteCountFlag();
        if( byteCount == "1" )
        {
            reader->HasByteCountOn();
        }
        else
        {
            reader->HasByteCountOff();
        }

        std::string forceRead =
            dynamic_cast<  plot3dReader::Plot3DPreTranslateCbk* >( plot3DToVTK->GetPreTranslateCallback() )->GetForceReadFlag();
        if( forceRead == "1" )
        {
            reader->ForceReadOn();
        }
        else
        {
            reader->ForceReadOff();
        }

        std::string dimensions =
            dynamic_cast<  plot3dReader::Plot3DPreTranslateCbk* >( plot3DToVTK->GetPreTranslateCallback() )->GetNumberOfDimensions();
        if( dimensions == "1" )
        {
            reader->TwoDimensionalGeometryOn();
        }
        else
        {
            reader->TwoDimensionalGeometryOff();
        }
        reader->DoNotReduceNumberOfOutputsOn();


        //reader->CanReadBinaryFile( gridName );
        reader->SetXYZFileName( dynamic_cast<  plot3dReader::Plot3DPreTranslateCbk* >( plot3DToVTK->GetPreTranslateCallback() )->GetXYZFilename().c_str() );
        //reader->CanReadBinaryFile( solutionName );
        reader->SetQFileName( dynamic_cast<  plot3dReader::Plot3DPreTranslateCbk* >( plot3DToVTK->GetPreTranslateCallback() )->GetQFilename().c_str() );
        reader->SetScalarFunctionNumber( 100 );
        reader->SetVectorFunctionNumber( 200 );
        reader->AddFunction( 110 );
        reader->AddFunction( 120 );
        reader->AddFunction( 140 );
        reader->AddFunction( 144 );
        reader->AddFunction( 153 );
        reader->AddFunction( 184 );
        reader->Update();

        numGrids = reader->GetNumberOfGrids();
        grids = new vtkStructuredGrid*[ numGrids ];

        //reader->Print( std::cout );
        std::cout << "|   Number of grids in " <<
                  numGrids << std::endl;
        //writer->DebugOn();
        writer->BreakOnError();
        for( size_t i = 0; i < static_cast<size_t>( numGrids ); i++ )
        {
            grids[ i ] = vtkStructuredGrid::New();
            grids[ i ]->ShallowCopy( ( vtkStructuredGrid* ) reader->GetOutput( static_cast<int>( i ) ) );
            //std::ostringstream number;
            //number << i;
            //std::string testname = "test_" + number.str() + '.' + "vts";
            //const char* Filename = testname.c_str();
            //writer->SetFileName( Filename );
            //if (!debug)
            //writer->SetFileTypeToBinary();
            //writer->SetInput( grids[ i ] );
            //writer->Print( std::cout );
            //writer->Write();
        }

        //filter->DebugOn();
        //filter->BreakOnError();

        for( size_t i = 0; i < static_cast<size_t>( numGrids ); i++ )
        {
            filter->AddInput( grids[ i ] );
            filter->Update();
            //filter->Print( cout );
        }

        unsgrid->ShallowCopy( ( vtkUnstructuredGrid* ) filter->GetOutput() );
        this->unsgrid->GetPointData()->SetActiveVectors( "Velocity" );
        //this->unsgrid->Print( cout );
        //vtkFilename = prefix + '.' + suffix;
        //Filename = vtkFilename.c_str();
        //unswriter->DebugOn();
        //unswriter->BreakOnError();
        //unswriter->SetFileName( Filename );
        //if (!debug)
        //unswriter->SetFileTypeToBinary();
        //unswriter->SetInput( ( vtkUnstructuredGrid * ) filter->GetOutput() );
        //unswriter->Print( cout );
        //unswriter->Write();

        /*int numPoints = ((vtkPointSet *)filter->GetOutput())->GetNumberOfPoints();
        std::cout << "|   Number of vertices in unstructured grid : " <<
           numPoints << std::endl;
        vtkExtractUnstructuredGrid *extunsgrid = vtkExtractUnstructuredGrid::New();
        //extunsgrid->DebugOn();
        extunsgrid->BreakOnError();
        extunsgrid->PointClippingOn();
        extunsgrid->CellClippingOff();
        extunsgrid->ExtentClippingOff();
        extunsgrid->MergingOn();

        extunsgrid->SetInput( ( vtkUnstructuredGrid * ) filter->GetOutput() );
        extunsgrid->SetPointMinimum( 0 );
        extunsgrid->SetPointMaximum( numPoints );
        extunsgrid->Update();
        std::cout << "|   Number of points after merged grid : " <<
           ( (vtkPointSet *)extunsgrid->GetOutput() )->GetNumberOfPoints() << std::endl;
        unswriter->SetFileName( "merged.vtk" );
        unswriter->SetInput( ( vtkUnstructuredGrid * ) extunsgrid->GetOutput() );
        //unswriter->Print( cout );
        //if (!debug)
        unswriter->SetFileTypeToBinary();
        unswriter->Write();
        */
        if( !outputDataset )
        {
            outputDataset = vtkUnstructuredGrid::New();
        }

        outputDataset->ShallowCopy( unsgrid );
        outputDataset->Update();
        //outputDataset->Print( std::cout );

        if( writer != NULL )
        {
            writer->Delete();
        }

        if( reader != NULL )
        {
            reader->Delete();
        }

        if( unsgrid != NULL )
        {
            unsgrid->Delete();
        }

        if( unswriter != NULL )
        {
            unswriter->Delete();
        }

        if( filter != NULL )
        {
            filter->Delete();
        }

        for( size_t i = 0; i < static_cast<size_t>( numGrids ); i++ )
        {
            grids[ i ]->Delete();
        }

        delete [] grids;
    }
}
////////////////////////////////////////////////////////////////////////////////
void plot3dReader::DisplayHelp( void )
{
    std::cout << "|\tPlot3D Translator Usage:" << std::endl
              << "\t -geometryFileXYZ <filename_to_load> -dataFileQ <filename_to_load> " << std::endl
              << "-o <output_dir> -multiGridFlag <0|1> -iblankFlag <0|1> -twoDimensions <0|1> " << std::endl
              << "-forceRead <0|1> -bigEndian <0|1> -bytePad <0|1> -binaryFile <0|1> " << std::endl
              << "-outFileName <output_filename> -loader xyz -w file" << std::endl;
}
////////////////////////////////////////////////////////////////////////////////
/*void plot3dReader::GetFileNames( void )
{
   numOfSurfaceGrids = 0;
   std::cout << "Do you want to create an unstrcutured grid or a surface" << std::endl;
   std::cout << "Answer (0) grid (1) surface" << std::endl;
   answer = fileIO::getIntegerBetween( 0, 1 );
   if(answer == 1 )
   {
      do
      {
         plot3dSurfaceFileName[ numOfSurfaceGrids ] = new char[ 100 ];
         do
         {
            std::cout << "Geometry file name (xyz file):\t" << std::endl;
            std::cin >> plot3dSurfaceFileName[ numOfSurfaceGrids ];
            std::cin.ignore();
         }
         while(! fileIO::isFileReadable( plot3dSurfaceFileName[ numOfSurfaceGrids ] ) );
         numOfSurfaceGrids+=1;

         std::cout << "Do you have another file to enter" << std::endl;
         std::cout << "Answer (0) No (1) Yes" << std::endl;
         answer = fileIO::getIntegerBetween( 0, 1 );

         std::cout << answer << std::endl;
         std::cout << "numgrid points   " << numOfSurfaceGrids << std::endl;
      }
      while(answer == 1 );
   }
   else
   {
      do
      {
         std::cout << "Geometry file name (xyz file):\t" << std::endl;
         std::cin >> plot3dGeomFileName;
         std::cin.ignore();
      }
      while(! fileIO::isFileReadable( plot3dGeomFileName ) );

      do
      {
         std::cout << "Data file name (q file):\t" << std::endl;
         std::cin >> plot3dDataFileName;
         std::cin.ignore();
      }
      while(! fileIO::isFileReadable( plot3dDataFileName ) );
   }
}*/
////////////////////////////////////////////////////////////////////////////////
/*vtkUnstructuredGrid *plot3dReader::MakeVTKSurfaceFromGeomFiles( void )
{
   int i;
   // Render the shell of the dataset
   vtkPLOT3DReader **body3;
   body3 = new vtkPLOT3DReader*[ numOfSurfaceGrids - 1 ];
  vtkStructuredGridGeometryFilter *cFilter = vtkStructuredGridGeometryFilter::New( );
   vtkAppendPolyData *polyfilter = vtkAppendPolyData::New();
   for(i = 0; i < numOfSurfaceGrids; i++ )
   {
    body3[i] = vtkPLOT3DReader::New();

std::cout << plot3dSurfaceFileName[ i ] << std::endl;
      body3[i]->SetXYZFileName( plot3dSurfaceFileName[ i ] );
    body3[i]->Update( );

     cFilter->SetInput( body3[i]->GetOutput( ) );
     cFilter->SetExtent( 0, 100, 0, 100, 0, 100 );
     cFilter->Update( );


      polyfilter->AddInput( (vtkPolyData *)cfdGrid2Surface( (vtkStructuredGrid * ) cFilter->GetOutput(), 0.6 ) );
      //cFilter->SetInput( body3->GetOutput( ) );
      //cFilter->SetExtent( 0, 100, 0, 100, 0, 100 );
      //cFilter->Update( );
      // filter->AddInput( (vtkStructuredGrid * ) cFilter->GetOutput() );
      polyfilter->Update();
   }

   writeVtkThing( (vtkDataSet *)polyfilter->GetOutput(), "testsurf.vtk", 1 );   //1 is for binary
   exit( 1 );
   //unsgrid->ShallowCopy( ( vtkUnstructuredGrid * ) filter->GetOutput() );
   //body3->Delete();
   return ( unsgrid );
}*/
