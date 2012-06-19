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
#include <latticefx/utils/vtk/setScalarAndVector.h>
#include <iostream>

#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>

using namespace lfx::vtk_utils;

void lfx::vtk_utils::activateScalar( vtkDataSet * dataSet )
{
    // if there are data arrays, count the number of arrays
    int numPDArrays = dataSet->GetPointData()->GetNumberOfArrays();
    //std::cout << "numPDArrays = " << numPDArrays << std::endl;
    if( numPDArrays )
    {
        //count number of scalars...
        int numScalars = 0;
        for( int i = 0; i < numPDArrays; i++ )
        {
            if( dataSet->GetPointData()->GetArray( i )->GetNumberOfComponents() == 1 )
                numScalars++;
        }

        if( numScalars == 0 )
            return;
        else if( numScalars == 1 )
        {
            for( int i = 0; i < numPDArrays; i++ )
            {
                if( dataSet->GetPointData()->GetArray( i )->GetNumberOfComponents() == 1 )
                {
                    dataSet->GetPointData()->SetActiveScalars(
                        dataSet->GetPointData()->GetArray( i )->GetName() );
                }
            }
        }
        else
        {
            std::cout << "\nThe available scalars are ..." << std::endl;
            for( int i = 0; i < numPDArrays; i++ )
            {
                if( dataSet->GetPointData()->GetArray( i )->GetNumberOfComponents() == 1 )
                    std::cout << "\t" << i << "\t" << dataSet->GetPointData()->GetArray( i )->GetName() << std::endl;
            }
            std::cout << std::endl;

            int choice;
            do
            {
                std::cout << "Enter the integer corresponding to the scalar you want to activate: ";
                std::cin >> choice;

                // verify that the choice corresponds to a valid scalar...
                if( dataSet->GetPointData()->GetArray( choice )
                        ->GetNumberOfComponents() == 1 )
                    break;
                else
                    std::cout << "ERROR!: " << std::flush;
            }
            while( 1 );

            dataSet->GetPointData()->SetActiveScalars(
                dataSet->GetPointData()->GetArray( choice )->GetName() );
        }
    }
    return;
}

void lfx::vtk_utils::activateVector( vtkDataSet * dataSet )
{
    // if there are data arrays, count the number of arrays
    int numPDArrays = dataSet->GetPointData()->GetNumberOfArrays();
    //std::cout << "numPDArrays = " << numPDArrays << std::endl;
    if( numPDArrays )
    {
        //count number of vectors...
        int numVectors = 0;
        for( int i = 0; i < numPDArrays; i++ )
        {
            if( dataSet->GetPointData()->GetArray( i )->GetNumberOfComponents() == 3 )
                numVectors++;
        }

        if( numVectors == 0 )
            return;
        else if( numVectors == 1 )
        {
            for( int i = 0; i < numPDArrays; i++ )
            {
                if( dataSet->GetPointData()->GetArray( i )->GetNumberOfComponents() == 3 )
                {
                    dataSet->GetPointData()->SetActiveVectors(
                        dataSet->GetPointData()->GetArray( i )->GetName() );
                }
            }
        }
        else
        {
            std::cout << "\nThe available vectors are ..." << std::endl;
            for( int i = 0; i < numPDArrays; i++ )
            {
                if( dataSet->GetPointData()->GetArray( i )->GetNumberOfComponents() == 3 )
                    std::cout << "\t" << i << "\t" << dataSet->GetPointData()->GetArray( i )->GetName() << std::endl;
            }

            int choice;
            do
            {
                std::cout << "\nEnter the integer corresponding to the vector you want to activate: " << std::endl;
                std::cin >> choice;

                // verify that the choice corresponds to a valid vector...
                if( dataSet->GetPointData()->GetArray( choice )
                        ->GetNumberOfComponents() == 3 )
                    break;
                else
                    std::cout << "ERROR!: " << std::flush;
            }
            while( 1 );

            dataSet->GetPointData()->SetActiveVectors(
                dataSet->GetPointData()->GetArray( choice )->GetName() );
        }
    }
    return;
}

