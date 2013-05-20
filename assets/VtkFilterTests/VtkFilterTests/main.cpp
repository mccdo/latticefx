/*=========================================================================

  Program:   Visualization Toolkit
  Module:    MultiBlock.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// This example demonstrates how multi-block datasets can be processed
// using the new vtkMultiBlockDataSet class.
//
// The command line arguments are:
// -D <path> => path to the data (VTKData); the data should be in <path>/Data/

#include "vtksys/ios/sstream"
#include "vtkActor.h"
#include "vtkCellDataToPointData.h"
#include "vtkCompositeDataGeometryFilter.h"
#include "vtkCompositeDataPipeline.h"
#include "vtkContourFilter.h"
#include "vtkDebugLeaks.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkOutlineCornerFilter.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkShrinkPolyData.h"
#include "vtkStructuredGrid.h"
#include "vtkStructuredGridOutlineFilter.h"
#include "vtkTestUtilities.h"
#include "vtkXMLStructuredGridReader.h"
#include "vtkSmartPointer.h"
#include "vtkBox.h"
#include "vtkExtractGeometry.h"
#include "vtkStructuredGridGeometryFilter.h"
#include "vtkExtractUnstructuredGrid.h"
#include "vtkExtractPolyDataGeometry.h"
#include "vtkExtractVOI.h"
#include "vtkSource.h"
#include "vtkPolyDataNormals.h"
#include "vtkPolyDataMapper.h"
#include "vtkTriangleFilter.h"
#include "vtkStripper.h"

#pragma comment(lib, "LSDyna.lib")
#pragma comment(lib, "MapReduceMPI.lib")
#pragma comment(lib, "mpistubs.lib")
#pragma comment(lib, "vtkalglib.lib")
#pragma comment(lib, "vtkCharts.lib")
#pragma comment(lib, "vtkCommon.lib")
#pragma comment(lib, "vtkDICOMParser.lib")
#pragma comment(lib, "vtkexoIIc.lib")
#pragma comment(lib, "vtkexpat.lib")
#pragma comment(lib, "vtkFiltering.lib")
#pragma comment(lib, "vtkfreetype.lib")
#pragma comment(lib, "vtkftgl.lib")
#pragma comment(lib, "vtkGenericFiltering.lib")
#pragma comment(lib, "vtkGeovis.lib")
#pragma comment(lib, "vtkGraphics.lib")
#pragma comment(lib, "vtkhdf5.lib")
#pragma comment(lib, "vtkhdf5_hl.lib")
#pragma comment(lib, "vtkHybrid.lib")
#pragma comment(lib, "vtkImaging.lib")
#pragma comment(lib, "vtkInfovis.lib")
#pragma comment(lib, "vtkIO.lib")
#pragma comment(lib, "vtkjpeg.lib")
#pragma comment(lib, "vtklibxml2.lib")
#pragma comment(lib, "vtkmetaio.lib")
#pragma comment(lib, "vtkNetCDF.lib")
#pragma comment(lib, "vtkNetCDF_cxx.lib")
#pragma comment(lib, "vtkpng.lib")
#pragma comment(lib, "vtkproj4.lib")
#pragma comment(lib, "vtkRendering.lib")
#pragma comment(lib, "vtksqlite.lib")
#pragma comment(lib, "vtksys.lib")
#pragma comment(lib, "vtktiff.lib")
#pragma comment(lib, "vtkverdict.lib")
#pragma comment(lib, "vtkViews.lib")
#pragma comment(lib, "vtkVolumeRendering.lib")
#pragma comment(lib, "vtkWidgets.lib")
#pragma comment(lib, "vtkzlib.lib")







void renderGraphBase(vtkRenderer *ren, vtkRenderWindowInteractor *iren, vtkMultiBlockDataSet* mb);
void renderGraphNoBox(vtkRenderer *ren, vtkRenderWindowInteractor *iren, vtkMultiBlockDataSet* mb);
void renderGraphExtract(vtkRenderer *ren, vtkRenderWindowInteractor *iren, vtkMultiBlockDataSet* mb, double bbox[]);
void renderGraphExtractSurface(vtkRenderer *ren, vtkRenderWindowInteractor *iren, vtkMultiBlockDataSet* mb, double bbox[]);
void renderGraphExtractPolys(vtkRenderer *ren, vtkRenderWindowInteractor *iren, vtkMultiBlockDataSet* mb, double bbox[]);
void renderGraphAddOutline(vtkRenderer *ren, vtkMultiBlockDataSet* mb);

int main(int argc, char* argv[])
{
  vtkCompositeDataPipeline* exec = vtkCompositeDataPipeline::New();
  vtkAlgorithm::SetDefaultExecutivePrototype(exec);
  exec->Delete();

  // Standard rendering classes
  vtkRenderer *ren = vtkRenderer::New();
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  renWin->AddRenderer(ren);
  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();

	iren->SetRenderWindow(renWin);
	ren->SetBackground(1,1,1);
	renWin->SetSize(800,600);

  // We will read three files and collect them together in one
  // multi-block dataset. I broke the combustor dataset into
  // three pieces and wrote them out separately.
  int i;
  vtkXMLStructuredGridReader* reader = vtkXMLStructuredGridReader::New();

  // vtkMultiBlockDataSet respresents multi-block datasets. See
  // the class documentation for more information.
  vtkMultiBlockDataSet* mb = vtkMultiBlockDataSet::New();
  double bbox[6];
  bbox[0] = 999999;
  bbox[1] = -999999;
  bbox[2] = 999999;
  bbox[3] = -999999;
  bbox[4] = 999999;
  bbox[5] = -999999;

 // m_bbox.expandBy(osg::Vec3d(box[0], box[2], box[4])); // min
//	m_bbox.expandBy(osg::Vec3d(box[1], box[3], box[5])); // max

  for (i=0; i<3; i++)
    {
    // Here we load the three separate files (each containing
    // a structured grid dataset)
    vtksys_ios::ostringstream fname;
    fname << "mb/mb_" << i << ".vts" << ends;
    char* cfname =
      vtkTestUtilities::ExpandDataFileName(argc, argv, fname.str().c_str());
    reader->SetFileName(cfname);
    // We have to update since we are working without a VTK pipeline.
    // This will read the file and the output of the reader will be
    // a valid structured grid data.
    reader->Update();
    delete[] cfname;

    // We create a copy to avoid adding the same data three
    // times (the output object of the reader does not change
    // when the filename changes)
    vtkStructuredGrid* sg = vtkStructuredGrid::New();
    sg->ShallowCopy(reader->GetOutput());

	double box[6];

	sg->GetBounds(box);

	// update bounding box;
	bool min = true;
	for (int j=0; j<6; j++)
	{
		if (min)
		{
			if (box[j] < bbox[j]) bbox[j] = box[j]; 
		}
		else
		{
			if (box[j] > bbox[j]) bbox[j] = box[j]; 
		}

		min = !min;
	}


    // Add the structured grid to the multi-block dataset
    mb->SetBlock(i, sg);
    sg->Delete();
    }
  reader->Delete();

  double boxsm[6];
  double boxleft[6];

  boxsm[0] = bbox[0];
  boxsm[2] = bbox[2];
  boxsm[4] = bbox[4];

  boxsm[1] = bbox[0] + (bbox[1] - bbox[0])/2.0;
  boxsm[3] = bbox[2] + (bbox[3] - bbox[2])/2.0;
  boxsm[5] = bbox[4] + (bbox[5] - bbox[4])/2.0;

  for (int i=0; i<6; i++)
  {
	boxleft[i] = bbox[i];
  }
  boxleft[1] = bbox[0] + (bbox[1] - bbox[0])/2.0;

  

	/* extraction types
		vtkExtractVOI
		vtkExtractGrid
		vtkExtractUnstructuredGrid
		vtkExtractGeometry - A more efficient version of this filter is available for vtkPolyData input. See..
		vtkExtractPolyDataGeometry
	*/



  renderGraphExtractPolys(ren, iren, mb, boxleft);
  //renderGraphExtractSurface(ren, iren, mb, boxleft);
  //renderGraphExtract(ren, iren, mb, boxleft);
  //renderGraphBase(ren, iren, mb);

 

  vtkAlgorithm::SetDefaultExecutivePrototype(0);
  ren->Delete();
  renWin->Delete();
  iren->Delete();
  mb->Delete();

  return 0;
}

