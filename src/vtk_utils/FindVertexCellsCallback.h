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
#pragma once
/*!\file FindVertexCellsCallback.h
FindVertexCellsCallback API.
*/

/*!\class ves::xplorer::util::FindVertexCellsCallback
*
*/
class vtkDataSet;

#include <vtkType.h>

#include <ves/VEConfig.h>
#include <ves/xplorer/util/DataObjectHandler.h>

#include <utility>
#include <vector>

namespace ves
{
namespace xplorer
{
namespace util
{
class VE_UTIL_EXPORTS FindVertexCellsCallback:
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
}// end of ves namesapce
