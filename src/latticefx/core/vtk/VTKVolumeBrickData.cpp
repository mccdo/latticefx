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


namespace lfx {
namespace core {
namespace vtk {


VTKVolumeBrickData::VTKVolumeBrickData(DataSetPtr dataSet, bool prune, int dataNum, bool isScalar, osg::Vec3s brickRes, osg::Vec3s totalNumBricks)
: VolumeBrickData(prune)
{
	m_dataSet = dataSet;
	m_pds = NULL; 
	m_dataNum = dataNum;
	m_isScalar = isScalar;
	m_brickRes = brickRes;
	m_nPtDataArrays = 0;
	m_maxPts = 0;
	m_cellCache = -1;

	setNumBricks(totalNumBricks);

	// get the low level vtkDataSet that we need
	if (!m_dataSet.get() || !m_dataSet->GetDataSet()) return;
	m_pds = vtkDataSet::SafeDownCast(m_dataSet->GetDataSet());
	if (!m_pds) return;

	m_cellLocator = vtkSmartPointer<vtkCellTreeLocator>::New();
	m_cellLocator->SetDataSet(m_pds);
	m_cellLocator->BuildLocator();

	// can get rid of this
	m_nPtDataArrays = m_pds->GetPointData()->GetNumberOfArrays();

	initMaxPts();
	initDataArrays();


		/*
   _nScalars = countNumberOfParameters(1);
   _nVectors = countNumberOfParameters(3);
   _scalarNames = getParameterNames(1,_nScalars);
   _vectorNames = getParameterNames(3,_nVectors);
   */
}

bool VTKVolumeBrickData::isValid()
{
	if (!m_pds) return false;

	return true;
}

osg::Image* VTKVolumeBrickData::getBrick( const osg::Vec3s& brickNum ) const
{
	if (!m_pds) return NULL;

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
	double bbox[6] = {0,0,0,0,0,0};
	m_pds->GetBounds(bbox);

	// vtkBrickSize - the size of our brick in vtk coordniates
	// vtkDelta  - the abount of space to move in vtk coorniates for each pixel, or m_brickRes
	//
	osg::Vec3d vtkBrickSize, vtkDelta;
	vtkBrickSize.x() = fabs(bbox[1] - bbox[0]) / _numBricks.x();
	vtkBrickSize.y() = fabs(bbox[3] - bbox[2]) / _numBricks.y();
	vtkBrickSize.z() = fabs(bbox[5] - bbox[4]) / _numBricks.z();
	vtkDelta.x()   = vtkBrickSize.x() / m_brickRes.x();
	vtkDelta.y()   = vtkBrickSize.y() / m_brickRes.y();
	vtkDelta.z()   = vtkBrickSize.z() / m_brickRes.z();

	// need to compute the bounding box for this brick in the vtk dataset coordinate space
	osg::Vec3d min, max;
	min[0] = bbox[0] + brickNum[0] * vtkBrickSize[0];
	min[1] = bbox[2] + brickNum[1] * vtkBrickSize[1];
	min[2] = bbox[4] + brickNum[2] * vtkBrickSize[2];
	max = min + vtkBrickSize;


	// currently just dealing with a single scalar flow but there could be multiple scalars of data
	// where each scalar of data gets it own brick
	//
	// also there could be multiple vector data sets, where each brick would represent a volume of velocity data
	// but not dealing with this currently

	vtkSmartPointer<vtkGenericCell> cell = vtkSmartPointer<vtkGenericCell>::New();
	vtkIdType cellId;
	double pcoords[3], curPos[3];
	std::vector<double> weights(m_maxPts); // need to find out max points in a cell for the whole dataset
	int subId = 0;
	osg::Vec4ub value;
	int debugNumPts = 0;
	vtkDoubleArray* tuples = vtkDoubleArray::New();
	bool haveCache = false;
	
	// start at left, bottom, back
	curPos[0] = min[0];
	curPos[1] = min[1];
	curPos[2] = min[2];
	for (int z = 0; z < m_brickRes.z(); z++)
	{
		// new depth slice so start at the bottom
		curPos[1] = min[1];

		for (int y = 0; y < m_brickRes.y(); y++)
		{
			// new scanline start back at left most point;
			curPos[0] = min[0];

			for (int x = 0; x < m_brickRes.x(); x++)
			{
				cellId = m_cellLocator->FindCell(curPos, 0, cell, pcoords, &weights[0]);

				if (cellId < 0)
				{
					// todo: deal with vector data that has 4 values

					*ptr = getOutSideCellValue();
					ptr++;
				}
				else
				{
					if (m_cellCache == cellId) haveCache = false;

					if (cell->GetNumberOfPoints() > weights.size())
					{
						int idebug = 1;
					}
					/*
					cell->EvaluateLocation(subId, pcoords, pt, &weights[0]);
					value = lerpDataInCell(cell, &weights[0], m_dataNum, m_isScalar); 
					*/

					value = lerpDataInCell(cell, &weights[0], tuples, m_dataNum, m_isScalar, haveCache); 


					// todo: deal with vector data that has 4 values
					*ptr = value[0];
					ptr++;

					if (!m_isScalar)
					{
						*ptr = value[1];
						ptr++;
						*ptr = value[2];
						ptr++;
						*ptr = value[3];
						ptr++;
					}
				}

				curPos[0] += vtkDelta[0];
				debugNumPts++;
			}

			// jump to next vertical scanline
			curPos[1] += vtkDelta[1];
			
		}

		// jump to next depth slice
		curPos[2] += vtkDelta[2];
	}

	tuples->Delete();
	
	// create an image with our data and return it
	osg::ref_ptr< osg::Image > image( new osg::Image() );
        image->setImage( m_brickRes[0], m_brickRes[1], m_brickRes[2],
            textureFrmt, textureFrmt, GL_UNSIGNED_BYTE,
            (unsigned char*) data, osg::Image::USE_NEW_DELETE );
        return( image.release() );
}

// TODO: just take a pointer to the Vec4ub to avoid the copys everytime
osg::Vec4ub VTKVolumeBrickData::lerpDataInCell(vtkGenericCell* cell, double* weights, vtkDataArray* tuples, int whichValue, bool isScalar, bool haveCache) const
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

