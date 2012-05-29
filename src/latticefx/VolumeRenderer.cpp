/*************** <auto-copyright.rb BEGIN do not edit this line> *************
*
* latticeFX is (C) Copyright 2011-2012 by Ames Laboratory
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; 
* version 2.1 of the License.
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
*************** <auto-copyright.rb END do not edit this line> **************/

#include <latticefx/VolumeRenderer.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/ChannelDataOSGImage.h>
#include <latticefx/TextureUtils.h>

#include <osgDB/ReadFile>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/BlendFunc>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>
#include <osgDB/FileUtils>

namespace lfx {


VolumeRenderer::VolumeRenderer()
  : _maxSlices(1024), _planeSpacing(0.3f), _volumeDims(60.0f, 60.0f, 30.0f), _volumeOrigin(0.0f, 0.0f, 0.0f)
{
}
VolumeRenderer::VolumeRenderer( const VolumeRenderer& rhs )
  : lfx::Renderer( rhs )
{
}
VolumeRenderer::~VolumeRenderer()
{
}

// <<<>>> temporarily here until integration is complete
void createDAIGeometry( osg::Geometry& geom, int nInstances=1 )
{
	const float halfDimX( .5 );
	const float halfDimZ( .5 );

	osg::Vec3Array* v = new osg::Vec3Array;
	v->resize( 4 );
	geom.setVertexArray( v );

	// Geometry for a single quad.
	(*v)[ 0 ] = osg::Vec3( -halfDimX, -halfDimZ, 0. );
	(*v)[ 1 ] = osg::Vec3( halfDimX, -halfDimZ, 0. );
	(*v)[ 2 ] = osg::Vec3( halfDimX, halfDimZ, 0. );
	(*v)[ 3 ] = osg::Vec3( -halfDimX, halfDimZ, 0. );

	// Use the DrawArraysInstanced PrimitiveSet and tell it to draw nInstances instances.
	geom.addPrimitiveSet( new osg::DrawArrays( GL_QUADS, 0, 4, nInstances ) );
}


osg::Node* VolumeRenderer::getSceneGraph( const lfx::ChannelDataPtr maskIn )
{
	osg::ref_ptr< osg::Geode > geode( new osg::Geode );

	osg::Geometry* geom( new osg::Geometry );
	geom->setUseDisplayList( false );
	geom->setUseVertexBufferObjects( true );
	// OSG has no clue where our vertex shader will place the geometric data,
	// so specify an initial bound to allow proper culling and near/far computation.
	osg::BoundingBox bb( _volumeDims * -.5, _volumeDims * .5 ); // <<<>>> incorporate origin
	geom->setInitialBound( bb );
	// Add geometric data and the PrimitiveSet. Specify numInstances as _maxSlices.
	createDAIGeometry( *geom, _maxSlices );
	geode->addDrawable( geom );


	osg::StateSet* stateSet( geode->getOrCreateStateSet() );

    ChannelDataPtr dataPtr( getInput( "volumedata" ) );
    if( dataPtr == NULL )
    {
        OSG_WARN << "VolumeRenderer::getSceneGraph(): Unable to find required volumedata ChannelData." << std::endl;
        return( NULL );
    }
    ChannelDataOSGImage* dataImagePtr( static_cast<
        ChannelDataOSGImage* >( dataPtr.get() ) );
    osg::Image* volumeImage( dataImagePtr->getImage() );

    osg::Texture3D* volumeTexture = new osg::Texture3D( volumeImage );
	volumeTexture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
	volumeTexture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
	volumeTexture->setWrap(osg::Texture2D::WRAP_R, osg::Texture2D::CLAMP_TO_EDGE);
	volumeTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
	volumeTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
    stateSet->setTextureAttributeAndModes(
        getOrAssignTextureUnit( "volumeTex" ), volumeTexture );
    stateSet->setTextureAttributeAndModes(
        getOrAssignTextureUnit( "tfInput" ), volumeTexture );

	osg::Texture2D* transferTexture = new osg::Texture2D(
        osgDB::readImageFile( "Spectrum.png" ) );
	transferTexture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
	transferTexture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
	transferTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
	transferTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);

    const unsigned int transTexUnit( getOrAssignTextureUnit( "transferHackTBD" ) );
    stateSet->setTextureAttributeAndModes( transTexUnit, transferTexture );

	osg::Uniform* transUni( new osg::Uniform( osg::Uniform::SAMPLER_2D, "TransferFunction" ) ); transUni->set( (int)transTexUnit );
	stateSet->addUniform( transUni );


	return( geode.release() );
}


osg::StateSet* VolumeRenderer::getRootState()
{
	osg::ref_ptr< osg::StateSet > stateSet( new osg::StateSet() );

	// position, direction, transfer function input, and hardware mask input texture
	// units are the same for all time steps, so set their sampler uniform unit
	// values in the root state.
	const unsigned int volTexUnit( getOrAssignTextureUnit( "volumeTex" ) );
	osg::Uniform* volUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "VolumeTexture" ) ); volUni->set( (int)volTexUnit );
	stateSet->addUniform( volUni );

	if( getTransferFunction() != NULL )
	{
		const unsigned int tfInputUnit( getOrAssignTextureUnit( "tfInput" ) );
		osg::Uniform* tfInputUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "TransferFunction" ) ); tfInputUni->set( (int)tfInputUnit );
		stateSet->addUniform( tfInputUni );
	}

	// <<<>>> need to setup uniforms for VolumeDims, VolumeCenter, PlaneSpacing
    osg::Uniform* dimsUni( new osg::Uniform( "VolumeDims", _volumeDims ) );
    stateSet->addUniform( dimsUni );
    osg::Uniform* centerUni( new osg::Uniform( "VolumeCenter", _volumeOrigin ) );
    stateSet->addUniform( centerUni );
    osg::Uniform* spaceUni( new osg::Uniform( "PlaneSpacing", _planeSpacing ) );
    stateSet->addUniform( spaceUni );

	osg::BlendFunc *fn = new osg::BlendFunc();
	fn->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
	stateSet->setAttributeAndModes( fn, osg::StateAttribute::ON );

    stateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );

    // Set base class transfer function and volume texture uniforms.
	addHardwareFeatureUniforms( stateSet.get() );

	osg::Program* program = new osg::Program();
	stateSet->setAttribute( program );
	program->addShader( loadShader( osg::Shader::VERTEX, "lfx-volumetricslice.vs" ) );
	program->addShader( loadShader( osg::Shader::FRAGMENT, "lfx-volumetricslice.fs" ) );

	return( stateSet.release() );
}

void VolumeRenderer::setMaxSlices( const unsigned int& maxSlices )
{
	_maxSlices = maxSlices;
}
unsigned int VolumeRenderer::getMaxSlices() const
{
	return( _maxSlices );
}

void VolumeRenderer::setPlaneSpacing( const float& planeSpacing )
{
	_planeSpacing = planeSpacing;
}
float VolumeRenderer::getPlaneSpacing() const
{
	return( _planeSpacing );
}
// lfx
}
