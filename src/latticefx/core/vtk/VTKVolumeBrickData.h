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

#ifndef __LFX_CORE_VTK_VOLUME_BRICK_DATA_H__
#define __LFX_CORE_VTK_VOLUME_BRICK_DATA_H__ 1

#include <latticefx/core/vtk/Export.h>
#include <latticefx/core/HierarchyUtils.h>
#include <latticefx/core/vtk/DataSet.h>
#include <latticefx/core/vtk/DataSetPtr.h>

#include <vtkCellTreeLocator.h>
#include <vtkSmartPointer.h>


class vtkDataArray;

namespace lfx {
namespace core {
namespace vtk {

/** \class VTKVolumeBrickData VTKVolumeBrickData.h <latticefx/core/vtk/VTKVolumeBrickData.h>
\brief To be done
\details To be done
*/
class LATTICEFX_CORE_VTK_EXPORT VTKVolumeBrickData : public lfx::core::VolumeBrickData
{
public:
	VTKVolumeBrickData(	DataSetPtr dataSet,
						bool prune = false,
						int dataNum = 0, 
						bool isScalar = true, 
						osg::Vec3s brickRes = osg::Vec3s(32,32,32), 
						osg::Vec3s totalNumBricks = osg::Vec3s(8,8,8));

    /** To be done. Override this. */
    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const;

	bool isValid();

protected:
	osg::Vec4ub lerpDataInCell(vtkGenericCell* cell, double* weights, int whichValue, bool isScalar) const;
	osg::Vec4ub lerpPixelData(vtkDataArray* ptArray, double* weights, int npts, int whichValue, bool isScalar) const;
	void extractTuplesForVector(vtkIdList* ptIds, vtkDataArray* vector, int whichVector) const;
	void extractTuplesForScalar(vtkIdList* ptIds, vtkDataArray* scalar, int whichScalar) const;

	unsigned char getOutSideCellValue() const;

protected:
	DataSetPtr m_dataSet;
	vtkDataSet* m_pds;
	vtkSmartPointer<vtkCellTreeLocator> m_cellLocator;
	bool m_isScalar;
	int m_dataNum;
	int m_nPtDataArrays;
	osg::Vec3s m_brickRes;
	osg::Vec3s m_totalNumBricks;
};


// vtk
}
// core
}
// lfx
}


// __LFX_CORE_VTK_VOLUME_BRICK_DATA_H__
#endif
