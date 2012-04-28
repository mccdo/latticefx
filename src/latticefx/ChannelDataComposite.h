
#ifndef __LATTICEFX_CHANNEL_DATA_COMPOSITE_H__
#define __LATTICEFX_CHANNEL_DATA_COMPOSITE_H__ 1


#include <latticefx/Export.h>
#include <latticefx/ChannelData.h>

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>
#include <set>
#include <map>


namespace lfx {


typedef std::set< double > TimeSet;
typedef std::map< double, ChannelDataPtr > TimeDataMap;


/** \class ChannelDataComposite ChannelDataComposite.h <latticefx/ChannelDataComposite.h>
\brief Composite pattern container for ChannelData objects
\details This class allows ChannelData per time step and/or ChannelData
per level of detail.

In typical usage, the app stores one or more concrete ChannelData objects
in a ChannelDataComposite, one for each time step. Concrete ChannelData
classes are ChannelData objects that store actual data, such as
ChannelDataOSGArray, or (TBD not yet implemented) ChannelDataDBKey.

Preprocessing operations that create multiple levels of detail or octant
bricks of 3D textures replace the concrete ChannelData at each time step
with yet another ChannelDataCompositire containing LOD or octant data.
ChannelDataComposites may be arranged hierarchically multiple levels deep
if necessary.

Behavior is undefined if both a parent and a subordinate ChannelDataComposite
store multiple time series data. */
class LATTICEFX_EXPORT ChannelDataComposite : public lfx::ChannelData
{
public:
    ChannelDataComposite( const std::string& name=std::string( "" ) );
    ChannelDataComposite( const ChannelDataComposite& rhs );
    virtual ~ChannelDataComposite();


    /** \brief Add a data channel to the ChannelDataList for a specific time value \c time. */
    void addChannel( const ChannelDataPtr channel, const double time=0. );
    void addChannel( const ChannelDataPtr channel, const unsigned int level=0 );

    /** \brief Get a channel for a specific time value \c time.
    \returns ChannelData for the specified \c time. If the exact time doesn't
    have a ChannelData, this function returns the previous time's channelData. */
    ChannelDataPtr getChannel( const double time=0. );
    ChannelDataPtr getChannel( const unsigned int level=0 );

    const ChannelDataPtr getChannel( const double time=0. ) const;
    const ChannelDataPtr getChannel( const unsigned int level=0 ) const;

    const TimeSet getTimeSet() const;


    /** \brief
    \details */
    virtual void getDimensions( unsigned int& x, unsigned int& y, unsigned int& z );

    /** \brief Set all elements of the data to the same value.
    \details */
    virtual void setAll( const char value ) {}
    virtual void setAll( const float value ) {}

    /** \brief Boolean AND \c rhs with the existing data.
    \details Assumes both the existing data and \c rhs are osg::ByteArray type, zero
    representing false and non-zero representing true. */
    virtual void andValues( const ChannelData* rhs ) {}

    /** \brief Prepare the ChannelData for processing by the DataSet.
    \details Prior to processing ChannelData in the LatticeFX data pipeline,
    the DataSet calls ChannelData::reset() on all attached ChannelData
    instances. This is useful in situations where data processing modifies
    ChannelData (such as a convolution filter or Gaussion blur). Dereived
    ChannelData classes must store a copy of the original data to protect
    against such destruction. reset() allows the ChannelData to refresh the
    working copy of the data from the original. */
    virtual void reset() {}

    /** \brief Specify the size of the ChannelData.
    \details Used by derived classes to resize internal arrays. */
    virtual void resize( size_t size ) {}

protected:
    TimeDataMap _timeData;
};

typedef boost::shared_ptr< ChannelDataComposite > ChannelDataCompositePtr;


// lfx
}


// __LATTICEFX_CHANNEL_DATA_COMPOSITE_H__
#endif
