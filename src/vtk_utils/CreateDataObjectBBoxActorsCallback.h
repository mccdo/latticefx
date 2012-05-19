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
#ifndef CREATE_DATAOBJECT_BBOX_GEODE_CALLBACK
#define  CREATE_DATAOBJECT_BBOX_GEODE_CALLBACK
/*!\file CreateDataObjectBBoxActorsCallback.h
CreateDataObjectBBoxActorsCallback API.
*/

/*!\class ves::xplorer::util::CreateDataObjectBBoxActorsCallback
*
*/
class vtkDataSet;
class vtkActor;

#include <ves/VEConfig.h>
#include <ves/xplorer/util/DataObjectHandler.h>


#include <vector>
namespace ves
{
namespace xplorer
{
namespace util
{
class VE_UTIL_EXPORTS CreateDataObjectBBoxActorsCallback:
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
}// end of ves namesapce
#endif

