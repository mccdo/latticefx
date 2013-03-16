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

#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkRectilinearGrid.h>

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActor.h>
#include <vtkFollower.h>
#include <vtkCubeSource.h>
#include <vtkOutlineCornerFilter.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkDataSetMapper.h>
#include <vtkCamera.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkSTLReader.h>
#include <vtkScalarsToColors.h>
#include <vtkLookupTable.h>
#include <vtkCellDataToPointData.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkOutlineFilter.h>
#include <vtkCompositeDataPipeline.h>
#include <vtkGeometryFilter.h>

#include <latticefx/utils/vtk/setScalarAndVector.h>
#include <latticefx/utils/vtk/viewCells.h>
#include <latticefx/utils/vtk/Grid2Surface.h>

#include <latticefx/utils/vtk/fileIO.h>
#include <latticefx/utils/vtk/readWriteVtkThings.h>

using namespace lfx::vtk_utils;

void viewWhatsInFile( std::string vtkFilename, const float shrinkFactor );
vtkActor* getActorFromDataSet( vtkDataSet* dataset );
vtkActor* getActorFromFile( std::string vtkFilename );

int main( const int argc, char* argv[] )
{
    vtkCompositeDataPipeline* prototype = vtkCompositeDataPipeline::New();
    vtkAlgorithm::SetDefaultExecutivePrototype( prototype );
    prototype->Delete();

    int i;
    std::string vtkFilename;
    float shrinkFactor = 1.0;
    if( argc == 1 ) // Process a single vtk file that will be obtained from the user...
    {
        std::string fileContents( "input" );
        std::string defaultName( "../ves/share/vesuite/examples/simple/3scl.vtk" );
        vtkFilename = fileIO::getReadableFileFromDefault( fileContents,
                      defaultName );
        shrinkFactor = 0.95;
        std::cout << "Recommended shrinkFactor = " << shrinkFactor << std::endl;
        do
        {
            std::cout << "Input your shrinkFactor (0 < X <= 1, with 1 for no shrinkage)" << std::endl;
            std::cin >> shrinkFactor;
        }
        while( shrinkFactor <= 0.0 || 1.0 < shrinkFactor );

        viewWhatsInFile( vtkFilename, shrinkFactor );
    }
    else    // Process multiple vtk files and view in a simulated C6...
    {
        //Create one-time graphics stuff
        vtkRenderer* ren1 = vtkRenderer::New();
        ren1->TwoSidedLightingOn();
        vtkRenderWindow* renWin = vtkRenderWindow::New();
        renWin->AddRenderer( ren1 );
        vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();
        iren->SetRenderWindow( renWin );

        int numActors = argc - 1;
        vtkActor** actor = new vtkActor* [numActors];
        for( i = 0; i < numActors; i++ )
        {
            vtkFilename.assign( argv[i + 1] );
            actor[i] = getActorFromFile( vtkFilename );
            if( actor[i] == NULL )
            {
                continue;
            }

            //Add the actors to the renderer
            ren1->AddActor( actor[i] );
        }

        vtkActor* axesActor = NULL;
        vtkFollower* xActor = NULL;
        vtkFollower* yActor = NULL;
        vtkFollower* zActor = NULL;

        // Create the cube corners and the associated mapper and actor.
        vtkCubeSource* C6 = vtkCubeSource::New();
        C6->SetBounds( -5, 5, -5, 5, 0, 10 );
        vtkOutlineCornerFilter* C6CornerFilter = vtkOutlineCornerFilter::New();
        C6CornerFilter->SetInput( C6->GetOutput() );
        vtkPolyDataMapper* C6Mapper = vtkPolyDataMapper::New();
        C6Mapper->SetInput( C6CornerFilter->GetOutput() );
        vtkActor* C6Actor = vtkActor::New();
        C6Actor->SetMapper( C6Mapper );

        // intermediate cleanup
        C6->Delete();
        C6CornerFilter->Delete();
        C6Mapper->Delete();

        // Create the axes symbol actor.
        axesActor = vtkActor::New();
        GetAxesSymbol( axesActor );

        // Create the axes labels
        xActor = vtkFollower::New();
        yActor = vtkFollower::New();
        zActor = vtkFollower::New();
        GetAxesLabels( xActor, yActor, zActor );

        // Add the actors to the renderer.
        ren1->AddActor( C6Actor );
        ren1->AddActor( axesActor );
        ren1->AddActor( xActor );
        ren1->AddActor( yActor );
        ren1->AddActor( zActor );

        // Force the axis labels to always face camera
        xActor->SetCamera( ren1->GetActiveCamera() );
        yActor->SetCamera( ren1->GetActiveCamera() );
        zActor->SetCamera( ren1->GetActiveCamera() );

        // Set the position of the camera in world coordinates.
        // The default position is (0,0,1).
        ren1->GetActiveCamera()->SetPosition( 0, -10, 5 );

        //Set the view up direction for the camera. The default is (0,1,0)
        ren1->GetActiveCamera()->SetViewUp( 0, 0, 1 );

        ren1->GetActiveCamera()->Zoom( 0.3 );

        //ren1->SetViewport(0,0,1,1);
        //ren1->SetBackground( 1, 1, 1 );     // white window conflicts with white text and white C6 corners

        /*
               vtkCamera *cam1 = ren1->GetActiveCamera();
                   //cam1->Zoom( 1.5 );
                   cam1->Zoom( );
                   cam1->SetParallelProjection(1);    // no perspective
                   //cam1->ParallelProjectionOn();    // no perspective
        */

        // Reset the clipping range of the camera
        ren1->ResetCameraClippingRange();

        std::cout << "\nWith cursor on the graphics window, press 'e' to exit the viewer" << std::endl;

        // interact with data
        renWin->SetSize( 800, 800 );
        renWin->Render();
        iren->Start();

        // delete all the instances that have been created.
        for( i = 0; i < numActors; i++ )
        {
            if( actor[i] != NULL )
            {
                actor[i]->Delete();
            }
        }
        delete [] actor;
        actor = NULL;

        axesActor->Delete();
        xActor->Delete();
        yActor->Delete();
        zActor->Delete();
        C6Actor->Delete();

        ren1->Delete();
        renWin->Delete();
        iren->Delete();
    }

    return 0;
}

