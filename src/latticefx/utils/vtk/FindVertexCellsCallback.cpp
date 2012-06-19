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
#include <vtk_utils/FindVertexCellsCallback.h>

#include <vtkDataSet.h>
#include <vtkCell.h>
#include <vtkPoints.h>

#include <iostream>

using namespace lfx::vtk_utils;

////////////////////////////////////////////////////////////////////////////////
FindVertexCellsCallback::FindVertexCellsCallback()
{
    ;
}
////////////////////////////////////////////////////////////////////////////////
std::vector< std::pair< vtkIdType, double* > > FindVertexCellsCallback::GetVertexCells()
{
    return m_pointGroup;
}
////////////////////////////////////////////////////////////////////////////////
void FindVertexCellsCallback::OperateOnDataset( vtkDataSet* dataset )
{
    vtkIdType numCells = dataset->GetNumberOfCells();
    for( vtkIdType i = 0; i < numCells; ++i )
    {
        vtkCell* tempCell = dataset->GetCell( i );
        if( VTK_VERTEX == tempCell->GetCellType() )
        {
            std::pair< vtkIdType, double* > tempPair = std::make_pair< vtkIdType, double* >( i, 0 );
            tempPair.second = new double[3];
            tempCell->GetPoints()->GetPoint( 0, &*tempPair.second );
            //double* tempData = tempPair.second;
            //std::cout << tempPair.first << " " << tempData[ 0 ] 
            //    << " " << tempData[ 1 ] << " " << tempData[ 2 ] << std::endl;

            m_pointGroup.push_back( tempPair );
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
void FindVertexCellsCallback::ResetPointGroup()
{
    m_pointGroup.clear();
}
////////////////////////////////////////////////////////////////////////////////
