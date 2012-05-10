
#ifndef __LATTICEFX_CHANNEL_DATA_H__
#define __LATTICEFX_CHANNEL_DATA_H__ 1


#include <latticefx/Export.h>

#include <osg/Array>
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>

#include <string>
#include <vector>


namespace lfx {


class ChannelData;
typedef boost::shared_ptr< ChannelData > ChannelDataPtr;


/** \class ChannelData ChannelData.h <latticefx/ChannelData.h>
\brief Scalar data container class.
\details Create one instance for each scalar array in
the data set, and add it to the DataSet class. Add ChannelData
instances as inputs to Preprocess, RTPOperation, and Renderer
instances. All ChannelData
objects within a single DataSet must have the same data size
(number of elements in the array).

Currently, ChannelData is just a wrapper around an osg::Array
(see ChannelDataOSGArray),
and only osg::ByteArray (for masking) and osg::Vec3Array (for
vertices) are supported. */
class LATTICEFX_EXPORT ChannelData : public boost::enable_shared_from_this< ChannelData >
{
public:
    ChannelData( const std::string& name=std::string( "" ) );
    ChannelData( const ChannelData& rhs );
    virtual ~ChannelData();


    /** \brief
    \details */
    void setName( const std::string& name );
    /** \brief
    \details */
    const std::string& getName() const;

    /** \brief
    \details */
    virtual void getDimensions( unsigned int& x, unsigned int& y, unsigned int& z ) {}

    /** \brief Return a pointer to (possibly a copy of) the data.
    \details This function should be overridden in derived classes.

    Derived classes that store actual data (such as ChannelDataOSGArray) may
    return a pointer to the actual data. Other classes (such as those that
    represent procedural data or stat stored in a DB) may return NULL or a copy.
    See the derived class documentation for any special notes.
    \returns NULL if the operation is unsupported, memory is exhausted, or the
    operation fails for any reason. */
    virtual char* asCharPtr() { return( NULL ); };
    /** \overload char* ChannelData::asCharPtr(); */
    virtual const char* asCharPtr() const { return( NULL ); };

    /** \brief
    \details */
    virtual osg::Array* asOSGArray() { return( NULL ); }
    /** \overload osg::Array* ChannelData::asOSGArray(); */
    virtual const osg::Array* asOSGArray() const { return( NULL ); }

    /** \brief Return this ChannelData with the specified mask applied.
    \detailt If \maskIn indicates no masking (no zero values), getMaskedChannel()
    may return a pointer to the original ChannelData. Otherwise, a data copy
    would likely be created. The actual implementation is determined by the
    derived class. */
    virtual ChannelDataPtr getMaskedChannel( const ChannelDataPtr maskIn )
    {
        return( shared_from_this() );
    }

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
    std::string _name;
};

typedef std::vector< ChannelDataPtr > ChannelDataList;


LATTICEFX_EXPORT ChannelDataPtr findChannelData( const std::string& name, const ChannelDataList& dataList );


// lfx
}


// __LATTICEFX_CHANNEL_DATA_H__
#endif
