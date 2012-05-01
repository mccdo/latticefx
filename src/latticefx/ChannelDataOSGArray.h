
#ifndef __LATTICEFX_CHANNEL_DATA_OSG_ARRAY_H__
#define __LATTICEFX_CHANNEL_DATA_OSG_ARRAY_H__ 1


#include <latticefx/Export.h>
#include <latticefx/ChannelData.h>

#include <osg/Array>
#include <osg/ref_ptr>
#include <boost/shared_ptr.hpp>

#include <string>
#include <list>


namespace lfx {


/** \class ChannelDataOSGArray ChannelDataOSGArray.h <latticefx/ChannelDataOSGArray.h>
\brief Scalar data container class.
\details Create one instance for each scalar array in
the data set, and add it to the DataSet class. All ChannelData
objects within a single DataSet must have the same data size
(number of elements in the array).

Currently, ChannelData is just a wrapper around an osg::Array,
and only osg::ByteArray (for masking) and osg::Vec3Array (for
vertices) are supported. */
class LATTICEFX_EXPORT ChannelDataOSGArray : public ChannelData
{
public:
    ChannelDataOSGArray( const std::string& name=std::string( "" ) );
    ChannelDataOSGArray( osg::Array* data, const std::string& name=std::string( "" ) );
    ChannelDataOSGArray( const ChannelDataOSGArray& rhs );
    virtual ~ChannelDataOSGArray();


    /** \brief
    \details */
    void setOSGArray( osg::Array* array );

    /** \brief
    \details */
    virtual void getDimensions( unsigned int& x, unsigned int& y, unsigned int& z );

    /** \brief
    \details */
    virtual char* asCharPtr();
    /** \override char* ChannelDataOSGArray::asCharPtr(); */
    virtual const char* asCharPtr() const;

    /** \brief
    \details */
    virtual osg::Array* asOSGArray() { return( _workingData.get() ); }
    /** \override osg::Array* ChannelDataOSGArray::asOSGArray(); */
    virtual const osg::Array* asOSGArray() const { return( _workingData.get() ); }

    /** \brief Set all elements of the data to the same value.
    \details */
    virtual void setAll( const char value );
    virtual void setAll( const float value );

    /** \brief Boolean AND \c rhs with the existing data.
    \details Assumes both the existing data and \c rhs are osg::ByteArray type. */
    virtual void andValues( const ChannelData* rhs );

    /**
    */
    virtual void reset();

    /**
    */
    virtual void resize( size_t size );

protected:
    /** \brief _data is the original data, as specified by the calling code. */
    osg::ref_ptr< osg::Array > _data;
    /** \brief _workingData is a copy og the original data.
    \details _workingData could be modified by RTPOperation filters such as
    Gaussian blur or convolution filters. The reset() member function refreshes
    _workingData from _data prior to re-executing the data processing pipeline. */
    osg::ref_ptr< osg::Array > _workingData;

    /** \brief Copy all elements in rhs to lhs.
    \details Displays an error and returns if the array types don't match
    or \c lhs::getTotalDatSize() < \c rhs::getTotalDatSize(). */
    void copyArray( osg::Array* lhs, const osg::Array* rhs );
};

typedef boost::shared_ptr< ChannelDataOSGArray > ChannelDataOSGArrayPtr;


// lfx
}


// __LATTICEFX_CHANNEL_DATA_OSG_ARRAY_H__
#endif
