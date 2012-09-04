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
#ifndef CREATE_DATAOBJECT_BBOX_GEODE_CALLBACK
#define  CREATE_DATAOBJECT_BBOX_GEODE_CALLBACK
/*!\file CreateDataObjectBBoxActorsCallback.h
CreateDataObjectBBoxActorsCallback API.
*/

/*!\class lfx::vtk_utils::CreateDataObjectBBoxActorsCallback
*
*/
class vtkDataSet;
class vtkActor;

#include <latticefx/utils/vtk/Export.h>
#include <latticefx/utils/vtk/DataObjectHandler.h>


#include <vector>
namespace lfx
{
namespace vtk_utils
{
class LATTICEFX_VTK_UTILS_EXPORT CreateDataObjectBBoxActorsCallback:
            public DataObjectHandler::DatasetOperatorCallback
{
public:
    ///Constructor
    CreateDataObjectBBoxActorsCallback();
    ///Destructor
    virtual ~CreateDataObjectBBoxActorsCallback();
    ///The operation to do on each vtkDataSet in the vtkDataObject
    ///\param dataset The vtkDataSet to operate on
    virtual void OperateOnDataset( vtkDataSet* dataset );

    ///Get the actors representing the bounding boxes of the datasets
    std::vector< vtkActor* >& GetBBoxActors();
protected:
    ///<The bounds of the vtkDataObject as vtkActors;
    std::vector< vtkActor* > m_bboxActors;
};
}// end of util namesapce
}// end of xplorer namesapce
#endif

