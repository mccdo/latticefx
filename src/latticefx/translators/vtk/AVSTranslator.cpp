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
#include <latticefx/translators/vtk/AVSTranslator.h>
#include <vtkDataSet.h>
#include <vtkAVSucdReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellDataToPointData.h>
#include <vtkPointData.h>

#include <iostream>

using namespace lfx::vtk_translator;
////////////////////////////////////////
//Constructors                        //
////////////////////////////////////////
AVSTranslator::AVSTranslator()
{

    SetTranslateCallback( &_AVSToVTK );
    SetPreTranslateCallback( &_cmdParser );
}
/////////////////////////////////////////
AVSTranslator::~AVSTranslator()
{}
//////////////////////////////////////////////////////////////////////////
void AVSTranslator::AVSPreTranslateCbk::Preprocess( int argc, char** argv,
        cfdTranslatorToVTK* toVTK )
{
    PreTranslateCallback::Preprocess( argc, argv, toVTK );
}
////////////////////////////////////////////////////////////////////////////////
void AVSTranslator::AVSTranslateCbk::Translate( vtkDataObject*& outputDataset,
        cfdTranslatorToVTK* toVTK,
        vtkAlgorithm*& )
{
    AVSTranslator* AVSToVTK =
        dynamic_cast<AVSTranslator*>( toVTK );
    if( AVSToVTK )
    {
        vtkAVSucdReader* avsReader = vtkAVSucdReader::New();
        avsReader->SetFileName( AVSToVTK->GetFile( 0 ).c_str() );
        avsReader->Update();
        if( !outputDataset )
        {
            outputDataset = vtkUnstructuredGrid::New();
        }
        vtkDataSet* tmpDSet = vtkUnstructuredGrid::New();
        tmpDSet->ShallowCopy( avsReader->GetOutput() );

        //get the info about the data in the data set
        /*unsigned int nPtDataArrays = tmpDSet->GetPointData()->GetNumberOfArrays();
        if( !nPtDataArrays )
        {
            std::cout << "Warning!!!" << std::endl;
            std::cout << "No point data found!" << std::endl;
            std::cout << "Attempting to convert cell data to point data." << std::endl;

            vtkCellDataToPointData* dataConvertCellToPoint = vtkCellDataToPointData::New();

            dataConvertCellToPoint->SetInput( tmpDSet );
            dataConvertCellToPoint->PassCellDataOff();
            dataConvertCellToPoint->Update();
            outputDataset->DeepCopy( dataConvertCellToPoint->GetOutput() );
            outputDataset->Update();
            return;
        }
        else*/
        {
            outputDataset->ShallowCopy( tmpDSet );
            outputDataset->Update();
        }
        avsReader->Delete();
        tmpDSet->Delete();
    }
}
////////////////////////////////////////////////////////////////////////////////
void AVSTranslator::DisplayHelp( void )
{
    std::cout << "|\tAVS Translator Usage:" << std::endl
              << "\t -singleFile <filename_to_load> -o <output_dir> "
              << "-outFileName <output_filename> -loader avs -w file" << std::endl;
}
