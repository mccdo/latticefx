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
#include <latticefx/translators/vtk/converter.h>

#include <iostream>

#include <vtkPointData.h>
#include <vtkFloatArray.h>

using namespace lfx::vtk_translator;

void lfx::vtk_translator::letUsersAddParamsToField( const int numParams, vtkFloatArray** data,
        vtkPointData* pointData, int verbose )
{
    int debug  = 0;

    if( debug )
        std::cout << "in letUsersAddParamsToField with numParams = "
                  << numParams << std::endl;

    char response;
    for( int i = 0; i < numParams; i++ )
    {
        if( debug > 1 )
            std::cout << "data[" << i << "]->GetNumberOfTuples() = "
                      << data[i]->GetNumberOfTuples() << std::endl;

        // if there is just one scalar/vector then write it to field without
        // checking with user.  Otherwise, check with user if the scalar/vector
        // should be written to the flowdata.vtk file...
        // verbose sets wether or not just to skip the questions from being asked
        if( numParams > 1 && verbose == 1 )
        {
            do
            {
                if( data[i]->GetNumberOfComponents() == 1 )
                {
                    std::cout << "Do you want to store scalar value \"";
                }
                else
                {
                    std::cout << "Do you want to store vector value \"";
                }

                std::cout << data[i]->GetName() << "\" in flowdata.vtk? (y/n): ";
                std::cout.flush();
                std::cin >> response;
                std::cin.ignore();
                //cout << "response = " << response << endl;
            }
            while( response != 'y' && response != 'Y' &&
                    response != 'n' && response != 'N' );

            // if other than y/Y was input then don't write parameter to field...
            if( response != 'y' && response != 'Y' )
            {
                continue;
            }
        }
        pointData->AddArray( data[i] );
    }

    // Breaks Deere's Build
    //if ( debug > 1 )
    //   pointData->Print( std::cout );
}

