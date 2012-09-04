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
#include <latticefx/translators/vtk/AnsysTranslator.h>
#include <latticefx/translators/vtk/ansysReader.h>
#include <vtkDataSet.h>
#include <vtkUnstructuredGrid.h>

#include <iostream>

using namespace lfx::vtk_translator;
////////////////////////////////////////
//Constructors                        //
////////////////////////////////////////
AnsysTranslator::AnsysTranslator()
{

    SetTranslateCallback( &ansysToVTK );
    SetPreTranslateCallback( &_cmdParser );
}
/////////////////////////////////////////
AnsysTranslator::~AnsysTranslator()
{}
//////////////////////////////////////////////////////////////////////////
void AnsysTranslator::AnsysPreTranslateCbk::Preprocess( int argc, char** argv,
        cfdTranslatorToVTK* toVTK )
{
    PreTranslateCallback::Preprocess( argc, argv, toVTK );
}
////////////////////////////////////////////////////////////////////////////////
void AnsysTranslator::AnsysTranslateCbk::Translate( vtkDataObject*& outputDataset,
        cfdTranslatorToVTK* toVTK,
        vtkAlgorithm*& dataReader )
{
    AnsysTranslator* ansysTransVTK =
        dynamic_cast<AnsysTranslator*>( toVTK );
    if( ansysTransVTK )
    {
        ansysReader* ansys = new ansysReader( ansysTransVTK->GetFile( 0 ).c_str() );

        if( !outputDataset )
        {
            outputDataset = vtkUnstructuredGrid::New();
        }
        outputDataset->ShallowCopy( ansys->GetUGrid() );
        outputDataset->Update();
        delete ansys;
    }
}
////////////////////////////////////////////////////////////////////////////////
void AnsysTranslator::DisplayHelp( void )
{
    std::cout << "|\tAnsys Translator Usage:" << std::endl
              << "\t -singleFile <rst_filename_to_load> -o <output_dir> "
              << "-outFileName <output_filename> -loader rst -w file" << std::endl;
}

