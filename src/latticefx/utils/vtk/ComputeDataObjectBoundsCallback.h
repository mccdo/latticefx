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
#ifndef LFX_COMPUTE_DATAOBJECT_BOUNDS_CALLBACK
#define LFX_COMPUTE_DATAOBJECT_BOUNDS_CALLBACK
/*!\file ComputeDataObjectBoundsCallback.h
ComputeDataObjectBoundsCallback API.
*/

/*!\class lfx::vtk_utils::ComputeDataObjectBoundsCallback
*
*/
class vtkDataSet;

#include <latticefx/utils/vtk/Export.h>
#include <latticefx/utils/vtk/DataObjectHandler.h>


namespace lfx
{
namespace vtk_utils
{
class LATTICEFX_VTK_UTILS_EXPORT ComputeDataObjectBoundsCallback:
            public DataObjectHandler::DatasetOperatorCallback
{
public:
    ///Constructor
    ComputeDataObjectBoundsCallback();
    ///Destructor
    virtual ~ComputeDataObjectBoundsCallback()
    {};
    ///The operation to do on each vtkDataSet in the vtkDataObject
    ///\param dataset The vtkDataSet to operate on
    virtual void OperateOnDataset( vtkDataSet* dataset );

    ///Get the bounds
    ///\param  boundsv The bounding box as:\n
    /// min-x, max-x, min-y, max-y, min-z, max-z
    void GetDataObjectBounds( double* bounds );

    ///Get the diagonal of the calculated bbox
    double GetDataObjectBoundsDiagonal();
protected:
    double m_bounds[6];///<The bounds of the vtkDataObject;
};
}// end of util namesapce
}// end of xplorer namesapce
#endif

