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
#include <latticefx/utils/vtk/ComputeVectorMagnitudeAndScalarsCallback.h>
#include <latticefx/utils/vtk/AccessoryFunctions.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>

#include <iostream>

using namespace lfx::vtk_utils;

//////////////////////////////////////////////////////////////////////
ComputeVectorMagnitudeAndScalarsCallback::ComputeVectorMagnitudeAndScalarsCallback()
{

}
///////////////////////////////////////////////////////////////////////////////
/*void ComputeVectorMagnitudeRangeCallback::GetVectorMagnitudeRange( double*& vMagRange )
{
}*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComputeVectorMagnitudeAndScalarsCallback::OperateOnDataset( vtkDataSet* dataset )
{
    //this portion is to grab scalar data out of the vectors and rewrite it back
    //into the VTK file
    int numArrays = dataset->GetPointData()->GetNumberOfArrays();
    //std::cout << "Number of arrays " << numArrays << std::endl;
    int numComponents;
    int numOfTuples;
    std::string name;
    std::string scalName;
    double component;
    double velMag;
    for( int i = 0; i < numArrays; ++i ) //loop pver number of arrays
    {
        numComponents = dataset->GetPointData()->GetArray( i )->GetNumberOfComponents();
        //std::cout << "Number of components " << numComponents << std::endl;
        vtkDataArray* activeDataArray = dataset->GetPointData()->GetArray( i );
        if( numComponents > 1 ) //it is a vector
        {
            vtkFloatArray** scalarsFromVector;

            scalarsFromVector = new vtkFloatArray* [ numComponents + 1 ];
            name = activeDataArray->GetName();
            numOfTuples = activeDataArray->GetNumberOfTuples();
            //std::cout<<" Name :" << name <<std::endl
            //    <<" Number of Tuples :"<< numOfTuples <<std::endl;
            for( int compLoop = 0; compLoop < numComponents; ++compLoop )
            {
                scalName = name;
                if( compLoop == 0 )
                {
                    scalName.append( "_u" );
                }
                else if( compLoop == 1 )
                {
                    scalName.append( "_v" );
                }
                else if( compLoop == 2 )
                {
                    scalName.append( "_w" );
                }
                scalarsFromVector[ compLoop ] = vtkFloatArray::New();
                scalarsFromVector[ compLoop ]->SetNumberOfComponents( 1 );
                scalarsFromVector[ compLoop ]->SetNumberOfTuples( numOfTuples );
                //std::cout << "Scalar name " <<scalName <<std::endl;
                scalarsFromVector[ compLoop ]->SetName( scalName.c_str() );
                scalName.clear();
                for( int tupLoop = 0; tupLoop < numOfTuples; ++tupLoop )
                {
                    //get the component data
                    component =
                        activeDataArray->GetComponent( tupLoop, compLoop );
                    scalarsFromVector[ compLoop ]->SetComponent( tupLoop, 0, component );
                }
            }
            //now calculate magnitude of the vector
            scalName = name;
            scalarsFromVector[ numComponents ] = vtkFloatArray::New();
            scalName.append( "_magnitude" );
            scalarsFromVector[ numComponents ]->SetNumberOfComponents( 1 );
            scalarsFromVector[ numComponents ]->SetNumberOfTuples( numOfTuples );
            //std::cout << "Scalar name " <<scalName <<std::endl;
            scalarsFromVector[ numComponents ]->SetName( scalName.c_str() );
            for( int tupLoop = 0; tupLoop < numOfTuples; tupLoop++ )
            {
                velMag = double( 0 );
                for( int compLoop = 0; compLoop < numComponents; compLoop++ )
                {
                    component =
                        activeDataArray->GetComponent( tupLoop, compLoop );
                    velMag = velMag + component * component;
                }
                velMag = sqrt( velMag );
                scalarsFromVector[ numComponents ]->SetComponent( tupLoop, 0, velMag );
            }
            //letUsersAddParamsToField( numComponents+1, scalarsFromVector, dataset->GetPointData() );
            for( int j = 0; j < numComponents + 1; j++ )
            {
                dataset->GetPointData()->AddArray( scalarsFromVector[ j ] );
                scalarsFromVector[ j ]->Delete();
            }

            delete [] scalarsFromVector;
            scalarsFromVector = NULL;
        }  //end if loop
    } //end for loop
}
