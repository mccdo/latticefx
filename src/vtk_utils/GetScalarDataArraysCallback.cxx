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
#include <vtk_utils/GetScalarDataArraysCallback.h>
#include <vtk_utils/cfdAccessoryFunctions.h>

#include <vtkDataSet.h>
#include <vtkCell.h>
#include <vtkPoints.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>

#include <iostream>

using namespace lfx::vtk_utils;

////////////////////////////////////////////////////////////////////////////////
GetScalarDataArraysCallback::GetScalarDataArraysCallback()
{
    ;
}
////////////////////////////////////////////////////////////////////////////////
std::vector< std::pair< std::string, std::vector< double > > > GetScalarDataArraysCallback::GetCellData()
{
    return m_pointGroup;
}
////////////////////////////////////////////////////////////////////////////////
void GetScalarDataArraysCallback::SetScalarNames( std::vector< std::string > scalarNames )
{
    m_scalarNames = scalarNames;
}
////////////////////////////////////////////////////////////////////////////////
void GetScalarDataArraysCallback::OperateOnDataset( vtkDataSet* dataset )
{
    vtkIdType numCells = dataset->GetNumberOfCells();
    for( size_t j = 0; j < m_scalarNames.size(); ++j )
    {
        vtkDataArray* scalarArray = dataset->GetCellData()->GetScalars( m_scalarNames.at( j ).c_str() );
        std::vector< double > scalarVector;
        for( vtkIdType i = 0; i < numCells; ++i )
        {
            vtkCell* tempCell = dataset->GetCell( i );
            if( VTK_VERTEX == tempCell->GetCellType() )
            {
                scalarVector.push_back( scalarArray->GetTuple1( i ) );
            }
        }
        m_pointGroup.push_back( std::make_pair< std::string, std::vector< double > >( m_scalarNames.at( j ), scalarVector ) );
    }
}
////////////////////////////////////////////////////////////////////////////////
void GetScalarDataArraysCallback::ResetPointGroup()
{
    m_pointGroup.clear();
}
////////////////////////////////////////////////////////////////////////////////
