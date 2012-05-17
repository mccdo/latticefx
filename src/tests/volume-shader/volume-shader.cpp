/* OpenSceneGraph example, osgdrawinstanced.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/
//
// This code is copyright (c) 2008 Skew Matrix Software LLC. You may use
// the code under the licensing terms described above.
//

#include <iostream>

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osgDB/FileUtils>
#include <osgDB/WriteFile>
#include <osg/BlendFunc>
#include <osg/Depth>

///How many slices, maximum
#define MAX_SLICES	1024

// How far apart the slices will be, in world-units
#define PLANE_SPACING 0.3f

// size in world-units of the volume box
#define VOLUME_DIMS 60.0f, 60.0f, 30.0f

// location in world-units of the center of the volume
#define VOLUME_ORIGIN 0.0f, 0.0f, -0.0f

/// Texture unit for volume data
#define TEXUNIT_VOL                 4
/// Texture unit for transfer function
#define TEXUNIT_XFR                 5

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


osg::StateSet* createStateSet(std::string volumeName, std::string transferName)
{
	osg::ref_ptr< osg::StateSet > ss = new osg::StateSet;

	osg::ref_ptr< osg::Program > program = new osg::Program();
	ss->setAttribute( program.get() );
	osg::Shader* vertexShader = new osg::Shader( osg::Shader::VERTEX );
	vertexShader->loadShaderSourceFromFile( osgDB::findDataFile( "lfx-volumetricslice.vs" ) );
	program->addShader( vertexShader );
	osg::Shader* fragmentShader = new osg::Shader( osg::Shader::FRAGMENT );
	fragmentShader->loadShaderSourceFromFile( osgDB::findDataFile( "lfx-volumetricslice.fs" ) );
	program->addShader( fragmentShader );

	ss->setAttribute( program.get(),
		osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

	osg::ref_ptr< osg::Image> volumeImage = osgDB::readImageFile( volumeName );
	if( !volumeImage.valid() )
	{
		osg::notify( osg::ALWAYS ) << "Can't open image file " << volumeName << std::endl;
		return( NULL );
	}
	osg::Texture3D* volumeTexture = new osg::Texture3D( volumeImage.get() );
	volumeTexture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
	volumeTexture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
	volumeTexture->setWrap(osg::Texture2D::WRAP_R, osg::Texture2D::CLAMP_TO_EDGE);
	volumeTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
	volumeTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);

	osg::ref_ptr< osg::Image> transferImage = osgDB::readImageFile( transferName );
	if( !transferImage.valid() )
	{
		osg::notify( osg::ALWAYS ) << "Can't open image file " << transferName << std::endl;
		return( NULL );
	}
	osg::Texture2D* transferTexture = new osg::Texture2D( transferImage.get() );
	transferTexture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
	transferTexture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
	transferTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
	transferTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);

	ss->setTextureAttribute( TEXUNIT_VOL, volumeTexture );
	ss->setTextureAttribute( TEXUNIT_XFR, transferTexture );

	// 3D volume texture image
	osg::ref_ptr< osg::Uniform > volumeUniform = new osg::Uniform( "VolumeTexture", TEXUNIT_VOL );
	ss->addUniform( volumeUniform.get() );

	// 2D transfer function texture (can be 1D image too)
	osg::ref_ptr< osg::Uniform > transferUniform = new osg::Uniform( "TransferFunction", TEXUNIT_XFR );
	ss->addUniform( transferUniform.get() );

	//world-space size of volume box
	osg::ref_ptr< osg::Uniform > volumeDimsUniform = new osg::Uniform( "VolumeDims", osg::Vec3(VOLUME_DIMS) );
	ss->addUniform( volumeDimsUniform.get() );

	// world-space position of center of volume box
	osg::ref_ptr< osg::Uniform > volumeCenterUniform = new osg::Uniform( "VolumeCenter", osg::Vec3(VOLUME_ORIGIN) );
	ss->addUniform( volumeCenterUniform.get() );

	// world-space distance between slice planes
	osg::ref_ptr< osg::Uniform > planeSpacingUniform = new osg::Uniform( "PlaneSpacing", PLANE_SPACING );
	ss->addUniform( planeSpacingUniform.get() );

	// render state
	ss->setMode( GL_CULL_FACE, osg::StateAttribute::ON );

	osg::BlendFunc *fn = new osg::BlendFunc();
	ss->setAttributeAndModes( fn, osg::StateAttribute::ON );
	fn->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask(false); // don't bother writing the depth buffer
	ss->setAttributeAndModes(depth, osg::StateAttribute::ON );

	return( ss.release() );
}


int main( int argc, char **argv )
{
	osg::ArgumentParser arguments(&argc, argv);

	// Make a scene graph consisting of a single Geode, containing
	// a single Geometry, and a single PrimitiveSet.
	osg::ref_ptr< osg::Geode > geode = new osg::Geode;

	osg::ref_ptr< osg::Geometry > geom = new osg::Geometry;
	// Configure the Geometry for use with EXT_draw_arrays:
	// DL off and buffer objects on.
	geom->setUseDisplayList( false );
	geom->setUseVertexBufferObjects( true );
	// OSG has no clue where out vertex shader will place the geometric data,
	// so specify an initial bound to allow proper culling and near/far computation.
	osg::BoundingBox bb( -60., -.60, -60., 60., 60., 60. );
	geom->setInitialBound( bb );
	// Add geometric data and the PrimitiveSet. Specify numInstances as 1024.
	createDAIGeometry( *geom, MAX_SLICES );
	geode->addDrawable( geom.get() );

	// Create a StateSet to render the instanced Geometry.
	osg::ref_ptr< osg::StateSet > ss = createStateSet("HeadVolume.dds", "Spectrum.png");

	geode->setStateSet( ss.get() );

	// osgDB::writeNodeFile(*geode, "volume.osgt");

	osgViewer::Viewer viewer(arguments);
	viewer.setSceneData( geode.get() );
	return viewer.run();
}
