#include "shapeVolumes.h"

using namespace lfx::core;

CubeVolumeBrickData::CubeVolumeBrickData( const bool prune, const bool soft )
	: VolumeBrickData( prune ),
	_brickRes( 32, 32, 32 ),
	_cubeMin( .2, .2, .2 ),
	_cubeMax( .8, .8, .8 ),
	_soft( soft )
{
}

osg::Image* CubeVolumeBrickData::getBrick( const osg::Vec3s& brickNum ) const
{
	const int idx( brickIndex( brickNum ) );
	if( ( idx < 0 ) || ( idx >= _images.size() ) )
	{
		return( NULL );
	}


	const osg::Vec3f brick( brickNum[0], brickNum[1], brickNum[2] );
	const osg::Vec3f invNumBricks( 1. / _numBricks[0], 1. / _numBricks[1], 1. / _numBricks[2] );

	// Compute brick min, max, and extents in normalized 0 <-> 1 volume coordinates.
	const osg::Vec3f brickMin( brick[0] * invNumBricks[0], brick[1] * invNumBricks[1],
		brick[2] * invNumBricks[2] );
	const osg::Vec3f brickMax( brickMin + invNumBricks );
	const osg::Vec3f extent( brickMax - brickMin );

	if( _prune && pruneTest( brickMin, brickMax ) )
	{
		return( NULL );
	}

	unsigned char* data( new unsigned char[ _brickRes[0] * _brickRes[1] * _brickRes[2] ] );
	unsigned char* ptr( data );
	for( int rIdx = 0; rIdx < _brickRes[2]; ++rIdx )
	{
		const float rVal( ( float )rIdx / ( float )_brickRes[2] * extent[2] + brickMin[2] );
		for( int tIdx = 0; tIdx < _brickRes[1]; ++tIdx )
		{
			const float tVal( ( float )tIdx / ( float )_brickRes[1] * extent[1] + brickMin[1] );
			for( int sIdx = 0; sIdx < _brickRes[0]; ++sIdx )
			{

				if (checkCancel())
				{
					throw std::exception("cube brick volume creation canceled");
				}

				const float sVal( ( float )sIdx / ( float )_brickRes[0] * extent[0] + brickMin[0] );

				if( ( sVal >= _cubeMin[0] ) && ( sVal <= _cubeMax[0] ) &&
					( tVal >= _cubeMin[1] ) && ( tVal <= _cubeMax[1] ) &&
					( rVal >= _cubeMin[2] ) && ( rVal <= _cubeMax[2] ) )
				{
					if( !_soft )
					{
						*ptr++ = 255;
					}
					else
					{
						float val( sVal + tVal );
						if( val > 1. )
						{
							val = 2. - val;
						}
						*ptr++ = ( unsigned char )( val * 255 );
					}
				}
				else
				{
					*ptr++ = 0;
				}
			}
		}
	}

	osg::ref_ptr< osg::Image > image( new osg::Image() );
	image->setImage( _brickRes[0], _brickRes[1], _brickRes[2],
		GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
		( unsigned char* ) data, osg::Image::USE_NEW_DELETE );
	return( image.release() );
}

bool CubeVolumeBrickData::pruneTest( const osg::Vec3f& bMin, const osg::Vec3f& bMax ) const
{
	return( ( bMin[0] > _cubeMax[0] ) ||
		( bMax[0] < _cubeMin[0] ) ||
		( bMin[1] > _cubeMax[1] ) ||
		( bMax[1] < _cubeMin[1] ) ||
		( bMin[2] > _cubeMax[2] ) ||
		( bMax[2] < _cubeMin[2] )
		);
}



SphereVolumeBrickData::SphereVolumeBrickData( const bool prune, const bool soft )
	: VolumeBrickData( prune ),
	_brickRes( 32, 32, 32 ),
	_center( .5, .5, .5 ),
	_minRad( soft ? .1f : .45f ),
	_maxRad( .45f )
{
}

osg::Image* SphereVolumeBrickData::getBrick( const osg::Vec3s& brickNum ) const
{
	const int idx( brickIndex( brickNum ) );
	if( ( idx < 0 ) || ( idx >= _images.size() ) )
	{
		return( NULL );
	}


	const osg::Vec3f brick( brickNum[0], brickNum[1], brickNum[2] );
	const osg::Vec3f invNumBricks( 1. / _numBricks[0], 1. / _numBricks[1], 1. / _numBricks[2] );

	// Compute brick min, max, and extents in normalized 0 <-> 1 volume coordinates.
	const osg::Vec3f brickMin( brick[0] * invNumBricks[0], brick[1] * invNumBricks[1],
		brick[2] * invNumBricks[2] );
	const osg::Vec3f brickMax( brickMin + invNumBricks );
	const osg::Vec3f extent( brickMax - brickMin );

	if( _prune && pruneTest( brickMin, brickMax ) )
	{
		return( NULL );
	}

	const float transition( _maxRad - _minRad );

	unsigned char* data( new unsigned char[ _brickRes[0] * _brickRes[1] * _brickRes[2] ] );
	unsigned char* ptr( data );
	for( int rIdx = 0; rIdx < _brickRes[2]; ++rIdx )
	{
		const float rVal( ( float )rIdx / ( float )_brickRes[2] * extent[2] + brickMin[2] );
		for( int tIdx = 0; tIdx < _brickRes[1]; ++tIdx )
		{
			const float tVal( ( float )tIdx / ( float )_brickRes[1] * extent[1] + brickMin[1] );
			for( int sIdx = 0; sIdx < _brickRes[0]; ++sIdx )
			{
				if (checkCancel())
				{
					throw std::exception("sphere brick volume creation canceled");
				}


				const float sVal( ( float )sIdx / ( float )_brickRes[0] * extent[0] + brickMin[0] );

				const osg::Vec3f pt( sVal - _center[0], tVal - _center[1], rVal - _center[2] );
				const float length( pt.length() );

				if( length <= _minRad )
				{
					*ptr++ = 255;
				}
				else if( length > _maxRad )
				{
					*ptr++ = 0;
				}
				else
				{
					float norm( ( length - _minRad ) / transition );
					*ptr++ = ( unsigned char )( ( 1.f - norm ) * 255.f );
				}
			}
		}
	}

	osg::ref_ptr< osg::Image > image( new osg::Image() );
	image->setImage( _brickRes[0], _brickRes[1], _brickRes[2],
		GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
		( unsigned char* ) data, osg::Image::USE_NEW_DELETE );
	return( image.release() );
}

