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

	setNumBricks(totalNumBricks);

	// get the low level vtkDataSet that we need
	if (!m_dataSet.get() || !m_dataSet->GetDataSet()) return;
	m_pds = vtkDataSet::SafeDownCast(m_dataSet->GetDataSet());
	if (!m_pds) return;

	m_cellLocator = vtkSmartPointer<vtkCellTreeLocator>::New();
	m_cellLocator->SetDataSet(m_pds);
	m_cellLocator->BuildLocator();

	m_nPtDataArrays = m_pds->GetPointData()->GetNumberOfArrays();
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
	unsigned char* data( new unsigned char[ m_brickRes[0] * m_brickRes[1] * m_brickRes[2] ] );
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
	std::vector<double> weights(100); // TODO: need to find out max points in a cell for the whole dataset
	int subId = 0;
	osg::Vec4ub value;
	int debugNumPts = 0;
	
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
					if (cell->GetNumberOfPoints() > weights.size())
					{
						int idebug = 1;
					}
					/*
					cell->EvaluateLocation(subId, pcoords, pt, &weights[0]);
					value = lerpDataInCell(cell, &weights[0], m_dataNum, m_isScalar); 
					*/

					value = lerpDataInCell(cell, &weights[0], m_dataNum, m_isScalar); 


					// todo: deal with vector data that has 4 values
					*ptr = value[0];
					ptr++;
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

	if (debugNumPts != m_brickRes[0] * m_brickRes[1] * m_brickRes[2])
	{
		int debug = 1;
	}
	
	// create an image with our data and return it
	osg::ref_ptr< osg::Image > image( new osg::Image() );
        image->setImage( m_brickRes[0], m_brickRes[1], m_brickRes[2],
            GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
            (unsigned char*) data, osg::Image::USE_NEW_DELETE );
        return( image.release() );
}

// TODO: just take a pointer to the Vec4ub to avoid the copys everytime
osg::Vec4ub VTKVolumeBrickData::lerpDataInCell(vtkGenericCell* cell, double* weights, int whichValue, bool isScalar) const
{
	osg::Vec4ub value;

   //list of point ids in cell
   vtkIdList* pointIds = cell->GetPointIds();
   
   //number of verts in the cell
   int nCellPts = cell->GetNumberOfPoints();
   
   if (isScalar)
   {
      vtkDoubleArray* scalar = vtkDoubleArray::New();
      scalar->SetNumberOfComponents(1);
      scalar->SetNumberOfTuples(nCellPts);

      extractTuplesForScalar(pointIds, scalar, whichValue);
      value = lerpPixelData(scalar, weights, nCellPts, whichValue, isScalar);
      
      scalar->Delete();
   }
   else
   {
      vtkDoubleArray* vector = vtkDoubleArray::New();
      vector->SetNumberOfComponents(3);
      vector->SetNumberOfTuples(nCellPts);

      extractTuplesForVector(pointIds, vector, whichValue);
      value = lerpPixelData(vector, weights, nCellPts, whichValue, isScalar);

      vector->Delete();
   }

   return value;
}

