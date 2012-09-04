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
#include <latticefx/utils/vtk/viewCells.h>
#include <iostream>

#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkShrinkFilter.h>        // for visualization
#include <vtkStripper.h>            // for visualization
#include <vtkDataSetMapper.h>         // for visualization
#include <vtkActor.h>              // for visualization
#include <vtkRenderWindowInteractor.h>   // for visualization
#include <vtkPolyDataMapper.h>  // for visualization
#include <vtkRectilinearGrid.h> // for visualization
#include <vtkRectilinearGridGeometryFilter.h>   // for visualization
#include <vtkCubeSource.h>
#include <vtkOutlineCornerFilter.h>
#include <vtkVectorText.h>
#include <vtkFollower.h>

#include <vtkIdList.h>
#include <vtkGenericCell.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkCellArray.h>
#include <vtkLookupTable.h>

using namespace lfx::vtk_utils;

vtkUnstructuredGrid* lfx::vtk_utils::extractExteriorCellsOnly( vtkUnstructuredGrid *output )
{
    int pt, npts, ptId;
    vtkIdList *pts = vtkIdList::New();
    vtkPoints *extCellPoints = vtkPoints::New();
    vtkIdList *cellIds = vtkIdList::New();
    vtkGenericCell *cell = vtkGenericCell::New();
    vtkUnstructuredGrid *exteriorCells = vtkUnstructuredGrid::New();
    double *x;
    //    exteriorCells->DebugOn();

    int numCells = output->GetNumberOfCells();
    std::cout << "     At start of extractExteriorCellsOnly, the number of cells is " << numCells << std::endl;
    exteriorCells->Allocate( numCells, numCells );

    // Loop over the cells and remove interior cells...
    for( int cellId = 0; cellId < numCells; cellId++ )
    {
        int thisIsExteriorCell = 0;
        output->GetCell( cellId, cell );

        vtkCell *face = NULL;
        for( int j = 0; j < cell->GetNumberOfFaces(); j++ )
        {
            face = cell->GetFace( j );
            output->GetCellNeighbors( cellId, face->PointIds, cellIds );
            //face->Delete();
//              std::cout << "cellId=" << cellId << ", face " << j << ", cellIds->GetNumberOfIds()=" << cellIds->GetNumberOfIds() << std::endl;
            if( cellIds->GetNumberOfIds() <= 0 ) // exterior faces have a zero here
            {
                thisIsExteriorCell = 1;
                break;
            }
        }

        if( thisIsExteriorCell )
        {
            //std::cout << "ext cell found" << std::endl;
            npts = cell->GetNumberOfPoints();
            pts->Reset();
            for( int i = 0; i < npts; i++ )
            {
                ptId = cell->GetPointId( i );
                x = output->GetPoint( ptId );
                pt = extCellPoints->InsertNextPoint( x );
                pts->InsertId( i, pt );
//                std::cout << "   Inserted point pt=" << pt << std::endl;
            }
//            std::cout << " cell->GetCellType()=" <<  cell->GetCellType() << std::endl;
            exteriorCells->InsertNextCell( cell->GetCellType(), pts );
        }
    }//for all cells

    exteriorCells->SetPoints( extCellPoints );
    //exteriorCells->Squeeze();    // Reclaim any extra memory used to store data. vtk says: THIS METHOD IS NOT THREAD SAFE
//    exteriorCells->Print( std::cout );

    //numCells = exteriorCells->GetNumberOfCells();
    //std::cout << "     After removing interior cells, the number of cells is " << numCells << std::endl;

    pts->Delete();
    extCellPoints->Delete();
    cellIds->Delete();
    cell->Delete();
    return exteriorCells;
}

