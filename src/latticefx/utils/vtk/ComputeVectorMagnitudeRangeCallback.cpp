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
#include <latticefx/utils/vtk/ComputeVectorMagnitudeRangeCallback.h>
#include <latticefx/utils/vtk/AccessoryFunctions.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

using namespace lfx::vtk_utils;

//////////////////////////////////////////////////////////////////////
ComputeVectorMagnitudeRangeCallback::ComputeVectorMagnitudeRangeCallback()
{
    m_magnitudeRange[0] = 0.0;
    m_magnitudeRange[1] = 1.0;
}
///////////////////////////////////////////////////////////////////////////////
void ComputeVectorMagnitudeRangeCallback::GetVectorMagnitudeRange( double*& vMagRange )
{
    vMagRange[0] = m_magnitudeRange[0];
    vMagRange[1] = m_magnitudeRange[1];
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComputeVectorMagnitudeRangeCallback::OperateOnDataset( vtkDataSet* dataset )
{
    if( !dataset->GetPointData()->GetVectors() )
    {
        return;
    }
    double* range =
        AccessoryFunctions::ComputeVectorMagnitudeRange( dataset->GetPointData()->GetVectors() );
    m_magnitudeRange[0] = ( range[0] < m_magnitudeRange[0] ) ? range[0] : m_magnitudeRange[0];
    m_magnitudeRange[1] = ( range[1] > m_magnitudeRange[1] ) ? range[1] : m_magnitudeRange[1];
    delete [] range;
}