//===========================================================================
//===========================================================================
void renderGraphExtractVectorSlice(vtkRenderer *ren, vtkRenderWindowInteractor *iren, vtkMultiBlockDataSet* mb, double bbox[])
{
	renderGraphAddOutline(ren, mb);

	//TODO:
}

//===========================================================================
//===========================================================================
void renderGraphExtractSurface(vtkRenderer *ren, vtkRenderWindowInteractor *iren, vtkMultiBlockDataSet* mb, double bbox[])
{
	renderGraphAddOutline(ren, mb);

	vtkSmartPointer<vtkBox> boxExtract = vtkSmartPointer<vtkBox>::New();
	boxExtract->SetBounds(bbox);

	vtkSmartPointer<vtkExtractGeometry> extract = vtkSmartPointer<vtkExtractGeometry>::New();
	extract->SetImplicitFunction(boxExtract);
	extract->SetExtractBoundaryCells(1);
	extract->SetInput(mb);
	

	vtkSmartPointer<vtkCellDataToPointData> c2p = vtkSmartPointer<vtkCellDataToPointData>::New();
	c2p->SetInputConnection(0, extract->GetOutputPort(0));

	// generates on output isosurfaces
	vtkSmartPointer<vtkContourFilter> contour =  vtkSmartPointer<vtkContourFilter>::New();
	contour->SetInputConnection(0, c2p->GetOutputPort(0));
	contour->SetValue(0, 0.45);

	// geometry filter
	vtkSmartPointer<vtkCompositeDataGeometryFilter> geom2 = vtkSmartPointer<vtkCompositeDataGeometryFilter>::New();
	geom2->SetInputConnection(0, contour->GetOutputPort(0));

	vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
	normals->SetInputConnection( geom2->GetOutputPort( 0 ) );
	normals->Update();

	// Rendering objects
	vtkSmartPointer<vtkPolyDataMapper> contMapper =  vtkSmartPointer<vtkPolyDataMapper>::New();
	contMapper->SetInputConnection(0, normals->GetOutputPort(0));

	contMapper->GetInput();

	vtkSmartPointer<vtkActor> contActor =  vtkSmartPointer<vtkActor>::New();
	contActor->SetMapper(contMapper);
	contActor->GetProperty()->SetColor(1, 0, 0);
	ren->AddActor(contActor);

	iren->Initialize();
	iren->Start();
}

