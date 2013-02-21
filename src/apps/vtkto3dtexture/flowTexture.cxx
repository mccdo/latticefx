/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * VE-Suite is (C) Copyright 1998-2012 by Iowa State University
 *
 * Original Development Team:
 *   - ISU's Thermal Systems Virtual Engineering Group,
 *     Headed by Kenneth Mark Bryden, Ph.D., www.vrac.iastate.edu/~kmbryden
 *   - Reaction Engineering International, www.reaction-eng.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
 * -----------------------------------------------------------------
 * Date modified: $Date$
 * Version:       $Rev$
 * Author:        $Author$
 * Id:            $Id$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.rb END do not edit this line> ***************/
#include "flowTexture.h"

#include <ves/xplorer/util/readWriteVtkThings.h>

#include <iostream>
#include <fstream>
//#include <vtkZLibDataCompressor.h>
#include <vtkImageData.h>
//#include <vtkXMLImageDataWriter.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

//////////////////////////////
//FlowPointData class       //
//////////////////////////////

//////////////////////////////
//Constructors              //
//////////////////////////////
FlowPointData::FlowPointData()
{
   //_data = new float[4];
   _data[0] = 0;
   _data[1] = 0;
   _data[2] = 0;
   _data[3] = 0;
   _dType = VECTOR;
   _valid = 1;
   
}
//////////////////////////////////////////////////////
FlowPointData::FlowPointData(const FlowPointData& fpd)
{
   //_data = new float[4];

   //set the type
   setDataType(fpd._dType);

   _data[0] = fpd._data[0];
   _data[1] = fpd._data[1];
   _data[2] = fpd._data[2];
   _data[3] = fpd._data[3];
   _valid = fpd._valid;
}
///////////////////////////////
//Destructor                 //
///////////////////////////////
FlowPointData::~FlowPointData()
{
   /*if(_data){
      delete [] _data;
      _data = 0;
   }*/
   
}
////////////////////////////////////////
//set the data at this flow point     //
////////////////////////////////////////
void FlowPointData::setData(float val0,
                            float val1,
                            float val2,
                            float val3)
{
   /*if(!_data){
      _data = new float[4];
   }*/
   _data[0] = val0;
   _data[1] = val1;
   _data[2] = val2;
   _data[3] = val3;


}
///////////////////////////////////////
//equal operator                     //
///////////////////////////////////////
FlowPointData& FlowPointData::operator=
              (const FlowPointData& rhs)
{
   if(this != &rhs){
      setData(rhs._data[0],
              rhs._data[1],
              rhs._data[2],
              rhs._data[3]);
      _dType = rhs._dType;
      _valid = rhs._valid;
   }
   return *this;
}

