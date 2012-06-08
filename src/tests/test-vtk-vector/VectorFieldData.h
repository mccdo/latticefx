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
#ifndef VES_XPLORER_SCENEGRAPH_VECTOR_FIELD_DATA_H
#define VES_XPLORER_SCENEGRAPH_VECTOR_FIELD_DATA_H

#include <osg/Geometry>
#include <osg/Texture3D>
#include <osg/Texture1D>
#include <osg/Uniform>
#include <osg/ClipPlane>

namespace ves
{
namespace xplorer
{
namespace scenegraph
{
// Base class for abstracting vector field data storage
class VectorFieldData : public osg::Referenced
{
public:
    VectorFieldData();
    
    void loadData();
    
    osg::Texture3D* getPositionTexture();
    osg::Texture3D* getDirectionTexture();
    osg::Texture3D* getScalarTexture();
    osg::Texture3D* getDiameterTexture();

    osg::Vec3s getTextureSizes();
    unsigned int getDataCount();
    
    // You must override this to return a bounding box for your
    // vector field data.
    virtual osg::BoundingBox getBoundingBox() = 0;
    
protected:
    osg::ref_ptr< osg::Texture3D > _texPos, _texDir, _texScalar, _diaScalar;
    osg::Vec3s _texSizes;
    unsigned int _dataSize;
    
    float* _pos;
    float* _dir;
    float* _scalar;
    float* _dia;
    
    virtual ~VectorFieldData();
    
    // You must override this to load your data and create textures from that data.
    // The code below is intended only as a template/example. See also
    // MyVectorFieldData::internalLoad().
    virtual void internalLoad();
    
    
    //
    // The following set of protected member functions exist in support
    // of your probable implementation of internalLoad. You can use them
    // if you wish, but you are not required to do so.
    //
    
    //
    // ceilPower2 - Originally in OpenGL Distilled example source code.
    //
    // Return next highest power of 2 greater than x
    // if x is a power of 2, return x.
    static unsigned short ceilPower2( unsigned short x );
    
    // Given we need to store 'dataCount' values in a texture, compute
    // optimal 3D texture dimensions large enough to hold those values.
    // (This might not be the best implementation. Cubed root would
    // come in handy here.)
    static void compute3DTextureSize( unsigned int dataCount, int& s, int& t, int& p );
    
    // Creates a 3D texture containing floating point somponents.
    osg::Texture3D* makeFloatTexture( unsigned char* data, int numComponents, osg::Texture::FilterMode filter );
};
}
}
}
#endif //VES_XPLORER_SCENEGRAPH_VECTOR_FIELD_DATA_H