//===========================================================================
//===========================================================================
void renderGraphExtractPolys(vtkRenderer *ren, vtkRenderWindowInteractor *iren, vtkMultiBlockDataSet* mb, double bbox[])
{
	renderGraphAddOutline(ren, mb);

	/*
	vtkSmartPointer<vtkBox> boxExtract = vtkSmartPointer<vtkBox>::New();
	boxExtract->SetBounds(bbox);

	
	vtkSmartPointer<vtkExtractGeometry> extract = vtkSmartPointer<vtkExtractGeometry>::New();
	extract->SetImplicitFunction(boxExtract);
	extract->SetExtractBoundaryCells(1);
	extract->SetInput(mb);
	*/
	

	vtkSmartPointer<vtkCellDataToPointData> c2p = vtkSmartPointer<vtkCellDataToPointData>::New();
	c2p->SetInput(mb);

	// generates on output isosurfaces
	vtkSmartPointer<vtkContourFilter> contour =  vtkSmartPointer<vtkContourFilter>::New();
	contour->SetInputConnection(0, c2p->GetOutputPort(0));
	contour->SetValue(0, 0.45);

	// geometry filter
	vtkSmartPointer<vtkCompositeDataGeometryFilter> geom2 = vtkSmartPointer<vtkCompositeDataGeometryFilter>::New();
	geom2->SetInputConnection(0, contour->GetOutputPort(0));

	vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
	normals->SetInputConnection( geom2->GetOutputPort( 0 ) );
	normals->Update();

	// extract
	vtkSmartPointer<vtkBox> boxExtract = vtkSmartPointer<vtkBox>::New();
	boxExtract->SetBounds(bbox);

	vtkSmartPointer<vtkExtractPolyDataGeometry> extract = vtkSmartPointer<vtkExtractPolyDataGeometry>::New();
	extract->SetImplicitFunction(boxExtract);
	extract->SetExtractBoundaryCells(1);
	extract->SetInputConnection(0, normals->GetOutputPort(0));

	// Rendering objects
	vtkSmartPointer<vtkPolyDataMapper> contMapper =  vtkSmartPointer<vtkPolyDataMapper>::New();
	contMapper->SetInputConnection(0, extract->GetOutputPort(0));

	contMapper->GetInput();

	vtkSmartPointer<vtkActor> contActor =  vtkSmartPointer<vtkActor>::New();
	contActor->SetMapper(contMapper);
	contActor->GetProperty()->SetColor(1, 0, 0);
	ren->AddActor(contActor);

	iren->Initialize();
	iren->Start();
}