osg::Vec4ub VTKVolumeBrickData::lerpPixelData(vtkDataArray* ptArray, double* weights, int npts, int whichValue, bool isScalar) const
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
         ptArray->GetTuple(j, vectorData);
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
      
      //normalize data
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
         ptArray->GetTuple(j,&scalarData);
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
#if 0
void VTKDataToTexture::createTextures()
{
   wxString msg = wxString("Creating textures.", wxConvUTF8);
   _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );
   //check for a dataset
   if(!_dataSet){
      if(_vFileName){
         createDataSetFromFile(std::string( _vFileName ) );
      }else{
         std::cout<<"No dataset available to";
         std::cout<<" create a texture!!"<<std::endl;
         std::cout<<"ERROR: VTKDataToTexture::createTextures()"<<std::endl;
         return;
      }
   }

   //was the resolution initialized?
   if(_resolution[0] == 2 &&
      _resolution[1] == 2 &&
      _resolution[2] == 2){
      std::cout<<"WARNING: Resolution set to the min!:"<<std::endl;
      std::cout<<" : VTKDataToTexture::createTextures()"<<std::endl;
   }
   msg = wxString("Building octree.", wxConvUTF8);
   _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );

   //get the info about the data in the data set
   _nPtDataArrays = _dataSet->GetPointData()->GetNumberOfArrays();
   _nScalars = countNumberOfParameters(1);
   _nVectors = countNumberOfParameters(3);
   _scalarNames = getParameterNames(1,_nScalars);
   _vectorNames = getParameterNames(3,_nVectors);

   _cleanUpFileNames();
   _applyCorrectedNamesToDataArrays();
   //by default, _recreateValidityBetweenTimeSteps is false
   if(!_madeValidityStructure || _recreateValidityBetweenTimeSteps)
   {
      msg = wxString("Sampling valid domain. . .", wxConvUTF8);
      _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );
      //build the octree
      long timeID = (long)time( NULL );
      std::cout << timeID << std::endl;
      /*bbLocator = vtkOBBTree::New();
      bbLocator->CacheCellBoundsOn();
      bbLocator->AutomaticOn();
      //bbLocator->SetNumberOfCellsPerBucket( 50 );
      //bbLocator->SetMaxLevel( 10 )
      vtkDataSet* polyData = ves::xplorer::util::readVtkThing( "./step1_0.vtp" );
      bbLocator->SetDataSet( polyData );
      //build the octree
      bbLocator->BuildLocator();*/
      
      /*
       vtkNew<vtkCellTreeLocator> locator;
       locator->SetDataSet(sphere2->GetOutput());
       locator->SetCacheCellBounds(cachedCellBounds);
       locator->AutomaticOn();
       locator->BuildLocator();
       */
      
      vectorCellLocators.resize( numThreads );
     for ( int i=0;i<numThreads;i++  )
     {
        vectorCellLocators.at( i ) = vtkCellLocator::New();
        vectorCellLocators.at( i )->CacheCellBoundsOn();
        vectorCellLocators.at( i )->AutomaticOn();
        vectorCellLocators.at( i )->SetNumberOfCellsPerBucket( 50 );
        //vtkDataSet* polyData = ves::xplorer::util::readVtkThing("./tempDataDir/surface.vtp");
        vectorCellLocators.at( i )->SetDataSet(_dataSet);
        //build the octree
        vectorCellLocators.at( i )->BuildLocator();
     }
      //long endtimeID = (long)time( NULL );
      //std::cout << endtimeID - timeID << std::endl;
      //Now use it...
      _createValidityTexture();
      //bbLocator->Delete();
      for ( int i=0;i<numThreads;i++  )
      {
         vectorCellLocators.at( i )->Delete();
      }

   }

   msg = wxString("Processing scalars. . .", wxConvUTF8);
   _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );

   for ( int i = 0; i < _nScalars; ++i )
   {
      double bbox[6] = {0,0,0,0,0,0};
      //a bounding box
      _dataSet->GetBounds(bbox);
      
      FlowTexture texture;
      msg = wxString("Scalar: ", wxConvUTF8) + wxString(_scalarNames[i].c_str(), wxConvUTF8);
      _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );

      texture.setTextureDimension(_resolution[0],_resolution[1],_resolution[2]);
      texture.setBoundingBox(bbox);
      _curScalar.push_back(texture);

      msg = wxString("Resampling scalar data.", wxConvUTF8);
      _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );
      _resampleData(i,1);
      //std::cout<<"Writing data to texture."<<std::endl;
      writeScalarTexture(i);
      //std::cout<<"Cleaning up."<<std::endl;
      _curScalar.clear();
   }
   //std::cout<<"Processing vectors:"<<std::endl;
    msg = wxString("Processing vectors. . .", wxConvUTF8);
   _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );

   for(int i = 0; i < _nVectors; i++){ 
       double bbox[6] = {0,0,0,0,0,0};
      //a bounding box
      _dataSet->GetBounds(bbox);
      FlowTexture texture;
      wxString msg = wxString("Vector: ", wxConvUTF8) + wxString(_vectorNames[i].c_str(), wxConvUTF8);
      _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );

      texture.setTextureDimension(_resolution[0],_resolution[1],_resolution[2]);
      texture.setBoundingBox(bbox);
      _velocity.push_back(texture);
      
      msg = wxString("Resampling vector data", wxConvUTF8);
      _updateTranslationStatus( ConvertUnicode( msg.c_str()) );
      _resampleData(i,0);
      
      //std::cout<<"         Writing data to texture."<<std::endl;
      writeVelocityTexture(i);
      //std::cout<<"      Cleaning up."<<std::endl;
      _velocity.clear();
   }   
}
#endif


// vtk
}
// core
}
// lfx
}
