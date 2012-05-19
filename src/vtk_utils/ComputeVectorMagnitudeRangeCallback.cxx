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
#include <ves/xplorer/util/ComputeVectorMagnitudeRangeCallback.h>
#include <ves/xplorer/util/cfdAccessoryFunctions.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

using namespace ves::xplorer::util;

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
        cfdAccessoryFunctions::ComputeVectorMagnitudeRange( dataset->GetPointData()->GetVectors() );
    m_magnitudeRange[0] = ( range[0] < m_magnitudeRange[0] ) ? range[0] : m_magnitudeRange[0];
    m_magnitudeRange[1] = ( range[1] > m_magnitudeRange[1] ) ? range[1] : m_magnitudeRange[1];
}

