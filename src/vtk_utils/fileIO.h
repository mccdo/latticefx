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

#ifndef FILEIO_H
#define FILEIO_H
/*!\file fileIO.h
 * fileIO API
 * \class ves::xplorer::util::fileIO
 *
 */
#include <iosfwd>
#include <string>
#include <vector>
#include <ves/VEConfig.h>


namespace ves
{
namespace xplorer
{
namespace util
{
class VE_UTIL_EXPORTS fileIO
{
public:
    ///Constructor
    fileIO();
    ///Destructor
    ~fileIO();

    ///Checks file to ensure it is readable.
    ///\param filename Name of the file to be checked.
    static int isFileReadable( std::string const& filename );
    ///Makes sure that the file isn't write-protected (currently disabled).
    ///\param filename Name of the file to be checked.
    static int isFileWritable( std::string const& filename );
    ///Checks to ensure directory exists.
    ///\param dirName Name of the directory being verified.
    static int DirectoryExists( std::string const& dirName );
    ///Checks to ensure directory ins't write-protected.
    ///\param dirname Name of the directory being verified.
    static int isDirWritable( std::string const& dirname );
    ///Gets user input for directory to write to.
    static const std::string getWritableDir();
    ///Asks user to use default filename or specify a new one.
    static std::string getFilenameFromDefault( std::string, std::string );///<The file contents and default file name.
    ///Returns a file with the default file name for input.  If not readable, it asks for a valid file name.
    ///\param stringDescribingfileContents Contents of file being checked.
    ///\param defaultName Default name of file to be read.
    static std::string getReadableFileFromDefault(
        std::string stringDescribingfileContents,
        const std::string defaultName );
    ///Returns a file with the default file name for output.  If not writable, it asks for a valid file name.
    ///\param defaultName Default name of file to be written.
    static std::string getWritableFile( std::string defaultName );
    ///(????)This reads in file from different forms
    static int readNByteBlockFromFile( void *ptr, const unsigned int nByte,
                                       const unsigned int num, FILE *stream,
                                       const bool endian_flip = 1 );
    ///Sets up the file names by taking in command line arguments and using either those or default values.
    ///\param inFileName The name of the input file passed in.
    ///\param outFileName The name of the output file passed in.
    static void processCommandLineArgs( int argc, char *argv[], const std::string verb,
                                        std::string & inFileName, std::string & outFileName );
    ///Returns the file extension of the file passed in.
    ///\param filename The file passed into the function.
    static std::string getExtension( std::string filename );
    ///Reports how many floats were read before the end of file was reached.
    ///\param *inputFile pointer to the input file to be read
    static void readToFileEnd( FILE *inputFile );
    ///Removes trailing spaces from the passed in line.
    ///\param line The line to have spaces removed from.
    static void StripTrailingSpaces( std::string line );
    ///Removes leading spaces from the passed in line.
    ///\param line The line to have spaces removed from.
    static std::string StripLeadingSpaces( std::string line );
    //static int extractIntegerBeforeExtension( std::string filename );
    ///(????)Identifies if tag and value are StarCD vertex data.
    ///\param TagName parameter file tag name
    ///\param TagValue parameter file tag value
    static void IdentifyTagAssignValue( std::string TagName, std::string TagValue );
    ///Reads in the tag name and value from parameter file.
    ///\param textline The line to extract the tag name and value from.
    ///\param TagName parameter file tag name
    ///\param TagValue parameter file tag value
    static void getTagAndValue( std::string textline, std::string& TagName,  std::string& TagValue );
    ///Have user input an integer value between the minimum and the maximum values.
    ///\param min The minimum integer value allowed.
    ///\param max The maximum integer value allowed.
    static int getIntegerBetween( const int min, const int max );

    //static int ExtractIntegerFromString( std::string filename );
    //static int ExtractIntegerBeforeExtension( std::string filename );
    ///Returns a listing of the files with a certain extension in the directory queried.
    ///\param dir The directory queried.
    ///\param extension The file extension of the files searched for.
    static std::vector<std::string> GetFilesInDirectory( std::string dir, std::string extension );
    ///(????) Returns file name without extension.
    ///\param fileName File name to be manipulated.
    static std::string ExtractBaseFileNameFromFullPath( std::string fileName );
    ///(????)Returns the relative path from the full path with no leading or trainliing slashes or backslashes
    ///\param fullPath The path the relative path is being extracted from.
    static std::string ExtractRelativeDirectoryFromFullPath( std::string fullPath );
};
}// end of util namesapce
}// end of xplorer namesapce
}// end of ves namesapce
#endif
