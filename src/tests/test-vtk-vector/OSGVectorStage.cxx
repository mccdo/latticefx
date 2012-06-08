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
#include "OSGVectorStage.h"

#include "VTKTextureCreator.h"

#include <osg/PositionAttitudeTransform>
#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/Texture1D>
#include <osg/Geode>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <vtkLookupTable.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>

#include <iostream>

#include <cmath>


////////////////////////////////////////////////////////////////////////////////
OSGVectorStage::OSGVectorStage(void)
{
    ;
}
////////////////////////////////////////////////////////////////////////////////
OSGVectorStage::~OSGVectorStage(void)
{
}
////////////////////////////////////////////////////////////////////////////////
void OSGVectorStage::createArrow( osg::Geometry& geom, int nInstances, float scaleFactor )
{
    float sD( .05 ); // shaft diameter
    float hD( .075 ); // head diameter
    float len( 1. ); // length
    float sh( .65 ); // length from base to start of head

    sD = scaleFactor * sD;
    hD = scaleFactor * hD;
    len = scaleFactor * len;
    sh = scaleFactor * sh;

    osg::Vec3Array* v = new osg::Vec3Array;
    v->resize( 22 );
    geom.setVertexArray( v );

    osg::Vec3Array* n = new osg::Vec3Array;
    n->resize( 22 );
    geom.setNormalArray( n );
    geom.setNormalBinding( osg::Geometry::BIND_PER_VERTEX );

    // Shaft
    (*v)[ 0 ] = osg::Vec3( sD, 0., 0. );
    (*v)[ 1 ] = osg::Vec3( sD, 0., sh );
    (*v)[ 2 ] = osg::Vec3( 0., -sD, 0. );
    (*v)[ 3 ] = osg::Vec3( 0., -sD, sh );
    (*v)[ 4 ] = osg::Vec3( -sD, 0., 0. );
    (*v)[ 5 ] = osg::Vec3( -sD, 0., sh );
    (*v)[ 6 ] = osg::Vec3( 0., sD, 0. );
    (*v)[ 7 ] = osg::Vec3( 0., sD, sh );
    (*v)[ 8 ] = osg::Vec3( sD, 0., 0. );
    (*v)[ 9 ] = osg::Vec3( sD, 0., sh );

    (*n)[ 0 ] = osg::Vec3( 1., 0., 0. );
    (*n)[ 1 ] = osg::Vec3( 1., 0., 0. );
    (*n)[ 2 ] = osg::Vec3( 0., -1., 0. );
    (*n)[ 3 ] = osg::Vec3( 0., -1., 0. );
    (*n)[ 4 ] = osg::Vec3( -1., 0., 0. );
    (*n)[ 5 ] = osg::Vec3( -1., 0., 0. );
    (*n)[ 6 ] = osg::Vec3( 0., 1., 0. );
    (*n)[ 7 ] = osg::Vec3( 0., 1., 0. );
    (*n)[ 8 ] = osg::Vec3( 1., 0., 0. );
    (*n)[ 9 ] = osg::Vec3( 1., 0., 0. );

    //if( nInstances > 1 )
        geom.addPrimitiveSet( new osg::DrawArrays( GL_QUAD_STRIP, 0, 10, nInstances ) );
    //else
    //    geom.addPrimitiveSet( new osg::DrawArrays( GL_QUAD_STRIP, 0, 10 ) );

    // Head
    (*v)[ 10 ] = osg::Vec3( hD, -hD, sh );
    (*v)[ 11 ] = osg::Vec3( hD, hD, sh );
    (*v)[ 12 ] = osg::Vec3( 0., 0., len );
    osg::Vec3 norm = ((*v)[ 11 ] - (*v)[ 10 ]) ^ ((*v)[ 12 ] - (*v)[ 10 ]);
    norm.normalize();
    (*n)[ 10 ] = norm;
    (*n)[ 11 ] = norm;
    (*n)[ 12 ] = norm;

    (*v)[ 13 ] = osg::Vec3( hD, hD, sh );
    (*v)[ 14 ] = osg::Vec3( -hD, hD, sh );
    (*v)[ 15 ] = osg::Vec3( 0., 0., len );
    norm = ((*v)[ 14 ] - (*v)[ 13 ]) ^ ((*v)[ 15 ] - (*v)[ 13 ]);
    norm.normalize();
    (*n)[ 13 ] = norm;
    (*n)[ 14 ] = norm;
    (*n)[ 15 ] = norm;

    (*v)[ 16 ] = osg::Vec3( -hD, hD, sh );
    (*v)[ 17 ] = osg::Vec3( -hD, -hD, sh );
    (*v)[ 18 ] = osg::Vec3( 0., 0., len );
    norm = ((*v)[ 17 ] - (*v)[ 16 ]) ^ ((*v)[ 18 ] - (*v)[ 16 ]);
    norm.normalize();
    (*n)[ 16 ] = norm;
    (*n)[ 17 ] = norm;
    (*n)[ 18 ] = norm;

    (*v)[ 19 ] = osg::Vec3( -hD, -hD, sh );
    (*v)[ 20 ] = osg::Vec3( hD, -hD, sh );
    (*v)[ 21 ] = osg::Vec3( 0., 0., len );
    norm = ((*v)[ 20 ] - (*v)[ 19 ]) ^ ((*v)[ 21 ] - (*v)[ 19 ]);
    norm.normalize();
    (*n)[ 19 ] = norm;
    (*n)[ 20 ] = norm;
    (*n)[ 21 ] = norm;

    //if( nInstances > 1 )
        geom.addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLES, 10, 12, nInstances ) );
    //else
    //    geom.addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLES, 10, 12 ) );
}
////////////////////////////////////////////////////////////////////////////////
osg::Geode* OSGVectorStage::createInstanced(vtkPolyData* glyph, std::string vectorName, std::string scalarName, float scaleFactor )
{
    //Now pull in the vtk data
    vtkPolyData *polyData = glyph;
    if (polyData==NULL)
    {
        std::cout << "pd is null " << std::endl;
        return NULL;
    }
    polyData->Update();

    vtkPointData *pointData = polyData->GetPointData();
    if (pointData==NULL)
    {
        std::cout << " pd point data is null " << std::endl;
        return NULL;
    }
    //pointData->Update();

    vtkPoints *points = polyData->GetPoints();    
    if (points==NULL)
    {
        std::cout << " points are null " << std::endl;
        return NULL;
    }
    
    vtkDataArray* vectorArray = pointData->GetVectors(vectorName.c_str());
    vtkDataArray* scalarArray = pointData->GetScalars(scalarName.c_str());

    if (vectorArray==NULL)
    {
        std::cout << " vectors are null " << std::endl;
        return NULL;
    }

    if (scalarArray==NULL)
    {
        std::cout << " scalars are null " << std::endl;
        return NULL;
    }

    //osg::Group* grp = new osg::Group;
    osg::Geode* geode = new osg::Geode;
    //ves::xplorer::scenegraph::Geode* geode = new ves::xplorer::scenegraph::Geode();
    osg::Geometry* geom = new osg::Geometry;
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );

    osg::ref_ptr< ves::xplorer::scenegraph::VTKTextureCreator > rawVTKData = new ves::xplorer::scenegraph::VTKTextureCreator();
    //ves::xplorer::scenegraph::VTKTextureCreator* rawVTKData = new ves::xplorer::scenegraph::VTKTextureCreator();
    rawVTKData->SetPolyData( glyph );
    rawVTKData->SetActiveVectorAndScalar( vectorName, scalarName );
    rawVTKData->loadData();

    createArrow( *geom, rawVTKData->getDataCount(), 1.0f );//scaleFactor );
    geode->addDrawable( geom );
    //grp->addChild( geode );

    geom->setInitialBound( rawVTKData->getBoundingBox() );

    //osg::ref_ptr< osg::Shader > vertexShader = new osg::Shader( osg::Shader::VERTEX );
    //vertexShader->loadShaderSourceFromFile( osgDB::findDataFile( "vectorfield.vs" ) );

    //osg::ref_ptr< osg::Program > program = new osg::Program();
    //program->addShader( vertexShader.get() );

    //Create the rendering shader
    std::string vertexSource =
        //Setup the color control textures
        "uniform vec2 scalarMinMax;\n"
        "uniform sampler1D texCS; \n"
        //Setup the texture arrays for data
        "uniform vec3 sizes; \n"
        "uniform sampler3D texPos; \n"
        "uniform sampler3D texDir; \n"
        "uniform sampler3D scalar; \n"
        "uniform float userScale;\n"
        "uniform float modulo;\n"
        " \n"
        // There is no straightforward way to "discard" a vertex in a vertex shader,
        // (unlike discard in a fragment shader). So we use an aspect of clipping to
        // "clip away" unwanted vertices and vectors. Here's how it works:
        // The gl_Position output of the vertex shader is an xyzw value in clip coordinates,
        // with -w < xyz < w. All xyz outside the range -w to w are clipped by hardware
        // (they are outside the view volume). So, to discard an entire vector, we set all
        // its gl_Positions to (1,1,1,0). All vertices are clipped because -0 < 1 < 0 is false.
        // If all vertices for a given instance have this value, the entire instance is
        // effectively discarded.
        "bool\n"
        "discardInstance( const in float fiid )\n"
        "{\n"
        "    bool discardInstance = ( mod( fiid, modulo ) > 0.0 );\n"
        "    if( discardInstance )\n"
        "        gl_Position = vec4( 1.0, 1.0, 1.0, 0.0 );\n"
        "    return( discardInstance );\n"
        "}\n"
        " \n"
        // Based on the global 'sizes' uniform that contains the 3D stp texture dimensions,
        // and the input parameter current instances, generate an stp texture coord that
        // indexes into a texture to obtain data for this instance.
        "vec3 \n"
        "generateTexCoord( const in float fiid ) \n"
        "{ \n"
        "    float p1 = fiid / (sizes.x*sizes.y); \n"
        "    float t1 = fract( p1 ) * sizes.y; \n"

        "    vec3 tC; \n"
        "    tC.s = fract( t1 ); \n"
        "    tC.t = floor( t1 ) / sizes.y; \n"
        "    tC.p = floor( p1 ) / sizes.z; \n"

        "    return( tC ); \n"
        "} \n"
        "vec4 \n"
        "simpleLighting( const in vec4 color, const in vec3 normal, const in float diffCont, const in float ambCont ) \n"
        "{ \n"
        "    const vec4 amb = color * ambCont; \n"
        "    const vec3 eyeVec = vec3( 0.0, 0.0, 1.0 ); \n"
        "    const float dotVal = max( dot( normal, eyeVec ), 0.0 ); \n"
        "    const vec4 diff = color * dotVal * diffCont; \n"
        "    return( amb + diff ); \n"
        "} \n" //36
        " \n"
        "mat3 \n"
        "makeOrientMat( const in vec3 dir ) \n"
        "{ \n"
        // Compute a vector at a right angle to the direction.
        // First try projection direction into xy rotated -90 degrees.
        // If that gives us almost the same vector we started with,
        // then project into yz instead, rotated 90 degrees.
        "   vec3 c = vec3( dir.y, -dir.x, 0.0 ); \n"
        "   if( dot( c, c ) < 0.1 ) \n"
        "   { \n"
        "       c = vec3( 0.0, dir.z, -dir.y ); \n"
        "   } \n"
        //normalize( c.xyz );
        "   float l = length( c );\n"
        "   c /= l;\n"    
        " \n"
        "   vec3 up = normalize( cross( dir, c ) ); \n"
        " \n"
        // Orientation uses the cross product vector as x,
        // the up vector as y, and the direction vector as z.
        "   return( mat3( c, up, dir ) ); \n"
        "} \n"
        " \n"
        "void main() \n"
        "{ \n"
        "    float fiid = gl_InstanceID; \n"
        "    if( discardInstance( fiid ) )\n"
        "        return;\n"
        "\n"
        // Generate stp texture coords from the instance ID.
        "    vec3 tC = generateTexCoord( fiid ); \n"

        // Create orthonormal basis to position and orient this instance.
        //"   vec4 newZ = texture3D( texDir, tC ); \n"
        //"   vec3 newX = cross( newZ.xyz, vec3( 0,0,1 ) ); \n"
        //"   normalize( newX ); \n"
        //"   vec3 newY = cross( newZ.xyz, newX ); \n"
        //"   normalize( newY ); \n"
        //"   vec4 pos = texture3D( texPos, tC ); \n"
        //"   mat4 mV = mat4( newX.x, newX.y, newX.z, 0., newY.x, newY.y, newY.z, 0., newZ.x, newZ.y, newZ.z, 0., pos.x, pos.y, pos.z, 1. ); \n"
        //"   gl_Position = (gl_ModelViewProjectionMatrix * mV * gl_Vertex); \n"
        "   vec4 pos = texture3D( texPos, tC ); \n"
        // Create an orientation matrix. Orient/transform the arrow.
        // Sample (look up) direction vector and obtain the scale factor
        "   vec4 dir = texture3D( texDir, tC );\n"
        "   mat3 orientMat = makeOrientMat( normalize( dir.xyz ) ); \n"
        "   float scale = userScale * length( dir.xyz );\n"
        "   vec3 oVec = orientMat * (scale * gl_Vertex.xyz);\n"
        "   vec4 hoVec = vec4( oVec + pos, 1.0 ); \n"
        "   gl_Position = gl_ModelViewProjectionMatrix * hoVec; \n"

        //Old color mapping
        // Use just the orientation components to transform the normal.
        // Orient the normal.
        "   vec3 norm = normalize( gl_NormalMatrix * orientMat * gl_Normal ); \n"
        // Diffuse lighting with light at the eyepoint.
        ////"   vec4 color = texture3D( scalar, tC ); \n"
        
        //New way of mapping colors
        "   // Scalar texture containg key to color table. \n"
        "   vec4 activeScalar = texture3D( scalar, tC );\n"
        "   float normScalarVal = 0.;\n"
        "   normScalarVal = (activeScalar.a - scalarMinMax.x) / (scalarMinMax.y - scalarMinMax.x);\n"
        
        "   if( normScalarVal < 0. )\n"
        "   {\n"
        "       normScalarVal = 0.;\n"
        "   }\n"
        "   if( normScalarVal > 1. )\n"
        "   {\n"
        "       normScalarVal = 1.;\n"
        "   }\n"
        "   vec4 colorResult = texture1D( texCS, normScalarVal );\n"
        "   colorResult[3]=1.0; \n"
        //"   gl_FrontColor = colorResult; \n"
        //"   vec4 color = colorResult.rgb; \n"
    
        //Get the color into GLSL
        "   gl_FrontColor = simpleLighting( colorResult, norm, 0.7, 0.3 ); \n"
        "} \n";

    osg::StateSet* ss = geom->getOrCreateStateSet();
    osg::ref_ptr< osg::Program > program = new osg::Program();

    {
        osg::ref_ptr< osg::Shader > vertexShader = new osg::Shader();
        vertexShader->setType( osg::Shader::VERTEX );
        vertexShader->setShaderSource( vertexSource );
        
        program->addShader( vertexShader.get() );
    }

    {
        std::string shaderName = osgDB::findDataFile( "null_glow.fs" );
        osg::ref_ptr< osg::Shader > fragShader = 
            osg::Shader::readShaderFile( osg::Shader::FRAGMENT, shaderName );
        
        program->addShader( fragShader.get() );
    }
    ss->setAttributeAndModes( program.get(),
        osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
    
    // Posidion array.
    ss->setTextureAttribute( 0, rawVTKData->getPositionTexture() );
    osg::ref_ptr< osg::Uniform > texPosUniform =
    new osg::Uniform( "texPos", 0 );
    ss->addUniform( texPosUniform.get() );

    // Direction array.
    ss->setTextureAttribute( 1, rawVTKData->getDirectionTexture() );
    osg::ref_ptr< osg::Uniform > texDirUniform =
    new osg::Uniform( "texDir", 1 );
    ss->addUniform( texDirUniform.get() );

    // Scalar array.
    ss->setTextureAttribute( 2, rawVTKData->getScalarTexture() );
    osg::ref_ptr< osg::Uniform > texScalarUniform =
        new osg::Uniform( "scalar", 2 );
    ss->addUniform( texScalarUniform.get() );

    {
        // Pass the 3D texture dimensions to the shader as a "sizes" uniform.
        osg::Vec3s ts( rawVTKData->getTextureSizes() );
        osg::ref_ptr< osg::Uniform > sizesUniform =
            new osg::Uniform( "sizes", osg::Vec3( (float)ts.x(), (float)ts.y(), (float)ts.z() ) );
        ss->addUniform( sizesUniform.get() );
    }

    {
        // Pass the scale to the  "scaleUniform" uniform.
        osg::ref_ptr< osg::Uniform > scaleUniform =
            new osg::Uniform( "userScale", scaleFactor );
        ss->addUniform( scaleUniform.get() );
    }
    
    {
        osg::ref_ptr< osg::Uniform > uModulo = 
            new osg::Uniform( "modulo", 1.0f );
        uModulo->setDataVariance( osg::Object::DYNAMIC );
        ss->addUniform( uModulo.get() );
    }
    
    {
        double dataRange[2]; 
        scalarArray->GetRange(dataRange);
        
        // Pass the min/max for the scalar range into the shader as a uniform.
        osg::Vec2 ts( dataRange[ 0 ], dataRange[ 1 ] );
        osg::ref_ptr< osg::Uniform > scalarMinMaxUniform =
            new osg::Uniform( "scalarMinMax",
            osg::Vec2( (float)ts.x(), (float)ts.y() ) );
        ss->addUniform( scalarMinMaxUniform.get() );
        
        // Set up the color spectrum.
        osg::Texture1D* texCS = 
            new osg::Texture1D( rawVTKData->CreateColorTextures( dataRange ) );
        texCS->setFilter( osg::Texture::MIN_FILTER, osg::Texture2D::LINEAR);
        texCS->setFilter( osg::Texture::MAG_FILTER, osg::Texture2D::LINEAR );
        texCS->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
        
        ss->setTextureAttribute( 3, texCS );
        osg::ref_ptr< osg::Uniform > texCSUniform = 
            new osg::Uniform( "texCS", 3 );
        ss->addUniform( texCSUniform.get() );        
    }

    return geode;
}
////////////////////////////////////////////////////////////////////////////////
