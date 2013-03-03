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
#include <string>

#include <ves/xplorer/util/fileIO.h>
#include <ves/xplorer/util/readWriteVtkThings.h>
#include <ves/xplorer/util/cfdAccessoryFunctions.h>

#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetReader.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkCellLocator.h>
#include <vtkFloatArray.h>

using namespace ves::xplorer::util;

/* the CFD grid is much more dense than the PIV grid. 
CFD dataset used in testing ------------> CFX_bin_ss_04.vtk  numPoints = 468782   numCells = 231025
PIV dataset used in testing -------------> PIV_All_linear_combine.vtk   numPoints = 23528    numCells = 11524
*/
int main( int argc, char *argv[] )
{   
   if ( argc < 2 )
   {
      std::cout<<"============================="<<std::endl;
      std::cout<<"Please enter two input files for comparison"<<std::endl;      
      std::cout<<"Usage : compareScalars <CFD_Dataset.vtk> <PIV_Dataset.vtk>"<<std::endl;
      
      std::cout<<"============================="<<std::endl;
      exit ( -1 );
   }
   std::cout<<"CFD dataset :"<<argv[1]<<std::endl;
   std::cout<<"PIV dataset :"<<argv[2]<<std::endl;
   //read in the CFD and PIV datasets
   ///This will need to be changed to handle both vtkDataset and vtkMultigroupDataSet
   vtkDataSet * datasetCFD = dynamic_cast<vtkDataSet*>(readVtkThing( argv[1], 0 ));
   ///This will need to be changed to handle both vtkDataset and vtkMultigroupDataSet
   vtkDataSet * datasetPIV = dynamic_cast<vtkDataSet*>(readVtkThing( argv[2], 0 ));
   
   int numArraysCFD, numArraysPIV;   
   int numVerticesPIV;
   double closestPoint[3]; //closest point in the CFD dataset
   double point[3];  //point in the PIV dataset
   numArraysCFD = datasetCFD->GetPointData()->GetNumberOfArrays();
   numArraysPIV = datasetPIV->GetPointData()->GetNumberOfArrays();
   numVerticesPIV = datasetPIV->GetNumberOfPoints();
   std::cout<<"number of vertices in PIV dataset ="<<numVerticesPIV<<std::endl;
   
   std::cout<<"number of arrays in datasetCFD ="<<numArraysCFD<<std::endl;   
   std::cout<<"number of arrays in datasetCFD ="<<numArraysPIV<<std::endl;
   vtkUnstructuredGrid* uGrid;
   //create an unstructuredgrid and copy the PIV grid into it
   vtkDataSetReader* reader = vtkDataSetReader::New();
   reader->SetFileName( argv[ 2 ] );
   uGrid = reader->GetUnstructuredGridOutput();
   //now match up the grids approximately
   
   //first scale down the PIV dataset ans then translate it
   vtkTransform *aTransform = vtkTransform::New();
   aTransform->Translate( 0.0, 0.8, 0.0 );
   aTransform->Scale( 0.305, 0.305, 0.235 );
   vtkTransformFilter* transFilter = vtkTransformFilter::New();
   transFilter->SetInput( (vtkPointSet* ) datasetPIV );
   transFilter->SetTransform( aTransform );
   std::cout<<"writing out translated grid to tempUGrid"<<std::endl;
   
   //vtkUnstructuredGrid* tempUGrid;
   //tempUGrid = (vtkUnstructuredGrid*) (transFilter->GetOutput() );
   //writeVtkThing( tempUGrid, "transformed.vtk", 1 );
   //now grids are matched up approximately, do the differencing
   //all scalars that are equal to zero, differencing is not done
   vtkCellLocator* cLocator = vtkCellLocator::New();   
   cLocator->SetDataSet( datasetPIV ); //give it the PIV dataset
   cLocator->BuildLocator();
   //loop through the vertices of the PIV dataset
   vtkIdType cellId;
   int subId;
   double dist = 0.0;
   int countPoints; countPoints = 0;
   double absVelPIV, absVelCFD, diff;
   vtkFloatArray* scalarDiff = vtkFloatArray::New();
   scalarDiff->SetNumberOfComponents( 1 );
   scalarDiff->SetNumberOfTuples( datasetPIV->GetPointData()->GetNumberOfTuples() );
   scalarDiff->SetName("Scalar_Difference");
   for ( int i=0;i<numVerticesPIV;i++ ) //since the dataset has been made 3-D
   {
      //go into the PIV dataset and get the co-ordinates
      datasetPIV->GetPoint( i, point );
      //use that point to query the CFD dataset for x, y, z information
      cLocator->FindClosestPoint( point, closestPoint, cellId, subId, dist );
      //grab absolute velocity scalar values from CFD and PIV datasets
      datasetPIV->GetPointData()->SetActiveScalars("Absolute Velocity");
       datasetCFD->GetPointData()->SetActiveScalars("Velocity_magnitude");
       absVelPIV = datasetPIV->GetPointData()->GetArray( "Absolute Velocity" )->GetTuple1( countPoints );
       absVelCFD = datasetCFD->GetPointData()->GetArray( "Velocity_magnitude" )->GetTuple1( countPoints );
      diff = fabs( absVelPIV - absVelCFD );
      scalarDiff->SetComponent( countPoints, 0, diff );
      countPoints++;      
   }
   uGrid->GetPointData()->AddArray( scalarDiff );
   writeVtkThing( uGrid, "scalarDifference.vtk", 0 );
   std::cout<<"Count Points :"<<countPoints<<std::endl;
   //clean up
   scalarDiff->Delete(); scalarDiff = NULL;
   datasetCFD->Delete();
   datasetPIV->Delete();
   reader->Delete(); reader = NULL;
   aTransform->Delete(); aTransform = NULL;
   transFilter->Delete(); transFilter = NULL;
   cLocator->Delete(); cLocator = NULL;
   return 0;
}

