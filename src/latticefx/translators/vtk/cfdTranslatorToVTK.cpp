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
#include <latticefx/translators/vtk/cfdTranslatorToVTK.h>
#include <bitset>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <vtkDataObject.h>
#include <vtkAlgorithm.h>
#include <latticefx/utils/vtk/readWriteVtkThings.h>
#include <latticefx/utils/vtk/fileIO.h>
using namespace lfx::vtk_translator;
////////////////////////////////////////
//Constructor                         //
////////////////////////////////////////
cfdTranslatorToVTK::cfdTranslatorToVTK()
    :
    _nFoundFiles( 0 ),
    _preTCbk( 0 ),
    _postTCbk( 0 ),
    _translateCbk( 0 ),
    _outputDataset( 0 ),
    mVTKReader( 0 ),
    isTransient( false ),
    m_extractGeometry( false )
{

    _baseFileName = "flowdata";
    _fileExtension = ".vtk";
    _inputDir = ".";
    _outputDir = ".";
    _outputFile = "vtkFile";
}
/////////////////////////////////////////
cfdTranslatorToVTK::~cfdTranslatorToVTK()
{
    if( _outputDataset )
    {
        _outputDataset->Delete();
    }

    _outfileNames.clear();
    _infileNames.clear();
}
////////////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::SetFileExtension( std::string fileExtension )
{
    _fileExtension = fileExtension;
}
/////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::SetInputDirectory( std::string inDir )
{
    baseFileNames.clear();
    _inputDir = inDir;
    _infileNames = lfx::vtk_utils::fileIO::GetFilesInDirectory( inDir, _fileExtension );
    for( size_t i = 0; i < _infileNames.size(); i++ )
    {
        ExtractBaseName( _infileNames.at( i ) );
    }
}
///////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::SetOutputDirectory( std::string outDir )
{
    _outputDir = outDir;
}
///////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::SetFileName( std::string fileName )
{
    _outputFile = fileName;
}
////////////////////////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::SetPreTranslateCallback( cfdTranslatorToVTK::PreTranslateCallback* preTCbk )
{
    _preTCbk = preTCbk;
}
////////////////////////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::SetPostTranslateCallback( cfdTranslatorToVTK::PostTranslateCallback* postTCbk )
{
    _postTCbk = postTCbk;
}
////////////////////////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::SetTranslateCallback( cfdTranslatorToVTK::TranslateCallback* tCbk )
{
    _translateCbk = tCbk;
}
////////////////////////////////////////////////////////////////////////////////
cfdTranslatorToVTK::PreTranslateCallback* cfdTranslatorToVTK::GetPreTranslateCallback( void )
{
    return _preTCbk;
}
////////////////////////////////////////////////////////////////////////////////
cfdTranslatorToVTK::PostTranslateCallback* cfdTranslatorToVTK::GetPostTranslateCallback( void )
{
    return _postTCbk;
}
////////////////////////////////////////////////////////////////////////////////
cfdTranslatorToVTK::TranslateCallback* cfdTranslatorToVTK::GetTranslateCallback( void )
{
    return _translateCbk;
}
////////////////////////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::SetNumberOfFoundFiles( unsigned int nFilesFound )
{
    _nFoundFiles = nFilesFound;
}
////////////////////////////////////////////////////////////////
std::string cfdTranslatorToVTK::GetFile( unsigned int fileNumber )
{
    return _infileNames.at( fileNumber );
}
//////////////////////////////////////////////////
std::string cfdTranslatorToVTK::GetFileExtension()
{
    return _fileExtension;
}
//////////////////////////////////////////////////
std::string cfdTranslatorToVTK::GetInputDirectory()
{
    return _inputDir;
}
////////////////////////////////////////////////////
std::string cfdTranslatorToVTK::GetOutputDirectory()
{
    return _outputDir;
}
////////////////////////////////////////////////////
std::string cfdTranslatorToVTK::GetBaseFileName()
{
    return _baseFileName;
}
////////////////////////////////////////////////////
std::string cfdTranslatorToVTK::GetOutputFileName()
{
    return _outputFile;
}
////////////////////////////////////////////////////////
unsigned int cfdTranslatorToVTK::GetNumberOfFoundFiles()
{
    return _nFoundFiles;
}
///////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::SetBaseFileName( std::string baseFileName )
{
    baseFileName.clear();
    AddBaseName( baseFileName );
}
///////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::SetIsTransient()
{
    isTransient = true;
}
////////////////////////////////////////////////////////////////////////////
bool cfdTranslatorToVTK::TranslateToVTK( int argc, char** argv )
{
    //this may be modified later to handle multiple files
    if( _preTCbk )
    {
        _preTCbk->Preprocess( argc, argv, this );
    }

    //just in case the Preprocess didn't set this
    if( !_nFoundFiles && _infileNames.size() )
    {
        _nFoundFiles = static_cast< int >( _infileNames.size() );
    }

    //so that the file name is correct for dicom data
    if( _nFoundFiles > 1 )
    {
        isTransient = true;
    }

    std::bitset< 16 > status;

    for( unsigned int i = 0; i < _nFoundFiles; ++i )
    {
        if( _translateCbk )
        {
            _translateCbk->Translate( _outputDataset, this, mVTKReader );
        }

        if( _postTCbk )
        {
            _postTCbk->PostProcess( this );
        }
        // Determine if we need to write the file or not
        std::string writeOption;
        if( _extractOptionFromCmdLine( argc, argv, std::string( "-w" ), writeOption ) )
        {
            if( writeOption == std::string( "file" ) )
            {
                status.set( i, _writeToVTK( i ) );
                // because we are done with it after this...
                _outputDataset->Delete();
                _outputDataset = 0;
            }
            else
            {
                status.set( i, true );
            }
        }
    }

    for( size_t i = 0; i < status.size(); ++i )
    {
        if( !status.test( i ) )
        {
            return false;
        }
    }
    return true;
}
//////////////////////////////////////////////////////////
bool cfdTranslatorToVTK::_writeToVTK( unsigned int fileNum )
{
    if( _outputDataset )
    {
        if( !isTransient )
        {
            std::stringstream tempName;
            tempName << _outputDir << "/"
                     << _outputFile << ".vtu";
            _outfileNames.push_back( tempName.str() );
        }
        else
        {
            std::stringstream tempName;
            tempName << _outputDir << "/"
                     << _outputFile << "_"
                     << std::setfill( '0' )
                     << std::setw( 6 )
                     << fileNum << ".vtu";
            _outfileNames.push_back( tempName.str() );
        }

        lfx::vtk_utils::writeVtkThing( _outputDataset,
                                           ( char* )_outfileNames.at( fileNum ).c_str(), 1 );
        return true;
    }
    else
    {
        std::cerr
                << "|\tcfdTranslatorToVTK::_writeToVTK : Invalid output vtk dataset"
                << std::endl;
        return false;
    }
}
////////////////////////////////////////////////////////////////////////////////////
vtkDataObject* cfdTranslatorToVTK::GetVTKFile( unsigned int )
{
    if( _outputDataset )
    {
        return _outputDataset;
    }
    else
    {
        std::cerr
                << "|\tcfdTranslatorToVTK::GetVTKFile : Invalid output vtk dataset."
                << std::endl;
        return 0;
    }
}
////////////////////////////////////////////////////////////////////////////////////
vtkAlgorithm* cfdTranslatorToVTK::GetVTKAlgorithm()
{
    return mVTKReader;
}
////////////////////////////////////////////////////////////////////////////////////
//This is the default behavior to look for the input and output directory from the//
//command line.                                                                   //
////////////////////////////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::PreTranslateCallback::Preprocess( int argc, char** argv,
        cfdTranslatorToVTK* toVTK )
{
    if( toVTK )
    {
        std::string inDir;
        std::string singleFile;

        if( toVTK->_extractOptionFromCmdLine( argc, argv, std::string( "-singleFile" ), singleFile ) )
        {
            toVTK->AddFoundFile( singleFile );
            toVTK->ExtractBaseName( singleFile );
        }
        else if( toVTK->_extractOptionFromCmdLine( argc, argv, std::string( "-i" ), inDir ) )
        {
            toVTK->SetInputDirectory( inDir );
            toVTK->SetNumberOfFoundFiles( 1 );
            toVTK->AddBaseName( "outdata" );
        }
        std::string outDir;
        if( toVTK->_extractOptionFromCmdLine( argc, argv, std::string( "-o" ), outDir ) )
        {
            toVTK->SetOutputDirectory( outDir );
        }
        std::string outFileName;
        if( toVTK->_extractOptionFromCmdLine( argc, argv, std::string( "-outFileName" ), outFileName ) )
        {
            toVTK->SetFileName( outFileName );
        }

        if( toVTK->_extractOptionFromCmdLine( argc, argv, std::string( "-geometry" ), outFileName ) )
        {
            toVTK->SetExtractGeometry( true );
        }

        std::string writeOption;
        if( toVTK->_extractOptionFromCmdLine( argc, argv, std::string( "-w" ), writeOption ) )
        {
            toVTK->SetWriteOption( writeOption );
        }
    }
}
//////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::ExtractBaseName( std::string fileName )
{
    size_t period = fileName.rfind( "." );
    ///remove extension
    std::string outfileMinusExtension( fileName, 0, period );
    ///remove leading slashes
#ifdef WIN32
    size_t backslash = outfileMinusExtension.rfind( "\\" );
    size_t frontslash = outfileMinusExtension.rfind( "/" );
    size_t slash = 0;
    if( backslash > outfileMinusExtension.size() )
    {
        backslash = 0;
    }
    if( frontslash > outfileMinusExtension.size() )
    {
        frontslash = 0;
    }
    slash = ( backslash > frontslash ) ? backslash : frontslash;
#else
    size_t slash = outfileMinusExtension.rfind( "/" );
#endif
    if( slash == 0 )
    {
        _baseFileName = std::string( outfileMinusExtension, 0, period );
    }
    else
    {
        _baseFileName = std::string( outfileMinusExtension, slash + 1, period );
    }

    AddBaseName( _baseFileName );
    SetFileName( _baseFileName );
}
////////////////////////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::AddBaseName( std::string baseName )
{
    baseFileNames.push_back( baseName );
}
////////////////////////////////////////////////////////////////////////////////
std::string cfdTranslatorToVTK::GetBaseName( unsigned int whichFile )
{
    return baseFileNames.at( whichFile );
}
////////////////////////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::AddFoundFile( std::string singleFile )
{
    _infileNames.push_back( singleFile );
    _nFoundFiles = static_cast< int >( _infileNames.size() );
}
////////////////////////////////////////////////////////////////////////////////
bool cfdTranslatorToVTK::_extractOptionFromCmdLine( int argc, char** argv,
        std::string optionFlag,
        std::string& optionArg )
{
    for( int i = 0; i < argc; i++ )
    {
        std::string curArg( argv[i] );
        if( curArg == optionFlag )
        {
            optionArg = std::string( argv[++i] );
            std::cout << "|\tFound option: " << optionFlag << ":" << optionArg << std::endl;

            return true;
        }
    }
    return false;
}
////////////////////////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::SetScalarsAndVectorsToRead( std::vector< std::string > activeArrays )
{
    m_activeArrays = activeArrays;
}
////////////////////////////////////////////////////////////////////////////////
std::vector< std::string > cfdTranslatorToVTK::GetActiveArrays()
{
    return m_activeArrays;
}
////////////////////////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::SetExtractGeometry( bool extractGeometry )
{
    m_extractGeometry = extractGeometry;
}
////////////////////////////////////////////////////////////////////////////////
bool cfdTranslatorToVTK::GetExtractGeometry()
{
    return m_extractGeometry;
}
////////////////////////////////////////////////////////////////////////////////
void cfdTranslatorToVTK::SetWriteOption( std::string writeOption )
{
    m_writeOption = writeOption;
}
////////////////////////////////////////////////////////////////////////////////
std::string cfdTranslatorToVTK::GetWriteOption()
{
    return m_writeOption;
}
////////////////////////////////////////////////////////////////////////////////
