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

#include <latticefx/core/PlayControl.h>
#include <latticefx/core/PagingCallback.h>
#include <latticefx/core/LogMacros.h>

#include <boost/foreach.hpp>


namespace lfx
{
namespace core
{


PlayControl::PlayControl( osg::Node* scene )
    : LogBase( "lfx.core.play" ),
      _time( 0. ),
      _playRate( 1. ),
      _minTime( 0. ),
      _maxTime( 1. ),
      _lastFrameHold( 0. ),
      _holdCount( 0. )
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
      _minTime( rhs._minTime ),
      _maxTime( rhs._maxTime ),
      _lastFrameHold( rhs._lastFrameHold ),
      _holdCount( 0. )
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

void PlayControl::elapsedClockTick( TimeValue elapsed )
{
    const TimeValue delta = elapsed * _playRate;
    if( _time + delta > _maxTime )
    {
        // Played forwards past end.
        if( ( _lastFrameHold > 0. ) && ( _holdCount < _lastFrameHold ) )
        {
            // Stay at _maxTime for a count of _lastFrameHold seconds.
            _time = _maxTime;
            _holdCount += delta;
        }
        else
        {
            // Loop to beginning.
            _time = _minTime + _time + delta - _maxTime;
            _holdCount = 0.;
        }
    }
    else if( _time + delta < _minTime )
    {
        // Played backwards past beginning.
        if( ( _lastFrameHold > 0. ) && ( _holdCount < _lastFrameHold ) )
        {
            // Stay at _minTime for a count of _lastFrameHold seconds.
            _time = _minTime;
            _holdCount += delta;
        }
        else
        {
            // Loop to end.
            _time = _maxTime + _time + delta - _minTime;
            _holdCount = 0.;
        }
    }
    else
    {
        _time += delta;
    }


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

void PlayControl::setAnimationTime( TimeValue time )
{
    _time = time;
    elapsedClockTick( 0. );
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
void PlayControl::setTimeRange( const TimeValue minTime, const TimeValue maxTime )
{
    _minTime = minTime;
    _maxTime = maxTime;
}
osg::Vec2d PlayControl::getTimeRange() const
{
    return( osg::Vec2d( _minTime, _maxTime ) );
}
void PlayControl::getTimeRange( TimeValue& minTime, TimeValue& maxTime ) const
{
    minTime = _minTime;
    maxTime = _maxTime;
}

void PlayControl::setLastFrameHold( const TimeValue hold )
{
    _lastFrameHold = hold;
}


// core
}
// lfx
}
