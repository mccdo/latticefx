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
#include <latticefx/translators/vtk/STLTranslator.h>
#include <latticefx/utils/vtk/fileIO.h>

#include <vtkDataSet.h>
#include <vtkDataObject.h>
#include <vtkSTLReader.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>

#include <vtkXMLMultiBlockDataWriter.h>
#include <iostream>

using namespace lfx::vtk_translator;

STLTranslator::STLTranslator()
{

    SetTranslateCallback( &stlToVTK );
    SetPreTranslateCallback( &cmdParser );
}

STLTranslator::~STLTranslator()
{}

void STLTranslator::STLPreTranslateCbk::Preprocess( int argc, char** argv,
        cfdTranslatorToVTK* toVTK )
{
    PreTranslateCallback::Preprocess( argc, argv, toVTK );
}

void STLTranslator::STLTranslateCbk::Translate( vtkDataObject*& outputDataset,
        cfdTranslatorToVTK* toVTK,
        vtkAlgorithm*& dataReader )
{
    STLTranslator* stlToVTK =
        static_cast< STLTranslator* >( toVTK );
    if( !stlToVTK )
    {
        return;
    }

    //check and see if data file is present with cas file
    std::string casFile = stlToVTK->GetFile( 0 );
    //now convert cas and dat file
    vtkSTLReader* reader = vtkSTLReader::New();
    reader->SetFileName( casFile.c_str() );
    reader->Update();
    dataReader = reader;

    if( !outputDataset )
    {
        outputDataset = vtkPolyData::New();
    }

    outputDataset->ShallowCopy( reader->GetOutput() );
    outputDataset->Update();
    reader->Delete();
}

void STLTranslator::DisplayHelp( void )
{
    std::cout << "|\tSTL Translator Usage:" << std::endl
              << "\t -singleFile <filename_to_load> -o <output_dir> "
              << "-outFileName <output_filename> -loader stl -w file" << std::endl;
}