void viewWhatsInFile( std::string vtkFilename, const float shrinkFactor )
{
    std::cout << "viewWhatsInFile" << std::endl;
    //
    ///This will need to be changed to handle both vtkDataset and vtkMultigroupDataSet
    vtkDataObject* dataset = readVtkThing( vtkFilename, 1 );
    if( dataset->GetDataObjectType() == VTK_UNSTRUCTURED_GRID )
    {
        int showGraphics;
        std::cout << "\nPick an option for displaying cells..." << std::endl;
        std::cout << "Answer  "
                  << "(1) drawExteriorCellsOnly (faster)   (2) drawAllCells" << std::endl;
        std::cin >> showGraphics;

        if( showGraphics )
        {
            if( showGraphics == 1 )
            {
                vtkUnstructuredGrid* cTemp = extractExteriorCellsOnly(
                                                 ( vtkUnstructuredGrid* )dataset );
                viewCells( cTemp, shrinkFactor );
                cTemp->Delete();
            }
            else
            {
                // let the user pick the active scalar
                activateScalar( static_cast< vtkDataSet* >( dataset ) );

                // if multiple point data scalar arrays are present,
                // allow user to change active scalar
                //do
                {
                    viewCells( static_cast< vtkDataSet* >( dataset ), shrinkFactor );
                    //activateScalar( dataset );
                }
                //while ( 1 );
            }
        }
    }
    else if( dataset->IsA( "vtkCompositeDataSet" ) )
    {
        
    }
    else if( dataset->GetDataObjectType() == VTK_RECTILINEAR_GRID )
    {
        int showGraphics;
        std::cout << "\nPick an option for displaying cells "
                  << "(0=3D mesh, 1=cross-section): " << std::endl;
        std::cin >> showGraphics;
        if( showGraphics == 0 )
        {
            viewCells( static_cast< vtkDataSet* >( dataset ), shrinkFactor );
        }
        else
        {
            std::cout << " calling viewXSectionOfRectilinearGrid" << std::endl;
            viewXSectionOfRectilinearGrid( ( vtkRectilinearGrid* )dataset );
        }
    }
    else if( dataset->GetDataObjectType() == VTK_POLY_DATA )
    {
        // let the user pick the active scalar
        activateScalar( static_cast< vtkDataSet* >( dataset ) );
        viewCells( static_cast< vtkDataSet* >( dataset ), shrinkFactor );
    }
    else if( dataset->GetDataObjectType() == VTK_STRUCTURED_GRID )
    {
        std::cout << "IsFileStructuredGrid" << std::endl;
        viewCells( static_cast< vtkDataSet* >( dataset ), shrinkFactor );
    }
    else
    {
        std::cout << "ERROR - Unsupported vtk file format" << std::endl;
        exit( 1 );
    }
    dataset->Delete();

    return;
}

