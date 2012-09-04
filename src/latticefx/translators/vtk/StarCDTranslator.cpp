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
#include <latticefx/translators/vtk/StarCDTranslator.h>
#include <latticefx/translators/vtk/starReader.h>
#include <vtkDataSet.h>
#include <vtkUnstructuredGrid.h>

#include <iostream>

using namespace lfx::vtk_translator;
////////////////////////////////////////
//Constructors                        //
////////////////////////////////////////
StarCDTranslator::StarCDTranslator()
{

    SetTranslateCallback( &starToVTK );
    SetPreTranslateCallback( &_cmdParser );
}
/////////////////////////////////////////
StarCDTranslator::~StarCDTranslator()
{}
//////////////////////////////////////////////////////////////////////////
void StarCDTranslator::StarCDPreTranslateCbk::Preprocess( int argc, char** argv,
        cfdTranslatorToVTK* toVTK )
{
    PreTranslateCallback::Preprocess( argc, argv, toVTK );
}
////////////////////////////////////////////////////////////////////////////////
void StarCDTranslator::StarCDTranslateCbk::Translate( vtkDataObject*& outputDataset,
        cfdTranslatorToVTK* toVTK,
        vtkAlgorithm*& )
{
    StarCDTranslator* starCDToVTK =
        dynamic_cast<StarCDTranslator*>( toVTK );
    if( starCDToVTK )
    {
        starReader* star = new starReader( starCDToVTK->GetFile( 0 ).c_str() );
        star->ReadParameterFile();

        if( !outputDataset )
        {
            outputDataset = vtkUnstructuredGrid::New();
        }
        outputDataset->ShallowCopy( star->GetUnsGrid() );
        delete star;
        outputDataset->Update();
    }
}
////////////////////////////////////////////////////////////////////////////////
void StarCDTranslator::DisplayHelp( void )
{
    std::cout << "|\tStarCD Translator Usage:" << std::endl
              << "\t -singleFile <filename_to_load> -o <output_dir> "
              << "-outFileName <output_filename> -loader star -w file" << std::endl;
}