bool SphereVolumeBrickData::pruneTest( const osg::Vec3f& bMin, const osg::Vec3f& bMax ) const
{
	const osg::Vec3f p0( bMin[0] - .5f, bMin[1] - .5f, bMin[2] - .5f );
	const osg::Vec3f p1( bMax[0] - .5f, bMin[1] - .5f, bMin[2] - .5f );
	const osg::Vec3f p2( bMin[0] - .5f, bMax[1] - .5f, bMin[2] - .5f );
	const osg::Vec3f p3( bMax[0] - .5f, bMax[1] - .5f, bMin[2] - .5f );
	const osg::Vec3f p4( bMin[0] - .5f, bMin[1] - .5f, bMax[2] - .5f );
	const osg::Vec3f p5( bMax[0] - .5f, bMin[1] - .5f, bMax[2] - .5f );
	const osg::Vec3f p6( bMin[0] - .5f, bMax[1] - .5f, bMax[2] - .5f );
	const osg::Vec3f p7( bMax[0] - .5f, bMax[1] - .5f, bMax[2] - .5f );

	return( ( p0.length() > _maxRad ) &&
		( p1.length() > _maxRad ) &&
		( p2.length() > _maxRad ) &&
		( p3.length() > _maxRad ) &&
		( p4.length() > _maxRad ) &&
		( p5.length() > _maxRad ) &&
		( p6.length() > _maxRad ) &&
		( p7.length() > _maxRad )
		);
}

ConeVolumeBrickData::ConeVolumeBrickData( const bool prune )
	: VolumeBrickData( prune ),
	_brickRes( 32, 32, 32 ),
	_baseRad( .475f )
{}

osg::Image* ConeVolumeBrickData::getBrick( const osg::Vec3s& brickNum ) const
{
	const int idx( brickIndex( brickNum ) );
	if( ( idx < 0 ) || ( idx >= _images.size() ) )
	{
		return( NULL );
	}


	const osg::Vec3f brick( brickNum[0], brickNum[1], brickNum[2] );
	const osg::Vec3f invNumBricks( 1. / _numBricks[0], 1. / _numBricks[1], 1. / _numBricks[2] );

	// Compute brick min, max, and extents in normalized 0 <-> 1 volume coordinates.
	const osg::Vec3f brickMin( brick[0] * invNumBricks[0], brick[1] * invNumBricks[1],
		brick[2] * invNumBricks[2] );
	const osg::Vec3f brickMax( brickMin + invNumBricks );
	const osg::Vec3f extent( brickMax - brickMin );

	if( _prune && pruneTest( brickMin, brickMax ) )
	{
		//std::cout << "Pruning " << brickNum << std::endl;
		return( NULL );
	}

	unsigned char* data( new unsigned char[ _brickRes[0] * _brickRes[1] * _brickRes[2] ] );
	unsigned char* ptr( data );
	for( int rIdx = 0; rIdx < _brickRes[2]; ++rIdx )
	{
		const float rVal( ( float )rIdx / ( float )_brickRes[2] * extent[2] + brickMin[2] );
		const float scaledRad( ( 1.f - rVal ) * _baseRad );
		for( int tIdx = 0; tIdx < _brickRes[1]; ++tIdx )
		{
			const float tVal( ( float )tIdx / ( float )_brickRes[1] * extent[1] + brickMin[1] );
			for( int sIdx = 0; sIdx < _brickRes[0]; ++sIdx )
			{
				if (checkCancel())
				{
					throw std::exception("cone brick volume creation canceled");
				}


				const float sVal( ( float )sIdx / ( float )_brickRes[0] * extent[0] + brickMin[0] );

				const osg::Vec2f pt( sVal - .5f, tVal - .5f );
				const float length( pt.length() );

				if( length <= scaledRad )
				{
					*ptr++ = 255;
				}
				else
				{
					*ptr++ = 0;
				}
			}
		}
	}

	osg::ref_ptr< osg::Image > image( new osg::Image() );
	image->setImage( _brickRes[0], _brickRes[1], _brickRes[2],
		GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
		( unsigned char* ) data, osg::Image::USE_NEW_DELETE );
	return( image.release() );
}

bool ConeVolumeBrickData::pruneTest( const osg::Vec3f& bMin, const osg::Vec3f& bMax ) const
{
	const osg::Vec2f p0( bMin[0] - .5f, bMin[1] - .5f );
	const osg::Vec2f p1( bMax[0] - .5f, bMin[1] - .5f );
	const osg::Vec2f p2( bMin[0] - .5f, bMax[1] - .5f );
	const osg::Vec2f p3( bMax[0] - .5f, bMax[1] - .5f );
	const float scaledRad( ( 1.f - bMin[2] ) * _baseRad );

	return( ( p0.length() > scaledRad ) &&
		( p1.length() > scaledRad ) &&
		( p2.length() > scaledRad ) &&
		( p3.length() > scaledRad ) );
}