void ExtractVTKPrimitives(vtkPolyData *m_pd, std::vector<double> *pBox)
{
    m_pd->Update();

    vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
    triangleFilter->SetInput( m_pd );
    triangleFilter->PassVertsOff();
    triangleFilter->PassLinesOff();
    //triangleFilter->Update();

    vtkSmartPointer<vtkStripper> triangleStripper = vtkSmartPointer<vtkStripper>::New();  
    triangleStripper->SetInput( triangleFilter->GetOutput() );
    int stripLength = triangleStripper->GetMaximumLength();
    triangleStripper->SetMaximumLength( stripLength * 1000 );
    //triangleStripper->Update();

    vtkSmartPointer<vtkPolyDataNormals> normalGen = vtkSmartPointer<vtkPolyDataNormals>::New();
    normalGen->SetInput( triangleStripper->GetOutput() );
    normalGen->NonManifoldTraversalOn();
    normalGen->AutoOrientNormalsOn();
    normalGen->ConsistencyOn();
    normalGen->SplittingOn();
 
    vtkSmartPointer<vtkStripper> reTriangleStripper = vtkSmartPointer<vtkStripper>::New();
    reTriangleStripper->SetInput( normalGen->GetOutput() );
    reTriangleStripper->SetMaximumLength( stripLength * 1000 );
    reTriangleStripper->Update();


    vtkPolyData* pd = reTriangleStripper->GetOutput();
	vtkCellArray* pcells = pd->GetStrips();
	/*
    { 
        VTKPrimitiveSetGeneratorPtr primitiveGenerator =
            VTKPrimitiveSetGeneratorPtr( new VTKPrimitiveSetGenerator( pd->GetStrips() ) );
        setPrimitiveSetGenerator( primitiveGenerator );
    }
	*/
}

