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

#ifndef __LFX_CORE_PLAY_CONTROL_H__
#define __LFX_CORE_PLAY_CONTROL_H__ 1

#include <latticefx/core/Export.h>
#include <latticefx/core/PagingCallback.h>
#include <latticefx/core/LogBase.h>

#include <osg/Node>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <map>


namespace lfx {
namespace core {


/** \addtogroup TimeSeriesSupport Support for time series animations
\brief Support for animating data sets sampled at multiple times.
\details
*/
/**@{*/


/** \class PlayControl PlayControl.h <latticefx/core/PlayControl.h>
\brief Manages time series animation on one or more scenes.
\details To add a subgraph for management by PlayControl, specify the subgraph root
node in the PlayControl constructor, or pass it to addScene(). Each root node must
have PagingCallback attached as an update callback.

To specify the time series animation play speed, call setPlayRate(). The default
play rate is 1.0.

The application must call elapsedClockTick() to advance the animation at the specified
play rate. Typically, the application calls elapsedClockTick() with the elapsed real
time since the last call to elapsedClockTick().

elapsedClockTick() multiplies the elapsed time parameter by the play rate and adds it
to \c _time, the time series animation time. If \c time exceeds the time range (see
setTimeRange()), elapsedClockTick() wraps \c _time to stay within that range.

After updating \c _time, elapsedClockTick() calls PagingCallback::setAnimationTime(),
passing \c _time as a parameter.

TBD Future work:
\li Add a wrap control. App would then specify what happens when \c time hits a time
range limit: loop, stop, reverse, etc.
\li Add typical player control API (pause(), stop(), play(), ffwd(), rwd(), etc).
*/
class LATTICEFX_EXPORT PlayControl : protected LogBase
{
public:
    PlayControl( osg::Node* scene=NULL );
    PlayControl( const PlayControl& rhs );
    ~PlayControl();

    /** \brief Add a subgraph for management by PlayControl.
    \details \c scene must have a PagingCallback attached as an update callback. */
    void addScene( osg::Node* scene );

    /** \brief Specify the elapsed clock time.
    \details In typical usage, the application calls this function once per frame and passed
    the elapsed real clock time in seconds since the previous call to elapsedClockTick().

    This function adds \c elapsed to the current animation time \c _time, and wraps the
    resulting animation time if it is out of range. Then the function calls into all
    managed scene graphs to set their current animation time. */
    void elapsedClockTick( double elapsed );

    /** \brief Sets the current animation time.
    \details Immediately jump to \c time. Implicitly calls elapsedClockTick(0.0),
    which handles wrapping in the event that \c time is out of range, and update all
    managed scene graphs. */
    void setAnimationTime( double time );
    /** \brief Get the current animation time.
    \details Apps don't typically call this function, as elapsedClockTick() updates the
    registered subgraph. */
    double getAnimationTime() const { return( _time ); }

    /** \brief Set the play rate.
    \details The default play rate is 1.0, which plays the animation in sync with the time
    passed to elapsedClockTick(). If \c playRate is 2.0, the animation plays at twice that
    rate. 0.5 plays at half that rate. Negative values play the animation backwards. */
    void setPlayRate( double playRate );
    /** \brief Get the play rate. */
    double getPlayRate() const;

    /** \brief Set the range of time values covered by the time series animation.
    \details The time range is used to clamp/wrap \c _time (the animation time) when it is
    calculated in the elapsedClockTick() function. */
    void setTimeRange( const osg::Vec2d& timeRange );
    /** \overload */
    void setTimeRange( const double minTime, const double maxTime );
    /** \brief Retrieve the time range. */
    osg::Vec2d getTimeRange() const;
    /** \overload */
    void getTimeRange( double& minTime, double& maxTime ) const;

    /** \brief TBD
    \details TBD */
    void setLastFrameHold( const double hold );

protected:
    typedef std::map< osg::ref_ptr< osg::Node >, osg::ref_ptr< PagingCallback > > NodeCBMap;
    NodeCBMap _scenes;

    double _time;
    double _playRate;
    double _minTime, _maxTime;

    double _lastFrameHold, _holdCount;
};

typedef boost::shared_ptr< PlayControl > PlayControlPtr;


/**@}*/


// core
}
// lfx
}


// __LFX_CORE_PLAY_CONTROL_H__
#endif
