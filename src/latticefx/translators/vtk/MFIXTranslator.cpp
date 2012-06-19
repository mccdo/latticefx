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
#include <vtk_translator/MFIXTranslator.h>
#include <vtkDataSet.h>
#include <vtkMFIXReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>

#include <iostream>

using namespace lfx::vtk_translator;
////////////////////////////////////////
//Constructors                        //
////////////////////////////////////////
MFIXTranslator::MFIXTranslator()
{

    SetTranslateCallback( &mfixToVTK );
    SetPreTranslateCallback( &cmdParser );
}
/////////////////////////////////////////
MFIXTranslator::~MFIXTranslator()
{}
//////////////////////////////////////////////////////////////////////////
void MFIXTranslator::MFIXPreTranslateCbk::Preprocess( int argc, char** argv,
        cfdTranslatorToVTK* toVTK )
{
    PreTranslateCallback::Preprocess( argc, argv, toVTK );
}
////////////////////////////////////////////////////////////////////////////////
void MFIXTranslator::MFIXTranslateCbk::Translate( vtkDataObject*& outputDataset,
        cfdTranslatorToVTK* toVTK,
        vtkAlgorithm*& )
{
    MFIXTranslator* MFIXToVTK =
        dynamic_cast< MFIXTranslator* >( toVTK );
    if( MFIXToVTK )
    {
        vtkMFIXReader* reader = vtkMFIXReader::New();
        reader->SetFileName( MFIXToVTK->GetFile( 0 ).c_str() );
        reader->Update();
        int TimeStepRange[2];
        reader->GetTimeStepRange( TimeStepRange );
        reader->SetTimeStep( TimeStepRange[0] );

        if( !outputDataset )
        {
            outputDataset = vtkUnstructuredGrid::New();
        }
        vtkDataSet* tmpDSet = vtkUnstructuredGrid::New();
        tmpDSet->DeepCopy( reader->GetOutput() );

        //get the info about the data in the data set
        /*if( tmpDSet->GetPointData()->GetNumberOfArrays() == 0 )
        {
            vtkCellDataToPointData* dataConvertCellToPoint = vtkCellDataToPointData::New();
            dataConvertCellToPoint->SetInput( tmpDSet );
            dataConvertCellToPoint->PassCellDataOff();
            dataConvertCellToPoint->Update();
            outputDataset->DeepCopy( dataConvertCellToPoint->GetOutput() );
            dataConvertCellToPoint->Delete();
        }
        else*/
        {
            outputDataset->DeepCopy( tmpDSet );
        }
        outputDataset->Update();
        reader->Delete();
        tmpDSet->Delete();
    }
}
////////////////////////////////////////////////////////////////////////////////
void MFIXTranslator::DisplayHelp( void )
{
    std::cout << "|MFIX Translator Usage:" << std::endl
              << "\t -singleFile <filename_to_load> -o <output_dir> "
              << "-outFileName <output_filename> -loader mfix -w file" << std::endl;
}