vtkActor* getActorFromDataSet( vtkDataSet* dataset )
{
    vtkActor* actor  = NULL;
    int numCells = dataset->GetNumberOfCells();
    if( numCells == 0 )
    {
        std::cout << "\tNothing to plot: The number of cells is " << numCells << std::endl;
        return actor;
    }
    else
    {
        std::cout << "\tgetActorFromDataSet: The number of cells is " << numCells << std::endl;
    }

    vtkDataSetMapper* map = vtkDataSetMapper::New();
    // By default, VTK uses OpenGL display lists which results in another copy
    // of the data being stored in memory. For most large datasets you will be
    // better off saving memory by not using display lists.  You can turn off
    // display lists by turning on ImmediateModeRendering.
    map->ImmediateModeRenderingOn();
    map->SetInput( dataset );

    actor = vtkActor::New();

    std::cout << "dataset->GetPointData()->GetNumberOfArrays() = "
              << dataset->GetPointData()->GetNumberOfArrays() << std::endl;

    if( dataset->GetPointData()->GetScalars() )
    {
        vtkLookupTable* lut = vtkLookupTable::New();
        lut->SetNumberOfColors( 256 ); //default is 256

        std::cout << "dataset->GetPointData()->GetScalars()->GetName() = "
                  << dataset->GetPointData()->GetScalars()->GetName() << std::endl;
        std::cout << "dataset->GetPointData()->GetScalars()->GetLookupTable() = "
                  << dataset->GetPointData()->GetScalars()->GetLookupTable() << std::endl;

        double minMax[ 2 ];
        minMax[ 0 ] = 0.0;
        minMax[ 1 ] = 1.0;

        if( dataset->GetPointData()->GetScalars()->GetLookupTable() )
        {
            std::cout << "lookup table is added in getActorFromDataSet" << std::endl;
        }
        else
        {
            lut->SetHueRange( 2.0f / 3.0f, 0.0f ); //a blue-to-red scale
            dataset->GetPointData()->GetScalars()->GetRange( minMax );
            std::cout << "minMax: " << minMax[0] << " " << minMax[1] << std::endl;
            lut->SetTableRange( minMax );
            lut->Build();
            /*
                     // create a white-to-clear lookup table...
                     //for ( int i=0; i<256; i++ ) lut->SetTableValue( i, 1.0, 1.0, 1.0, float(255-i)/255.0 );
                     for ( int i=0; i<256; i++ ) lut->SetTableValue( i, 1.0, 1.0, 1.0, cos(0.5*3.1415926*float(i)/255.0) );
            */
            /*
                     //debugging
                     dataset->GetPointData()->GetScalars()->SetLookupTable( lut );
                     writeVtkThing( dataset, "junk.vtk", 0 );
            */
        }
        map->SetScalarRange( minMax );
        map->SetLookupTable( lut );
        lut->Delete();
    }
    else
    {
        actor->GetProperty()->SetColor( 1, 0, 0 );
    }

    actor->SetMapper( map );
    map->Delete();

    actor->GetProperty()->SetOpacity( 0.5 );

    return actor;
}