			extractTuplesForScalar2(pointIds, tuples, whichValue);
		}
		value = lerpPixelData(tuples, weights, nCellPts, whichValue, isScalar);
      
		//scalar->Delete();
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

		extractTuplesForVector2(pointIds, tuples, whichValue);
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

      //data.setData(vector[0],vector[1],vector[2],vector[3]);
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

	  /*
      if(scalar == 0)
	  {
         scalar = _scalarRanges.at(whichValue)[0];
      }
	  
      data.setData(scalar,0,0,0);
	  */

	  if (scalar > 1)
	  {
		  scalar = 1;
	  }
	  else if (scalar < 0)
	  {
		  scalar = 0;
	  }

	  if (scalar > 0.01)
	  {
		  int idebug = 1;
	  }

	  data[0] = (unsigned char) (255.0 * scalar);
   }

   return data;
}

void VTKVolumeBrickData::extractTuplesForScalar2(vtkIdList* ptIds, vtkDataArray* tuples, int num) const
{
	if (m_dataArraysScalar.size() <= num) return;
	m_dataArraysScalar[num]->GetTuples(ptIds, tuples);
}

void VTKVolumeBrickData::extractTuplesForVector2(vtkIdList* ptIds, vtkDataArray* tuples, int num) const
{
	if (m_dataArraysVector.size() <= num) return;
	m_dataArraysVector[num]->GetTuples(ptIds, tuples);
}

void VTKVolumeBrickData::initMaxPts()
{
	m_maxPts = 0;
	if (!m_pds) return;

	for (int i=0; i<m_pds->GetNumberOfCells(); i++)
	{
		vtkCell *pCell = m_pds->GetCell(i);
		if (pCell->GetNumberOfPoints() > m_maxPts)
		{
			m_maxPts = pCell->GetNumberOfPoints();
		}
	}
}

void VTKVolumeBrickData::initDataArrays()
{
	m_dataArraysScalar.clear();
	m_dataArraysVector.clear();
	if (!m_pds) return;

	int count = m_pds->GetPointData()->GetNumberOfArrays();
	for (int i = 0; i < count; i++)
	{
		vtkDataArray *ptArray = m_pds->GetPointData()->GetArray(i);
		if (ptArray->GetNumberOfComponents() == 1)
		{
			m_dataArraysScalar.push_back(ptArray);
			continue;
		}

		if (ptArray->GetNumberOfComponents() == 3 && strcmp(ptArray->GetName(), "normals"))
		{
			m_dataArraysVector.push_back(ptArray);
		}
	}
}

void VTKVolumeBrickData::extractTuplesForVector(vtkIdList* ptIds, vtkDataArray* vector, int whichVector) const
{
   int vecNumber = 0;
   for (int i = 0; i < m_nPtDataArrays; i++)
   {
      vtkDataArray * ptArray = m_pds->GetPointData()->GetArray(i);

      if (ptArray->GetNumberOfComponents() != 3)
	  {
         continue;
      }

      // also, ignore arrays of normals...
      if (ptArray->GetNumberOfComponents() == 3  && (!strcmp(ptArray->GetName(),"normals")))
	  {
         continue; 
      }

      if(vecNumber != whichVector)
	  {
         vecNumber++;
         continue;
      }
	  else
	  {
         ptArray->GetTuples(ptIds,vector);
         vecNumber++;
		 break; // should be done now..
      }
   }
}

///////////////////////////////////////////////////////////////////
void VTKVolumeBrickData::extractTuplesForScalar(vtkIdList* ptIds, vtkDataArray* scalar, int whichScalar) const
{
   int scaleNum = 0;
   for (int i = 0; i < m_nPtDataArrays; i++)
   {
      vtkDataArray * ptArray = m_pds->GetPointData()->GetArray(i);

      if (ptArray->GetNumberOfComponents() != 1)
	  {
         continue;
      }

      if (scaleNum != whichScalar)
	  {
         scaleNum++;
         continue;
      }
	  else
	  {
         ptArray->GetTuples(ptIds,scalar);
         scaleNum++;

		 // should be done now.. 
		 break;
      }
   }
}

//
// TODO: need to implement this
//
unsigned char VTKVolumeBrickData::getOutSideCellValue() const//(int index)
{
	return 127;

	/*
   if(m_isScalar){
      FlowPointData data;
      
      data.setData(_scalarRanges.at(index)[0],0,0,0);
      data.setDataType(FlowPointData::SCALAR);
      _curScalar.at(0).addPixelData(data);
   }else{
      FlowPointData data;
      //the vectors
      //this is quantized in the range(-1,1)
      data.setData(127,127,127,0);
      data.setDataType(FlowPointData::VECTOR);
      _velocity.at(0).addPixelData(data); 
   }
   */
}


// vtk
}
// core
}
// lfx
}
