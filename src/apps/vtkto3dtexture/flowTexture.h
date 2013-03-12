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
#ifndef _BIV_FLOW_TEXTURE_H_
#define _BIV_FLOW_TEXTURE_H_

#include <vector>
#include <ostream>
#include <sstream>

//////////////////////////////////
//this class represents data for//
//a pixel in the flow texture   //
//////////////////////////////////
class FlowPointData
{
public:
    FlowPointData();
    FlowPointData( const FlowPointData& fpd );
    virtual ~FlowPointData();

    //data types on the point
    enum DataType {VECTOR, SCALAR};

    //set the data type
    void setDataType( DataType dt )
    {
        _dType = dt;
    }

    //set the data at this flow point
    void setData( float val0,
                  float val1,
                  float val2,
                  float val3 = 10 );

    //set this pixel as "NULL" because it is
    //out of our real data set
    void setNullPixel()
    {
        _valid = 0;
    }


    //is this pixel valid(in our data set)?
    int isValid()
    {
        return _valid;
    }

    //get the data type
    DataType type()
    {
        return _dType;
    }

    //for the specified index return
    //the data value
    float data( int index )
    {
        return _data[index];
    }

    //get a pointer to the data
    float* data()
    {
        return _data;
    }

    //equal operator
    FlowPointData& operator=( const FlowPointData& rhs );

    inline friend std::ostream& operator<<( std::ostream& os,
                                            const FlowPointData& fpd )
    {
        if( fpd._dType == FlowPointData::VECTOR )
        {
            os << fpd._data[0] << " " << fpd._data[1] << " " << fpd._data[2] << " " << fpd._data[3] << " ";
        }
        else if( fpd._dType == FlowPointData::SCALAR )
        {
            os << fpd._data[0] << " ";
        }
        return os;
    }
    inline friend std::ostream& operator<<( std::ostream& os,
                                            const FlowPointData* fpd )
    {
        if( fpd->_dType == FlowPointData::VECTOR )
        {
            os << fpd->_data[0] << " " << fpd->_data[1] << " " << fpd->_data[2] << " " << fpd->_data[3] << " ";
        }
        else if( fpd->_dType == FlowPointData::SCALAR )
        {
            os << fpd->_data[0] << " ";
        }
        return os;
    }

    inline friend std::ostringstream& operator<<( std::ostringstream& os,
            const FlowPointData& fpd )
    {
        if( fpd._dType == FlowPointData::VECTOR )
        {
            os << fpd._data[0] << " " << fpd._data[1] << " " << fpd._data[2] << " " << fpd._data[3] << " ";
        }
        else if( fpd._dType == FlowPointData::SCALAR )
        {
            os << fpd._data[0] << " ";
        }
        return os;
    }
    inline friend std::ostringstream& operator<<( std::ostringstream& os,
            const FlowPointData* fpd )
    {
        if( fpd->_dType == FlowPointData::VECTOR )
        {
            os << fpd->_data[0] << " " << fpd->_data[1] << " " << fpd->_data[2] << " " << fpd->_data[3] << " ";
        }
        else if( fpd->_dType == FlowPointData::SCALAR )
        {
            os << fpd->_data[0] << " ";
        }
        return os;
    }
protected:
    int _valid;
    DataType _dType;
    float _data[4];
};

//////////////////////////////////////////
//this class represents the texture data//
//describing a flow field               //
//////////////////////////////////////////
class FlowTexture
{
public:
    FlowTexture();
    FlowTexture( const FlowTexture& ft );
    virtual ~FlowTexture();

    enum DataType {VECTOR, SCALAR};

    //set the data type
    void setDataType( DataType dt )
    {
        _dType = dt;
    }

    //set the resolution of the texture
    void setTextureDimension( int x, int y, int z = 0 );

    //i == x location
    //j == y location
    //k == z location
    //set the data at a pixel
    void addPixelData( FlowPointData fpd );
    void setBoundingBox( double* bbox );

    //write out the flow texture data to
    //an ascii file
    //it is an rgba file w/ float data
    void writeFlowTexture( std::string, std::string );

    float* boundingBox()
    {
        return _bbox;
    }

    //get the resolution of the texture
    int* textureDimension()
    {
        return _dims;
    }

    //get pixel data
    FlowPointData& pixelData( int i, int j, int k = 0 );

    //get the data type
    DataType dataType()
    {
        return _dType;
    }

    FlowTexture& operator=( const FlowTexture& rhs );
protected:
    DataType _dType;
    int _nPixels;
    int _dims[3];
    float _bbox[6];
    std::vector<FlowPointData> _pointData;
};
#endif //_BIV_FLOW_TEXTURE_H_