void lfx::vtk_utils::viewCells( vtkDataSet *dataset, const float shrinkFactor )
{
    std::cout << "\nviewCells: Preparing to view mesh..." << std::endl;
    int numCells = dataset->GetNumberOfCells();
    std::cout << "     The number of cells is " << numCells << std::endl;
    int numPts = dataset->GetNumberOfPoints();
    std::cout << "     The number of points is " << numPts << std::endl;

    if( numCells == 0 ) return;

    //Create one-time graphics stuff
    vtkRenderer* ren1 = vtkRenderer::New();
    vtkRenderWindow* renWin = vtkRenderWindow::New();
    renWin->AddRenderer( ren1 );
    vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();
    iren->SetRenderWindow( renWin );

    //Create actor from dataset and add to the renderer
    AddToRenderer( dataset, ren1, shrinkFactor );

    vtkActor * axesActor = NULL;
    vtkFollower * xActor = NULL;
    vtkFollower * yActor = NULL;
    vtkFollower * zActor = NULL;
    vtkActor * C6Actor = NULL;

    int hasCaveCorners = 1;
    if( hasCaveCorners )
    {
        // Create the cube corners and the associated mapper and actor.
        vtkCubeSource * C6 = vtkCubeSource::New();
        C6->SetBounds( -5, 5, -5, 5, 0, 10 );
        vtkOutlineCornerFilter * C6CornerFilter = vtkOutlineCornerFilter::New();
        C6CornerFilter->SetInput( C6->GetOutput() );
        vtkPolyDataMapper * C6Mapper = vtkPolyDataMapper::New();
        C6Mapper->SetInput( C6CornerFilter->GetOutput() );

        C6Actor = vtkActor::New();
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

        // Set the position of the camera in world coordinates. The default position is (0,0,1).
        ren1->GetActiveCamera()->SetPosition( 0, -10, 5 );

        //Set the view up direction for the camera. The default is (0,1,0)
        ren1->GetActiveCamera()->SetViewUp( 0, 0, 1 );

        ren1->GetActiveCamera()->Zoom( 0.3 );
    }

    //ren1->SetViewport(0,0,1,1);
    //ren1->SetBackground( 1, 1, 1 );     // white window conflicts with white text and white C6 corners

    // Reset the clipping range of the camera; set the camera of the follower; render.
    ren1->ResetCameraClippingRange();

    /*
        vtkCamera *cam1 = ren1->GetActiveCamera();
            //cam1->Zoom( 1.5 );
            cam1->Zoom( );
            cam1->SetParallelProjection(1);    // no perspective
            //cam1->ParallelProjectionOn();    // no perspective
    */
    std::cout << "\nWith cursor on the graphics window, press 'e' to exit the viewer" << std::endl;

    // interact with data
    renWin->SetSize( 800, 800 );
    renWin->Render();
    iren->Start();

    // delete all the instances that have been created.
    if( hasCaveCorners )
    {
        C6Actor->Delete();
        axesActor->Delete();
        xActor->Delete();
        yActor->Delete();
        zActor->Delete();
    }

    ren1->Delete();
    renWin->Delete();
    iren->Delete();
}

void lfx::vtk_utils::viewXSectionOfRectilinearGrid( vtkRectilinearGrid *output )
{
    std::cout << "\nPreparing to view mesh..." << std::endl;
    int numCells = output->GetNumberOfCells();
    std::cout << "     The number of cells is " << numCells << std::endl;

    int dim[3];
    output->GetDimensions( dim );
    int nx = dim[0];
    int ny = dim[1];
    int nz = dim[2];
    std::cout << "nx = " << nx << ", ny = " << ny << ", nz = " << nz << std::endl;

    //view a cross-section parallel to y-z axes, at half-way point of model
    vtkRectilinearGridGeometryFilter *plane = vtkRectilinearGridGeometryFilter::New();
    plane->SetInput( output );
    plane->SetExtent( nx / 2, nx / 2, 0, ny, 0, nz );
    plane->Update();

    vtkPolyDataMapper *rgridMapper = vtkPolyDataMapper::New();
    rgridMapper->SetInput( plane->GetOutput() );

//    vtkPolyDataMapper *rgridMapper = vtkPolyDataMapper::New();
//    rgridMapper->SetInput(rgrid);
    vtkActor *wireActor = vtkActor::New();
    wireActor->SetMapper( rgridMapper );
//    wireActor->GetProperty()->SetRepresentationToWireframe();
    wireActor->GetProperty()->SetColor( 0, 0, 0 );

    //Create graphics stuff
    vtkRenderer* ren1 = vtkRenderer::New();
    vtkRenderWindow* renWin = vtkRenderWindow::New();
    renWin->AddRenderer( ren1 );
    vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();
    iren->SetRenderWindow( renWin );

    //Add the actors to the renderer, set the background and size
    ren1->AddActor( wireActor );
    ren1->SetBackground( 1, 1, 1 );

    vtkCamera *cam1 = ren1->GetActiveCamera();
    cam1->Zoom( 1.5 );
    cam1->SetParallelProjection( 1 );  // no perspective

    std::cout << "\nWith cursor on the graphics window, press 'e' to exit the viewer" << std::endl;

    // interact with data
    renWin->SetSize( 500, 500 );
    renWin->Render();
    iren->Start();

    //clean up
    plane->Delete();
    rgridMapper->Delete();
    wireActor->Delete();
    ren1->Delete();
    renWin->Delete();
    iren->Delete();
    cam1->Delete();

    return;
}

