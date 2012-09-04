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

#include <latticefx/utils/vtk/ActiveDataInformationCallback.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>

using namespace lfx::vtk_utils;

////////////////////////////////////////////////////////////////////////////////
ActiveDataInformationCallback::ActiveDataInformationCallback()
{}
////////////////////////////////////////////////////////////////////////////////
ActiveDataInformationCallback::~ActiveDataInformationCallback()
{}
////////////////////////////////////////////////////////////////////////////////
void ActiveDataInformationCallback::OperateOnDataset( vtkDataSet* dataset )
{
    if( !m_activeScalar.empty() )
    {
        dataset->GetPointData()->SetActiveScalars( m_activeScalar.c_str() );
        dataset->GetCellData()->SetActiveScalars( m_activeScalar.c_str() );
    }
    if( !m_activeVector.empty() )
    {
        dataset->GetPointData()->SetActiveVectors( m_activeVector.c_str() );
        dataset->GetCellData()->SetActiveVectors( m_activeVector.c_str() );
    }
}
////////////////////////////////////////////////////////////////////////////////
void ActiveDataInformationCallback::SetActiveDataName( std::string name,
                                                       bool isVector )
{
    ( isVector ) ? m_activeVector = name : m_activeScalar = name;
}
////////////////////////////////////////////////////////////////////////////////
std::string ActiveDataInformationCallback::GetActiveDataName( bool isVector )
{
    return ( isVector ) ? m_activeVector : m_activeScalar;
}

