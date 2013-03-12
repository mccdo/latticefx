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
#include <iostream>
#include <fstream>
#include <vector>

#include <ves/xplorer/util/fileIO.h>
#include <ves/xplorer/util/readWriteVtkThings.h>

#include <gmtl/Matrix.h>
#include <gmtl/Math.h>
#include <gmtl/Vec.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Generate.h>
#include <gmtl/VecOps.h>
#include <gmtl/AxisAngle.h>
#include <gmtl/AxisAngleOps.h>


using namespace ves::xplorer::util;
gmtl::Matrix44f GetVjMatrix( std::vector< double > perfMat );


int main( int argc, char* argv[] )
{
    std::ifstream inFile( argv[ 1 ] );
    std::ofstream outFile( argv[ 2 ] );
    char lineData[ 512 ];
    inFile.getline( lineData, 512 );
    outFile << lineData;
    outFile << std::endl;

    int numPoints = 0;
    inFile >> numPoints;
    outFile << numPoints;
    inFile.getline( lineData, 512 );
    outFile << lineData;
    outFile << std::endl;

    double data;
    for( size_t i = 0; i < numPoints; ++i )
    {
        std::vector< double > matrixVector;
        for( size_t j = 0; j < 16; ++j )
        {
            inFile >> data;
            matrixVector.push_back( data );
        }
        //write the converted data back out
        gmtl::Matrix44f mat = GetVjMatrix( matrixVector );
        for( unsigned int j = 0; j < 4; j++ )
        {
            for( unsigned int k = 0; k < 4; k++ )
            {
                outFile << mat[ j ][ k ] << " ";
            }
        }
        //now move on...
        inFile.getline( lineData, 512 );
        outFile << lineData;
        outFile << std::endl;
        inFile.getline( lineData, 512 );
        outFile << lineData;
        outFile << std::endl;
    }

    //process stored flythroughs
    inFile.getline( lineData, 512 );
    outFile << lineData;
    outFile << std::endl;
    inFile >> numPoints;
    outFile << numPoints;
    inFile.getline( lineData, 512 );
    outFile << lineData;
    outFile << std::endl;
    for( size_t i = 0; i < numPoints; ++i )
    {
        inFile.getline( lineData, 512 );
        outFile << lineData;
        outFile << std::endl;
        inFile.getline( lineData, 512 );
        outFile << lineData;
        outFile << std::endl;
    }
    outFile.close();
    inFile.close();
    return 0;
}

gmtl::Matrix44f GetVjMatrix( std::vector< double > perfMat )
{
    gmtl::Matrix44f mat;
    gmtl::Vec3f x_axis( 1.0f, 0.0f, 0.0f );
    mat.set( perfMat.at( 0 ), perfMat.at( 1 ), perfMat.at( 2 ), perfMat.at( 3 ),
             perfMat.at( 4 ), perfMat.at( 5 ), perfMat.at( 6 ), perfMat.at( 7 ),
             perfMat.at( 8 ), perfMat.at( 9 ), perfMat.at( 10 ), perfMat.at( 11 ),
             perfMat.at( 12 ), perfMat.at( 13 ), perfMat.at( 14 ), perfMat.at( 15 )
           );
    gmtl::postMult( mat, gmtl::makeRot<gmtl::Matrix44f>( gmtl::AxisAnglef( gmtl::Math::deg2Rad( 90.0f ), x_axis ) ) );
    gmtl::preMult( mat, gmtl::makeRot<gmtl::Matrix44f>( gmtl::AxisAnglef( gmtl::Math::deg2Rad( -90.0f ), x_axis ) ) );

    return mat;
}
