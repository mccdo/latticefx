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

#include <osg/Image>

#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkGenericCell.h>
#include <vtkDoubleArray.h>

#include <boost/thread.hpp>

namespace lfx
{
namespace core
{
namespace vtk
{


VTKVolumeBrickData::VTKVolumeBrickData(DataSetPtr dataSet, 
									   bool prune, 
									   int dataNum, 
									   bool isScalar, 
									   osg::Vec3s brickRes, 
									   osg::Vec3s totalNumBricks, 
									   int threadCount)
    :
    VolumeBrickData(prune)
{
	m_dataSet = dataSet;
	m_dataNum = dataNum;
	m_isScalar = isScalar;
	m_brickRes = brickRes;
	m_maxPts = 0; 
	m_cellCache = -1;
	m_bbox.init();

	m_threadCount = threadCount;
	if (m_threadCount <= 0) m_threadCount = 1;

	setNumBricks(totalNumBricks);

	if (!initCellTrees()) return;

	initMaxPts();
	initDataArrays();
}

bool VTKVolumeBrickData::isValid()
{
	if (!m_dataSets.size()) return false;

	return true;
}

osg::Image* VTKVolumeBrickData::getBrick( const osg::Vec3s& brickNum ) const
{
	//if (!m_pds) return NULL;
	if (!m_cellLocators.size()) return NULL;

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
	// vtkDelta  - the abount of space to move in vtk coorniates for each pixel, or m_brickRes
	//
	osg::Vec3d vtkBrickSize, vtkDelta;
	vtkBrickSize.x() = fabs(m_bbox.xMax() - m_bbox.xMin()) / _numBricks.x();
	vtkBrickSize.y() = fabs(m_bbox.yMax() - m_bbox.yMin()) / _numBricks.y();
	vtkBrickSize.z() = fabs(m_bbox.zMax() - m_bbox.zMin()) / _numBricks.z();
	/*
	vtkBrickSize.x() = fabs(bbox[1] - bbox[0]) / _numBricks.x();
	vtkBrickSize.y() = fabs(bbox[3] - bbox[2]) / _numBricks.y();
	vtkBrickSize.z() = fabs(bbox[5] - bbox[4]) / _numBricks.z();
	*/
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

	for (int i=0; i<m_threadCount; i++)
	{
		SThreadDataPtr pData = SThreadDataPtr( new SThreadData() );
		pData->pVBD = this;
		pData->ptrPixels = ptr;
		pData->bytesPerPixel = bytesPerPixel;

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

	// create an image with our data and return it
	osg::ref_ptr< osg::Image > image( new osg::Image() );
        image->setImage( m_brickRes[0], m_brickRes[1], m_brickRes[2],
            textureFrmt, textureFrmt, GL_UNSIGNED_BYTE,
            (unsigned char*) data, osg::Image::USE_NEW_DELETE );
        return( image.release() );
}

void VTKVolumeBrickData::BrickThread::operator()()
{
	if (!m_pData.get()) return;

	vtkSmartPointer<vtkGenericCell> cell = vtkSmartPointer<vtkGenericCell>::New();
	vtkIdType cellId;
	double pcoords[3], curPos[3];
	std::vector<double> weights(m_pData->pVBD->m_maxPts); // need to find out max points in a cell for the whole dataset
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
				//cellId = m_pData->pVBD->m_cellLocator->FindCell(curPos, 0, cell, pcoords, &weights[0]);
				cellId = m_pData->pVBD->findCell(curPos, pcoords, &weights, cell, &dataSetNum);

				if (cellId < 0)
				{
					value = m_pData->pVBD->getOutSideCellValue(); 
				}
				else
				{
					// cell cache is not showing any signs of a speed up
					//if (m_cellCache == cellId) haveCache = false;
					value = m_pData->pVBD->lerpDataInCell(cell, &weights[0], m_pData->tuples, m_pData->pVBD->m_dataNum, m_pData->pVBD->m_isScalar, haveCache, dataSetNum); 
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

osg::Vec4ub VTKVolumeBrickData::lerpDataInCell(vtkGenericCell* cell, double* weights, vtkDataArray* tuples, int whichValue, bool isScalar, bool haveCache, int dsNum) const
{
	osg::Vec4ub value;

	//list of point ids in cell
	vtkIdList* pointIds = cell->GetPointIds();
   
	//number of verts in the cell
	int nCellPts = cell->GetNumberOfPoints();
   
	if (isScalar)
	{
		if (!haveCache)
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
		}
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

void VTKVolumeBrickData::extractTuplesForScalar(vtkIdList* ptIds, vtkDataArray* tuples, int num, int dsNum) const
{
	if (m_dataArraysScalar.size() <= dsNum) return;
	PDataArrayVectorPtr pScalars = m_dataArraysScalar[dsNum];

	if (pScalars->size() <= num) return;
	(*pScalars)[num]->GetTuples(ptIds, tuples);
}

void VTKVolumeBrickData::extractTuplesForVector(vtkIdList* ptIds, vtkDataArray* tuples, int num, int dsNum) const
{
	if (m_dataArraysVector.size() <= dsNum) return;
	PDataArrayVectorPtr pVectors = m_dataArraysVector[dsNum];

	if (pVectors->size() <= num) return;
	(*pVectors)[num]->GetTuples(ptIds, tuples);
}

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

int VTKVolumeBrickData::findCell(double curPos[3], double pcoords[3], std::vector<double> *pweights, vtkSmartPointer<vtkGenericCell> &cell, int *pdsNum) const
{
	//int hitcount = 0;
	for (unsigned int i=0; i<m_cellLocators.size(); i++)
	{
		vtkIdType cellId = m_cellLocators[i]->FindCell(curPos, 0, cell, pcoords, &(*pweights)[0]);
		if (cellId >= 0)
		{
			//hitcount++;
			*pdsNum = i;
			return cellId;
		}
	}

	/*
	if (hitcount > 1)
	{
		int debug=1;
	}
	*/

	return -1;
}

osg::Vec4ub VTKVolumeBrickData::getOutSideCellValue() const//(int index)
{
	if (m_isScalar)
	{
		//return osg::Vec4ub(127, 0,0,0);
		return osg::Vec4ub(0, 0,0,0);
	}

	return osg::Vec4ub(127, 127, 127, 0);
}


// vtk
}
// core
}
// lfx
}
