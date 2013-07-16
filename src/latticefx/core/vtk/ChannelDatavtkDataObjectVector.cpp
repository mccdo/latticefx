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

#include <latticefx/core/vtk/ChannelDatavtkDataObjectVector.h>

//#include <latticefx/utils/vtk/ComputeDataObjectBoundsCallback.h>
//#include <latticefx/utils/vtk/GetNumberOfPointsCallback.h>
//#include <latticefx/utils/vtk/DataObjectHandler.h>
//#include <latticefx/utils/vtk/ProcessScalarRangeCallback.h>

#include <vtkDataObject.h>

namespace lfx
{
namespace core
{
namespace vtk
{
////////////////////////////////////////////////////////////////////////////////
ChannelDatavtkDataObjectVector::ChannelDatavtkDataObjectVector( std::vector< vtkDataObject* > dobj, const std::string& name, const std::string& logName )
    :
    ChannelData( name, logName ),
    m_dobj( dobj )
{
    m_bounds[0] =  100000;
    m_bounds[1] = -100000;
    m_bounds[2] =  100000;
    m_bounds[3] = -100000;
    m_bounds[4] =  100000;
    m_bounds[5] = -100000;
}
////////////////////////////////////////////////////////////////////////////////
ChannelDatavtkDataObjectVector::ChannelDatavtkDataObjectVector( const ChannelDatavtkDataObjectVector& rhs )
    :
    ChannelData( rhs ),
    m_dobj( rhs.m_dobj )
{
    m_bounds[0] =  100000;
    m_bounds[1] = -100000;
    m_bounds[2] =  100000;
    m_bounds[3] = -100000;
    m_bounds[4] =  100000;
    m_bounds[5] = -100000;
}
////////////////////////////////////////////////////////////////////////////////
ChannelDatavtkDataObjectVector::~ChannelDatavtkDataObjectVector()
{
    ;
}
////////////////////////////////////////////////////////////////////////////////
std::vector< vtkDataObject* > ChannelDatavtkDataObjectVector::GetDataObjectVector()
{
    return m_dobj;
}
////////////////////////////////////////////////////////////////////////////////
/*unsigned int ChannelDatavtkDataObjectVector::GetNumberOfPoints()
{
    lfx::vtk_utils::FindVertexCellsCallback* findVertexCellsCbk =
        new lfx::vtk_utils::FindVertexCellsCallback();
    lfx::vtk_utils::DataObjectHandler dataObjectHandler();
    dataObjectHandler.SetDatasetOperatorCallback( findVertexCellsCbk );

    size_t maxNumPoints = 0;
    for( size_t i = 0; i < m_dobj.size(); ++i )
    {
        vtkDataObject* tempDataSet = m_dobj.at( i )->GetDataSet();

        dataObjectHandler->OperateOnAllDatasetsInObject( tempDataSet );
        std::vector< std::pair< vtkIdType, double* > > tempCellGroups =
        findVertexCellsCbk->GetVertexCells();
        m_pointCollection.push_back( tempCellGroups );
        findVertexCellsCbk->ResetPointGroup();
        if( maxNumPoints < tempCellGroups.size() )
        {
            maxNumPoints = tempCellGroups.size();
        }
    }
    delete findVertexCellsCbk;
    return 0;
}*/
////////////////////////////////////////////////////////////////////////////////
/*double* ChannelDatavtkDataObjectVector::GetBounds()
{
    GetBounds( m_bounds );
    return m_bounds;
}
////////////////////////////////////////////////////////////////////////////////
void ChannelDatavtkDataObjectVector::GetBounds( double* bounds )
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
void ChannelDatavtkDataObjectVector::GetScalarRange( std::string const scalarName, double* scalarRange )
{
    lfx::vtk_utils::DataObjectHandler dataObjectHandler;
    lfx::vtk_utils::ProcessScalarRangeCallback* processScalarRangeCbk =
        new lfx::vtk_utils::ProcessScalarRangeCallback();
    dataObjectHandler.SetDatasetOperatorCallback( processScalarRangeCbk );
    dataObjectHandler.OperateOnAllDatasetsInObject( m_dobj );
    processScalarRangeCbk->GetScalarRange( scalarName, scalarRange );
    delete processScalarRangeCbk;
}*/
////////////////////////////////////////////////////////////////////////////////
}
}
// lfx
}
