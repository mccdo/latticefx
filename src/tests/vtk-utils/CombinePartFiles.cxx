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

// a utility created by hgx to merge fluent particle files

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int main( int argc, char** argv )
{
    if( argc < 1 )
    {
        std::cout << " What ! " << argv[ 0 ] << std::endl;
        return 1;
    }

    std::string inPartFileName;

    char textLine[256];
    int i, j;
    for( i = 1; i <= 8; i++ )
    {
        std::ostringstream infileName;
        infileName << "file" << i << ".part";
        inPartFileName = infileName.str();
        std::ifstream inPartFile;
        inPartFile.open( ( inPartFileName ).c_str() );
        std::cout << inPartFileName << std::endl;
        if( i != 1 )
        {
            for( j = 0; j < 19; j++ )
            {
                inPartFile.getline( textLine, 256 );
            }
        }

        std::ofstream outPartFile;
        outPartFile.precision( 10 );
        outPartFile.open( "file.part", std::ios::app | std::ios::ate );
        //outPartFile.open("file.part",std::ios::app);
        while( !inPartFile.eof() )
        {
            inPartFile.getline( textLine, 256 );
            outPartFile << textLine << std::endl;
        }
        inPartFile.close();
        outPartFile.close();
    }

    return 0;
}
