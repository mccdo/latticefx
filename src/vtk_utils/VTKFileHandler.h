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
#ifndef CFD_VTK_FILE_HANDLER_H
#define CFD_VTK_FILE_HANDLER_H
/*!\file cfdVTKFileHandler.h
cfdVTKFileHandler API
*/

/*!\class lfx::vtk_utils::VTKFileHandler
*
*/
class vtkXMLFileReadTester;
class vtkDataSet;
class vtkDataObject;
class vtkAlgorithm;
class vtkXMLReader;

#include <vtk_utils/Export.h>
#include <string>
#include <vector>

namespace lfx
{
namespace vtk_utils
{
class LATTICEFX_VTK_UTILS_EXPORT VTKFileHandler
{
public:
    ///Constructor
    VTKFileHandler();
    ///Copy Constructor
    ///\param fh Right hand side
    VTKFileHandler( const VTKFileHandler& fh );
    ///Destructor
    virtual ~VTKFileHandler();

    enum OutFileType
    {
        CFD_XML, VTK_CLASSIC
    };
    enum OutFileMode
    {
        CFD_ASCII = 0, CFD_BINARY
    };

    ///Set the input filename
    ///\param inFile The input filename
    void SetInputFileName( std::string inFile );
    ///Set the output filename
    ///\param oFile The output filename
    void SetOutputFileName( std::string oFile );
    ///Set the output file type. Default is CFD_XML
    void SetVTKOutFileType( OutFileType type );
    ///Set the output file mode. Default is CFD_BINARY
    void SetOutFileWriteMode( OutFileMode mode );
    ///Set scalars to read
    ///\param activeArrays The names of the vectors and scalars to activate
    void SetScalarsAndVectorsToRead( std::vector< std::string > activeArrays );

    ///\param vtkFileName The fileName of the vtkDataSet to read in.
    ///vtkDataSet* GetDataSetFromFile(std::string vtkFileName);

    ///Get the dataobject from the file
    ///\param vtkFileName The name of the file to read in.
    vtkDataObject* GetDataSetFromFile( const std::string& vtkFileName );
    ///Get the vtkAlgorithm for the reader being used
    //vtkAlgorithm* GetAlgorithm();
    std::vector< std::string > GetDataSetArraysFromFile( const std::string& vtkFileName );
    ///Write the DataObject to file
    ///\param dataObject The vtkDataObject to write
    ///\param outFileName The output filename.
    bool WriteDataSet( vtkDataObject* dataObject, std::string outFileName );

    ///Equal operator
    ///\param fh The right hand side
    VTKFileHandler& operator=( const VTKFileHandler& fh );
protected:
    ///Read XML UnstructredGrid data
    void _getXMLUGrid();
    ///Read XML StructuredGrid data
    void _getXMLSGrid();
    ///Read xML RectilinearGrid data
    void _getXMLRGrid();
    ///Read XML Polydata
    void  _getXMLPolyData();
    ///Read MultiGroup data
    ///\param isMultiBlock Determines if the data is MultiBlock or Hierachical
    void _getXMLMultiGroupDataSet( bool isMultiBlock = true );
    ///Reader function to open an vtkImageData file
    void GetXMLImageData( void );
    ///Reader for hierarchical data sets
    void GetXMLHierarchicalDataSet();
    ///Read old style(non-XML) vtk file
    void _readClassicVTKFile();
    ///Write old style(non-XML) vtk file
    void _writeClassicVTKFile( vtkDataObject * vtkThing,
                               std::string vtkFilename, int binaryFlag = 0 );
    ///Update the xml reader with the active arrays
    void UpdateReaderActiveArrays( vtkXMLReader* reader );

    OutFileType _outFileType;///<output XML or classic
    OutFileMode _outFileMode;///<output binary/ascii

    std::string _inFileName;///<input vtk file name
    std::string _outFileName;///<output vtk file name
    
    vtkXMLFileReadTester* _xmlTester;///<Test if file is XML format
    vtkDataObject* _dataSet;///<The vtk data.
    ///Hold a pointer to the raw reader
    vtkAlgorithm* mDataReader;
    ///THe scalars and vectors to activate
    std::vector< std::string > m_activeArrays;
};
}// end of util namesapce
}// end of xplorer namesapce
#endif// CFD_VTK_FILE_HANDLER_H
