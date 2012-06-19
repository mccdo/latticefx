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

#include <vtk_utils/ActiveDataInformationCallback.h>
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

