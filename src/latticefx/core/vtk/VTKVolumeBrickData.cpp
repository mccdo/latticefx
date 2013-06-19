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

#include <latticefx/core/vtk/VTKVolumeBrickData.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osg/Image>
#include <osg/io_utils>

#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>

#include <boost/thread.hpp>

//static FILE* s_pfDebug = NULL;

namespace lfx
{
namespace core
{
namespace vtk
{

////////////////////////////////////////////////////////////////////////////////
VTKVolumeBrickData::VTKVolumeBrickData(DataSetPtr dataSet,
									   bool prune,
									   int dataNum,
									   bool isScalar,
									   osg::Vec3s brickRes,
									   int threadCount)
    :
    VolumeBrickData(prune)
{
	m_dataSet = dataSet;
	m_dataNum = dataNum;
	m_isScalar = isScalar;
	m_brickRes = brickRes;
	m_maxPts = 0; 
	m_bbox.init();

	m_threadCount = threadCount;
	if (m_threadCount <= 0) m_threadCount = 1;

	if (!initCellTrees()) return;

	initMaxPts();
	initDataArrays();
	

	m_cacheUse = true;
	m_cacheCreate = true;
	m_bricksDone = 0;
	m_brickCount = 0;


	m_pstLogDbg = NULL;

	//if (!s_pfDebug) s_pfDebug = fopen("D:/skewmatrix/latticefx/bld/src/apps/shapeCreator/log_prune.txt", "wt");
}

VTKVolumeBrickData::~VTKVolumeBrickData()
{
	if (m_pstLogDbg) fclose(m_pstLogDbg);
	//if (s_pfDebug) fclose(s_pfDebug);
	//s_pfDebug = NULL;
}

////////////////////////////////////////////////////////////////////////////////
void VTKVolumeBrickData::setDepth( const unsigned int depth )
{
	VolumeBrickData::setDepth(depth);
	initCache();
}

////////////////////////////////////////////////////////////////////////////////
bool VTKVolumeBrickData::isValid()
{
	if (!m_dataSets.size()) return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
osg::Image* VTKVolumeBrickData::getBrick( const osg::Vec3s& brickNum ) const
{
	// make sure the brick is in range
	if (brickNum[0] >= _numBricks[0] || brickNum[1] >= _numBricks[1] || brickNum[2] >= _numBricks[2])
	{
		return NULL;
	}

    {
        // If we already generated an image for this brickNum and
        // it's still in the cache, return the cached imaged.
        osg::Image* image( getCachedImage( brickNum ) );
        if( image != NULL )
            return( image );
    }

	((VTKVolumeBrickData *)this)->debugLogBrick(brickNum);

	//if (!m_pds) return NULL;
	if (!m_cellLocators.size()) return NULL;

	bool brickCached = isBrickCached(brickNum);

	int totbricks = (_numBricks[0] * _numBricks[1] * _numBricks[2]) - 1;
	std::string logname = "lfx.core.hier.vtk";
	std::stringstream ss;

	ss << "brick: (" << brickNum << "), " << "depth: " << _depth << ", " << m_bricksDone << " of " << totbricks << ", cached: " << brickCached;
	LFX_INFO_STATIC( logname, ss.str() );
	//printf("%s\n", ss.str().c_str());
	//if (s_pfDebug) { fprintf(s_pfDebug, "%s\n", ss.str().c_str()); }

	// NOTE: IF m_isScalar == FALSE, then a vector type requires 4 values for each pixel, not 1 as in the case of scalar.
	// 
	GLint textureFrmt = GL_LUMINANCE;
	int bytesPerPixel = 1;
	if (!m_isScalar) 
	{
		textureFrmt = GL_RGBA;
		bytesPerPixel = 4;
	}
	unsigned char* data( new unsigned char[ m_brickRes[0] * m_brickRes[1] * m_brickRes[2] * bytesPerPixel ] );
	unsigned char* ptr( data );

	// 0,0,0 is the left, top, front cube and we have 8x8x8 boxes
	// or whatever m_totalNumBoxes is set to
		
	//a bounding box
	//double bbox[6] = {0,0,0,0,0,0};
	//m_pds->GetBounds(bbox);

	// vtkBrickSize - the size of our brick in vtk coordniates
	osg::Vec3d vtkBrickSize, vtkDelta;
	vtkBrickSize.x() = fabs(m_bbox.xMax() - m_bbox.xMin()) / (_numBricks.x());
	vtkBrickSize.y() = fabs(m_bbox.yMax() - m_bbox.yMin()) / (_numBricks.y());
	vtkBrickSize.z() = fabs(m_bbox.zMax() - m_bbox.zMin()) / (_numBricks.z());

	// vtkDelta  - the abount of space to move in vtk coorniates for each pixel, or m_brickRes
    vtkDelta.x()   = vtkBrickSize.x() / m_brickRes.x();
	vtkDelta.y()   = vtkBrickSize.y() / m_brickRes.y();
	vtkDelta.z()   = vtkBrickSize.z() / m_brickRes.z();

	// need to compute the bounding box for this brick in the vtk dataset coordinate space
	osg::Vec3d min, max;
	min[0] = m_bbox.xMin() + brickNum[0] * vtkBrickSize[0];
	min[1] = m_bbox.yMin() + brickNum[1] * vtkBrickSize[1];
	min[2] = m_bbox.zMin() + brickNum[2] * vtkBrickSize[2];
	max = min + vtkBrickSize;

	// compute brick delta for each thread (how much of the brick will each thread work on
	double brickThreadDeltaX = (double)m_brickRes[0] / (double)m_threadCount;	
	double curBrickEndX = 0;
	boost::thread_group group;

	std::vector<SThreadDataPtr> theadData;
	for (int i=0; i<m_threadCount; i++)
	{
		SThreadDataPtr pData = SThreadDataPtr( new SThreadData() );
		theadData.push_back(pData);
		pData->pVBD = this;
		pData->ptrPixels = ptr;
		pData->bytesPerPixel = bytesPerPixel;
		pData->brickNum = brickNum;
		pData->brickCached = brickCached;

		// update brick coordinates
		pData->brickStart[0] = curBrickEndX;
		pData->brickStart[1] = 0;
		pData->brickStart[2] = 0;
		curBrickEndX += brickThreadDeltaX;
		pData->brickEnd[0] = curBrickEndX;
		pData->brickEnd[1] = m_brickRes[1];
		pData->brickEnd[2] = m_brickRes[2];

		// avoid a round off error, make sure to cover the last x location with the last thread
		if (i == (m_threadCount - 1))
		{
			pData->brickEnd[0] = m_brickRes[0];
		}

		// update vtk coordinates
		pData->vtkDelta = vtkDelta;
		pData->vtkMin[0] = min[0];
		pData->vtkMin[1] = min[1];
		pData->vtkMin[2] = min[2];

		group.create_thread(BrickThread(pData));
	}

	group.join_all();

	// check if canceled
	if (checkCancel())
	{
		//throw std::runtime_error("vtk volume brick creation canceled");
		return NULL;
	}

	((VTKVolumeBrickData *)this)->cacheBrick(brickNum, brickCached);

	if (_prune)
	{
		int totalCellHits = 0;
		for (unsigned int i=0; i<theadData.size(); i++)
		{
			totalCellHits += theadData[i]->hits;
		}

		//printf("Cell Hits: %d\n", totalCellHits);
		if (totalCellHits <= 0) 
		{
			ss.str(std::string());
			//ss << "prune brick: (" << brickNum << "), " << "depth: " << _depth;
			ss << "PRUNE - brick: (" << brickNum << "), " << "depth: " << _depth << ", " << m_bricksDone << " of " << totbricks; 
			LFX_INFO_STATIC( logname, ss.str() );
			//printf("%s\n", ss.str().c_str());

			//if (s_pfDebug) { fprintf(s_pfDebug, "%s\n", ss.str().c_str()); }

			((VTKVolumeBrickData *)this)->m_bricksDone++;
            // Cache a NULL Image.
            cacheImage( brickNum, NULL );
			return( NULL );
		}
	}

	// create an image with our data and return it
	osg::ref_ptr< osg::Image > image( new osg::Image() );
    image->setImage( m_brickRes[0], m_brickRes[1], m_brickRes[2],
        textureFrmt, textureFrmt, GL_UNSIGNED_BYTE,
        (unsigned char*) data, osg::Image::USE_NEW_DELETE );

    // Cache the computed Image.
    cacheImage( brickNum, image.get() );

	((VTKVolumeBrickData *)this)->m_bricksDone++;
	return( image.release() );
}


////////////////////////////////////////////////////////////////////////////////
void VTKVolumeBrickData::BrickThread::operator()()
{
	if (!m_pData.get()) return;

	vtkSmartPointer<vtkGenericCell> cell = vtkSmartPointer<vtkGenericCell>::New();
	vtkIdType cellId;
	double pcoords[3], curPos[3];
	std::vector<double> weights(m_pData->pVBD->m_maxPts); // need to find out max points in a cell for the whole dataset
	PTexelData pdata(new STexelData(m_pData->pVBD->m_maxPts));
	int subId = 0;
	osg::Vec4ub value;
	int debugNumPts = 0;
	bool haveCache = false;
	int dataSetNum = 0;

	unsigned char* ptr = NULL;

	// start at left, bottom, back
	curPos[2] = m_pData->vtkMin.z() + (m_pData->vtkDelta.z() * m_pData->brickStart.z());
	for (int z = m_pData->brickStart.z(); z < m_pData->brickEnd.z(); z++)
	{
		// new depth slice so start at the bottom most point
		curPos[1] = m_pData->vtkMin.y() + (m_pData->vtkDelta.y() * m_pData->brickStart.y());


		for (int y = m_pData->brickStart.y(); y < m_pData->brickEnd.y(); y++)
		{
			// new scanline start back at left most point;
			curPos[0] = m_pData->vtkMin.x() + (m_pData->vtkDelta.x() * m_pData->brickStart.x());

			// get the correct line
			ptr = m_pData->ptrPixels + (z * m_pData->pVBD->m_brickRes[0]*m_pData->pVBD->m_brickRes[1] * m_pData->bytesPerPixel); // get to the correct slice;
			ptr += y * m_pData->pVBD->m_brickRes[0] * m_pData->bytesPerPixel; // get to the correct line in the slice
			ptr += m_pData->brickStart.x() * m_pData->bytesPerPixel; // get to the correct x starting position;


			for (int x = m_pData->brickStart.x(); x < m_pData->brickEnd.x(); x++)
			{
				if (m_pData->pVBD->checkCancel()) { return; } // canceled;

				if (m_pData->pVBD->m_cacheUse)
				{
								int cacheLoc = m_pData->pVBD->getCacheLoc(x, y, z, m_pData->brickNum);
								PTexelData cache;

								if (m_pData->pVBD->m_cacheCreate)
								{
									//cache = m_pData->pVBD->findCell(curPos, cacheLoc, cell, pdata);
									cache = m_pData->pVBD->findCell(curPos, cacheLoc, cell, weights, m_pData->brickCached);
								}
								else
								{ 
									cache = m_pData->pVBD->m_texelDataCache[cacheLoc];
								} 

								((VTKVolumeBrickData *)m_pData->pVBD)->debugLogCache(x, y, z, cacheLoc);

								if (!cache.get())
								{
									value = m_pData->pVBD->getOutSideCellValue(); 
								}
								else
								{
									value = m_pData->pVBD->lerpDataInCell(cache->pointIds, &cache->weights[0], m_pData->tuples, m_pData->pVBD->m_dataNum, m_pData->pVBD->m_isScalar, cache->dsNum); 
									m_pData->hits++;
								}
				}
				else
				{
								cellId = m_pData->pVBD->findCell(curPos, pcoords, &weights, cell, &dataSetNum);
								if (cellId < 0)
								{
									value = m_pData->pVBD->getOutSideCellValue(); 
								}
								else
								{
									// cell cache is not showing any signs of a speed up
									//if (m_cellCache == cellId) haveCache = false;
									value = m_pData->pVBD->lerpDataInCell(cell->GetPointIds(), &weights[0], m_pData->tuples, m_pData->pVBD->m_dataNum, m_pData->pVBD->m_isScalar, dataSetNum); 
									m_pData->hits++;
								}
				}

				// copy values into the image buffer
				*ptr = value[0];
				ptr++;

				if (!m_pData->pVBD->m_isScalar)
				{
					*ptr = value[1];
					ptr++;
					*ptr = value[2];
					ptr++;
					*ptr = value[3];
					ptr++;
				}

				curPos[0] += m_pData->vtkDelta[0];
				debugNumPts++;
			}

			// jump to next vertical scanline
			curPos[1] += m_pData->vtkDelta[1];
			
		}

		// jump to next depth slice
		curPos[2] += m_pData->vtkDelta[2];
	}
}

////////////////////////////////////////////////////////////////////////////////
void VTKVolumeBrickData::debugLogBrick(const osg::Vec3s& brickNum)
{
	if (!m_pstLogDbg) return;

	fprintf(m_pstLogDbg, "\nstart of brick (%d,%d,%d)\n", brickNum[0], brickNum[1], brickNum[2]);
}

////////////////////////////////////////////////////////////////////////////////
void VTKVolumeBrickData::debugLogOpen(const char *file)
{
	if (m_pstLogDbg) fclose(m_pstLogDbg);

	m_pstLogDbg = fopen(file, "wt");
}

////////////////////////////////////////////////////////////////////////////////
void VTKVolumeBrickData::debugLogCache(int x, int y, int z, int cachePos)
{
	if (!m_pstLogDbg) return;

	PTexelData cache = m_texelDataCache[cachePos];
	std::stringstream ss;
	
	fprintf(m_pstLogDbg, "x:%d y:%d z:%d cache:%d\n", x, y, z, cachePos);

	
	//fprintf(m_pstLogDbg, "id:%d x:%.2f y:%.2f z:%.2f ds:%d\n", 
	//		cache->cellid, cache->pcoords[0], cache->pcoords[1], cache->pcoords[2], cache->dsNum);

	if (cache.get())
	{
		/*
		ss << "weights: " << cache->weights.size() << " ";
		for (unsigned int i=0; i<cache->weights.size(); i++)
		{
			ss << cache->weights[i] << ",";
		}
		*/
	}
	else
	{
		ss << "weights: none";
	}

	fprintf(m_pstLogDbg, "%s\n", ss.str().c_str());
	//vtkSmartPointer<vtkGenericCell> cell;	
}

////////////////////////////////////////////////////////////////////////////////
void VTKVolumeBrickData::debugLog(const char *msg)
{
	if (!m_pstLogDbg) return;
	fprintf(m_pstLogDbg, "%s\n", msg);
}

////////////////////////////////////////////////////////////////////////////////
osg::Vec4ub VTKVolumeBrickData::lerpDataInCell(vtkIdList* pointIds, double* weights, vtkDataArray* tuples, int whichValue, bool isScalar, int dsNum) const
{
	osg::Vec4ub value;

	//list of point ids in cell
	//vtkIdList* pointIds = cell->GetPointIds();
   
	//number of verts in the cell
	//int nCellPts = cell->GetNumberOfPoints();
	int nCellPts = pointIds->GetNumberOfIds();
   
	if (isScalar)
	{
		if (tuples->GetNumberOfComponents() != 1)
		{
			tuples->SetNumberOfComponents(1);
			tuples->SetNumberOfTuples(nCellPts);
		}
		else if (tuples->GetNumberOfTuples() != nCellPts)
		{
			tuples->SetNumberOfTuples(nCellPts);
		}

		extractTuplesForScalar(pointIds, tuples, whichValue, dsNum);
		value = lerpPixelData(tuples, weights, nCellPts, whichValue, isScalar);
	}
	else
	{
		if (tuples->GetNumberOfComponents() != 3)
		{
			tuples->SetNumberOfComponents(3);
			tuples->SetNumberOfTuples(nCellPts);
		}
		else if (tuples->GetNumberOfTuples() != nCellPts)
		{
			tuples->SetNumberOfTuples(nCellPts);
		}

		extractTuplesForVector(pointIds, tuples, whichValue, dsNum);
		value = lerpPixelData(tuples, weights, nCellPts, whichValue, isScalar);
   }

   return value;
}

////////////////////////////////////////////////////////////////////////////////
osg::Vec4ub VTKVolumeBrickData::lerpDataInCell(vtkIdList* pointIds, float* weights, vtkDataArray* tuples, int whichValue, bool isScalar, int dsNum) const
{
	osg::Vec4ub value;

	//list of point ids in cell
	//vtkIdList* pointIds = cell->GetPointIds();
   
	//number of verts in the cell
	//int nCellPts = cell->GetNumberOfPoints();
	int nCellPts = pointIds->GetNumberOfIds();
   
	if (isScalar)
	{
		if (tuples->GetNumberOfComponents() != 1)
		{
			tuples->SetNumberOfComponents(1);
			tuples->SetNumberOfTuples(nCellPts);
		}
		else if (tuples->GetNumberOfTuples() != nCellPts)
		{
			tuples->SetNumberOfTuples(nCellPts);
		}

		extractTuplesForScalar(pointIds, tuples, whichValue, dsNum);
		value = lerpPixelData(tuples, weights, nCellPts, whichValue, isScalar);
	}
	else
	{
		if (tuples->GetNumberOfComponents() != 3)
		{
			tuples->SetNumberOfComponents(3);
			tuples->SetNumberOfTuples(nCellPts);
		}
		else if (tuples->GetNumberOfTuples() != nCellPts)
		{
			tuples->SetNumberOfTuples(nCellPts);
		}

		extractTuplesForVector(pointIds, tuples, whichValue, dsNum);
		value = lerpPixelData(tuples, weights, nCellPts, whichValue, isScalar);
   }

   return value;
}

////////////////////////////////////////////////////////////////////////////////
osg::Vec4ub VTKVolumeBrickData::lerpPixelData(vtkDataArray* tuples, double* weights, int npts, int whichValue, bool isScalar) const
{
	osg::Vec4ub data(0,0,0,0);

	// vector data
   if (!isScalar) 
   {
      double vector[4];
      vector[0] = 0;
      vector[1] = 0;
      vector[2] = 0;
      double vectorData[3];
      for(int j = 0; j < npts; j++)
	  {
         tuples->GetTuple(j, vectorData);
         //the weighted sum of the velocities
         vector[0] += vectorData[0]*weights[j];
         vector[1] += vectorData[1]*weights[j];
         vector[2] += vectorData[2]*weights[j];
      }
      //calulate the magnitude
      double vMag = 0;
      double iMag = 0;
      vMag = sqrt(vector[0]*vector[0]
                +vector[1]*vector[1]
                +vector[2]*vector[2]);
         
      //inverse magnitude 
      if(vMag >= 1e-6){
         iMag = 1.0/vMag;
      }else{
         iMag = 0;
      }
      
      //normaliz e data
      vector[0] *= iMag;
      vector[1] *= iMag;
      vector[2] *= iMag;
      vector[3] = vMag;

      //quantize data
      vector[0] = 255*((vector[0] + 1)*.5);
      vector[1] = 255*((vector[1] + 1)*.5);
      vector[2] = 255*((vector[2] + 1)*.5);
      
	  data[0] = vector[0];
	  data[1] = vector[1];
	  data[2] = vector[2];
	  data[3] = vector[3];
   }
   else // scalar data
   {
      double scalarData;
      double scalar = 0;
      for(int j = 0; j < npts; j++)
	  {
         tuples->GetTuple(j,&scalarData);
         scalar += scalarData*weights[j];
      }

	  if (scalar > 0 || scalar < 0)
	  {
		  int idebug = 1;
	  }

	  if (scalar > 1)
	  {
		  scalar = 1;
	  }
	  else if (scalar < 0)
	  {
		  scalar = 0;
	  }

	  data[0] = (unsigned char) (255.0 * scalar);
   }

   return data;
}

////////////////////////////////////////////////////////////////////////////////
osg::Vec4ub VTKVolumeBrickData::lerpPixelData(vtkDataArray* tuples, float* weights, int npts, int whichValue, bool isScalar) const
{
	osg::Vec4ub data(0,0,0,0);

	// vector data
   if (!isScalar) 
   {
      double vector[4];
      vector[0] = 0;
      vector[1] = 0;
      vector[2] = 0;
      double vectorData[3];
      for(int j = 0; j < npts; j++)
	  {
         tuples->GetTuple(j, vectorData);
         //the weighted sum of the velocities
         vector[0] += vectorData[0]*weights[j];
         vector[1] += vectorData[1]*weights[j];
         vector[2] += vectorData[2]*weights[j];
      }
      //calulate the magnitude
      double vMag = 0;
      double iMag = 0;
      vMag = sqrt(vector[0]*vector[0]
                +vector[1]*vector[1]
                +vector[2]*vector[2]);
         
      //inverse magnitude 
      if(vMag >= 1e-6){
         iMag = 1.0/vMag;
      }else{
         iMag = 0;
      }
      
      //normaliz e data
      vector[0] *= iMag;
      vector[1] *= iMag;
      vector[2] *= iMag;
      vector[3] = vMag;

      //quantize data
      vector[0] = 255*((vector[0] + 1)*.5);
      vector[1] = 255*((vector[1] + 1)*.5);
      vector[2] = 255*((vector[2] + 1)*.5);
      
	  data[0] = vector[0];
	  data[1] = vector[1];
	  data[2] = vector[2];
	  data[3] = vector[3];
   }
   else // scalar data
   {
      double scalarData;
      double scalar = 0;
      for(int j = 0; j < npts; j++)
	  {
         tuples->GetTuple(j,&scalarData);
         scalar += scalarData*weights[j];
      }

	  if (scalar > 0 || scalar < 0)
	  {
		  int idebug = 1;
	  }

	  if (scalar > 1)
	  {
		  scalar = 1;
	  }
	  else if (scalar < 0)
	  {
		  scalar = 0;
	  }

	  data[0] = (unsigned char) (255.0 * scalar);
   }

   return data;
}

////////////////////////////////////////////////////////////////////////////////
void VTKVolumeBrickData::extractTuplesForScalar(vtkIdList* ptIds, vtkDataArray* tuples, int num, int dsNum) const
{
	if (m_dataArraysScalar.size() <= dsNum) return;
	PDataArrayVectorPtr pScalars = m_dataArraysScalar[dsNum];

	if (pScalars->size() <= num) return;
	(*pScalars)[num]->GetTuples(ptIds, tuples);
}

////////////////////////////////////////////////////////////////////////////////
void VTKVolumeBrickData::extractTuplesForVector(vtkIdList* ptIds, vtkDataArray* tuples, int num, int dsNum) const
{
	if (m_dataArraysVector.size() <= dsNum) return;
	PDataArrayVectorPtr pVectors = m_dataArraysVector[dsNum];

	if (pVectors->size() <= num) return;
	(*pVectors)[num]->GetTuples(ptIds, tuples);
}

////////////////////////////////////////////////////////////////////////////////
void VTKVolumeBrickData::initMaxPts()
{
	m_maxPts = 0;
	if (!m_dataSets.size()) return;

	for (unsigned int s=0; s<m_dataSets.size(); s++)
	{
		vtkDataSet *pds = m_dataSets[s];
		for (int i=0; i<pds->GetNumberOfCells(); i++)
		{
			vtkCell *pCell = pds->GetCell(i);
			if (pCell->GetNumberOfPoints() > m_maxPts)
			{
				m_maxPts = pCell->GetNumberOfPoints();
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void VTKVolumeBrickData::initDataArrays()
{
	m_dataArraysScalar.clear();
	m_dataArraysVector.clear();
	if (!m_dataSets.size()) return;

	for (unsigned int s=0; s<m_dataSets.size(); s++)
	{
		vtkDataSet *pds = m_dataSets[s];

		PDataArrayVectorPtr pScalarArray = PDataArrayVectorPtr(new DataArrayVector());
		m_dataArraysScalar.push_back(pScalarArray);

		PDataArrayVectorPtr pVectorArray = PDataArrayVectorPtr(new DataArrayVector());
		m_dataArraysVector.push_back(pVectorArray);

		int count = pds->GetPointData()->GetNumberOfArrays();
		for (int i = 0; i < count; i++)
		{
			vtkDataArray *ptArray = pds->GetPointData()->GetArray(i);
			if (ptArray->GetNumberOfComponents() == 1)
			{
				pScalarArray->push_back(ptArray);
				continue;
			}

			if (ptArray->GetNumberOfComponents() == 3 && strcmp(ptArray->GetName(), "normals"))
			{
				pVectorArray->push_back(ptArray);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
bool VTKVolumeBrickData::initCellTrees()
{
	m_bbox.init();
	m_dataSets.clear();
	m_cellLocators.clear();

	m_dataSet->GetAllDataSets(&m_dataSets);
	if (!m_dataSets.size()) return false;

	for (unsigned int i=0; i<m_dataSets.size(); i++)
	{
		vtkSmartPointer<vtkCellTreeLocator> celltree = vtkSmartPointer<vtkCellTreeLocator>::New();
		celltree->SetDataSet(m_dataSets[i]);
		celltree->BuildLocator();
		m_cellLocators.push_back(celltree);

		double box[6] = {0,0,0,0,0,0};
		m_dataSets[i]->GetBounds(box);
		m_bbox.expandBy(osg::Vec3d(box[0], box[2], box[4])); // min
		m_bbox.expandBy(osg::Vec3d(box[1], box[3], box[5])); // max
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
int VTKVolumeBrickData::findCell(double curPos[3], double pcoords[3], std::vector<double> *pweights, vtkSmartPointer<vtkGenericCell> &cell, int *pdsNum) const
{
	for (unsigned int i=0; i<m_cellLocators.size(); i++)
	{
		vtkIdType cellId = m_cellLocators[i]->FindCell(curPos, 0, cell, pcoords, &(*pweights)[0]);
		if (cellId >= 0)
		{
			*pdsNum = i;
			return cellId;
		}
	}

	return -1;
}

////////////////////////////////////////////////////////////////////////////////
// this method converts weights to floats, which creates a lower memory footprint but
//
VTKVolumeBrickData::PTexelData VTKVolumeBrickData::findCell(double curPos[3], int cacheLoc, vtkSmartPointer<vtkGenericCell> cell, std::vector<double> &weights, bool brickCached) const
{
	//if (m_bricksDone >= m_brickCount)
	if (brickCached)
	{
		return m_texelDataCache[cacheLoc];
	}  

	VTKVolumeBrickData* self = const_cast<VTKVolumeBrickData*> (this);
 
	double pcoords[3]; 
	int cellid;

	for (unsigned int i=0; i<m_cellLocators.size(); i++)
	{
		cellid = m_cellLocators[i]->FindCell(curPos, 0, cell, pcoords, &weights[0]);
		if (cellid >= 0)
		{
			PTexelData pdata(new STexelData(weights.size()));
			for (unsigned int w=0; w<weights.size(); w++)
			{
				pdata->weights[w] = weights[w];
			}

			pdata->pointIds->DeepCopy(cell->GetPointIds());	
			pdata->dsNum = i;
			self->m_texelDataCache[cacheLoc] = pdata;
			return pdata;
		}
	}

	return PTexelData();
}

/*
////////////////////////////////////////////////////////////////////////////////
VTKVolumeBrickData::PTexelData VTKVolumeBrickData::findCell(double curPos[3], int cacheLoc, vtkSmartPointer<vtkGenericCell> cell, PTexelData &pdata) const
{
	if (m_bricksDone >= m_brickCount)
	{
		return m_texelDataCache[cacheLoc];
	}

	VTKVolumeBrickData* self = const_cast<VTKVolumeBrickData*> (this);

	double pcoords[3];
	int cellid;

	for (unsigned int i=0; i<m_cellLocators.size(); i++)
	{
		cellid = m_cellLocators[i]->FindCell(curPos, 0, cell, pcoords, &pdata->weights[0]);
		if (cellid >= 0)
		{
			pdata->pointIds->DeepCopy(cell->GetPointIds());	
			pdata->dsNum = i;
			self->m_texelDataCache[cacheLoc] = pdata;
			pdata.reset(new STexelData(m_maxPts)); // create a new pdata now that this one is used
			return self->m_texelDataCache[cacheLoc];
		}
	}

	return PTexelData();
}
*/

////////////////////////////////////////////////////////////////////////////////
osg::Vec4ub VTKVolumeBrickData::getOutSideCellValue() const
{
	if (m_isScalar)
	{
		//return osg::Vec4ub(127, 0,0,0);
		return osg::Vec4ub(0, 0,0,0);
	}

	return osg::Vec4ub(127, 127, 127, 0);
}

////////////////////////////////////////////////////////////////////////////////
void VTKVolumeBrickData::initCache()
{
	m_texelDataCache.clear();

	m_brickCount = (_numBricks[0]) * (_numBricks[1]) * (_numBricks[2]);
	int brickSize = m_brickRes[0] * m_brickRes[1] * m_brickRes[2];
	int totalTexels = m_brickCount * brickSize;
	if (totalTexels <= 0) return;

	m_texelDataCache.resize(totalTexels);
}

////////////////////////////////////////////////////////////////////////////////
int VTKVolumeBrickData::getCacheLoc(int x, int y, int z, const osg::Vec3s &brickNum) const
{
	int bricksz = m_brickRes[0] * m_brickRes[1] * m_brickRes[2];
	//int loc = brickNum[0] * bricksz + brickNum[1] * bricksz + brickNum[2] * bricksz; // get to the start of this brick

	// get to the start of this brick
	int loc = brickNum[0] * bricksz;
	loc += (_numBricks[0]) * brickNum[1] * bricksz;
	loc += (_numBricks[0]) * (_numBricks[1]) * brickNum[2] * bricksz; 

	loc += z * m_brickRes[0] * m_brickRes[1]; // get to the correct depth slice
	loc += y * m_brickRes[0]; // get to the correct scan line
	loc += x; // get to the correct position on the current scan line

	return loc;
}

////////////////////////////////////////////////////////////////////////////////
bool VTKVolumeBrickData::isBrickCached(const osg::Vec3s &brickNum) const
{
	if (m_cacheCreate)
	{
		unsigned int brickid = getBrickId(brickNum);
		std::map<unsigned int, bool>::const_iterator it = m_mapBricksDone.find(brickid);

		if (it == m_mapBricksDone.end()) return false;
		return true;
		
	}

	// must just assume that is already be created if use is on but create is not
	if (m_cacheUse)
	{
		if (m_mapBricksDone.size() > 0) return true;
	}
	
	return false;
}

////////////////////////////////////////////////////////////////////////////////
void VTKVolumeBrickData::cacheBrick(const osg::Vec3s &brickNum, bool isCached)
{
	if (!m_cacheCreate) return;
	if (isCached) return;

	unsigned int brickid = getBrickId(brickNum);
	m_mapBricksDone.insert(std::make_pair(brickid, true));
}

////////////////////////////////////////////////////////////////////////////////
unsigned int VTKVolumeBrickData::getBrickId(const osg::Vec3s &brickNum) const
{
	return brickNum[0] + brickNum[1]*100 + brickNum[2]*10000;
}


// vtk
}
// core
}
// lfx
}