///////////////////////////////
//FlowTexture class          //
///////////////////////////////
///////////////////////////////
//Constructors               //
///////////////////////////////
FlowTexture::FlowTexture()
{
   _nPixels = 0;
   _dType = VECTOR;
   _bbox[0] = -1;
   _bbox[1] = 1;
   _bbox[2] = -1;
   _bbox[3] = 1;
   _bbox[4] = -1;
   _bbox[5] = 1;
}
///////////////////////////////////////////////
FlowTexture::FlowTexture(const FlowTexture& ft)
{
   _nPixels = ft._nPixels;
 
   _dType = ft._dType;
   _dims[0] = ft._dims[0];
   _dims[1] = ft._dims[1];
   _dims[2] = ft._dims[2];

   _bbox[0] = ft._bbox[0];
   _bbox[1] = ft._bbox[1];
   _bbox[2] = ft._bbox[2];
   _bbox[3] = ft._bbox[3];
   _bbox[4] = ft._bbox[4];
   _bbox[5] = ft._bbox[5];

   for(int i = 0; i < _nPixels; i++){
      _pointData.push_back(ft._pointData[i]);
   }
}
///////////////////////////
//Destructor             //
///////////////////////////
FlowTexture::~FlowTexture()
{
   if(_pointData.size()){
      _pointData.clear();
   }
}
////////////////////////////////////////////
//set the resolution of the texture       //
////////////////////////////////////////////
void FlowTexture::setTextureDimension(int x,
                              int y,
                      int z)
{
   _dims[0] = x;
   _dims[1] = y;
   _dims[2] = z;
}
/////////////////////////////////////////////
void FlowTexture::setBoundingBox(double* bbox)
{
   _bbox[0] = bbox[0];
   _bbox[1] = bbox[1];
   _bbox[2] = bbox[2];
   _bbox[3] = bbox[3];
   _bbox[4] = bbox[4];
   _bbox[5] = bbox[5];
}
//////////////////////////////////////////////////
//i == x location                               //
//j == y location                               //
//k == z location                               //
//set the data at a pixel                       //
//////////////////////////////////////////////////
void FlowTexture::addPixelData(FlowPointData fpd)
{
   _pointData.push_back(fpd);
}
////////////////////////////////////////////
//get pixel data                          //
////////////////////////////////////////////
FlowPointData& FlowTexture::pixelData(int col,
                                   int row,
                                  int depth)
{
   if ( !_dims[0] || !_dims[1] )
   {
      std::cout<<"Invalid texture dimensions!!"<<std::endl;
      std::cout<<_dims[0]<<" "<<_dims[1]<<" "<<_dims[2]<<std::endl;
   }

   return _pointData.at(depth*(_dims[1]*_dims[0])
                   + _dims[0]*row + col);
}
////////////////////////////////////////////////////
//write out the flow texture data to              //
//an ascii file it is an rgba file                //
//w/ float data                                   //
////////////////////////////////////////////////////
void FlowTexture::writeFlowTexture( std::string fileName, std::string scalarName)
{
   int pixelNum = 0;
   double bbox[ 6 ];
   for ( unsigned int i = 0; i < 6; ++i )
   {
      bbox[ i ] = _bbox[ i ];
   }

   if ( _pointData.size() )
   {
      vtkImageData* flowImage = vtkImageData::New();
      // used to determine texture size
      flowImage->SetOrigin( bbox[ 0 ], bbox[ 2 ], bbox[ 4 ] );
      //texture dimensions
      flowImage->SetDimensions( _dims[ 0 ], _dims[ 1 ], _dims[ 2 ] );

      // Create delta
      double delta[3] = {0,0,0};
      //delta x
      delta[0] = fabs( (bbox[1] - bbox[0])/(_dims[0]-1) );
      //delta y
      delta[1] = fabs( (bbox[3] - bbox[2])/(_dims[1]-1) );
      //delta z
      delta[2] = fabs( (bbox[5] - bbox[4])/(_dims[2]-1) );
      // used to determine texture size
      flowImage->SetSpacing( delta[0], delta[1], delta[2] );
      

      // setup container for scalar data
      vtkFloatArray* flowData = vtkFloatArray::New();
      flowData->SetName( scalarName.c_str() );

      //scalar or vector data
      if ( _pointData.at(0).type() == FlowPointData::SCALAR )
      {
         flowData->SetNumberOfComponents( 1 );
      }
      else
      {
         flowData->SetNumberOfComponents( 4 );
      }

      if(!_dims[2])
         _dims[2] = 1;
   
      flowData->SetNumberOfTuples( _dims[0]*_dims[1]*_dims[2] );

      //loop through and quantize the data
      for(int k = 0; k < _dims[2]; k++)
      {
         for(int j = 0; j < _dims[1]; j++)
         {
            for(int i = 0; i < _dims[0]; i++)
            {
               pixelNum = k*(_dims[0]*_dims[1]) + _dims[0]*j + i;
               if ( _pointData.at(0).type() == FlowPointData::SCALAR )
               {
                  flowData->SetTuple1( pixelNum, _pointData[ pixelNum ].data( 0 ) );
               }
               else
               {
                  double data[ 4 ];
                  for ( unsigned int i = 0; i < 4; ++i )
                     data[ i ] = _pointData[ pixelNum ].data( i );

                  flowData->SetTuple( pixelNum, data );
               }
            }
         }
      }
      flowImage->GetPointData()->AddArray( flowData );
      ves::xplorer::util::writeVtkThing( flowImage, fileName, 1 );

      flowImage->Delete();
      flowData->Delete();
   }
}

////////////////////////////////////
//equal operator                  //
////////////////////////////////////
FlowTexture& FlowTexture::operator=
               (const FlowTexture& rhs)
{
   if(this != &rhs){
      _dims[0] = rhs._dims[0];
      _dims[1] = rhs._dims[1];
      _dims[2] = rhs._dims[2];
      _nPixels = 0;

      _pointData.clear();

      for(int i = 0; i < _nPixels; i++){
         _pointData.push_back(rhs._pointData[i]);
      }
   }
   return *this;
}
