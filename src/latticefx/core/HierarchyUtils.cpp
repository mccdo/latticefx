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

#include <latticefx/core/HierarchyUtils.h>
#include <latticefx/core/ChannelData.h>
#include <latticefx/core/ChannelDataImageSet.h>
#include <latticefx/core/ChannelDataLOD.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/LogMacros.h>
#include <latticefx/core/PageData.h>
#include <latticefx/core/DBUtils.h>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <Poco/Path.h>
#include <Poco/Glob.h>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include <sstream>
#include <cstring>
#include <string>


namespace lfx
{
namespace core
{

TraverseHierarchy::TraverseHierarchy( ChannelDataPtr root, HierarchyCallback& cb )
    : _root( root ),
      _cb( cb )
{
    execute();
}
TraverseHierarchy::TraverseHierarchy( ChannelDataPtr root )
    : _root( root )
{
}

void TraverseHierarchy::execute()
{
    traverse( _root );
}
void TraverseHierarchy::traverse( ChannelDataPtr cdp )
{
    ChannelDataImageSet* imageData( NULL );
    ChannelDataLOD* lodData( NULL );
    ChannelDataComposite* comp( dynamic_cast< ChannelDataComposite* >( cdp.get() ) );
    if( comp != NULL )
    {
        imageData = comp->getAsSet();
        lodData = comp->getAsLOD();
    }

    if( lodData != NULL )
    {
        for( unsigned int idx = 0; idx < lodData->getNumChannels(); ++idx )
        {
            traverse( lodData->getChannel( idx ) );
        }
    }
    else if( imageData != NULL )
    {
        for( unsigned int idx = 0; idx < imageData->getNumChannels(); ++idx )
        {
            _name.push_back( idx );
            traverse( imageData->getChannel( idx ) );
            _name.pop_back();
        }
    }
    else
    {
        _cb( cdp, toString( _name ) );
    }
}

std::string TraverseHierarchy::toString( const NameList& nameList )
{
    std::ostringstream ostr;
    BOOST_FOREACH( int idx, nameList )
    {
        ostr << idx;
    }
    return( ostr.str() );
}




PruneHierarchy::PruneHierarchy( ChannelDataPtr root )
{
    traverse( root );
}

int PruneHierarchy::traverse( ChannelDataPtr cdp )
{
    if( cdp == NULL )
    {
        return( 0 );
    }

    ChannelDataComposite* comp( dynamic_cast< ChannelDataComposite* >( cdp.get() ) );
    if( comp != NULL )
    {
        for( unsigned int idx = comp->getNumChannels(); idx > 0; --idx )
        {
            if( traverse( comp->getChannel( idx - 1 ) ) == 0 )
            {
                comp->removeChannel( idx - 1 );
            }
        }

		// resolution prune check.
		// if the child nodes were pruned out because they added no resolution value..
		// we need to adjust the lod distance so this node doesn't get switched out and cause popping, 
		// it needs to stay visible.
		//
		if( comp->getNumChannels() == 1 )
		{
			ChannelDataLOD *pLod = comp->getAsLOD();
			if (pLod != NULL)
			{
				std::string name = pLod->getName();
				LFX_DEBUG_STATIC( "lfx.core.hier", "Resolution Prune at Brick: " + name );
				pLod->getRange( 0 ).second = FLT_MAX;
				//pLod->getRange( 1 ).first = FLT_MAX;
			}
		}

        return( comp->getNumChannels() );
    }
    else
    {
        return( 1 );
    }
}




AssembleHierarchy::AssembleHierarchy( RangeVec ranges )
    : _ranges( ranges )
{
    if( _ranges.size() < 1 )
    {
        const std::string errorText( "AssembleHierarchy: RangeVec must have at least 1 element." );
        LFX_ERROR_STATIC( "lfx.core.hier", errorText );
        throw( std::runtime_error( errorText ) );
    }

    _root = boost::static_pointer_cast< ChannelData >( ChannelDataLODPtr( new ChannelDataLOD() ) );
    recurseInit( _root, 0 );
}
AssembleHierarchy::AssembleHierarchy( unsigned int maxDepth, double baseRange )
{
    if( maxDepth < 2 )
    {
        const std::string errorText( "AssembleHierarchy: maxDepth must be >= 2." );
        LFX_ERROR_STATIC( "lfx.core.hier", errorText );
        throw( std::runtime_error( errorText ) );
    }
    if( baseRange <= 0. )
    {
        const std::string errorText( "AssembleHierarchy: baseRange must be > 0." );
        LFX_ERROR_STATIC( "lfx.core.hier", errorText );
        throw( std::runtime_error( errorText ) );
    }

    double range( baseRange );
    unsigned int idx( maxDepth - 1 );
    do
    {
        _ranges.push_back( range );
        range *= 4.;
        --idx;
    }
    while( idx > 0 );

    _root = boost::static_pointer_cast< ChannelData >( ChannelDataLODPtr( new ChannelDataLOD() ) );
    recurseInit( _root, 0 );
}
AssembleHierarchy::AssembleHierarchy( const AssembleHierarchy& rhs )
    : _ranges( rhs._ranges )
{
    _root = boost::static_pointer_cast< ChannelData >( ChannelDataLODPtr( new ChannelDataLOD() ) );
    recurseInit( _root, 0 );
}
AssembleHierarchy::~AssembleHierarchy()
{
}

void AssembleHierarchy::addChannelData( ChannelDataPtr cdp, const std::string nameString,
                                        const osg::Vec3& offset, const unsigned int depth, const std::string namePrev )
{
    if( depth == 0 )
        // Special-case the initial condition.
    {
        _iterator = _root;
    }

    osg::Vec3 localOffset( offset );
    if( !( nameString.empty() ) && ( offset.length2() == 0. ) )
    {
        // No offset passed as a parameter. Interpret offset
        // from last digit of name string.
        // This can only happen when addChannelData is called from the app.
        //   Once we start recursing, offset will always be non-zero.
        std::string::const_iterator iter = nameString.end() - 1;

        switch( *iter )
        {
        case '0':
            localOffset.set( -1., -1., -1. );
            break;
        case '1':
            localOffset.set( 1., -1., -1. );
            break;
        case '2':
            localOffset.set( -1., 1., -1. );
            break;
        case '3':
            localOffset.set( 1., 1., -1. );
            break;
        case '4':
            localOffset.set( -1., -1., 1. );
            break;
        case '5':
            localOffset.set( 1., -1., 1. );
            break;
        case '6':
            localOffset.set( -1., 1., 1. );
            break;
        case '7':
            localOffset.set( 1., 1., 1. );
            break;
        default:
        {
            std::string message = "addChannelData: Invalid nameString digit: ";
            message.push_back( *iter );
            LFX_ERROR_STATIC( "lfx.core.hier", message );
            return;
            break;
        }
        }
    }

    // Walk down the hierarchy. We will encounter 1 of 3 situations.
    // Case 1: The nameString is empty. If so, the current iterator is an LOD and we
    //      must insert cdp as child 0 of that LOD.
    //
    // If the nameString is not empty, the second child of the LOD is an ImageSet.
    // We use the nameString to determine the ImageSet's child of interest.
    // Case 2: If that child is NULL, we're at the bottom of the hierarchy and must
    //     insert cdp as the child of the ImageSet.
    // Case 3: If the child is not NULL, recurse.

    ChannelDataComposite* comp( dynamic_cast< ChannelDataComposite* >( _iterator.get() ) );
    if( comp == NULL )
    {
        LFX_ERROR_STATIC( "lfx.core.hier", "addChannelData: Unexpected non-Composite." );
        return;
    }
    ChannelDataLOD* lodData( comp->getAsLOD() );
    if( lodData == NULL )
    {
        LFX_ERROR_STATIC( "lfx.core.hier", "addChannelData: Unexpected non-LOD." );
        return;
    }

    if( nameString.empty() )
    {
        // Case 1
        // This is the lower LOD at this level. The other (higher) LOD is an ImageSet.
        lodData->setChannel( 0, cdp );
        // Set to NULL for debugging purposes (not actually necessary).
        _iterator = ChannelDataPtr( ( ChannelData* )NULL );
        return;
    }

    _iterator = lodData->getChannel( 1 ); // Get second child.
    comp = dynamic_cast< ChannelDataComposite* >( _iterator.get() );
    if( comp == NULL )
    {
        LFX_ERROR_STATIC( "lfx.core.hier", "addChannelData: Unexpected non-Composite." );
        return;
    }
    ChannelDataImageSet* imageData( comp->getAsSet() );
    if( imageData == NULL )
    {
        LFX_ERROR_STATIC( "lfx.core.hier", "addChannelData: Unexpected non-ImageSet." );
        return;
    }

    std::string::const_iterator iter = nameString.begin();
    const unsigned int childIndex( ( unsigned int )( *iter - '0' ) );
    if( imageData->getChannel( childIndex ) == NULL )
    {
        // Case 2
        // This is the insertion point.
        imageData->setChannel( childIndex, cdp );
        imageData->setOffset( childIndex, localOffset );
        // Set to NULL for debugging purposes (not actually necessary).
        _iterator = ChannelDataPtr( ( ChannelData* )NULL );
    }
    else
    {
        // Case 3 Recurse
        const std::string newName( nameString.substr( 1 ) );
        if( newName.empty() )
        {
            // Next recursion is the insersion point, so set offset here.
            imageData->setOffset( childIndex, localOffset );
        }

        _iterator = imageData->getChannel( childIndex );

		if (_iterator != NULL)
		{
			_iterator->setName( namePrev ); // TODO: REMOVE
		}

        addChannelData( cdp, newName, localOffset, depth + 1, nameString );
    }
}

void AssembleHierarchy::prune()
{
    PruneHierarchy ph( _root );
}



void AssembleHierarchy::recurseInit( ChannelDataPtr cdp, unsigned int depth )
{
    ChannelDataComposite* comp( dynamic_cast< ChannelDataComposite* >( cdp.get() ) );
    if( comp == NULL )
    {
        LFX_ERROR_STATIC( "lfx.core.hier", "recurseInit: Unexpected non-Composite." );
        return;
    }
    ChannelDataLOD* cdLOD( comp->getAsLOD() );
    if( cdLOD == NULL )
    {
        LFX_ERROR_STATIC( "lfx.core.hier", "recurseInit: Unexpected non-LOD." );
        return;
    }

    cdLOD->reserveChannels( 2 );
    const double rangeVal( _ranges[ depth ] );
    cdLOD->setRange( 0, RangeValues( 0., rangeVal ) );
    cdLOD->setRange( 1, RangeValues( rangeVal, FLT_MAX ) );

    ChannelDataImageSetPtr cdImage( new ChannelDataImageSet() );
    cdLOD->setChannel( 1, cdImage );

    // For ChannelDataImageSet, reserveChannels resizes both the
    // ChannelData vector, and also the offset vector.
    cdImage->reserveChannels( 8 );

    if( depth + 1 < _ranges.size() )
    {
        // We haven't hit max depth yet, so we'll add LOD children and recurse.
        // Otherwise we do nothing, as the children will be inserted by the app
        // with a call to addChannelData().
        for( unsigned int idx = 0; idx < 8; ++idx )
        {
            ChannelDataLODPtr newLOD( new ChannelDataLOD() );
            cdImage->setChannel( idx, newLOD );
            recurseInit( newLOD, depth + 1 );
        }
    }
}



NameStringGenerator::NameStringGenerator( const osg::Vec3s& dimensions )
    : _dims( dimensions )
{
}

std::string NameStringGenerator::getNameString( const osg::Vec3s& subDims, const osg::Vec3s& subMinCorner )
{
    // First, find the depth.
    int ratio( _dims.x() / subDims.x() );
    if( ratio & ratio - 1 )
    {
        LFX_WARNING_STATIC( "lfx.core.hier", "getNameString: _dims.x()/subDims.x() must be a power of 2." );
    }
    int depth( 1 );
    while( ( ratio >>= 1 ) > 0 )
    {
        depth++;
    }

    // If depth is 1, we have the root of the hierarchy, so the
    // name string is the empty string.
    if( depth == 1 )
    {
        return( std::string( "" ) );
    }

    // Accumulate the name string.
    const osg::Vec3s subCenter( subDims / 2 + subMinCorner );
    osg::Vec3s octant( _dims / 2 );
    std::string nameString;
    for( int idx = 1; idx < depth; idx++ )
    {
        int octantValue( 0 );
        if( subCenter.x() > octant.x() )
        {
            octantValue += ( 1 << 0 );
        }
        if( subCenter.y() > octant.y() )
        {
            octantValue += ( 1 << 1 );
        }
        if( subCenter.z() > octant.z() )
        {
            octantValue += ( 1 << 2 );
        }

        switch( octantValue )
        {
        case 0:
            nameString += "0";
            break;
        case 1:
            nameString += "1";
            break;
        case 2:
            nameString += "2";
            break;
        case 3:
            nameString += "3";
            break;
        case 4:
            nameString += "4";
            break;
        case 5:
            nameString += "5";
            break;
        case 6:
            nameString += "6";
            break;
        case 7:
            nameString += "7";
            break;
        default:
            break;
        }

        octant = octant / 2;
    }

    return( nameString );
}

std::string NameStringGenerator::getNameString( osg::Vec3s& offset, const osg::Vec3s& subDims, const osg::Vec3s& subMinCorner )
{
    const std::string nameString( getNameString( subDims, subMinCorner ) );

    if( !( nameString.empty() ) )
    {
        std::string::const_iterator iter = nameString.end() - 1;

        switch( *iter )
        {
        case '0':
            offset.set( -1., -1., -1. );
            break;
        case '1':
            offset.set( 1., -1., -1. );
            break;
        case '2':
            offset.set( -1., 1., -1. );
            break;
        case '3':
            offset.set( 1., 1., -1. );
            break;
        case '4':
            offset.set( -1., -1., 1. );
            break;
        case '5':
            offset.set( 1., -1., 1. );
            break;
        case '6':
            offset.set( -1., 1., 1. );
            break;
        case '7':
            offset.set( 1., 1., 1. );
            break;
        default:
        {
            std::string message = "getNameString: Invalid nameString digit: ";
            message.push_back( *iter );
            LFX_ERROR_STATIC( "lfx.core.hier", message );
            break;
        }
        }
    }

    return( nameString );
}




VolumeBrickData::VolumeBrickData( const bool prune, bool resPrune )
    : _numBricks( 8, 8, 8 ),
      _depth( 4 ),
      _prune( prune ),
	  _resPrune( resPrune ),
      _imageCacheMaxSize( 8 ),
	  _pcbProgress( NULL )
{
}
VolumeBrickData::~VolumeBrickData()
{
}

void VolumeBrickData::setDepth( const unsigned int depth )
{
    _depth = depth;
    if( _depth < 1 )
    {
        LFX_WARNING_STATIC( "lfx.core.hier", "Depth < 1 invalid. Using depth 1." );
        _depth = 1;
    }
    else if( _depth > 15 )
    {
        LFX_WARNING_STATIC( "lfx.core.hier", "Depth > 15 invalid. Using depth 15." );
        _depth = 16;
    }
    short n( 0x1 << ( _depth - 1 ) );
    _numBricks.set( n, n, n );
    unsigned short usn( ( unsigned short )n );
    _images.resize( usn * usn * usn );
}
unsigned int VolumeBrickData::getDepth() const
{
    return( _depth );
}
osg::Vec3s VolumeBrickData::getNumBricks() const
{
    return( _numBricks );
}

unsigned int VolumeBrickData::getBrickCount() const
{
	return (_numBricks[0]) * (_numBricks[1]) * (_numBricks[2]);
}

void VolumeBrickData::addBrick( const osg::Vec3s& brickNum, osg::Image* image )
{
    const int idx( brickIndex( brickNum ) );
    if( ( idx < 0 ) || ( idx >= _images.size() ) )
    {
        return;
    }
    else
    {
        _images[ idx ] = image;
        cacheImage( brickNum, image );
    }
}
osg::Image* VolumeBrickData::getBrick( const osg::Vec3s& brickNum ) const
{
    const int idx( brickIndex( brickNum ) );
    if( ( idx < 0 ) || ( idx >= _images.size() ) )
    {
        return( NULL );
    }
    else
    {
        osg::Image* image( getCachedImage( brickNum ) );
        if( image != NULL )
            return( image );
        else
            return( _images[ idx ].get() );
    }
}
osg::Image* VolumeBrickData::getBrick( const std::string& brickName ) const
{
    const osg::Vec3s brickNum( nameToBrickNum( brickName ) );
    if( brickNum == osg::Vec3s( -1, -1, -1 ) )
    {
        return( NULL );
    }

    return( getBrick( brickNum ) );
}

osg::Image* VolumeBrickData::getSeamlessBrick( const osg::Vec3s& brickNum ) const
{
    osg::ref_ptr< osg::Image > dest;

    for( int idx = 0; idx < 8; ++idx )
    {
        osg::Vec3s current( brickNum );
        switch( idx )
        {
        case 0:
            break;
        case 1:
            current += osg::Vec3s( 1, 0, 0 );
            break;
        case 2:
            current += osg::Vec3s( 0, 1, 0 );
            break;
        case 3:
            current += osg::Vec3s( 1, 1, 0 );
            break;
        case 4:
            current += osg::Vec3s( 0, 0, 1 );
            break;
        case 5:
            current += osg::Vec3s( 1, 0, 1 );
            break;
        case 6:
            current += osg::Vec3s( 0, 1, 1 );
            break;
        case 7:
            current += osg::Vec3s( 1, 1, 1 );
            break;
        default:
            LFX_ERROR_STATIC( "lfx.core.hier", "Brick index out of range." );
            break;
        }

        // TBD we might be outside the boundary defined by _numBricks.
        // I think we should 'continue' here if that's the case.

		if (checkCancel())
		{
			return NULL;
		}

        osg::ref_ptr< osg::Image > srcBrick( getBrick( current ) );
        if( srcBrick.valid() )
        {
            if( !( dest.valid() ) )
            {
                dest = newBrick( srcBrick.get() );
            }
            overlap( dest.get(), srcBrick.get(), idx );
        }

		computeProgAndUpdate(1);
    }

    if( dest.valid() )
    {
        return( dest.release() );
    }
    else
    {
        return( NULL );
    }
}
osg::Image* VolumeBrickData::getSeamlessBrick( const std::string& brickName ) const
{
    const osg::Vec3s brickNum( nameToBrickNum( brickName ) );
    if( brickNum == osg::Vec3s( -1, -1, -1 ) )
    {
        return( NULL );
    }

    return( getSeamlessBrick( brickNum ) );
}

bool VolumeBrickData::checkCancel() const
{
	if (!_pcbProgress) return false;
	return _pcbProgress->checkCancel();
}

void VolumeBrickData::computeProgAndUpdate( int add ) const
{
	if (!_pcbProgress) return;
	_pcbProgress->computeProgAndUpdate( add );
}

void VolumeBrickData::sendProgressMsg(const char* msg) const
{
	if (!_pcbProgress) return;
	_pcbProgress->sendMsg( msg );
}

int VolumeBrickData::brickIndex( const osg::Vec3s& brickNum ) const
{
    if( ( brickNum[0] >= _numBricks[0] ) || ( brickNum[0] < 0 ) ||
            ( brickNum[1] >= _numBricks[1] ) || ( brickNum[1] < 0 ) ||
            ( brickNum[2] >= _numBricks[2] ) || ( brickNum[2] < 0 ) )
    {
        return( -1 );
    }

    return( brickNum[2] * _numBricks[0] * _numBricks[1] +
            brickNum[1] * _numBricks[0] +
            brickNum[0] );
}
osg::Vec3s VolumeBrickData::nameToBrickNum( const std::string& name ) const
{
    // Depth check
    short dim( 1 << name.length() );
    if( ( dim != _numBricks[0] ) || ( dim != _numBricks[1] ) || ( dim != _numBricks[2] ) )
    {
        return( osg::Vec3s( -1, -1, -1 ) );
    }

    osg::Vec3s brickNum;
    if( name.empty() )
    {
        return( brickNum );    // 0,0,0
    }

    osg::Vec3s half( _numBricks[0] / 2, _numBricks[1] / 2, _numBricks[2] / 2 );
    for( int idx = 0; idx < name.length(); ++idx )
    {
        int pos( ( char )( name[ idx ] ) - 0 );
        brickNum[0] += ( pos & 0x1 ) ? half[0] : 0;
        brickNum[1] += ( pos & 0x2 ) ? half[1] : 0;
        brickNum[2] += ( pos & 0x4 ) ? half[2] : 0;
        half.set( half[0] / 2, half[1] / 2, half[2] / 2 );
    }

    return( brickNum );
}
void VolumeBrickData::overlap( osg::Image* dest, const osg::Image* source, const unsigned int index ) const
{
    const unsigned int pixSize( dest->getPixelSizeInBits() >> 3 );
    const unsigned int rowSize( source->getRowSizeInBytes() );
    const unsigned int srcS( source->s() );
    const unsigned int srcT( source->t() );
    const unsigned int srcST( srcS * srcT );
    const unsigned int srcR( source->r() );
    const unsigned char* srcData( source->data() );
    const unsigned int destS( dest->s() );
    const unsigned int destT( dest->t() );
    const unsigned int destST( destS * destT );
    const unsigned int destR( dest->r() );
    unsigned char* destData( dest->data() );

#define SRC_ADDR(_s,_t,_r) \
    ( srcData + ( ( (_r) * srcST + (_t) * srcS + (_s) ) * pixSize ) )
#define DEST_ADDR(_s,_t,_r) \
    ( destData + ( ( (_r) * destST + (_t) * destS + (_s) ) * pixSize ) )

    switch( index )
    {
    case 0:
    {
        for( unsigned int rIdx = 0; rIdx < srcR; ++rIdx )
        {
            for( unsigned int tIdx = 0; tIdx < srcT; ++tIdx )
            {
                std::memcpy( DEST_ADDR( 0, tIdx, rIdx ),
                        SRC_ADDR( 0, tIdx, rIdx ), rowSize );
            }
        }
        break;
    }
    case 1:
    {
        for( unsigned int rIdx = 0; rIdx < srcR; ++rIdx )
        {
            for( unsigned int tIdx = 0; tIdx < srcT; ++tIdx )
            {
                std::memcpy( DEST_ADDR( destS - 1, tIdx, rIdx ),
                        SRC_ADDR( 0, tIdx, rIdx ), pixSize );
            }
        }
        break;
    }
    case 2:
    {
        for( unsigned int rIdx = 0; rIdx < srcR; ++rIdx )
        {
            std::memcpy( DEST_ADDR( 0, destT - 1, rIdx ),
                    SRC_ADDR( 0, 0, rIdx ), rowSize );
        }
        break;
    }
    case 3:
    {
        for( unsigned int rIdx = 0; rIdx < srcR; ++rIdx )
        {
            std::memcpy( DEST_ADDR( destS - 1, destT - 1, rIdx ),
                    SRC_ADDR( 0, 0, rIdx ), pixSize );
        }
        break;
    }
    case 4:
    {
        for( unsigned int tIdx = 0; tIdx < srcT; ++tIdx )
        {
            std::memcpy( DEST_ADDR( 0, tIdx, destR - 1 ),
                    SRC_ADDR( 0, tIdx, 0 ), rowSize );
        }
        break;
    }
    case 5:
    {
        for( unsigned int tIdx = 0; tIdx < srcT; ++tIdx )
        {
            std::memcpy( DEST_ADDR( destS - 1, tIdx, destR - 1 ),
                    SRC_ADDR( 0, tIdx, 0 ), pixSize );
        }
        break;
    }
    case 6:
    {
        std::memcpy( DEST_ADDR( 0, destT - 1, destR - 1 ),
                SRC_ADDR( 0, 0, 0 ), rowSize );
        break;
    }
    case 7:
    {
        std::memcpy( DEST_ADDR( destS - 1, destT - 1, destR - 1 ),
                SRC_ADDR( 0, 0, 0 ), pixSize );
        break;
    }
    default:
        break;
    }

#undef SRC_ADDR
#undef DEST_ADDR
}
osg::Image* VolumeBrickData::newBrick( const osg::Image* proto, const osg::Vec3s& overlap ) const
{
    osg::ref_ptr< osg::Image > image( new osg::Image() );

    osg::Vec3s dims( overlap );
    dims[0] += proto->s();
    dims[1] += proto->t();
    dims[2] += proto->r();
    unsigned int pixelSizeBytes( proto->getPixelSizeInBits() >> 3 );
    unsigned int totalSizeBytes( dims[0] * dims[1] * dims[2] * pixelSizeBytes );

    unsigned char* data( new unsigned char[ totalSizeBytes ] );
    memset( data, 0, totalSizeBytes );

    image->setImage( dims[0], dims[1], dims[2],
                     proto->getInternalTextureFormat(), proto->getPixelFormat(), proto->getDataType(),
                     ( unsigned char* ) data, osg::Image::USE_NEW_DELETE );

    return( image.release() );
}

void VolumeBrickData::cacheImage( const osg::Vec3s& brickNum, osg::Image* image ) const
{
    // If the image is already in the cache, remove it first.
    // Then we'll add it so that it's back-most on the std::list.
    osg::Image* cachedImage( getCachedImage( brickNum ) );
    if( cachedImage != NULL )
        removeFromCache( brickNum );

    _imageCache.push_back( ImageCacheData( brickNum, image ) );

    // Limit the cache size. Pop old Images off the front.
    if( _imageCache.size() > _imageCacheMaxSize )
        _imageCache.pop_front();
}
osg::Image* VolumeBrickData::getCachedImage( const osg::Vec3s& brickNum ) const
{
    // Find an Image in the cache and return it, or return NULL
    // if it's not in the cache. This is an O(n) search of the
    // std::list cache, looking for a match with the brickNum key.
    for( ImageCache::const_iterator it = _imageCache.begin();
        it != _imageCache.end(); ++it )
    {
        if( it->first == brickNum )
            return( it->second.get() );
    }
    return( NULL );
}
void VolumeBrickData::removeFromCache( const osg::Vec3s& brickNum ) const
{
    // Find and remove a cached Image. Do nothing if not found.
    for( ImageCache::iterator it = _imageCache.begin();
        it != _imageCache.end(); ++it )
    {
        if( it->first == brickNum )
        {
            _imageCache.erase( it );
            return;
        }
    }
}




Downsampler::Downsampler( const VolumeBrickDataPtr hiRes )
    :
    _hi( hiRes )
{}
Downsampler::~Downsampler()
{
}

VolumeBrickDataPtr Downsampler::getLow() const
{
    if( _low )
    {
        return( _low );
    }
    _low = VolumeBrickDataPtr( new VolumeBrickData() );

    _low->setDepth( _hi->getDepth() - 1 );
    osg::Vec3s numBricks( _low->getNumBricks() );

    for( int rIdx = 0; rIdx < numBricks[2]; ++rIdx )
    {
        for( int tIdx = 0; tIdx < numBricks[1]; ++tIdx )
        {
            for( int sIdx = 0; sIdx < numBricks[0]; ++sIdx )
            {
                osg::ref_ptr< osg::Image > i0( _hi->getBrick( osg::Vec3s( sIdx * 2,   tIdx * 2,   rIdx * 2 ) ) );
                osg::ref_ptr< osg::Image > i1( _hi->getBrick( osg::Vec3s( sIdx * 2 + 1, tIdx * 2,   rIdx * 2 ) ) );
                osg::ref_ptr< osg::Image > i2( _hi->getBrick( osg::Vec3s( sIdx * 2,   tIdx * 2 + 1, rIdx * 2 ) ) );
                osg::ref_ptr< osg::Image > i3( _hi->getBrick( osg::Vec3s( sIdx * 2 + 1, tIdx * 2 + 1, rIdx * 2 ) ) );
                osg::ref_ptr< osg::Image > i4( _hi->getBrick( osg::Vec3s( sIdx * 2,   tIdx * 2,   rIdx * 2 + 1 ) ) );
                osg::ref_ptr< osg::Image > i5( _hi->getBrick( osg::Vec3s( sIdx * 2 + 1, tIdx * 2,   rIdx * 2 + 1 ) ) );
                osg::ref_ptr< osg::Image > i6( _hi->getBrick( osg::Vec3s( sIdx * 2,   tIdx * 2 + 1, rIdx * 2 + 1 ) ) );
                osg::ref_ptr< osg::Image > i7( _hi->getBrick( osg::Vec3s( sIdx * 2 + 1, tIdx * 2 + 1, rIdx * 2 + 1 ) ) );

                osg::Image* image( sample( i0.get(), i1.get(), i2.get(), i3.get(),
                                           i4.get(), i5.get(), i6.get(), i7.get() ) );
                _low->addBrick( osg::Vec3s( sIdx, tIdx, rIdx ), image );

				// do we need to cancel
				if (_hi->checkCancel()) return VolumeBrickDataPtr();
				_hi->computeProgAndUpdate(1);

            }
        }
    }

    return( _low );
}

osg::Image* Downsampler::sample( const osg::Image* i0, const osg::Image* i1, const osg::Image* i2, const osg::Image* i3,
                                 const osg::Image* i4, const osg::Image* i5, const osg::Image* i6, const osg::Image* i7 ) const
{
    osg::ref_ptr< osg::Image > image( new osg::Image() );

    unsigned int sDim, tDim, rDim;
    if( i0 != NULL )
    {
        sDim = i0->s();
        tDim = i0->t();
        rDim = i0->r();
    }
    else if( i1 != NULL )
    {
        sDim = i1->s();
        tDim = i1->t();
        rDim = i1->r();
    }
    else if( i2 != NULL )
    {
        sDim = i2->s();
        tDim = i2->t();
        rDim = i2->r();
    }
    else if( i3 != NULL )
    {
        sDim = i3->s();
        tDim = i3->t();
        rDim = i3->r();
    }
    else if( i4 != NULL )
    {
        sDim = i4->s();
        tDim = i4->t();
        rDim = i4->r();
    }
    else if( i5 != NULL )
    {
        sDim = i5->s();
        tDim = i5->t();
        rDim = i5->r();
    }
    else if( i6 != NULL )
    {
        sDim = i6->s();
        tDim = i6->t();
        rDim = i6->r();
    }
    else if( i7 != NULL )
    {
        sDim = i7->s();
        tDim = i7->t();
        rDim = i7->r();
    }
    else
    {
        return( NULL );
    }

    unsigned char* data( new unsigned char[ sDim * tDim * rDim ] );
    image->setImage( sDim, tDim, rDim,
                     GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                     ( unsigned char* ) data, osg::Image::USE_NEW_DELETE );
    unsigned char* ptr( data );
    bool validData( false );

    for( int rIdx = 0; rIdx < image->r(); ++rIdx )
    {
        int rBit( ( rIdx < image->r() / 2 ) ? 0 : 1 );
        for( int tIdx = 0; tIdx < image->t(); ++tIdx )
        {
            int tBit( ( tIdx < image->t() / 2 ) ? 0 : 1 );
            for( int sIdx = 0; sIdx < image->s(); ++sIdx )
            {
                int sBit( ( sIdx < image->s() / 2 ) ? 0 : 1 );
                osg::Image* inImage;
                switch( ( rBit << 2 ) + ( tBit << 1 ) + sBit )
                {
                case 0:
                    inImage = const_cast< osg::Image* >( i0 );
                    break;
                case 1:
                    inImage = const_cast< osg::Image* >( i1 );
                    break;
                case 2:
                    inImage = const_cast< osg::Image* >( i2 );
                    break;
                case 3:
                    inImage = const_cast< osg::Image* >( i3 );
                    break;
                case 4:
                    inImage = const_cast< osg::Image* >( i4 );
                    break;
                case 5:
                    inImage = const_cast< osg::Image* >( i5 );
                    break;
                case 6:
                    inImage = const_cast< osg::Image* >( i6 );
                    break;
                case 7:
                    inImage = const_cast< osg::Image* >( i7 );
                    break;
                }

                int pixel( 0 );
                if( inImage != NULL )
                {
                    const int s( sIdx % ( inImage->s() / 2 ) );
                    const int t( tIdx % ( inImage->t() / 2 ) );
                    const int r( rIdx % ( inImage->r() / 2 ) );
                    pixel = *( inImage->data( s * 2,   t * 2,   r * 2 ) ) +
                            *( inImage->data( s * 2 + 1, t * 2,   r * 2 ) ) +
                            *( inImage->data( s * 2,   t * 2 + 1, r * 2 ) ) +
                            *( inImage->data( s * 2 + 1, t * 2 + 1, r * 2 ) ) +
                            *( inImage->data( s * 2,   t * 2,   r * 2 + 1 ) ) +
                            *( inImage->data( s * 2 + 1, t * 2,   r * 2 + 1 ) ) +
                            *( inImage->data( s * 2,   t * 2 + 1, r * 2 + 1 ) ) +
                            *( inImage->data( s * 2 + 1, t * 2 + 1, r * 2 + 1 ) );
                    pixel >>= 3; // Divide by 8.
                    validData = true;
                }
                *ptr++ = ( unsigned char )pixel;
            }
        }
    }

    return( validData ? image.release() : NULL );
}

SaveHierarchy::SaveHierarchy( const std::string baseName )
    : _depth( 0 ),
      _baseName( baseName )
{
}

SaveHierarchy::~SaveHierarchy()
{
    _lodVec.clear();
}

unsigned int SaveHierarchy::computeLevel( unsigned short numBricksX )
{
	unsigned int depth = 0;
    while( numBricksX > 0 )
    {
        ++depth;
        numBricksX >>= 1;
    }

	return depth;
}

void SaveHierarchy::addAllLevels( LODVector &allLevels )
{
	_lodVec.clear();
	_depth = 0;

	if (!allLevels.size()) return;
	_lodVec = allLevels;

	_depth = _lodVec.size();
}

void SaveHierarchy::addLevel( unsigned int level, VolumeBrickDataPtr base, bool addSubLevels )
{
	if (addSubLevels)
	{
		_lodVec.clear();
	}

	if (_lodVec.size() < level)
	{
		_lodVec.resize(level);
	}

	_lodVec[ level - 1 ] = base;

	if (addSubLevels)
	{
		for( int depthIdx = level - 1; depthIdx > 0; --depthIdx )
		{
			Downsampler ds( _lodVec[ depthIdx ] );
			_lodVec[ depthIdx - 1 ] = ds.getLow();
			if (base->checkCancel()) return;
		}
    }

	_depth = _lodVec.size();
}

void SaveHierarchy::addLevel( VolumeBrickDataPtr base )
{
	unsigned int level = computeLevel(base->getNumBricks().x());
	addLevel( level, base, true );
}

bool SaveHierarchy::save( DBBasePtr db, VolumeBrickDataPtr base )
{
	addLevel(base);
	return save( db );
}

bool SaveHierarchy::save( DBBasePtr db )
{
    if( db == NULL )
    {
        LFX_WARNING_STATIC( "lfx.core.hier", "NULL DB in SaveHierarchy." );
    }

	if (!_depth)
	{
		LFX_ERROR_STATIC( "lfx.core.hier", "LOD levels not set in SaveHierarchy." );
	}

    recurseSaveBricks( db, std::string( "" ) );

	return true;
}

void SaveHierarchy::recurseSaveBricks( DBBasePtr db, const std::string brickName )
{
    const int depth( brickName.length() );
    VolumeBrickDataPtr vbd( _lodVec[ depth ] );
	bool resPrune = false;
	osg::Image* image = NULL;

	resPrune = resolutionPruneTest( brickName );

	if (vbd->checkCancel()) return;

	osg::Vec3s brickNum = vbd->nameToBrickNum(brickName);
	image = vbd->getSeamlessBrick( brickName );

	if (vbd->checkCancel()) return;

    if( image != NULL )
    {
        const std::string fileName( _baseName + "-" + brickName + "-.ive" );
        image->setFileName( fileName );
        bool ret = db->storeImage( image, fileName );
        LFX_INFO_STATIC( "lfx.core.hier", "Saved brick " + fileName );
    }
    else
    {
		LFX_INFO_STATIC( "lfx.core.hier", "NULL brick " + brickName );
		return; // if parent brick is null, should be no need to go into child bricks
    }

	if (resPrune)
	{
		LFX_INFO_STATIC( "lfx.core.hier", "Max Resolution brick " + brickName );

		std::stringstream ss;
		ss << "Max Resolution brick " << brickName;
		vbd->sendProgressMsg(ss.str().c_str());
		return;
	}

    if( depth < _depth - 1 )
    {
        recurseSaveBricks( db, brickName + std::string( "0" ) );
		if (vbd->checkCancel()) return;
        recurseSaveBricks( db, brickName + std::string( "1" ) );
		if (vbd->checkCancel()) return;
        recurseSaveBricks( db, brickName + std::string( "2" ) );
		if (vbd->checkCancel()) return;
        recurseSaveBricks( db, brickName + std::string( "3" ) );
		if (vbd->checkCancel()) return;
        recurseSaveBricks( db, brickName + std::string( "4" ) );
		if (vbd->checkCancel()) return;
        recurseSaveBricks( db, brickName + std::string( "5" ) );
		if (vbd->checkCancel()) return;
        recurseSaveBricks( db, brickName + std::string( "6" ) );
		if (vbd->checkCancel()) return;
        recurseSaveBricks( db, brickName + std::string( "7" ) );
    }
}

bool SaveHierarchy::resolutionPruneTest( const std::string brickNameParent )
{
	const int depthParent( brickNameParent.length() );
	if (depthParent < 1) return false;
	if( depthParent >= _depth - 1 ) return false;

	/*
	// begin debug
	
	VolumeBrickDataPtr vbdp( _lodVec[ depthParent ] );
	osg::Vec3s brickNumParent = vbdp->nameToBrickNum(brickNameParent);
	osg::Vec3d pmin, pmax, childMin, childMax;
	vbdp->getDataBoundingBox(brickNumParent, pmin, pmax);

	std::stringstream ssd;
	ssd << "parent brick: " << brickNameParent << "\n";
	ssd << "min (" << pmin[0] << ", " << pmin[1] << ", " << pmin[2] << ")\n";
	ssd << "max (" << pmax[0] << ", " << pmax[1] << ", " << pmax[2] << ")\n";

	LFX_INFO_STATIC( "lfx.core.hier", ssd.str().c_str() );

	// test all child bricks
	for (int i=0; i<8; i++)
	{
		osg::Vec3d min, max;

		std::stringstream ss;
		ss << brickNameParent << i;
		std::string brickName = ss.str();

		int depth( brickName.length() );
		VolumeBrickDataPtr vbd( _lodVec[ depth ] );
		osg::Vec3s brickNum = vbd->nameToBrickNum(brickName);

		vbd->getDataBoundingBox(brickNum, min, max);
		ssd = std::stringstream();
		ssd << "child brick: " << brickName << "\n";
		ssd << " min (" << min[0] << ", " << min[1] << ", " << min[2] << ")\n";
		ssd << " max (" << max[0] << ", " << max[1] << ", " << max[2] << ")\n";
		LFX_INFO_STATIC( "lfx.core.hier", ssd.str().c_str() );

		if (vbd->checkCancel()) return false;

		if (i == 0)
		{
			childMin = min;
			childMax = max;
			continue;
		}

		for (int s=0; s<3; s++)
		{
			if (min[s] < childMin[s]) childMin[s] = min[s]; 
			if (max[s] > childMax[s]) childMax[s] = max[s];
		}
	}

	ssd = std::stringstream();
	ssd << "childs bounding box: \n";
	ssd << " min (" << childMin[0] << ", " << childMin[1] << ", " << childMin[2] << ")\n";
	ssd << " max (" << childMax[0] << ", " << childMax[1] << ", " << childMax[2] << ")\n\n";
	LFX_INFO_STATIC( "lfx.core.hier", ssd.str().c_str() );

	for (int s=0; s<3; s++)
	{
		if (fabs(pmin[s] - childMin[s]) > .001 || fabs(pmax[s] - childMax[s]) > .001)
		{
			ssd << "parent child bounding box problem on parent brick: " << brickNameParent << "\n";
			break;
		}
	}

	// end debug
	*/

	osg::Vec3s parentBrickNum = _lodVec[ depthParent ]->nameToBrickNum( brickNameParent );

	// test all child bricks
	for (int i=0; i<8; i++)
	{
		std::stringstream ss;
		ss << brickNameParent << i;
		std::string brickName = ss.str();

		int depth( brickName.length() );
		VolumeBrickDataPtr vbd( _lodVec[ depth ] );
		osg::Vec3s brickNum = vbd->nameToBrickNum( brickName );

		if (vbd->checkCancel()) return false;

		if ( !vbd->resolutionPrune( brickNum, parentBrickNum ) ) 
		{
			return false;
		}
	}

	return true;    
}

LoadHierarchy::LoadHierarchy( std::string channelName, std::string filter, DataTypeInfoPtr dataTypeInfo ) :
	_channelName( channelName ),
	_filter( filter ),
	_dataTypeInfo( dataTypeInfo ),
	_load( false )
{
    setActionType( Preprocess::ADD_DATA );
}
LoadHierarchy::~LoadHierarchy()
{
}

void LoadHierarchy::setLoadData( const bool load )
{
    _load = load;
}
bool LoadHierarchy::getLoadData() const
{
    return( _load );
}

ChannelDataPtr LoadHierarchy::operator()()
{
    if( _db == NULL )
    {
        LFX_FATAL_STATIC( "lfx.core.hier", "DB is NULL." );
        return( ChannelDataPtr( (ChannelData*)NULL ) );
    }

	if( _dataTypeInfo.get() )
	{
		return loadFromDataTypeInfo();
	}

    DBBase::StringSet results( _db->getAllKeys() );
    if( results.empty() )
    {
        LFX_FATAL_STATIC( "lfx.core.hier", "No keys in DB." );
    }

	int filecount = 0;
	std::vector<std::string> files;
    // Determine the hierarchy maxDepth from the longest hierarchy name.
    unsigned int maxDepth( 1 );
    BOOST_FOREACH( const std::string & fName, results )
    {
        if( !validWithFilter( fName, _filter ) )
        {
			if( _filter.size() > 0 && filecount > 0 ) break;
			continue;
        }

		if( !valid( fName ) )
		{
			continue;
		}

		filecount++;
		files.push_back( fName );

        Poco::Path pocoPath( fName );
        const std::string& actualName( pocoPath.getFileName() );
        size_t depth( actualName.find_last_of( "-" ) - actualName.find_first_of( "-" ) );
        if( depth > maxDepth )
        {
            maxDepth = depth;
        }
    }

	return loadChannelData(files, maxDepth);
}

ChannelDataPtr LoadHierarchy::loadFromDataTypeInfo()
{
	if( !_dataTypeInfo.get() ) return ChannelDataPtr();

	// Create the ChannelData hierarchy.
	return loadChannelData(_dataTypeInfo->fileNames, _dataTypeInfo->maxDepth);
} 

ChannelDataPtr LoadHierarchy::loadChannelData(const std::vector<std::string> &files, int maxDepth)
{
	ChannelDataPtr cdp( ( ChannelData* )NULL );

	// Create the ChannelData hierarchy.
    try
    {
		AssembleHierarchy ah( maxDepth, 60000. );
        BOOST_FOREACH( const std::string &fName, files )
        {
            // add the texture to the channel
           addFile( &ah, fName );
        }
        ah.prune();
        cdp = ah.getRoot();
    }
    catch( std::exception& /*exc*/ )
    {
        LFX_ERROR_STATIC( "lfx.core.hier", "Unable to assemble hierarchy." );
        return( cdp );
    }

    // Return the hierarchy root.
    cdp->setName( _channelName );
    return( cdp );
}
 
void LoadHierarchy::addFile( AssembleHierarchy *ah, const std::string &fName )
{
	 // Create this ChannelData.
	ChannelDataOSGImagePtr cdImage( new ChannelDataOSGImage() );
    cdImage->setDBKey( DBKey( fName ) );
	if( _load ) 
	{
		cdImage->setImage( _db->loadImage( DBKey( fName ) ) );
	}
    
	// Get the hierarchy name string.
    Poco::Path pocoPath( fName );
    const std::string& actualName( pocoPath.getFileName() );
    const size_t firstLoc( actualName.find_first_of( "-" ) + 1 );
    const size_t lastLoc( actualName.find_last_of( "-" ) );
    const std::string hierarchyName( actualName.substr( firstLoc, lastLoc - firstLoc ) );

    LFX_DEBUG_STATIC( "lfx.core.hier", "Adding " + fName + ": " + hierarchyName );
    cdImage->setName( _channelName );
    ah->addChannelData( cdImage, hierarchyName );
}

bool LoadHierarchy::parseTypeName(const std::string &actualName, std::string *ptype, bool *pbscalar)
{
	size_t pos1 = actualName.find_first_of( "_" );	
	if( pos1 == std::string::npos )
	{
		return false;
	}

	pos1++;

	size_t pos2 = actualName.find_first_of( "-", pos1 );
	if( pos2 == std::string::npos )
	{
		LFX_ERROR_STATIC( "lfx.core.hier", "identifyScalarsVectors - unexpected, - not found in name." );
		return false;
	}

	if( pos1 >= pos2 )
	{
		LFX_ERROR_STATIC( "lfx.core.hier", "identifyScalarsVectors - unexpected, _ and - not positioned properly, unrecoginzed image name." );
		return false;
	}

	std::string strType;
	for( size_t i=pos1; i<pos2; i++ )
	{
		strType.push_back( actualName.at(i) );
	}

	bool isScalar = true;
	if( strType.at( 0 ) == 's' )
	{
		isScalar = true;
	}
	else if( strType.at( 0 ) == 'v' )
	{
		isScalar = false;
	}
	else
	{
		LFX_TRACE_STATIC( "lfx.core.hier", "identifyScalarsVectors - unexpected type: " + strType + " for file: " + actualName );
		return false;
	}

	if( ptype ) *ptype = strType;
	if( pbscalar ) *pbscalar = isScalar;

	return true;
}

bool LoadHierarchy::identifyScalarsVectors( const DBBase *pdb, DataTypeInfoMap *types, std::vector< std::string > *pChanNames )
{
	if( pdb == NULL )
    {
		LFX_FATAL_STATIC( "lfx.core.hier", "identifyScalarsVectors - DB is NULL." );
        return false;
    }

    DBBase::StringSet results( pdb->getAllKeys() );
    if( results.empty() )
    {
        LFX_FATAL_STATIC( "lfx.core.hier", "identifyScalarsVectors - No keys in DB." );
		return false;
    }

	int filecount = 0;
	unsigned int maxDepth = 1;

    // Determine the hierarchy maxDepth from the longest hierarchy name.
    BOOST_FOREACH( const std::string & fName, results )
    {
        if( !valid( fName ) )
        {
            continue;
        }

		filecount++;

		std::string strType;
		bool isScalar = true;
        Poco::Path pocoPath( fName );
        const std::string& actualName( pocoPath.getFileName() );
		if( !parseTypeName(actualName, &strType, &isScalar) )
		{
			strType = "volumedata";
			LFX_TRACE_STATIC( "lfx.core.hier", "identifyScalarsVectors - unexpected type: " + strType + " for file: " + actualName + ", defaulting to volumedata." );
		}

		if ( types )
		{
			DataTypeInfoPtr dataType;
			DataTypeInfoMap::iterator it = types->find( strType );
			if( it == types->end() )
			{
				dataType.reset( new DataTypeInfo() );
				dataType->typeName = strType;
				dataType->isScalar = isScalar;
				dataType->maxDepth = 1;
				(*types)[strType] = dataType;
			}
			else
			{
				dataType = it->second;
			}

			size_t depth( actualName.find_last_of( "-" ) - actualName.find_first_of( "-" ) );
			if( depth > dataType->maxDepth )
			{
				dataType->maxDepth = depth;
			}

			dataType->fileNames.push_back( fName );
		}

		if( pChanNames )
		{
			if( std::find( pChanNames->begin(), pChanNames->end(), strType ) == pChanNames->end() )
			{
				pChanNames->push_back( strType );
			}
		}
    }

	if( filecount > 0 ) return true; // we have data

	return false; // no data
}

bool LoadHierarchy::valid( const std::string& fileName )
{
    const std::string nameOnly( osgDB::getSimpleFileName( fileName ) );
    const size_t firstDash( nameOnly.find_first_of( "-" ) );
    const size_t lastDash( nameOnly.find_last_of( "-" ) );
    if( ( firstDash == lastDash ) || ( firstDash == std::string::npos ) ||
            ( lastDash == std::string::npos ) )
    {
        return( false );
    }
    else
    {
        return( true );
    }
}

bool LoadHierarchy::validWithFilter( const std::string& fileName, const std::string& filter )
{
	if( filter.size() > 0 )
	{
		size_t pos = fileName.find( filter );
		if( pos == std::string::npos )
		{
			return false;
		}
	}

	return true;
}

void LoadHierarchy::setFilter( const char* filter )
{
	_filter = filter;
}

std::string LoadHierarchy::getFilter( ) const
{
	return _filter;
}

void LoadHierarchy::setChannelName( const char* name )
{
	_channelName = name;
}

std::string LoadHierarchy::getChannelName( ) const
{
	return _channelName;
}

void LoadHierarchy::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	Preprocess::serializeData( json );

	json->insertObj( LoadHierarchy::getClassName(), true);
	json->insertObjValue( "load",  _load );
	json->insertObjValue( "filter",  _filter );
	json->insertObjValue( "channelName",  _channelName );
	json->popParent();
}

bool LoadHierarchy::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !Preprocess::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( LoadHierarchy::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get LoadHierarchy data";
		return false;
	}

	json->getValue( "load", &_load, false );
	json->getValue( "filter", &_filter, "" );
	json->getValue( "channelName", &_channelName, "" );


	json->popParent();
	return true;
}


// core
}
// lfx
}
