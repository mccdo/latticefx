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
#ifndef LFX_GET_NUMBER_OF_POINTS_CALLBACK
#define LFX_GET_NUMBER_OF_POINTS_CALLBACK
/*!\file GetNumberOfPointsCallback.h
GetNumberOfPointsCallback API.
*/

/*!\class lfX::vtk_utils::GetNumberOfPointsCallback
*
*/
class vtkDataSet;

#include <latticefx/utils/vtk/Export.h>
#include <latticefx/utils/vtk/DataObjectHandler.h>


namespace lfx
{
namespace vtk_utils
{
class LATTICEFX_VTK_UTILS_EXPORT GetNumberOfPointsCallback:
    public DataObjectHandler::DatasetOperatorCallback
{
public:
    ///Constructor
    GetNumberOfPointsCallback();
    ///Destructor
    virtual ~GetNumberOfPointsCallback()
    {};
    ///The operation to do on each vtkDataSet in the vtkDataObject
    ///\param dataset The vtkDataSet to operate on
    virtual void OperateOnDataset( vtkDataSet* dataset );

    ///Get the total number of points for the vtkDataObject
    ///\note This does a dumb sum in that interior boundary points\n
    ///are counted twice...
    unsigned int GetNumberOfPoints();
protected:
    unsigned int m_numberOfPoints;///<The mean cell bbox length;
};
}// end of util namesapce
}// end of xplorer namesapce
#endif