//===========================================================================
//===========================================================================
void renderGraphExtract(vtkRenderer *ren, vtkRenderWindowInteractor *iren, vtkMultiBlockDataSet* mb, double bbox[])
{
	/*
	// create half sized block
	vtkSmartPointer<vtkBox> boxExtract = vtkSmartPointer<vtkBox>::New();
	boxExtract->SetBounds(boxsm);

	vtkSmartPointer<vtkExtractGeometry> extractGeom = vtkSmartPointer<vtkExtractGeometry>::New();
	extractGeom->SetImplicitFunction(boxExtract);

	vtkSmartPointer<vtkExtractUnstructuredGrid> extractGrid =  vtkSmartPointer<vtkExtractUnstructuredGrid>::New();
	extractGrid->SetExtent(boxsm);

	//vtkSmartPointer<vtkStructuredGridGeometryFilter> extractGridStruct =  vtkSmartPointer<vtkStructuredGridGeometryFilter>::New();
	//extractGridStruct->SetExtent(boxsm);

	vtkSmartPointer<vtkExtractPolyDataGeometry> extractPoly =  vtkSmartPointer<vtkExtractPolyDataGeometry>::New();
	extractPoly->SetImplicitFunction(boxExtract);

	vtkSmartPointer<vtkCellDataToPointData> c2p = vtkSmartPointer<vtkCellDataToPointData>::New();
	c2p->SetInput(mb);
	//c2p->SetInputData(mb);
	*/

	renderGraphAddOutline(ren, mb);


	vtkSmartPointer<vtkBox> boxExtract = vtkSmartPointer<vtkBox>::New();
	boxExtract->SetBounds(bbox);

	
	vtkSmartPointer<vtkExtractGeometry> extract = vtkSmartPointer<vtkExtractGeometry>::New();
	extract->SetImplicitFunction(boxExtract);
	extract->SetExtractBoundaryCells(1);
	extract->SetInput(mb);
	

	/*
	// doesn't workd
	int points[6];
	points[0] = 0;
	points[1] = 100;
	points[2] = 0;
	points[3] = 100;
	points[4] = 0;
	points[5] = 100;
	vtkSmartPointer<vtkExtractVOI> extract = vtkSmartPointer<vtkExtractVOI>::New();
	extract->SetVOI(points);
	extract->SetInput(mb);
	*/
	

	/*
	// doesn't work
	int points[6];
	points[0] = 0;
	points[1] = 100;
	points[2] = 0;
	points[3] = 100;
	points[4] = 0;
	points[5] = 100;
	vtkSmartPointer<vtkStructuredGridGeometryFilter> extract =  vtkSmartPointer<vtkStructuredGridGeometryFilter>::New();
	extract->SetExtent(points);
	extract->SetInput(mb);
	*/

	/*
	// doesn't work
	vtkSmartPointer<vtkExtractUnstructuredGrid> extract =  vtkSmartPointer<vtkExtractUnstructuredGrid>::New();
	extract->SetExtent(bbox);
	extract->SetInput(mb);
	*/

	/*
	// doesn't work
	vtkSmartPointer<vtkExtractPolyDataGeometry> extract = vtkSmartPointer<vtkExtractPolyDataGeometry>::New();
	extract->SetImplicitFunction(boxExtract);
	extract->SetInput(mb);
	*/

	// generates on output isosurfaces
	vtkSmartPointer<vtkContourFilter> contour =  vtkSmartPointer<vtkContourFilter>::New();
	contour->SetInputConnection(0, extract->GetOutputPort(0));
	contour->SetValue(0, 0.45);

	// geometry filter
	vtkSmartPointer<vtkCompositeDataGeometryFilter> geom2 = vtkSmartPointer<vtkCompositeDataGeometryFilter>::New();
	geom2->SetInputConnection(0, contour->GetOutputPort(0));

	// Rendering objects
	vtkSmartPointer<vtkPolyDataMapper> contMapper =  vtkSmartPointer<vtkPolyDataMapper>::New();
	contMapper->SetInputConnection(0, geom2->GetOutputPort(0));

	vtkSmartPointer<vtkActor> contActor =  vtkSmartPointer<vtkActor>::New();
	contActor->SetMapper(contMapper);
	contActor->GetProperty()->SetColor(1, 0, 0);
	ren->AddActor(contActor);

	iren->Initialize();
	iren->Start();
}

