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

#include <latticefx/core/PlayControl.h>
#include <latticefx/core/PagingCallback.h>
#include <latticefx/core/LogMacros.h>

#include <boost/foreach.hpp>


namespace lfx {


PlayControl::PlayControl( osg::Node* scene )
  : LogBase( "lfx.core.play" ),
    _time( 0. ),
    _playRate( 1. ),
    _minTime( 0. ),
    _maxTime( 1. )
{
    PagingCallback* rootcb( dynamic_cast< PagingCallback* >( scene->getUpdateCallback() ) );
    if( rootcb == NULL )
    {
        LFX_WARNING( "ctor(): Invalid scene update callback." );
        return;
    }
    _scenes[ scene ] = rootcb;
}
PlayControl::PlayControl( const PlayControl& rhs )
  : LogBase( rhs ),
    _scenes( rhs._scenes ),
    _time( rhs._time ),
    _playRate( rhs._playRate ),
    _minTime( 0. ),
    _maxTime( 1. )
{
}
PlayControl::~PlayControl()
{
}

void PlayControl::addScene( osg::Node* scene )
{
    PagingCallback* rootcb( dynamic_cast< PagingCallback* >( scene->getUpdateCallback() ) );
    if( rootcb == NULL )
    {
        LFX_WARNING( "addScene(): Invalid scene update callback." );
        return;
    }
    _scenes[ scene ] = rootcb;
}

void PlayControl::elapsedClockTick( double elapsed )
{
    const double delta = elapsed * _playRate;
    if( _time + delta > _maxTime )
        // Played forwards past end. Loop to beginning.
        _time = _minTime + _time + delta - _maxTime;
    else if( _time + delta < _minTime )
        // Played backwards past beginning. Loop to end.
        _time = _maxTime + _time + delta - _minTime;
    else
        _time += delta;


    // PlayControl can manage several scene graphs. Each scene registers itself
    // using the PlayControl contructor or PlayControl::addScene().
    // App calls elapsedClockTick() once per frame, and the following loop
    // sets the animation time on each registered scene.
    BOOST_FOREACH( NodeCBMap::value_type nodeCBPair, _scenes )
    {
        // .first is the scene itself, which we don't need in this loop.
        // .second is a ref_ptr to a PagingCallback. Set its animation time.
        nodeCBPair.second->setAnimationTime( _time );
    }
}

void PlayControl::setAnimationTime( double time )
{
    _time = time;
}

void PlayControl::setPlayRate( double playRate )
{
    _playRate = playRate;
}
double PlayControl::getPlayRate() const
{
    return( _playRate );
}

void PlayControl::setTimeRange( const osg::Vec2d& timeRange )
{
    setTimeRange( timeRange.x(), timeRange.y() );
}
void PlayControl::setTimeRange( const double minTime, const double maxTime )
{
    _minTime = minTime;
    _maxTime = maxTime;
}
osg::Vec2d PlayControl::getTimeRange() const
{
    return( osg::Vec2d( _minTime, _maxTime ) );
}
void PlayControl::getTimeRange( double& minTime, double& maxTime ) const
{
    minTime = _minTime;
    maxTime = _maxTime;
}


// lfx
}
