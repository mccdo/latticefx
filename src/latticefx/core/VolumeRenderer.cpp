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

#include <latticefx/core/VolumeRenderer.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/TextureUtils.h>
#include <latticefx/core/LogMacros.h>

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
#include <osg/Depth>


namespace lfx {


SpatialVolume::SpatialVolume()
  : _volumeDims( osg::Vec3( 1., 1., 1. ) ),
    _volumeOrigin( osg::Vec3( 0., 0., 0. ) )
{
}
SpatialVolume::SpatialVolume( const SpatialVolume& rhs )
  : _volumeDims( rhs._volumeDims ),
    _volumeOrigin( rhs._volumeOrigin )
{
}
SpatialVolume::~SpatialVolume()
{
}

void SpatialVolume::setVolumeDims( const osg::Vec3& volDims)
{
	_volumeDims = volDims;
}
osg::Vec3 SpatialVolume::getVolumeDims() const
{
	return( _volumeDims );
}

void SpatialVolume::setVolumeOrigin( const osg::Vec3& volOrigin)
{
	_volumeOrigin = volOrigin;
}
osg::Vec3 SpatialVolume::getVolumeOrigin() const
{
	return( _volumeOrigin );
}




VolumeRenderer::VolumeRenderer()
  : Renderer( "vol" ),
    _maxSlices( 1024 ),
    _planeSpacing( 1.f )
{
}
VolumeRenderer::VolumeRenderer( const VolumeRenderer& rhs )
  : Renderer( rhs ),
    _maxSlices( rhs._maxSlices ),
    _planeSpacing( rhs._planeSpacing )
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
	osg::BoundingBox bb( (_volumeDims * -.5) + _volumeOrigin, (_volumeDims * .5) + _volumeOrigin);
	geom->setInitialBound( bb );
	// Add geometric data and the PrimitiveSet. Specify numInstances as _maxSlices.
	createDAIGeometry( *geom, _maxSlices );
	geode->addDrawable( geom );


	osg::StateSet* stateSet( geode->getOrCreateStateSet() );

    ChannelDataPtr dataPtr( getInput( "volumedata" ) );
    if( dataPtr == NULL )
    {
        LFX_WARNING( "getSceneGraph(): Unable to find required volumedata ChannelData." );
        return( NULL );
    }
    ChannelDataOSGImage* dataImagePtr( static_cast<
        ChannelDataOSGImage* >( dataPtr.get() ) );
    osg::Image* volumeImage( dataImagePtr->getImage() );

    osg::Texture3D* volumeTexture = new osg::Texture3D( volumeImage );
	volumeTexture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
	volumeTexture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
	volumeTexture->setWrap(osg::Texture2D::WRAP_R, osg::Texture2D::CLAMP);
	volumeTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP);
	volumeTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP);
    volumeTexture->setBorderColor( osg::Vec4d( 0., 0., 0., 0. ) );
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

	// Setup uniforms for VolumeDims, VolumeCenter, PlaneSpacing, LightPosition, Diffuse and ambient lights
    osg::Uniform* dimsUni( new osg::Uniform( "VolumeDims", osg::Vec3f( _volumeDims ) ) );
    stateSet->addUniform( dimsUni );
    osg::Uniform* centerUni( new osg::Uniform( "VolumeCenter", osg::Vec3f( _volumeOrigin ) ) );
    stateSet->addUniform( centerUni );
    osg::Uniform* spaceUni( new osg::Uniform( "PlaneSpacing", _planeSpacing ) );
    stateSet->addUniform( spaceUni );

	osg::BlendFunc *fn = new osg::BlendFunc();
	fn->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
	stateSet->setAttributeAndModes( fn, osg::StateAttribute::ON );

    osg::Depth* depth( new osg::Depth( osg::Depth::LESS, 0., 1., false ) );
    stateSet->setAttributeAndModes( depth, osg::StateAttribute::ON );

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
