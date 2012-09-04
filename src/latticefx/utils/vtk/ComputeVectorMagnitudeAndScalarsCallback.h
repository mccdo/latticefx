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
#ifndef COMPUTE_VECTOR_MAGNITUDE_AND_SCALARS_CALLBACK
#define COMPUTE_VECTOR_MAGNITUDE_AND_SCALARS_CALLBACK
/*!\file ComputeVectorMagnitudeAndScalarsCallback.h
ComputeVectorMagnitudeAndScalarsCallback API.
*/

/*!\class lfx::vtk_utils::ComputeVectorMagnitudeAndScalarsCallback
*
*/
class vtkDataSet;

#include <latticefx/utils/vtk/Export.h>
#include <latticefx/utils/vtk/DataObjectHandler.h>


namespace lfx
{
namespace vtk_utils
{
class LATTICEFX_VTK_UTILS_EXPORT ComputeVectorMagnitudeAndScalarsCallback:
            public DataObjectHandler::DatasetOperatorCallback
{
public:
    ///Constructor
    ComputeVectorMagnitudeAndScalarsCallback();
    ///Destructor
    virtual ~ComputeVectorMagnitudeAndScalarsCallback()
    {};
    ///The operation to do on each vtkDataSet in the vtkDataObject
    ///\param dataset The vtkDataSet to operate on
    virtual void OperateOnDataset( vtkDataSet* dataset );

    ///Get the Mean Cell BBox length
    ///\param vMagRange The min and max magnitude of the active vector
    //void GetVectorMagnitudeRange( double*& vMagRange );
protected:
    ///double m_magnitudeRange[2];///<The mean cell bbox length;
};
}// end of util namesapce
}// end of xplorer namesapce
#endif

