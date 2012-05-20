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
#include <vtk_utils/fileIO.h>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/version.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <cstring>
namespace fs = boost::filesystem;

using namespace lfx::vtk_utils;
////////////////////////////////////////////////////////////////////////////////
fileIO::fileIO( )
{}
////////////////////////////////////////////////////////////////////////////////
fileIO::~fileIO( )
{}
////////////////////////////////////////////////////////////////////////////////
int fileIO::isFileReadable( std::string const& filename )
{
    std::ifstream fileIn( filename.c_str(), std::ios::in );
    if( !fileIn.good() )
    {
        return 0;
    }
    fileIn.close();
    return 1;
}
////////////////////////////////////////////////////////////////////////////////
int fileIO::isFileWritable( std::string const& )
{
    // This will not compile due to a bug in vapor.  Uncomment this when the
    // bug is resolved.
    /*
    vpr::FileHandle fh( std::string(filename) );
    fh.setBlocking(false);
    fh.setOpenWriteOnly();
    vpr::ReturnStatus status = fh.open();
    fh.close();
    return status.success();
    */
    return 1;
}
////////////////////////////////////////////////////////////////////////////////
int fileIO::DirectoryExists( std::string const& dirName )
{
    fs::path pathName( dirName );
    //fs::path pathName( dirName.c_str, fs::native );
    if( ! fs::exists( pathName ) )
    {
#if (BOOST_VERSION >= 104600) && (BOOST_FILESYSTEM_VERSION == 3)
        std::cout << "\nDirectory not found: "
        << pathName.string() << std::endl;
#else
        std::cout << "\nDirectory not found: "
        << pathName.native_file_string() << std::endl;
#endif
        return 0;
    }
    return 1;
}
////////////////////////////////////////////////////////////////////////////////
int fileIO::isDirWritable( std::string const& dirname )
{
    // create string of dirname and testing.txt file
    std::string fullDirName = dirname + std::string( "/" ) + std::string( "testing.txt" );

    // boost corrects for the "/"
    fs::path file_name( fullDirName );

    // test the file
    fs::ofstream test_file( file_name, std::ios_base::out | std::ios_base::app );
    test_file.close();
    bool result( false );
    if( fs::exists( file_name ) )
    {
        fs::remove( file_name );
        result = true;
    }
    return static_cast<int>( result );
}
////////////////////////////////////////////////////////////////////////////////
const std::string fileIO::getWritableDir()
{
    std::string dir_name;
    do
    {
        std::cout << "Input an existing writable directory: ";
        std::cout.flush();
        std::getline( std::cin, dir_name );
        if( isDirWritable( dir_name ) )
        {
            break;
        }
        std::cerr << "\nERROR: Can't write to \"" << dir_name
        << "\" directory" << std::endl;
    }
    while( 1 );
    return dir_name;
}
////////////////////////////////////////////////////////////////////////////////
std::string fileIO::getFilenameFromDefault( std::string fileContents, std::string defaultName )
{
    int answer = 1;

    // if supplied with a default name, ask if it is OK...
    if( ! defaultName.empty() )
    {
        std::cout << "\nThe default name for " << fileContents
        << " is \"" << defaultName << "\"" << std::endl;
        std::cout << "Do you want to change it ? (0) Use default (1) Change"
        << std::endl;
        answer = getIntegerBetween( 0, 1 );
    }

    std::string fileName;
    if( answer == 1 )
    {
        std::cout << "Enter filename for " << fileContents << ": ";
        std::cin >> fileName;
        std::cin.ignore();
    }
    else
    {
        fileName = defaultName;
    }

    return fileName;
}

std::string fileIO::getReadableFileFromDefault( std::string stringDescribingfileContents,
                                                const std::string defaultName )
{
    // initialize return name with default name, and if not readable set to null
    std::string validDefaultName;
    if( ! isFileReadable( defaultName ) )
        validDefaultName = "";
    else
        validDefaultName.assign( defaultName );

    std::string filename;
    do
    {
        filename = getFilenameFromDefault( stringDescribingfileContents,
                                           validDefaultName );

        if( ! isFileReadable( filename ) )
        {
            std::cerr << "\n\"" << filename << "\" is not readable."
            << std::endl;
        }
    }
    while( filename.empty() );

    return filename;
}

std::string fileIO::getWritableFile( std::string defaultName )
{
    std::string filename;
    // initialize return name with default name, and if not writable set to null
    // commented out because this will leave a small empty file if user doesn't accept the default
    do
    {
        filename = getFilenameFromDefault( "output", defaultName );

        if( ! isFileWritable( filename ) )
        {
            std::cerr << "\n\"" << filename << "\" is not writable." << std::endl;
        }
    }
    while( filename.empty() );

    return filename;
}

