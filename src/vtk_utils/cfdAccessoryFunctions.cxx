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

#include <vtk_utils/cfdAccessoryFunctions.h>
#include <vtkGenericCell.h>
#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <iostream>

using namespace lfx::vtk_utils;

cfdAccessoryFunctions::cfdAccessoryFunctions( )
{}

cfdAccessoryFunctions::~cfdAccessoryFunctions( )
{}

double cfdAccessoryFunctions::ComputeVectorMagnitude( double vectorComponents [ 3 ] )
{
    double magnitude = vectorComponents[ 0 ] * vectorComponents[ 0 ] +
                       vectorComponents[ 1 ] * vectorComponents[ 1 ] +
                       vectorComponents[ 2 ] * vectorComponents[ 2 ];

    magnitude = sqrt( magnitude );
    return magnitude;
}

double * cfdAccessoryFunctions::ComputeVectorMagnitudeRange( vtkDataArray * dataArray )
{
    // magnitudeRange is used by vector-based visualizations when
    // "scale by vector magnitude" is selected. magnitudeRange[ 0 ] determines
    // a vector magnitude that will have zero length.  magnitudeRange[ 1 ]
    // determines a vector magnitude that corresponds to unit length. In this
    // function, we compute the lower and upper bounds on vector magnitude,
    // then we override the lower bound so that a zero length vector corresponds
    // only to a zero magnitude vector.

    if( dataArray->GetNumberOfComponents() != 3 )
    {
        std::cerr << "ERROR: ComputeVectorMagnitudeRange requires 3-component "
        << "vector data" << std::endl;
        return NULL;
    }

    double * magnitudeRange = new double [ 2 ];

    int numTuples = dataArray->GetNumberOfTuples();

    /*
       vprDEBUG(vesDBG,1) << "\tnum vector tuples = " << numTuples
                              << std::endl << vprDEBUG_FLUSH;
    */

    // get the components of the first vector tuple...
    double vectorComponents[ 3 ];
    dataArray->GetTuple( 0, vectorComponents );

    // use this first tuple to initialize min and max range values
    magnitudeRange[ 0 ] = magnitudeRange[ 1 ]
                          = ComputeVectorMagnitude( vectorComponents );
    /*
       vprDEBUG(vesDBG,2) << " tuple 0, mag = " << magnitudeRange[ 0 ]
                              << std::endl << vprDEBUG_FLUSH;
    */

    // for all other vector tuples, compare to current magnitudeRange...
    for( int i = 1; i < numTuples; i++ )
    {
        dataArray->GetTuple( i, vectorComponents );

        double magnitude = ComputeVectorMagnitude( vectorComponents );
        /*
              vprDEBUG(vesDBG,3) << " tuple " << i << ", mag = " << magnitude
                                     << std::endl << vprDEBUG_FLUSH;
        */
        if( magnitudeRange[ 0 ] > magnitude )
            magnitudeRange[ 0 ] = magnitude;

        if( magnitudeRange[ 1 ] < magnitude )
            magnitudeRange[ 1 ] = magnitude;
    }

    // override the minimum so that a zero-length vector corresponds
    // only to a zero-magnitude vector...
    magnitudeRange[ 0 ] = 0.0;
    /*
       vprDEBUG(vesDBG,1) << "\tmagnitudeRange = "
          << magnitudeRange[ 0 ] << " : " << magnitudeRange[ 1 ]
          << std::endl << vprDEBUG_FLUSH;
    */

    return magnitudeRange;
}

double cfdAccessoryFunctions::ComputeMeanCellBBLength( vtkDataSet * dataSet )
{
    std::cout << "\tcomputing meanCellBBLength..." << std::endl;

    double meanCellBBLength = 0.0;

    // THIS METHOD IS THREAD SAFE IF FIRST CALLED FROM A SINGLE THREAD
    // AND THE DATASET IS NOT MODIFIED
    int numCells = dataSet->GetNumberOfCells();
    //std::cout << "\tnumCells = " << numCells << std::endl;
    if( numCells == 0 )
    {
        meanCellBBLength = 1.0;
        std::cout << "\tnumCells = 0, so setting meanCellBBLength to "
        << meanCellBBLength << std::endl;
        return meanCellBBLength;
    }

    vtkGenericCell *cell = vtkGenericCell::New();
    for( int cellId = 0; cellId < numCells; cellId++ )
    {
        dataSet->GetCell( cellId, cell );
        meanCellBBLength += sqrt( cell->GetLength2() );
        // THIS METHOD IS THREAD SAFE IF FIRST CALLED FROM A SINGLE THREAD
        // AND THE DATASET IS NOT MODIFIED
    }
    cell->Delete();
    meanCellBBLength /= numCells;

    return meanCellBBLength;
}
