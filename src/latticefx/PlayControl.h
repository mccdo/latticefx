
#ifndef __LATTICEFX_PLAY_CONTROL_H__
#define __LATTICEFX_PLAY_CONTROL_H__ 1

#include <latticefx/Export.h>
#include <osg/Node>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <list>


namespace lfx {


/** \class OperationValue OperationBase.h <latticefx/OperationBase.h>
\brief
\details
*/
class LATTICEFX_EXPORT PlayControl
{
public:
    PlayControl( osg::Node* scene=NULL );
    PlayControl( const PlayControl& rhs );
    ~PlayControl();

    void addScene( osg::Node* scene );

    void elapsedClockTick( double elapsed );

    void setAnimationTime( double time );
    double getAnimationTime() const { return( _time ); }

    void setPlayRate( double playRate );
    double getPlayRate() const;

    void setTimeRange( const double minTime, const double maxTime );
    void getTimeRange( double& minTime, double& maxTime );

protected:
    typedef std::list< osg::ref_ptr< osg::Node > > NodeList;
    NodeList _scenes;

    double _time;
    double _playRate;
    double _minTime, _maxTime;
};

typedef boost::shared_ptr< PlayControl > PlayControlPtr;


// lfx
}


// __LATTICEFX_PLAY_CONTROL_H__
#endif
