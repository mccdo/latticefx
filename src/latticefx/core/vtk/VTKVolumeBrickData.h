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
#include <latticefx/core/vtk/VTKVolumeBrickDataPtr.h>

#include <vtkGenericCell.h>
#include <vtkCellTreeLocator.h>
#include <vtkSmartPointer.h>
#include <vtkDoubleArray.h>

#include <boost/smart_ptr/shared_ptr.hpp>

class vtkDataArray;

namespace lfx
{
namespace core
{
namespace vtk
{

/** \class VTKVolumeBrickData VTKVolumeBrickData.h <latticefx/core/vtk/VTKVolumeBrickData.h>
\brief To be done
\details To be done
*/
class LATTICEFX_CORE_VTK_EXPORT VTKVolumeBrickData : public lfx::core::VolumeBrickData
{
protected:
	struct SThreadData
	{
		const VTKVolumeBrickData* pVBD;
		unsigned char* ptrPixels;
		int bytesPerPixel;
		osg::Vec3s brickNum;
		osg::Vec3s brickStart;
		osg::Vec3s brickEnd;
		osg::Vec3d vtkDelta;
		osg::Vec3d vtkMin;
		vtkDoubleArray* tuples;
		
		SThreadData()
		{
			tuples = vtkDoubleArray::New();
		}

		~SThreadData()
		{
			tuples->Delete();
		}

	};
    typedef boost::shared_ptr< SThreadData > SThreadDataPtr;

	class BrickThread
	{
	public:
		BrickThread( SThreadDataPtr pData) { m_pData = pData; }
		void operator()();

	protected:
		SThreadDataPtr m_pData;
	};

	friend class BrickThread;

	//
	// TexelDataCache Memory Usage
	//
	// 1. the number of storage slots will be totalNumBricks * brickRes
	// 2. then in the slot I store a cell id and a pointer to some cell data that is needed for final color calculation.
	//    when there is no cell (which happens a lot), I do not alloc the pointer and subsequent cell data.
	//    So if you had a vtk volume where every single one of our brick texel locations hit a valid cell of 
	//	  data then the memory usuage will be higher.
	//
	struct STexelData
	{
		float *weights;
		//std::vector<float> weights;
		vtkSmartPointer<vtkIdList> pointIds;
		unsigned char dsNum;  

		STexelData(int weightCount)
		{
			weights = new float[weightCount];
			//weights.reserve(weightCount);
			pointIds = vtkSmartPointer<vtkIdList>::New();
			dsNum = 0;
		}
		~STexelData()
		{
			delete [] weights;
		}
	};
	typedef boost::shared_ptr< STexelData > PTexelData;

	typedef std::vector< vtkDataArray* > DataArrayVector;
    typedef boost::shared_ptr< DataArrayVector > PDataArrayVectorPtr;

public:
	VTKVolumeBrickData(	DataSetPtr dataSet,
						bool prune = false,
						int dataNum = 0, 
						bool isScalar = true, 
						osg::Vec3s brickRes = osg::Vec3s(32,32,32), 
						int threadCount=4);
	virtual ~VTKVolumeBrickData();

	virtual void setDepth( const unsigned int depth );

    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const;
	

	bool isValid();

	void setDataNumber(int num) { m_dataNum = num; }
	void setIsScalar(bool isScalar) { m_isScalar = isScalar; }

	void cacheCreate(bool create) { m_cacheCreate = create; }
	void cacheUse(bool use) { m_cacheUse = use; }

	void debugLogOpen(const char *file);
	void debugLogCache(int x, int y, int z, int cachePos);
	void debugLogBrick(const osg::Vec3s& brickNum);
	void debugLog(const char *msg);

protected:

	osg::Vec4ub lerpDataInCell(vtkIdList* pointIds, double* weights, vtkDataArray* tuples, int whichValue, bool isScalar, int dsNum) const;
	osg::Vec4ub lerpPixelData(vtkDataArray* ptArray, double* weights, int npts, int whichValue, bool isScalar) const;

	osg::Vec4ub lerpDataInCell(vtkIdList* pointIds, float* weights, vtkDataArray* tuples, int whichValue, bool isScalar, int dsNum) const;
	osg::Vec4ub lerpPixelData(vtkDataArray* ptArray, float* weights, int npts, int whichValue, bool isScalar) const;

	void extractTuplesForScalar(vtkIdList* ptIds, vtkDataArray* tuples, int num, int dsNum) const;
	void extractTuplesForVector(vtkIdList* ptIds, vtkDataArray* tuples, int num, int dsNum) const;

	int findCell(double curPos[3], double pcoords[3], std::vector<double> *pweights, vtkSmartPointer<vtkGenericCell> &cell, int *pdsNum) const;
	PTexelData findCell(double curPos[3], int cacheLoc, vtkSmartPointer<vtkGenericCell> cell, std::vector<double> &weights) const;

	osg::Vec4ub getOutSideCellValue() const;

	void initMaxPts();
	void initDataArrays();
	bool initCellTrees();

	int getCacheLoc(int x, int y, int z, const osg::Vec3s &brickNum) const; 
	void initCache();

protected:
	osg::BoundingBoxd m_bbox;
	DataSetPtr m_dataSet;
	std::vector<vtkDataSet*> m_dataSets;

	std::vector< vtkSmartPointer<vtkCellTreeLocator> > m_cellLocators;
	bool m_isScalar;
	int m_dataNum;
	int m_maxPts;
	osg::Vec3s m_brickRes;
	std::vector< PDataArrayVectorPtr > m_dataArraysScalar;
	std::vector< PDataArrayVectorPtr > m_dataArraysVector;
	int m_threadCount;
	int m_bricksDone;

	std::vector<PTexelData> m_texelDataCache;
	bool m_cacheUse;
	bool m_cacheCreate;
	FILE *m_pstLogDbg;
};


// vtk
}
// core
}
// lfx
}


// __LFX_CORE_VTK_VOLUME_BRICK_DATA_H__
#endif