/*
void viewXSection( vtkDataObject *output )
{
    std::cout << "\nPreparing to view mesh..." << std::endl;
    int numCells = output->GetNumberOfCells();
    std::cout << "     The number of cells is " << numCells << std::endl;

    int dim[3];
    output->GetDimensions( dim );
    int nx = dim[0];
    int ny = dim[1];
    int nz = dim[2];
    std::cout << "nx = " << nx << ", ny = " << ny << ", nz = " << nz << std::endl;

    //view a cross-section parallel to y-z axes, at half-way point of model
    vtkGeometryFilter *plane = vtkGeometryFilter::New();
        plane->SetInput(output);
        plane->SetExtent(nx/2,nx/2, 0,ny, 0,nz);
        plane->Update();

    vtkPolyDataMapper *rgridMapper = vtkPolyDataMapper::New();
    rgridMapper->SetInput(plane->GetOutput());

//    vtkPolyDataMapper *rgridMapper = vtkPolyDataMapper::New();
//    rgridMapper->SetInput(rgrid);
    vtkActor *wireActor = vtkActor::New();
    wireActor->SetMapper(rgridMapper);
//    wireActor->GetProperty()->SetRepresentationToWireframe();
    wireActor->GetProperty()->SetColor(0,0,0);

 //Create graphics stuff
 vtkRenderer* ren1 = vtkRenderer::New();
 vtkRenderWindow* renWin = vtkRenderWindow::New();
     renWin->AddRenderer( ren1 );
 vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();
     iren->SetRenderWindow( renWin );

 //Add the actors to the renderer, set the background and size
 ren1->AddActor( wireActor );
 ren1->SetBackground( 1, 1, 1 );

 vtkCamera *cam1 = ren1->GetActiveCamera();
  cam1->Zoom( 1.5 );
        cam1->SetParallelProjection(1);    // no perspective

 // interact with data
 renWin->SetSize( 500, 500 );
 renWin->Render();
 iren->Start();

    //clean up
    plane->Delete();
    rgridMapper->Delete();
    wireActor->Delete();
    ren1->Delete();
    renWin->Delete();
    iren->Delete();

    return;
}
*/

void lfx::vtk_utils::GetAxesSymbol( vtkActor * axesActor )
{
    // Create the axes and the associated mapper and actor.
    vtkPoints *newPts = vtkPoints::New();
    newPts->Allocate( 6 );

    vtkCellArray *newLines = vtkCellArray::New();
    newLines->Allocate( 3 );

    float x[3];
    vtkIdType pts[2];

    pts[0] = 0;
    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    newPts->InsertPoint( pts[0], x );

    pts[1] = 1;
    x[0] = 1;
    x[1] = 0;
    x[2] = 0;
    newPts->InsertPoint( pts[1], x );
    newLines->InsertNextCell( 2, pts );

    pts[0] = 2;
    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    newPts->InsertPoint( pts[0], x );

    pts[1] = 3;
    x[0] = 0;
    x[1] = 1;
    x[2] = 0;
    newPts->InsertPoint( pts[1], x );
    newLines->InsertNextCell( 2, pts );

    pts[0] = 4;
    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    newPts->InsertPoint( pts[0], x );

    pts[1] = 5;
    x[0] = 0;
    x[1] = 0;
    x[2] = 1;
    newPts->InsertPoint( pts[1], x );
    newLines->InsertNextCell( 2, pts );

    vtkPolyData *output = vtkPolyData::New();
    output->SetPoints( newPts );
    newPts->Delete();

    output->SetLines( newLines );
    newLines->Delete();

    vtkPolyDataMapper * axesMapper = vtkPolyDataMapper::New();
    axesMapper->SetInput( output );

    axesActor->SetMapper( axesMapper );

    output->Delete();
    axesMapper->Delete();
}

void lfx::vtk_utils::GetAxesLabels( vtkFollower * xActor,
                                        vtkFollower * yActor,
                                        vtkFollower * zActor )
{
    // Create the 3D text and the associated mapper and follower
    vtkVectorText * xText = vtkVectorText::New();
    xText->SetText( "X" );
    vtkVectorText * yText = vtkVectorText::New();
    yText->SetText( "Y" );
    vtkVectorText * zText = vtkVectorText::New();
    zText->SetText( "Z" );

    vtkPolyDataMapper * xMapper = vtkPolyDataMapper::New();
    xMapper->SetInput( xText->GetOutput() );

    xActor->SetMapper( xMapper );
    xActor->SetScale( 0.2, 0.2, 0.2 );
    xActor->AddPosition( 1, -0.1, 0 );

    vtkPolyDataMapper * yMapper = vtkPolyDataMapper::New();
    yMapper->SetInput( yText->GetOutput() );

    yActor->SetMapper( yMapper );
    yActor->SetScale( 0.2, 0.2, 0.2 );
    yActor->AddPosition( 0, 1 - 0.1, 0 );

    vtkPolyDataMapper * zMapper = vtkPolyDataMapper::New();
    zMapper->SetInput( zText->GetOutput() );

    zActor->SetMapper( zMapper );
    zActor->SetScale( 0.2, 0.2, 0.2 );
    zActor->AddPosition( 0, -0.1, 1 );

    xText->Delete();
    yText->Delete();
    zText->Delete();

    xMapper->Delete();
    yMapper->Delete();
    zMapper->Delete();
}

