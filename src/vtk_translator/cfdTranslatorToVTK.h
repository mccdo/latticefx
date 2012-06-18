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
#ifndef _CFD_TRANSLATOR_TO_VTK_H_
#define _CFD_TRANSLATOR_TO_VTK_H_
#include <string>
#include <vector>


class vtkAlgorithm;
class vtkDataObject;
#include <vtk_translator/Export.h>
namespace lfx
{
namespace vtk_translator
{
class LATTICEFX_VTK_TRANSLATOR_EXPORT cfdTranslatorToVTK
{
public:
    cfdTranslatorToVTK();
    virtual ~cfdTranslatorToVTK();

    void SetFileExtension( std::string fileExtension );
    void SetNumberOfFoundFiles( unsigned int nFilesFound );
    void SetInputDirectory( std::string inDir );
    void SetOutputDirectory( std::string inDir );
    void SetFileName( std::string fileName );
    void SetBaseFileName( std::string baseFileName );
    void SetExtractGeometry( bool extractGeometry );
    bool GetExtractGeometry();
    void SetWriteOption( std::string writeOption );

    ///Function to list all the features of a respective translator
    ///when the -h option is specified
    virtual void DisplayHelp( void ) = 0;

    ///Utility function to process command line args
    ///\param argc The number of command line args
    ///\param argv The arg values as chars
    ///\param optionFlag The option you are looking for
    ///\param optionArg The returned argument
    bool _extractOptionFromCmdLine( int argc, char** argv,
                                    std::string optionFlag,
                                    std::string& optionArg );
    /////////////////////////////////////////////////////
    //The idea for these callback classes is similar to//
    //the setup in OSG (www.openscenegraph.org)        //
    /////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////
    //Any preprocessing can be easily setup to happen here//
    ////////////////////////////////////////////////////////
    class LATTICEFX_VTK_TRANSLATOR_EXPORT PreTranslateCallback
    {
    public:
        PreTranslateCallback()
        {};
        virtual ~PreTranslateCallback()
        {};
        virtual void Preprocess( int argc, char** argv,
                                 cfdTranslatorToVTK* toVTK );
    };

    /////////////////////////////////////////////////////////
    //translate callback must be defined or nothing happens//
    /////////////////////////////////////////////////////////
    class LATTICEFX_VTK_TRANSLATOR_EXPORT TranslateCallback
    {
    public:
        TranslateCallback()
        {};
        virtual ~TranslateCallback()
        {};
        //////////////////////////////////////////////////
        //ouputDataset should be populated              //
        //appropriately by the translate callback.      //
        //////////////////////////////////////////////////
        virtual void Translate( vtkDataObject*& outputDataset,
                                cfdTranslatorToVTK* toVTK,
                                vtkAlgorithm*& dataReader ) = 0;
    protected:
    };
    ///////////////////////////////////////////////////////////
    //Any post-processing can be easily setup to happen here //
    ///////////////////////////////////////////////////////////
    class LATTICEFX_VTK_TRANSLATOR_EXPORT PostTranslateCallback
    {
    public:
        PostTranslateCallback()
        {};
        virtual ~PostTranslateCallback()
        {};
        virtual void PostProcess( cfdTranslatorToVTK* toVTK ) = 0;
    protected:
    };

    ///Set all the callbacks for the required translator
    ///\param preTCbk Callback class
    void SetPreTranslateCallback( PreTranslateCallback* preTCbk );
    ///\param postTCbk Callback class
    void SetPostTranslateCallback( PostTranslateCallback* postTCbk );
    ///\param tCbk Callback class
    void SetTranslateCallback( TranslateCallback* tCbk );
    ///Get all the callbacks for the required translator
    PreTranslateCallback* GetPreTranslateCallback( void );
    PostTranslateCallback* GetPostTranslateCallback( void );
    TranslateCallback* GetTranslateCallback( void );

    void AddFoundFile( std::string singleFile );
    void AddBaseName( std::string baseName );
    ///Get the basename of the file used for setting the output filename
    std::string GetBaseName( unsigned int whichFile = 0 );
    ///Extract the basename for a file path
    void ExtractBaseName( std::string fileName );
    /////////////////////////////////////////
    //main translation calling method      //
    //Basically makes the following calls: //
    //PreTranslateCallback::Preprocess();  //
    //TranslateCallback::Translate();      //
    //PostTranslateCallback::PostProcess();//
    //_writeVTKFile();                      //
    /////////////////////////////////////////
    bool TranslateToVTK( int argc, char** argv );

    std::string GetFileExtension();
    std::string GetInputDirectory();
    std::string GetOutputDirectory();
    std::string GetBaseFileName();
    std::string GetOutputFileName();
    std::string GetWriteOption();

    unsigned int GetNumberOfFoundFiles();
    std::string GetFile( unsigned int fileNumber );

    vtkDataObject* GetVTKFile( unsigned int whichFile );
    ///Get the reader for this translator
    vtkAlgorithm* GetVTKAlgorithm();
    ///
    void SetIsTransient();
    ///
    void SetScalarsAndVectorsToRead( std::vector< std::string > activeArrays );
    ///
    std::vector< std::string > GetActiveArrays();
protected:
    bool _writeToVTK( unsigned int whichFile );
    ///Write the file to memory so that it is accessible
    ///through other interfaces
    unsigned int _nFoundFiles;

    std::vector<std::string> baseFileNames;
    std::string _fileExtension;
    std::string _inputDir;
    std::string _outputDir;
    std::string _baseFileName;
    std::vector<std::string> _infileNames;

    std::vector<std::string> _outfileNames;
    std::string _outputFile;
    std::string m_writeOption;

    PreTranslateCallback* _preTCbk;
    PostTranslateCallback* _postTCbk;
    TranslateCallback* _translateCbk;

    vtkDataObject* _outputDataset;
    vtkAlgorithm* mVTKReader;
    bool isTransient;
    std::vector< std::string > m_activeArrays;
    bool m_extractGeometry;
};
}
}
#endif //_CFD_TRANSLATOR_TO_VTK_H_
