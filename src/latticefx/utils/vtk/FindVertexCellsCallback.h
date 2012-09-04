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
#pragma once
/*!\file FindVertexCellsCallback.h
FindVertexCellsCallback API.
*/

/*!\class lfx::vtk_utils::FindVertexCellsCallback
*
*/
class vtkDataSet;

#include <vtkType.h>

#include <latticefx/utils/vtk/Export.h>
#include <latticefx/utils/vtk/DataObjectHandler.h>

#include <utility>
#include <vector>

namespace lfx
{
namespace vtk_utils
{
class LATTICEFX_VTK_UTILS_EXPORT FindVertexCellsCallback:
            public DataObjectHandler::DatasetOperatorCallback
{
public:
    ///Constructor
    FindVertexCellsCallback();
    ///Destructor
    virtual ~FindVertexCellsCallback()
    {;}
    ///The operation to do on each vtkDataSet in the vtkDataObject
    ///\param dataset The vtkDataSet to operate on
    virtual void OperateOnDataset( vtkDataSet* dataset );

    ///Get the vertex cells from this dataset
    ///\return The vector with the points for this dataset, the first id is 
    ///the cell id and the second id is the point id
    //std::vector< std::pair< vtkIdType, vtkIdType > > GetVertexCells();
    std::vector< std::pair< vtkIdType, double* > > GetVertexCells();

    ///Reset the map so that the callback can be used again with out
    ///reallocating memory for the callback.
    void ResetPointGroup();
protected:
    //std::vector< std::pair< vtkIdType, vtkIdType > >  m_pointGroup;
    std::vector< std::pair< vtkIdType, double* > >  m_pointGroup;
};
}// end of util namesapce
}// end of xplorer namesapce
