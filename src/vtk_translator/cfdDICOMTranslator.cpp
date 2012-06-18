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
#include <vtk_translator/cfdDICOMTranslator.h>
#include <vtkDataSet.h>
#include <vtkImageData.h>
#include <vtkDICOMImageReader.h>
#include <vtkStructuredGrid.h>
#include <vtkProbeFilter.h>
#include <vtkPoints.h>

#include <iostream>

using namespace lfx::vtk_translator;
////////////////////////////////////////
//Constructors                        //
////////////////////////////////////////
cfdDICOMTranslator::cfdDICOMTranslator()
{

    SetTranslateCallback( &_dicomToVTK );
    SetPreTranslateCallback( &_cmdParser );
}
/////////////////////////////////////////
cfdDICOMTranslator::~cfdDICOMTranslator()
{}
////////////////////////////////////////////////////////////////////////////////
void cfdDICOMTranslator::DICOMTranslateCbk::Translate( vtkDataObject*& outputDataset,
        cfdTranslatorToVTK* toVTK,
        vtkAlgorithm*& )
{
    cfdDICOMTranslator* dicomToVTK =
        dynamic_cast<cfdDICOMTranslator*>( toVTK );
    if( dicomToVTK )
    {
        vtkDICOMImageReader* dicomTranslator = vtkDICOMImageReader::New();
        dicomTranslator->SetDirectoryName( dicomToVTK->GetInputDirectory().c_str() );
        dicomTranslator->Update();
        if( !outputDataset )
        {
            outputDataset = vtkImageData::New();
        }
        outputDataset->ShallowCopy( dicomTranslator->GetOutput( 0 ) );
        outputDataset->Update();


        //      double delta[3] = {0,0,0};
        //      double origin[3] = {0,0,0};
        //     dicomTranslator->GetOutput()->GetOrigin(origin);
        //    dicomTranslator->GetOutput()->GetSpacing(delta);

        //   int dims[3];
        //  dicomTranslator->GetOutput()->GetDimensions(dims);
        /*     dims[0] = dicomTranslator->GetWidth();
             dims[1] = dicomTranslator->GetHeight();
             dims[2] = 16;//how do we get the number of files?dicomTranslator->GetNumberOfDICOMFileNames();
        */
        /* vtkProbeFilter* probeFilter = vtkProbeFilter::New();
         //probeFilter->DebugOn();

         if(!outputDataset){
            outputDataset = vtkStructuredGrid::New();
         }
         vtkStructuredGrid* tempGrid = vtkStructuredGrid::New();
         vtkPoints* tempPoints = vtkPoints::New();
         tempPoints->Allocate(dims[0]*dims[1]*dims[2]);

         unsigned int kOffset = 0;
         unsigned int jOffset = 0;
         double pt[3] = {0,0,0};
         for(int k=0; k < dims[2]; k++)
         {
            pt[2] = k*delta[2] + origin[2];
            kOffset = k * dims[0] * dims[1];
            for (int j=0; j<dims[1]; j++)
            {
               jOffset = j * dims[0];
               pt[1] = j*delta[1] + origin[1];

                for(int i=0; i<dims[0]; i++)
                {
                  pt[0] = i*delta[0] + origin[0];
                  tempPoints->InsertPoint((i + jOffset + kOffset),pt);
                }
            }
         }
         tempGrid->SetDimensions(dims);
         tempGrid->SetPoints(tempPoints);
         tempGrid->Update();

         probeFilter->SetInput(tempGrid);
         probeFilter->SetSource(dicomTranslator->GetOutput());
         probeFilter->Update();

         outputDataset->DeepCopy(probeFilter->GetStructuredGridOutput());
         outputDataset->Update();
         //outputDataset->Print(std::cerr);
         tempPoints->Delete();
         tempGrid->Delete();
         probeFilter->Delete();*/
        dicomTranslator->Delete();
    }
}
////////////////////////////////////////////////////////////////////////////////
void cfdDICOMTranslator::DisplayHelp( void )
{
    std::cout << "|\tDICOM Translator Usage:" << std::endl
              << "\t -i <input_directory> -o <output_dir> "
              << "-outFileName <output_filename> -loader dcm -w file" << std::endl;
}
