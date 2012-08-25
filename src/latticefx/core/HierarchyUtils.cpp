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

#include <latticefx/core/HierarchyUtils.h>
#include <latticefx/core/ChannelData.h>
#include <latticefx/core/ChannelDataImageSet.h>
#include <latticefx/core/ChannelDataLOD.h>
#include <latticefx/core/LogMacros.h>
#include <latticefx/core/PageData.h>

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include <deque>
#include <sstream>



namespace lfx {
namespace core {


typedef std::deque< int > NameDeque;
static NameDeque s_name;

std::string dequeToString( const NameDeque& nameDeque )
{
    std::ostringstream ostr;
    BOOST_FOREACH( int idx, nameDeque )
    {
        ostr << idx;
    }
    return( ostr.str() );
}

void traverseHeirarchy( ChannelDataPtr cdp, HierarchyCallback& cb )
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
            traverseHeirarchy( lodData->getChannel( idx ), cb );
        }
    }
    else if( imageData != NULL )
    {
        for( unsigned int idx=0; idx<imageData->getNumChannels(); ++idx )
        {
            s_name.push_back( idx );
            traverseHeirarchy( imageData->getChannel( idx ), cb );
            s_name.pop_back();
        }
    }
    else
    {
        cb( cdp, dequeToString( s_name ) );
    }
}




AssembleHierarchy::AssembleHierarchy( RangeVec ranges )
  : _ranges( ranges )
{
    _root = boost::static_pointer_cast< ChannelData >( ChannelDataLODPtr( new ChannelDataLOD() ) );
    recurseInit( _root, 0 );
}
AssembleHierarchy::AssembleHierarchy( unsigned int maxDepth, double baseRange )
{
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
        _iterator = _root;

    osg::Vec3 localOffset( offset );
    if( !( nameString.empty() ) && ( offset.length2() == 0. ) )
    {
        // No offset passed as a parameter. Interpret offset
        // from last digit of name string.
        // This can only happen when addChannelData is called from the app.
        //   Once we start recursing, offset will always be non-zero.
        switch( nameString.back() ) {
        case '0': localOffset.set( -1., -1., -1. ); break;
        case '1': localOffset.set( 1., -1., -1. ); break;
        case '2': localOffset.set( -1., 1., -1. ); break;
        case '3': localOffset.set( 1., 1., -1. ); break;
        case '4': localOffset.set( -1., -1., 1. ); break;
        case '5': localOffset.set( 1., -1., 1. ); break;
        case '6': localOffset.set( -1., 1., 1. ); break;
        case '7': localOffset.set( 1., 1., 1. ); break;
        default:
            LFX_ERROR_STATIC( "lfx.core.hier", "addChannelData: Invalid nameString digit: " + nameString.back() );
            break;
        }
    }

    if( !( nameString.empty() ) )
    {
        // Recurse
        const std::string newName( nameString.substr( 1 ) );
        const unsigned int newDepth( depth + 1 );

        // Walk down the hierarchy. We should have a composite that is an LOD.
        // It's first child is the ChannelData for that LOD, but since nameString
        // is not empty, we want to go further, so we take the second child which
        // must be a composite that is an ImageSet. We then take one of the ImageSet's
        // children based on the nameString, and that child in turn is an LOD.
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

        const unsigned int childIndex( (unsigned int)( nameString.front() - '0' ) );
        if( imageData->getChannel( childIndex ) == NULL )
        {
            // This is the insertion point!
            imageData->setChannel( childIndex, cdp );

            // Inserted! Clear the iterator. Not necessary, but nice to not have
            // garbage left here for debugging purposes.
            _iterator = ChannelDataPtr( (ChannelData*)NULL );
        }
        else
        {
            _iterator = imageData->getChannel( childIndex );
            if( newName.empty() )
            {
                // Next recursion is the insersion point, so set offset here.
                imageData->setOffset( childIndex, localOffset );
            }

            addChannelData( cdp, newName, localOffset, newDepth );
        }
    }
    else
    {
        LFX_ERROR_STATIC( "lfx.core.hier", "addChannelData: How did I get here?" );
    }
#if 0
    else
    {
        // nameString is empty, so we must be at the insertion point.
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

        lodData->setChannel( 0, cdp ); // First child of an LOD.

        // Inserted! Clear the iterator. Not necessary, but nice to not have
        // garbage left here for debugging purposes.
        _iterator = ChannelDataPtr( (ChannelData*)NULL );
    }
#endif
} 

ChannelDataPtr AssembleHierarchy::getRoot() const
{
    return( _root );
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


// core
}
// lfx
}
