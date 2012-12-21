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
#include <latticefx/core/LogMacros.h>
#include <latticefx/core/PageData.h>

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include <sstream>

#include <string>


namespace lfx {
namespace core {


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
        for( unsigned int idx=0; idx<lodData->getNumChannels(); ++idx )
        {
            traverse( lodData->getChannel( idx ) );
        }
    }
    else if( imageData != NULL )
    {
        for( unsigned int idx=0; idx<imageData->getNumChannels(); ++idx )
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
        return( 0 );

    ChannelDataComposite* comp( dynamic_cast< ChannelDataComposite* >( cdp.get() ) );
    if( comp != NULL )
    {
        for( unsigned int idx=comp->getNumChannels(); idx>0; --idx )
        {
            if( traverse( comp->getChannel( idx-1 ) ) == 0 )
                comp->removeChannel( idx-1 );
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
        throw( std::exception( errorText.c_str() ) );
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
        throw( std::exception( errorText.c_str() ) );
    }
    if( baseRange <= 0. )
    {
        const std::string errorText( "AssembleHierarchy: baseRange must be > 0." );
        LFX_ERROR_STATIC( "lfx.core.hier", errorText );
        throw( std::exception( errorText.c_str() ) );
    }

    double range( baseRange );
    unsigned int idx( maxDepth-1 );
    do {
        _ranges.push_back( range );
        range *= 4.;
        --idx;
    } while( idx > 0 );

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
        const osg::Vec3& offset, const unsigned int depth )
{
    if( depth == 0 )
        // Special-case the initial condition.
        _iterator = _root;

    osg::Vec3 localOffset( offset );
    if( !( nameString.empty() ) && ( offset.length2() == 0. ) )
    {
        // No offset passed as a parameter. Interpret offset
        // from last digit of name string.
        // This can only happen when addChannelData is called from the app.
        //   Once we start recursing, offset will always be non-zero.
        std::string::const_iterator iter = nameString.end() - 1;
        
        switch( *iter ) {
        case '0': localOffset.set( -1., -1., -1. ); break;
        case '1': localOffset.set( 1., -1., -1. ); break;
        case '2': localOffset.set( -1., 1., -1. ); break;
        case '3': localOffset.set( 1., 1., -1. ); break;
        case '4': localOffset.set( -1., -1., 1. ); break;
        case '5': localOffset.set( 1., -1., 1. ); break;
        case '6': localOffset.set( -1., 1., 1. ); break;
        case '7': localOffset.set( 1., 1., 1. ); break;
        default:
            LFX_ERROR_STATIC( "lfx.core.hier", "addChannelData: Invalid nameString digit: " + *iter );
            break;
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
        _iterator = ChannelDataPtr( (ChannelData*)NULL );
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
    const unsigned int childIndex( (unsigned int)( *iter - '0' ) );
    if( imageData->getChannel( childIndex ) == NULL )
    {
        // Case 2
        // This is the insertion point.
        imageData->setChannel( childIndex, cdp );
        imageData->setOffset( childIndex, localOffset );
        // Set to NULL for debugging purposes (not actually necessary).
        _iterator = ChannelDataPtr( (ChannelData*)NULL );
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
        addChannelData( cdp, newName, localOffset, depth+1 );
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

    if( depth+1 < _ranges.size() )
    {
        // We haven't hit max depth yet, so we'll add LOD children and recurse.
        // Otherwise we do nothing, as the children will be inserted by the app
        // with a call to addChannelData().
        for( unsigned int idx=0; idx<8; ++idx )
        {
            ChannelDataLODPtr newLOD( new ChannelDataLOD() );
            cdImage->setChannel( idx, newLOD );
            recurseInit( newLOD, depth+1 );
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
    if( ratio & ratio-1 )
    {
        LFX_WARNING_STATIC( "lfx.core.hier", "getNameString: _dims.x()/subDims.x() must be a power of 2." );
    }
    int depth( 1 );
    while( ( ratio >>= 1 ) > 0 )
        depth++;

    // If depth is 1, we have the root of the hierarchy, so the
    // name string is the empty string.
    if( depth == 1 )
        return( std::string( "" ) );

    // Accumulate the name string.
    const osg::Vec3s subCenter( subDims / 2 + subMinCorner );
    osg::Vec3s octant( _dims / 2 );
    std::string nameString;
    for( int idx=1; idx<depth; idx++ )
    {
        int octantValue( 0 );
        if( subCenter.x() > octant.x() )
            octantValue += ( 1 << 0 );
        if( subCenter.y() > octant.y() )
            octantValue += ( 1 << 1 );
        if( subCenter.z() > octant.z() )
            octantValue += ( 1 << 2 );

        switch( octantValue ) {
        case 0: nameString += "0"; break;
        case 1: nameString += "1"; break;
        case 2: nameString += "2"; break;
        case 3: nameString += "3"; break;
        case 4: nameString += "4"; break;
        case 5: nameString += "5"; break;
        case 6: nameString += "6"; break;
        case 7: nameString += "7"; break;
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
        
        switch( *iter ) {
        case '0': offset.set( -1., -1., -1. ); break;
        case '1': offset.set( 1., -1., -1. ); break;
        case '2': offset.set( -1., 1., -1. ); break;
        case '3': offset.set( 1., 1., -1. ); break;
        case '4': offset.set( -1., -1., 1. ); break;
        case '5': offset.set( 1., -1., 1. ); break;
        case '6': offset.set( -1., 1., 1. ); break;
        case '7': offset.set( 1., 1., 1. ); break;
        default:
            LFX_ERROR_STATIC( "lfx.core.hier", "getNameString: Invalid nameString digit: " + *iter );
            break;
        }
    }

    return( nameString );
}


// core
}
// lfx
}
