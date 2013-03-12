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
#ifndef LFX_EXTRACT_GEOMETRY_CALLBACK
#define LFX_EXTRACT_GEOMETRY_CALLBACK
/*!\file ExtractGeometryCallback.h
 * ExtractGeometryCallback API.
 * \class lfx::vtk_utils::ExtractGeometryCallback
 *
 */
class vtkDataSet;
class vtkMultiBlockDataSet;
class vtkUnstructuredGrid;
class vtkPolyData;

#include <latticefx/utils/vtk/Export.h>
#include <latticefx/utils/vtk/DataObjectHandler.h>


namespace lfx
{
namespace vtk_utils
{
class LATTICEFX_VTK_UTILS_EXPORT ExtractGeometryCallback:
    public DataObjectHandler::DatasetOperatorCallback
{
public:
    ///Constructor
    ExtractGeometryCallback();

    ///Destructor
    virtual ~ExtractGeometryCallback();

    ///The operation to do on each vtkDataSet in the vtkDataObject
    ///\param dataset The vtkDataSet to operate on
    virtual void OperateOnDataset( vtkDataSet* dataset );

    ///Get the Mean Cell BBox length
    vtkDataObject* GetDataset();
    ///The polydata representing of a surface
    void SetPolyDataSurface( vtkPolyData* surface );

protected:
    ///THe multibblock dataset holding the sub sample
    vtkMultiBlockDataSet* m_dataset;
    ///The number of blocks
    int m_dataCounter;
    ///The surface for the sample region
    vtkPolyData* m_surface;
    ///Bounds for the dataset
    double m_bbox[ 6 ];
};
}// end of util namesapce
}// end of xplorer namesapce
#endif
