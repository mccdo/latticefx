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

#include <latticefx/utils/vtk/CountNumberOfParametersCallback.h>
#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>

#include <algorithm>

using namespace lfx::vtk_utils;

//////////////////////////////////////////////////////////////////
CountNumberOfParametersCallback::CountNumberOfParametersCallback()
{
    m_numberOfParameters[0] = 0;
    m_numberOfParameters[1] = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////
CountNumberOfParametersCallback::~CountNumberOfParametersCallback()
{
    m_scalarNames.clear();
    m_vectorNames.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CountNumberOfParametersCallback::OperateOnDataset( vtkDataSet* dataset )
{
    unsigned int numPtDataArrays = dataset->GetPointData()
                                   ->GetNumberOfArrays();
    std::vector<std::string>::iterator name;
    // count the number of paraneters containing numComponents components...
    for( unsigned int i = 0; i < numPtDataArrays; i++ )
    {
        vtkDataArray* array = dataset->GetPointData()->GetArray( i );
        name = std::find( m_scalarNames.begin(),
                          m_scalarNames.end(),
                          array->GetName() );
        if( name != m_scalarNames.end() )
        {
            ///This scalar already exists
            continue;
        }
        name = std::find( m_vectorNames.begin(),
                          m_vectorNames.end(),
                          array->GetName() );
        if( name != m_vectorNames.end() )
        {
            ///This vector already exists
            continue;
        }

        // also, ignore arrays of normals...
        if( array->GetNumberOfComponents() == 3 && ( ! strcmp( array->GetName(), "normals" ) ) )
        {
            continue;
        }
        else if( array->GetNumberOfComponents() == 3 )
        {
            m_numberOfParameters[1]++;
            m_vectorNames.push_back( std::string( array->GetName() ) );
        }
        else if( array->GetNumberOfComponents() == 1 )
        {
            m_numberOfParameters[0]++;
            m_scalarNames.push_back( std::string( array->GetName() ) );
        }
    }

    unsigned int numCellDataArrays = dataset->GetCellData()
                                     ->GetNumberOfArrays();
    // count the number of paraneters containing numComponents components...
    for( unsigned int i = 0; i < numCellDataArrays; i++ )
    {
        vtkDataArray* array = dataset->GetCellData()->GetArray( i );
        name = std::find( m_scalarNames.begin(),
                          m_scalarNames.end(),
                          array->GetName() );
        if( name != m_scalarNames.end() )
        {
            ///This scalar already exists
            continue;
        }
        name = std::find( m_vectorNames.begin(),
                          m_vectorNames.end(),
                          array->GetName() );
        if( name != m_vectorNames.end() )
        {
            ///This vector already exists
            continue;
        }

        // also, ignore arrays of normals...
        if( array->GetNumberOfComponents() == 3 && ( ! strcmp( array->GetName(), "normals" ) ) )
        {
            continue;
        }
        else if( array->GetNumberOfComponents() == 3 )
        {
            m_numberOfParameters[1]++;
            m_vectorNames.push_back( std::string( array->GetName() ) );
        }
        else if( array->GetNumberOfComponents() == 1 )
        {
            m_numberOfParameters[0]++;
            m_scalarNames.push_back( std::string( array->GetName() ) );
        }
    }

}
//////////////////////////////////////////////////////////////////////////////////
unsigned int CountNumberOfParametersCallback::GetNumberOfParameters( bool isVector )
{
    return ( isVector ) ? m_numberOfParameters[1] : m_numberOfParameters[0];
}
//////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::string> CountNumberOfParametersCallback::GetParameterNames( bool isVector )
{
    return ( isVector ) ? m_vectorNames : m_scalarNames;
}

