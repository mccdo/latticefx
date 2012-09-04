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
#include <latticefx/translators/vtk/FluentTranslator.h>
#include <latticefx/utils/vtk/fileIO.h>

#include <vtkDataSet.h>
#include <vtkDataObject.h>
#include <vtkFLUENTReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkCompositeDataIterator.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>

#include <vtkXMLMultiBlockDataWriter.h>
#include <iostream>

using namespace lfx::vtk_translator;

FluentTranslator::FluentTranslator()
{

    SetTranslateCallback( &fluentToVTK );
    SetPreTranslateCallback( &cmdParser );
}

FluentTranslator::~FluentTranslator()
{}

void FluentTranslator::FluentPreTranslateCbk::Preprocess( int argc, char** argv,
        cfdTranslatorToVTK* toVTK )
{
    PreTranslateCallback::Preprocess( argc, argv, toVTK );
}

void FluentTranslator::FluentTranslateCbk::Translate( vtkDataObject*& outputDataset,
        cfdTranslatorToVTK* toVTK,
        vtkAlgorithm*& dataReader )
{
    FluentTranslator* FluentToVTK =
        dynamic_cast< FluentTranslator* >( toVTK );
    if( !FluentToVTK )
    {
        return;
    }

    //check and see if data file is present with cas file
    std::string casFile = FluentToVTK->GetFile( 0 );
    size_t period = casFile.rfind( "." );
    ///remove extension
    std::string casFileMinusExtension( casFile, 0, period );
    casFileMinusExtension.append( ".dat" );
    if( !lfx::vtk_utils::fileIO::isFileReadable( casFileMinusExtension ) )
    {
        std::cerr << "FLUENT dat file is not present with cas file."
                  << std::endl;
        return;
    }
    //now convert cas and dat file
    vtkFLUENTReader* reader = vtkFLUENTReader::New();
    reader->SetFileName( FluentToVTK->GetFile( 0 ).c_str() );
    reader->Update();
    dataReader = reader;
    if( !outputDataset )
    {
        outputDataset = vtkMultiBlockDataSet::New();
    }

    outputDataset->ShallowCopy( reader->GetOutput() );
    outputDataset->Update();
    reader->Delete();

    //vtkCompositeDataSet* mgd = outputDataset;
    vtkCompositeDataIterator* mgdIterator = vtkCompositeDataIterator::New();
    mgdIterator->SetDataSet( vtkCompositeDataSet::SafeDownCast( outputDataset ) );
    ///For traversal of nested multigroupdatasets
    mgdIterator->VisitOnlyLeavesOn();
    mgdIterator->GoToFirstItem();

    while( !mgdIterator->IsDoneWithTraversal() )
    {
        vtkDataSet* currentDataset =
            dynamic_cast<vtkDataSet*>( mgdIterator->GetCurrentDataObject() );
        CreateVectorFromScalar( currentDataset );
        mgdIterator->GoToNextItem();
    }
    mgdIterator->Delete();
    mgdIterator = 0;

    /*vtkXMLMultiBlockDataWriter* writer = vtkXMLMultiBlockDataWriter::New();
    writer->SetInput( outputDataset );
    writer->SetWriteMetaFile( 1 );
    writer->SetFileName( "test_multigrou.vtu" );
    writer->SetDataModeToAscii();
    std::cout << " write " << writer->Write() << std::endl;
    writer->Delete();*/
}

void FluentTranslator::DisplayHelp( void )
{
    std::cout << "|\tFluent Translator Usage:" << std::endl
              << "\t -singleFile <filename_to_load> -o <output_dir> "
              << "-outFileName <output_filename> -loader cas -w file" << std::endl;
}

void FluentTranslator::FluentTranslateCbk::CreateVectorFromScalar( vtkDataSet* dataSet )
{
    vtkCellData* cData = dataSet->GetCellData();
    if( !cData )
    {
        return;
    }

    vtkDataArray* xComp = cData->GetArray( "X_VELOCITY" );
    vtkDataArray* yComp = cData->GetArray( "Y_VELOCITY" );
    vtkDataArray* zComp = cData->GetArray( "Z_VELOCITY" );

    if( !xComp )
    {
        std::cout << "No X_VELOCITY scalar in FLUENT data." << std::endl;
    }

    if( !yComp )
    {
        std::cout << "No Y_VELOCITY scalar in FLUENT data." << std::endl;
    }

    if( !zComp )
    {
        std::cout << "No Z_VELOCITY scalar in FLUENT data." << std::endl;
    }

    vtkDoubleArray* velocityVec = vtkDoubleArray::New();
    velocityVec->SetNumberOfComponents( 3 );
    velocityVec->SetName( "Velocity" );
    double vec[ 3 ] = { 0, 0, 0 };
    vtkIdType size = xComp->GetSize();
    for( vtkIdType i = 0; i < size; ++i )
    {
        if( xComp )
        {
            vec[ 0 ] = xComp->GetTuple1( i );
        }

        if( yComp )
        {
            vec[ 1 ] = yComp->GetTuple1( i );
        }

        if( zComp )
        {
            vec[ 2 ] = zComp->GetTuple1( i );
        }

        velocityVec->InsertNextTupleValue( vec );
    }
    cData->AddArray( velocityVec );
    velocityVec->Delete();
}
