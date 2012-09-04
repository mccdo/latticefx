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
#ifndef ACTIVE_DATA_INFORMATION_CALLBACK
#define  ACTIVE_DATA_INFORMATION_CALLBACK
/*!\file ActiveDataInformationCallback.h
ActiveDataInformationCallback API.
*/

/*!\class lfx::vtk_utils::ActiveDataInformationCallback
*
*/
class vtkDataSet;

#include <latticefx/utils/vtk/Export.h>
#include <latticefx/utils/vtk/DataObjectHandler.h>

#include <vector>
#include <string>
#include <map>
namespace lfx
{
namespace vtk_utils
{
class LATTICEFX_VTK_UTILS_EXPORT ActiveDataInformationCallback:
            public DataObjectHandler::DatasetOperatorCallback
{
public:
    ///Constructor
    ActiveDataInformationCallback();
    ///Destructor
    virtual ~ActiveDataInformationCallback();
    ///The operation to do on each vtkDataSet in the vtkDataObject
    ///\param dataset The vtkDataSet to operate on
    virtual void OperateOnDataset( vtkDataSet* dataset );

    ///Set the active vector or scalar
    ///\param name The name of the scalar or vector to make active
    ///\param isVector Flag determining vector or scalar data
    void SetActiveDataName( std::string name, bool isVector = false );

    ///Get the name of the active scalar or vector
    ///\param isVector Flag determining vector or scalar data
    std::string GetActiveDataName( bool isVector = false );

    ///Get active scalar or vector
    ///\param  isVector Flag determining vector or scalar data to retrieve
    std::string GetActiveData( bool isVector = false );

protected:
    std::string m_activeScalar;///<The active scalar
    std::string m_activeVector;///<The active vector
};
}// end of util namesapce
}// end of xplorer namesapce
#endif

