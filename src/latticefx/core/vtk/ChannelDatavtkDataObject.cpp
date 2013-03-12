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

#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>

#include <latticefx/utils/vtk/ComputeDataObjectBoundsCallback.h>
#include <latticefx/utils/vtk/GetNumberOfPointsCallback.h>
#include <latticefx/utils/vtk/DataObjectHandler.h>
#include <latticefx/utils/vtk/ProcessScalarRangeCallback.h>

#include <vtkDataObject.h>

namespace lfx
{

namespace core
{

namespace vtk
{

////////////////////////////////////////////////////////////////////////////////
ChannelDatavtkDataObject::ChannelDatavtkDataObject( vtkDataObject* dobj, const std::string& name )
    :
    ChannelData( name ),
    m_dobj( dobj )
{
    m_bounds[0] = 100000;
    m_bounds[1] = -100000;
    m_bounds[2] = 100000;
    m_bounds[3] = -100000;
    m_bounds[4] = 100000;
    m_bounds[5] = -100000;
}
////////////////////////////////////////////////////////////////////////////////
ChannelDatavtkDataObject::ChannelDatavtkDataObject( const ChannelDatavtkDataObject& rhs )
    :
    ChannelData( rhs ),
    m_dobj( rhs.m_dobj )
{
    m_bounds[0] = 100000;
    m_bounds[1] = -100000;
    m_bounds[2] = 100000;
    m_bounds[3] = -100000;
    m_bounds[4] = 100000;
    m_bounds[5] = -100000;
}
////////////////////////////////////////////////////////////////////////////////
ChannelDatavtkDataObject::~ChannelDatavtkDataObject()
{
}
////////////////////////////////////////////////////////////////////////////////
vtkDataObject* ChannelDatavtkDataObject::GetDataObject()
{
    return m_dobj;
}
////////////////////////////////////////////////////////////////////////////////
unsigned int ChannelDatavtkDataObject::GetNumberOfPoints()
{
    lfx::vtk_utils::DataObjectHandler dataObjectHandler;
    lfx::vtk_utils::GetNumberOfPointsCallback* numberOfPointsCallback =
        new lfx::vtk_utils::GetNumberOfPointsCallback();
    dataObjectHandler.SetDatasetOperatorCallback( numberOfPointsCallback );
    dataObjectHandler.OperateOnAllDatasetsInObject( m_dobj );
    unsigned int numPoints = numberOfPointsCallback->GetNumberOfPoints();
    delete numberOfPointsCallback;
    return numPoints;
}
////////////////////////////////////////////////////////////////////////////////
double* ChannelDatavtkDataObject::GetBounds()
{
    GetBounds( m_bounds );
    return m_bounds;
}
////////////////////////////////////////////////////////////////////////////////
void ChannelDatavtkDataObject::GetBounds( double* bounds )
{
    lfx::vtk_utils::DataObjectHandler dataObjectHandler;
    lfx::vtk_utils::ComputeDataObjectBoundsCallback* boundsCallback =
        new lfx::vtk_utils::ComputeDataObjectBoundsCallback();
    dataObjectHandler.SetDatasetOperatorCallback( boundsCallback );
    dataObjectHandler.OperateOnAllDatasetsInObject( m_dobj );
    boundsCallback->GetDataObjectBounds( bounds );
    double bbDiagonal = boundsCallback->GetDataObjectBoundsDiagonal();
    delete boundsCallback;
}
////////////////////////////////////////////////////////////////////////////////
void ChannelDatavtkDataObject::GetScalarRange( std::string const scalarName, double* scalarRange )
{
    lfx::vtk_utils::DataObjectHandler dataObjectHandler;
    lfx::vtk_utils::ProcessScalarRangeCallback* processScalarRangeCbk =
        new lfx::vtk_utils::ProcessScalarRangeCallback();
    dataObjectHandler.SetDatasetOperatorCallback( processScalarRangeCbk );
    dataObjectHandler.OperateOnAllDatasetsInObject( m_dobj );
    processScalarRangeCbk->GetScalarRange( scalarName, scalarRange );
    delete processScalarRangeCbk;
}
////////////////////////////////////////////////////////////////////////////////
}
}
// lfx
}