int fileIO::readNByteBlockFromFile( void *ptr, const unsigned int nByte,
                                    const unsigned int num, FILE *stream,
                                    const bool endian_flip )
{
    if( feof( stream ) ) return 1;

    // num is the number of nByte byte blocks to be read
    if( fread( ptr, nByte, num, stream ) != num ) return 1;

    if( endian_flip )
    {
        char * buf = new char [nByte];
        for( unsigned int i = 0; i < num*nByte; i += nByte )
        {
            for( unsigned int j = 0; j < nByte; j++ )
                buf[nByte-1-j] = * ((( char* )ptr ) + i + j );

            memcpy((( char* )ptr ) + i, buf, nByte );
        }
        delete [] buf;
    }
    return 0;  //success
}
////////////////////////////////////////////////////////////////////////////////
void fileIO::processCommandLineArgs( int argc, char *argv[], const std::string verb,
                                     std::string & inFileName, std::string & outFileName )
{
    if( argc > 1 )
    {
        inFileName.assign( argv[1] );

        if( argc > 2 )
        {
            outFileName.assign( argv[2] );
        }
        else
        {
            if( outFileName.empty() )
                outFileName = fileIO::getWritableFile( "outFile.vtk" );
            else
            {
                std::string defaultName;
                defaultName.assign( outFileName );
                outFileName = fileIO::getWritableFile( defaultName );
            }
        }

        // if more than three arguments are on the commandline,
        // then assume that a shell script is in use and don't ask to verify
        if( argc > 3 )
            return;

        char response;
        do
        {
            std::cout << "\nSo you want to " << verb << " " << inFileName
            << " and save to " << outFileName << "? (y/n/q): ";
            std::cin >> response;
            std::cin.ignore();
        }
        while( response != 'y' && response != 'Y' &&
                response != 'n' && response != 'N' &&
                response != 'q' && response != 'Q' );

        if( response == 'q' || response == 'Q' )
        {
            inFileName.erase();
            outFileName.erase();
            return;
        }

        // if anything other than y/Y was input then reset argc to get
        // filenames from user...
        if( response != 'y' && response != 'Y' )
            argc = 1;
    }

    if( argc == 1 ) // then get filenames from user...
    {
        char phrase [] = {"input"};
        std::string tempText;
        tempText.assign( phrase );
        inFileName = fileIO::getReadableFileFromDefault( tempText, "inFile.vtk" );
        outFileName = fileIO::getWritableFile( "outFile.vtk" );
    }
}

std::string fileIO::getExtension( std::string filename )
{
    int len = filename.size();
    int i, foundPeriod = 0;
    std::string extension;
    for( i = len - 1; i >= 0; i-- )
    {
        if( filename[i] == '.' )
        {
            foundPeriod = 1;
            i++;   //increment i to eliminate the period
            break;
        }
    }

    if( foundPeriod )
    {
        len = filename.at( i );
        extension.assign( &filename.at( i ) );
    }

    return extension;
}

void fileIO::readToFileEnd( FILE *inputFile )
{
    // read to file end
    float junk2;
    for( int i = 0; i < 100000000; i++ )
    {
        if( fileIO::readNByteBlockFromFile( &junk2, sizeof( float ), 1, inputFile ) )
        {
            std::cout << "end of file found after reading " << i
            << " more floats" << std::endl;
            break;
        }
        else std::cout << "junk2 = " << junk2 << std::endl;
    }
}

void fileIO::StripTrailingSpaces( std::string line )
{
    for( int i = line.size() - 1; i >= 0; i-- )
    {
        if( line[i] != ' ' ) break;
        line[i] = '\0';
    }
}

std::string fileIO::StripLeadingSpaces( std::string line )
{
    std::string shortLine;
    for( int i =  0; i < ( int )line.size(); i++ )
    {
        if( line[i] != ' ' )
        {
            shortLine.assign( &line[i] );
            return shortLine;
        }
    }
    shortLine = new char [ 1 ];
    shortLine[0] = '\0';
    return shortLine;
}

