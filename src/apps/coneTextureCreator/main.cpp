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

#define CONE_HEIGHT 500
#define CONE_RADIUS 250

#define TEXTURE_X 512
#define TEXTURE_Y 512
#define TEXTURE_Z 512

#define TEXTURE_HALF_X 256
#define TEXTURE_HALF_Y 256

bool testVoxel( const int x, const int y, const int z )
{
    const int radiusTest = ( (x - TEXTURE_HALF_X) * (x - TEXTURE_HALF_X) +
        (y - TEXTURE_HALF_Y) * (y - TEXTURE_HALF_Y) );
    
    const double heightRadius = 
        ( double( CONE_RADIUS ) / double( CONE_HEIGHT ) ) * ( z - CONE_HEIGHT ) * ( z - CONE_HEIGHT );
    
    if( heightRadius > radiusTest )
    {
        return true;
    }
    return false;
}

void writeVoxel( const int val )
{
    ;
}

int main( int argc, char** argv )
{
    for( size_t k = 0; k < TEXTURE_Z; ++k )
    {
        for( size_t j = 0; j < TEXTURE_Y; ++j )
        {
            for( size_t i = 0; i < TEXTURE_Z; ++i )
            {
                int voxelVal = 0;
                if( testVoxel( i, j, k ) )
                {
                    voxelVal = 1;
                }
                writeVoxel( voxelVal );
            }
        }
    }

    return 0;
}