vtkActor* getActorFromFile( std::string vtkFilename )
{
    //std::cout << "getActorFromFile" << std::endl;
    vtkActor* actor = NULL;

    std::string extension = fileIO::getExtension( vtkFilename );
    //std::cout << "vtkFilename = \"" << vtkFilename << "\"" << std::endl;
    //std::cout << "extension = \"" << extension << "\"" << std::endl;

    if( !extension.compare( "bmp" ) || !extension.compare( "BMP" ) )
    {
        /*
        delete [] extension;
        cfdImage* image = new cfdImage( vtkFilename, 0 );
        return image->GetActor();
        */
        std::cout << "Bmp's not supported by cfdImage!!!" << std::endl;

    }
    else if( !extension.compare( "stl" ) || !extension.compare( "STL" ) )
    {
        vtkSTLReader* reader = vtkSTLReader::New();
        //reader->DebugOn();
        reader->SetFileName( vtkFilename.c_str() );
        reader->Update();
        actor = getActorFromDataSet( reader->GetOutput() );
        reader->Delete();
        extension.erase();
        return actor;
    }
    else
    {
        extension.erase();
    }
    ///This will need to be changed to handle both vtkDataset and vtkMultigroupDataSet
    vtkDataObject* dataset = readVtkThing( vtkFilename, 1 );
    if( dataset->GetDataObjectType() == VTK_UNSTRUCTURED_GRID )
    {
        std::cout << "IsFileUnstructuredGrid" << std::endl;

        int showGraphics;
        std::cout << "\nPick an option for displaying cells..." << std::endl;
        std::cout << "Answer  (1) drawExteriorFacesOnly (for faster interaction)   (2) drawAllCells" << std::endl;
        std::cin >> showGraphics;

        //if (showGraphics)
        {
            if( showGraphics == 1 )
            {
                vtkPolyData* cTemp = Grid2Surface( dataset, 0 );
                actor = getActorFromDataSet( cTemp );
                cTemp->Delete();
            }
            else
            {
                // let the user pick the active scalar
                activateScalar( static_cast< vtkDataSet* >( dataset ) );

                // if multiple point data scalar arrays are present,
                // allow user to change active scalar
                //do
                {
                    actor = getActorFromDataSet( static_cast< vtkDataSet* >( dataset ) );
                    //activateScalar( dataset );
                }
                //while ( 1 );
            }
        }
    }
    else if( dataset->IsA( "vtkCompositeDataSet" ) )
    {
        std::cout << "IsvtkCompositeDataSet" << std::endl;
        /*vtkCellDataToPointData* c2p = vtkCellDataToPointData::New();
        c2p->SetInput( dataset );

        vtkCompositeDataGeometryFilter* m_multiGroupGeomFilter =
            vtkCompositeDataGeometryFilter::New();
        m_multiGroupGeomFilter->SetInputConnection( c2p->GetOutputPort() );

        vtkOutlineFilter* outlineData = vtkOutlineFilter::New();
        outlineData->SetInputConnection( m_multiGroupGeomFilter->GetOutputPort() );

        vtkPolyDataMapper* mapOutline = vtkPolyDataMapper::New();
        mapOutline->SetInputConnection( outlineData->GetOutputPort() );
        
        actor = vtkActor::New();
        actor->SetMapper( mapOutline );
        actor->GetProperty()->SetColor( 1, 0, 0 );

        c2p->Delete();
        m_multiGroupGeomFilter->Delete();
        outlineData->Delete();
        mapOutline->Delete();*/

        vtkCellDataToPointData* c2p = vtkCellDataToPointData::New();
        if( dataset->IsA( "vtkCompositeDataSet" ) )
        {
            vtkCompositeDataGeometryFilter* wireframe = vtkCompositeDataGeometryFilter::New();
            wireframe->SetInput( dataset );
            
            c2p->SetInputConnection( wireframe->GetOutputPort() );
            
            wireframe->Delete();
        }
        else
        {
            vtkGeometryFilter* wireframe = vtkGeometryFilter::New();
            wireframe->SetInput( dataset );
            
            c2p->SetInputConnection( wireframe->GetOutputPort() );
            
            wireframe->Delete();
        }
        
        vtkPolyDataMapper* wireframeMapper = vtkPolyDataMapper::New();
        wireframeMapper->SetInputConnection( c2p->GetOutputPort() );
        //vtkPolyData* poly = lfx::vtk_utils::cfdGrid2Surface( this->GetDataSet(), 0.8f );
        wireframeMapper->SetScalarModeToUsePointFieldData();
        //mapper->SetScalarModeToDefault();
        wireframeMapper->UseLookupTableScalarRangeOn();
        //wireframeMapper->SelectColorArray( GetActiveScalarName().c_str() );
        //wireframeMapper->SetLookupTable( GetLookupTable() );
        //wireframeMapper->Update();
        c2p->Delete();
        
        actor = vtkActor::New();
        actor->SetMapper( wireframeMapper );
        //wireframeActor->GetProperty()->SetColor( 0, 0, 1 );
        actor->GetProperty()->SetOpacity( 0.7f );
        //actor->GetProperty()->SetRepresentationToWireframe();
        //actor->GetProperty()->EdgeVisibilityOn();
        //wireframeMapper->Delete();
    }
    else if( dataset->GetDataObjectType() == VTK_RECTILINEAR_GRID )
    {
        std::cout << "IsFileRectilinearGrid" << std::endl;
        actor = getActorFromDataSet( static_cast< vtkDataSet* >( dataset ) );
    }
    else if( dataset->GetDataObjectType() == VTK_POLY_DATA )
    {
        std::cout << "IsFilePolyData" << std::endl;
        actor = getActorFromDataSet( static_cast< vtkDataSet* >( dataset ) );
    }
    else if( dataset->GetDataObjectType() == VTK_STRUCTURED_GRID )
    {
        std::cout << "IsFileStructuredGrid" << std::endl;
        actor = getActorFromDataSet( static_cast< vtkDataSet* >( dataset ) );
    }
    else
    {
        std::cout << "ERROR - Unsupported vtk file format" << std::endl;
    }
    dataset->Delete();
    return actor;
}