//===========================================================================
//===========================================================================
void renderGraphBase(vtkRenderer *ren, vtkRenderWindowInteractor *iren, vtkMultiBlockDataSet* mb)
{
	renderGraphAddOutline(ren, mb);

	// cell 2 point and contour
	vtkSmartPointer<vtkCellDataToPointData> c2p = vtkSmartPointer<vtkCellDataToPointData>::New();
	c2p->SetInput(mb);
	//c2p->SetInputData(mb);

	vtkSmartPointer<vtkContourFilter> contour =  vtkSmartPointer<vtkContourFilter>::New();
	contour->SetInputConnection(0, c2p->GetOutputPort(0));
	contour->SetValue(0, 0.45);

	// geometry filter
	vtkSmartPointer<vtkCompositeDataGeometryFilter> geom2 = vtkSmartPointer<vtkCompositeDataGeometryFilter>::New();
	geom2->SetInputConnection(0, contour->GetOutputPort(0));

	// Rendering objects
	vtkSmartPointer<vtkPolyDataMapper> contMapper =  vtkSmartPointer<vtkPolyDataMapper>::New();
	contMapper->SetInputConnection(0, geom2->GetOutputPort(0));

	vtkSmartPointer<vtkActor> contActor =  vtkSmartPointer<vtkActor>::New();
	contActor->SetMapper(contMapper);
	contActor->GetProperty()->SetColor(1, 0, 0);
	ren->AddActor(contActor);

	iren->Initialize();
	iren->Start();
}

//===========================================================================
//===========================================================================
void renderGraphNoBox(vtkRenderer *ren, vtkRenderWindowInteractor *iren, vtkMultiBlockDataSet* mb)
{
	vtkSmartPointer<vtkCellDataToPointData> c2p = vtkSmartPointer<vtkCellDataToPointData>::New();
	c2p->SetInput(mb);
	//c2p->SetInputData(mb);

	vtkSmartPointer<vtkContourFilter> contour =  vtkSmartPointer<vtkContourFilter>::New();
	contour->SetInputConnection(0, c2p->GetOutputPort(0));
	contour->SetValue(0, 0.45);

	// geometry filter
	vtkSmartPointer<vtkCompositeDataGeometryFilter> geom2 = vtkSmartPointer<vtkCompositeDataGeometryFilter>::New();
	geom2->SetInputConnection(0, contour->GetOutputPort(0));

	// Rendering objects
	vtkSmartPointer<vtkPolyDataMapper> contMapper =  vtkSmartPointer<vtkPolyDataMapper>::New();
	contMapper->SetInputConnection(0, geom2->GetOutputPort(0));

	vtkSmartPointer<vtkActor> contActor =  vtkSmartPointer<vtkActor>::New();
	contActor->SetMapper(contMapper);
	contActor->GetProperty()->SetColor(1, 0, 0);
	ren->AddActor(contActor);

	iren->Initialize();
	iren->Start();
}

//===========================================================================
//===========================================================================
void renderGraphAddOutline(vtkRenderer *ren, vtkMultiBlockDataSet* mb)
{
	// Multi-block can be processed with regular VTK filters in two ways:
	// 1. Pass through a multi-block aware consumer. Since a multi-block
	//    aware mapper is not yet available, vtkCompositeDataGeometryFilter
	//    can be used
	// 2. Assign the composite executive (vtkCompositeDataPipeline) to
	//    all "simple" (that work only on simple, non-composite datasets) filters

	// outline
	vtkSmartPointer<vtkStructuredGridOutlineFilter> of = vtkSmartPointer<vtkStructuredGridOutlineFilter>::New();
	of->SetInput(mb);

	// geometry filter
	// This filter is multi-block aware and will request blocks from the
	// input. These blocks will be processed by simple processes as if they
	// are the whole dataset

	vtkSmartPointer<vtkCompositeDataGeometryFilter> geom1 = vtkSmartPointer<vtkCompositeDataGeometryFilter>::New();
	geom1->SetInputConnection(0, of->GetOutputPort(0));

	// Rendering objects
	vtkSmartPointer<vtkPolyDataMapper> geoMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	//geoMapper->SetInputConnection(0, geom1->GetOutputPort(0));
	geoMapper->SetInputConnection(0, geom1->GetOutputPort(0));

	vtkSmartPointer<vtkActor> geoActor = vtkSmartPointer<vtkActor>::New();
	geoActor->SetMapper(geoMapper);
	geoActor->GetProperty()->SetColor(0, 0, 0);
	ren->AddActor(geoActor);
}