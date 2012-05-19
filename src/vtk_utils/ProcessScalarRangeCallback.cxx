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

#include <ves/xplorer/util/ProcessScalarRangeCallback.h>
#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <iostream>

using namespace ves::xplorer::util;

//////////////////////////////////////////////////////////////////
ProcessScalarRangeCallback::ProcessScalarRangeCallback()
{}
///////////////////////////////////////////////////////////////////
ProcessScalarRangeCallback::~ProcessScalarRangeCallback()
{
    for( std::map<std::string, double* >::const_iterator iter = m_scalarRanges.begin();
            iter != m_scalarRanges.end();
            ++iter )
    {
        delete [] iter->second;
    }
    m_scalarRanges.clear();
}
//////////////////////////////////////////////////////////////////////////////////////////
void ProcessScalarRangeCallback::OperateOnDataset( vtkDataSet* dataset )
{
    // store actual range...
    std::map<std::string, double* >::iterator scalarRangeInfo;
    vtkDataArray* array = 0;
    double tempRange[ 2 ] = { 0.0f, 0.0f };;
    if( dataset->GetCellData()->GetNumberOfArrays() > 0 )
    {
        int numArrays = dataset->GetCellData()->GetNumberOfArrays();
        for( int i = 0; i < numArrays; ++i )
        {
            array = dataset->GetCellData()->GetArray( i );
            //if it is a vector
            if( array->GetNumberOfComponents() != 1 )
            {
                continue;
            }
            //Not already in the list so find the range
            //This assumes that there are only unique
            //scalars AND that each dataset has the same
            //unique scalars---This may be incorrect because
            //each dataset in the multiblock may have it's own scalar range but
            //it's not clear if that is the case...
            array->GetRange( tempRange );
            if( fabs( tempRange[ 0 ] ) < 1e-100 )
            {
                tempRange[ 0 ] = 0.0f;
            }
            if( fabs( tempRange[ 1 ] ) < 1e-100 )
            {
                tempRange[ 0 ] = 0.0f;
            }
            
            scalarRangeInfo = m_scalarRanges.find( array->GetName() );
            if( scalarRangeInfo == m_scalarRanges.end() )
            {
                m_scalarRanges[array->GetName()] = new double[2];
                m_scalarRanges[array->GetName()][ 0 ] = tempRange[ 0 ];
                m_scalarRanges[array->GetName()][ 1 ] = tempRange[ 1 ];
                //array->GetRange( m_scalarRanges[array->GetName()] );
            }
            else
            {
                if( scalarRangeInfo->second[ 0 ] > tempRange[ 0 ] )
                {
                    scalarRangeInfo->second[ 0 ] = tempRange[ 0 ];
                }
                
                if( scalarRangeInfo->second[ 1 ] < tempRange[ 1 ] )
                {
                    scalarRangeInfo->second[ 1 ] = tempRange[ 1 ];
                }                
            }
        }
    }
    //THe dataset could have both point and cell data
    if( dataset->GetPointData()->GetNumberOfArrays() > 0 )
    {
        int numArrays = dataset->GetPointData()->GetNumberOfArrays();
        for( int i = 0; i < numArrays; ++i )
        {
            array = dataset->GetPointData()->GetArray( i );
            //if it is a vector
            if( array->GetNumberOfComponents() != 1 )
            {
                continue;
            }
            //Not already in the list so find the range
            //This assumes that there are only unique
            //scalars AND that each dataset has the same
            //unique scalars---This may be incorrect because
            //each dataset in the multiblock may have it's own scalar range but
            //it's not clear if that is the case...
            scalarRangeInfo = m_scalarRanges.find( array->GetName() );
            if( scalarRangeInfo == m_scalarRanges.end() )
            {
                m_scalarRanges[array->GetName()] = new double[2];
                array->GetRange( m_scalarRanges[array->GetName()] );
            }
            else
            {
                double tempRange[ 2 ] = { 0.0f, 0.0f };
                array->GetRange( tempRange );
                if( scalarRangeInfo->second[ 0 ] > tempRange[ 0 ] )
                {
                    scalarRangeInfo->second[ 0 ] = tempRange[ 0 ];
                }
                
                if( scalarRangeInfo->second[ 1 ] < tempRange[ 1 ] )
                {
                    scalarRangeInfo->second[ 1 ] = tempRange[ 1 ];
                }                
            }
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessScalarRangeCallback::GetScalarRange( const std::string& scalarName, double*& range )
{
    std::map<std::string, double* >::const_iterator scalarRangeInfo = m_scalarRanges.find( scalarName );
    if( scalarRangeInfo == m_scalarRanges.end() )
    {
        range[0] = 0.0;
        range[1] = 0.0;
        return;
    }

    try
    {
        range[0] = scalarRangeInfo->second[0];
        range[1] = scalarRangeInfo->second[1];
    }
    catch ( ... )
    {
        std::cout << "Invalid scalar specified: " << scalarName << std::endl;
        std::cout << "ProcessScalarRangeCallback::GetScalarRange" << std::endl;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
