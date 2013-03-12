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

#include <latticefx/utils/vtk/ComputeDataObjectBoundsCallback.h>
#include <vtkDataSet.h>
#include <cmath>

using namespace lfx::vtk_utils;

//////////////////////////////////////////////////////////////////
ComputeDataObjectBoundsCallback::ComputeDataObjectBoundsCallback()
{
    m_bounds[0] = 100000;
    m_bounds[1] = -100000;
    m_bounds[2] = 100000;
    m_bounds[3] = -100000;
    m_bounds[4] = 100000;
    m_bounds[5] = -100000;
}
///////////////////////////////////////////////////////////////////////////
void ComputeDataObjectBoundsCallback::OperateOnDataset( vtkDataSet* dataset )
{
    double bounds[6];
    dataset->GetBounds( bounds );
    if( bounds[0] < m_bounds[0] )
    {
        m_bounds[0] = bounds[0];
    }
    if( bounds[1] > m_bounds[1] )
    {
        m_bounds[1] = bounds[1];
    }
    if( bounds[2] < m_bounds[2] )
    {
        m_bounds[2] = bounds[2];
    }
    if( bounds[3] > m_bounds[3] )
    {
        m_bounds[3] = bounds[3];
    }
    if( bounds[4] < m_bounds[4] )
    {
        m_bounds[4] = bounds[4];
    }
    if( bounds[5] > m_bounds[5] )
    {
        m_bounds[5] = bounds[5];
    }

}
//////////////////////////////////////////////////////////////////////////
void ComputeDataObjectBoundsCallback::GetDataObjectBounds( double* bounds )
{

    bounds[0] = m_bounds[0];
    bounds[1] = m_bounds[1];
    bounds[2] = m_bounds[2];
    bounds[3] = m_bounds[3];
    bounds[4] = m_bounds[4];
    bounds[5] = m_bounds[5];
}
///////////////////////////////////////////////////////////////////
double ComputeDataObjectBoundsCallback::GetDataObjectBoundsDiagonal()
{
    return sqrt( ( m_bounds[1] - m_bounds[0] ) * ( m_bounds[1] - m_bounds[0] ) +
                 ( m_bounds[3] - m_bounds[2] ) * ( m_bounds[3] - m_bounds[2] ) +
                 ( m_bounds[5] - m_bounds[4] ) * ( m_bounds[5] - m_bounds[4] ) );
}
