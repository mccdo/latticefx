
#include <latticefx/PlayControl.h>
#include <latticefx/RootCallback.h>

#include <boost/foreach.hpp>


namespace lfx {


PlayControl::PlayControl( osg::Node* scene )
  : _time( 0. ),
    _playRate( 1. ),
    _minTime( 0. ),
    _maxTime( 1. )
{
    _scenes.push_back( scene );
}
PlayControl::PlayControl( const PlayControl& rhs )
  : _scenes( rhs._scenes ),
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
    _scenes.push_back( scene );
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


    // TBD In the future, we want to be able to manage several scene graphs from
    // a single PlayControl. Each scene would register itself with the PlayControl.
    // App would call elapsedClockTick() once, and the following loop would
    // set the animation time on each registered scene.
    //
    // For now, PlayControl must be added to the RootCallback. Kind of a hack, as
    // direct app access is currently somewhat cumbersome. But this should be
    // sufficient for development testing of time series code.
#if 0
    BOOST_FOREACH( osg::ref_ptr< osg::Node > node, _scenes )
    {
        // TBD Runtime dynamic_cast should be avoided. We should cache pointers
        // to the RootCallbacks when the scenes are added.
        RootCallback* rootcb( dynamic_cast< lfx::RootCallback* >(
            node->getUserData() ) );
    }
#endif
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

void PlayControl::setTimeRange( const double minTime, const double maxTime )
{
    _minTime = minTime;
    _maxTime = maxTime;
}
void PlayControl::getTimeRange( double& minTime, double& maxTime )
{
    minTime = _minTime;
    maxTime = _maxTime;
}


// lfx
}