/*int fileIO::extractIntegerBeforeExtension( std::string filename )
{

   return 0;//atoi(secondLastToken);
}*/
////////////////////////////////////////////////////////////////////////////////
void fileIO::getTagAndValue( std::string textline, std::string& TagName, std::string& TagValue )
{
    //pre: param file must have TAGNAME=TAGVALUE on each line, TAGVALUE cannot
    //     have any spaces in it
    //mod: extracts TAGNAME from .param file and stores in TagName, same for TagValue
    //post: none
    int i = 0;
    int j = 0;

    //while(textline[i] != '=')
    while( textline[i] != '=' )
    {
        TagName.push_back( textline[i++] );
    }
    //TagName[j] = '\0';

    j = 0;
    i++;
    while( textline[i]  >= 33 && textline[i] <= 126 )
    {
        TagValue.push_back( textline[i++] );
    }
    //TagValue[j] = '\0';
    return;
}
////////////////////////////////////////////////////////////////////////////////
void fileIO::IdentifyTagAssignValue( std::string TagName, std::string TagValue )
{
    if( TagName == "STARVRT" )
    {
        std::cout << "Yes " << TagValue << std::endl;
    }
    else
    {
        std::cout << " " << std::endl;
    }
}
////////////////////////////////////////////////////////////////////////////////
int fileIO::getIntegerBetween( const int min, const int max )
{
    int value = 0;
    char string[100];
    string[0] = '\0';
    int index;

    // repeat while value is not between min and max inclusive
    do
    {
        index = 0;
        int haveNumber = 0;
        int finishedNumber = 0;
        std::cout << "Enter value : ";
        std::cout.flush();
        while( 1 )
        {
            char c = std::cin.get();
            //std::cout << "(int)c = " << (int)c << std::endl;
            if( c == '\n' ) // ENTER key pressed ( c == 10 works too )
            {
                //std::cout << "ENTER key pressed" << std::endl;
                string[index] = '\0';
                break;
            }
            else if( haveNumber && c == ' ' ) // SPACE key pressed
            {
                //std::cout << "SPACE key pressed" << std::endl;
                // read an entry of "1  2" as 1 instead of 12
                string[index] = '\0';
                finishedNumber = 1;
            }
            else if( ! haveNumber && c == '-' ) // minus sign pressed
            {
                //std::cout << "minus sign pressed" << std::endl;
                haveNumber = 1;
                string[index] = c;
                index++;
            }
            else if( ! finishedNumber )
            {
                // check if key entered is between 0 - 9.
                if (( c - '0' >= 0 ) && ( c - '0' <= 9 ) )
                {
                    string[index] = c;
                    index++;
                    haveNumber = 1;
                }
                else //if ( (c  >= 'A') && (c  <= 'z') )
                {
                    //std::cout << "A-z pressed" << std::endl;
                    string[0] = ' ';  // this will force a new loop
                    string[1] = '\0';
                    index = 2;
                    finishedNumber = 1;
                }
            }
        } // end while

        if( !strcmp( string, "" ) ) // if only ENTER was pressed
        {
            //std::cout << "ENTER key pressed" << std::endl;
            //return defaultVal;         // return defaultVal
        }
        else
        {
            StripTrailingSpaces( string );
            std::string shortString = StripLeadingSpaces( string );
            shortString.assign( string );
            if( !strcmp( string, "" ) ) // if all spaces
                index = 0;                 // force a new loop
            else if( !strcmp( string, "-" ) ) // if only neg sign
                index = 0;                       // force a new loop
            else
            {
                // convert array of chars to integer
                value = atoi( string );
            }
            //delete [] shortString;
        }
        //std::cout << "value = " << value << std::endl;
    }
    while( index == 0 || ( value < min || value > max ) );

    return value;
}
//////////////////////////////////////////////////////////////////////////////
std::string fileIO::ExtractRelativeDirectoryFromFullPath( std::string fullPath )
{
#ifdef WIN32
    size_t backslash = fullPath.rfind( "\\" );
    size_t frontslash = fullPath.rfind( "/" );
    size_t slash = 0;
    if( backslash > fullPath.size() )
    {
        backslash = 0;
    }
    if( frontslash > fullPath.size() )
    {
        frontslash = 0;
    }
    slash = ( backslash > frontslash ) ? backslash : frontslash;
#else
    size_t slash = fullPath.rfind( "/" );
#endif
    return std::string( fullPath, slash + 1, fullPath.size() );
}
/////////////////////////////////////////////////////////////////////
std::string fileIO::ExtractBaseFileNameFromFullPath( std::string fileName )
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
    return std::string( outfileMinusExtension, slash + 1, period );
}
////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::string> fileIO::GetFilesInDirectory( std::string dir, std::string extension )
{
#if (BOOST_VERSION >= 104600) && (BOOST_FILESYSTEM_VERSION == 3)
    boost::filesystem::path dir_path( dir.c_str() );
#else
    boost::filesystem::path dir_path( dir.c_str(), boost::filesystem::no_check );
#endif
    std::list< std::string > filesInDir;
    try
    {
        if( boost::filesystem::is_directory( dir_path ) )
        {
            boost::filesystem::directory_iterator end_iter;
            for( boost::filesystem::directory_iterator dir_itr( dir_path );
                    dir_itr != end_iter; ++dir_itr )
            {
                try
                {
                    if( dir_itr->path().extension() == extension )
                    {
#if (BOOST_VERSION >= 104600) && (BOOST_FILESYSTEM_VERSION == 3)
                        dir_path = dir_itr->path();
#else
                        dir_path /= dir_itr->leaf();
#endif
                        std::string pathAndFileName;
                        pathAndFileName.assign( dir_path.string() );

                        filesInDir.push_back( pathAndFileName );
                    }
                }
                catch ( const std::exception& ex )
                {
                    std::cout << ex.what() << std::endl;
                }
            }
        }
    }
    catch ( const std::exception& ex )
    {
        std::cout << ex.what() << std::endl;
    }
    filesInDir.sort();

    std::vector< std::string > filesList;
    std::list< std::string >::iterator iter;
    for( iter = filesInDir.begin(); iter != filesInDir.end(); ++iter )
    {
        filesList.push_back( *iter );
    }

    return filesList;
}
