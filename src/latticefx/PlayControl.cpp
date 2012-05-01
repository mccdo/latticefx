
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
    RootCallback* rootcb( dynamic_cast< RootCallback* >( scene->getUpdateCallback() ) );
    if( rootcb == NULL )
    {
        OSG_WARN << "PlayControl(): Invalid scene update callback." << std::endl;
        return;
    }
    _scenes[ scene ] = rootcb;
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
    RootCallback* rootcb( dynamic_cast< RootCallback* >( scene->getUpdateCallback() ) );
    if( rootcb == NULL )
    {
        OSG_WARN << "PlayControl::addScene(): Invalid scene update callback." << std::endl;
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
        // .second is a ref_ptr to a RootCallback. Set its animation time.
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