void lfx::vtk_utils::AddToRenderer( vtkDataSet *dataset, vtkRenderer* ren1, const float shrinkFactor )
{
    //std::cout << "\nPreparing to view mesh..." << std::endl;
    int numCells = dataset->GetNumberOfCells();
    /*
        std::cout << "     The number of cells is " << numCells << std::endl;
        int numPts = dataset->GetNumberOfPoints();
        std::cout << "     The number of points is "<< numPts << std::endl;
    */

    if( numCells == 0 )
    {
        std::cout << "\tNothing to plot in AddToRenderer: The number of cells is " << numCells << std::endl;
        return;
    }
    //else    std::cout << "\tAddToRenderer: The number of cells is " << numCells << std::endl;

    vtkDataSetMapper *map = vtkDataSetMapper::New();
//    By default, VTK uses OpenGL display lists which results in another copy of the data being stored
//    in memory. For most large datasets you will be better off saving memory by not using display lists.
//    You can turn off display lists by turning on ImmediateModeRendering.
    map->ImmediateModeRenderingOn();

    //std::cout << "Using shrinkFactor = " << shrinkFactor << std::endl;

    vtkShrinkFilter *shrink = NULL;
    if( shrinkFactor < 1.0 )
    {
        shrink = vtkShrinkFilter::New();
        shrink->SetInput( dataset );
        shrink->SetShrinkFactor( shrinkFactor );
        //shrink->GetOutput()->ReleaseDataFlagOn();
        map->SetInput( shrink->GetOutput() );
    }
    else
        map->SetInput( dataset );

    /*
    //    extract geometry from data (or convert data to polygonal type)...
        vtkGeometryFilter *geom = vtkGeometryFilter::New();
            geom->SetInput( shrink->GetOutput() );    //don't use this with stock version of vtk3.2: has code error!
            geom->GetOutput()->ReleaseDataFlagOn();

    //    Most filters in VTK produce independent triangles or polygons which are not the most compact or
    //    efficient to render. To create triangle strips from polydata you can first use vtkTriangleFilter
    //    to convert any polygons to triangles (not required if you only have triangles to start with)
    //    then run it through a vtkStipper to convert the triangles into triangle strips.
    //    The only disadvantage to using triangle strips is that they require time to compute, so if your
    //    data is changing every time you render, it could actually be slower.

        vtkTriangleFilter *tris = vtkTriangleFilter::New();
            tris->SetInput( geom->GetOutput() );
            tris->GetOutput()->ReleaseDataFlagOn();

        vtkStripper *strip = vtkStripper::New();
            strip->SetInput(tris->GetOutput());
            strip->GetOutput()->ReleaseDataFlagOn();

        vtkPolyDataMapper *map = vtkPolyDataMapper::New();
    //    By default, VTK uses OpenGL display lists which results in another copy of the data being stored
    //    in memory. For most large datasets you will be better off saving memory by not using display lists.
    //    You can turn off display lists by turning on ImmediateModeRendering.
            map->ImmediateModeRenderingOn();
            map->SetInput(tris->GetOutput());
    //        map->SetInput(strip->GetOutput());
    */

    if( shrink )
        shrink->Delete();

    vtkActor * actor = vtkActor::New();

    if( dataset->GetPointData()->GetScalars() )
    {
        vtkLookupTable *lut = vtkLookupTable::New();
        lut->SetNumberOfColors( 256 ); //default is 256

        std::cout << "dataset->GetPointData()->GetScalars()->GetName() = "
        << dataset->GetPointData()->GetScalars()->GetName() << std::endl;
        //std::cout << "dataset->GetPointData()->GetScalars()->GetLookupTable() = " << dataset->GetPointData()->GetScalars()->GetLookupTable() << std::endl;

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
            std::cout << "tableRange: " << minMax[ 0 ] << " " << minMax[ 1 ] << std::endl;
            lut->SetTableRange( minMax );
            lut->Build();
        }
        map->SetScalarRange( minMax );
        map->SetLookupTable( lut );
        lut->Delete();
    }
    else
        actor->GetProperty()->SetColor( 1, 0, 0 );

    actor->SetMapper( map );
    actor->GetProperty()->SetRepresentationToWireframe();
    actor->GetProperty()->EdgeVisibilityOn();

    //Add the actors to the renderer, set the viewport and background
    ren1->AddActor( actor );
    map->Delete();
    actor->Delete();

    return;
}

