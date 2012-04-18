
#ifndef __LATTICEFX_RTP_OPERATION_H__
#define __LATTICEFX_RTP_OPERATION_H__ 1


#include <latticefx/Export.h>
#include <latticefx/OperationBase.h>
#include <latticefx/ChannelData.h>

#include <boost/smart_ptr/shared_ptr.hpp>

#include <list>



namespace lfx {


/** \class RTPOperation RTPOperation.h <latticefx/RTPOperation.h>
\brief Base class for all Run-Time Processing operations.
\details Examples of operations include Filter, Mask,
and CreateChannel. See the example mask operation MyMask.

An implementation of an RTPOperation must override one of
the mask(), filter(), or channel() functions. The app creating
the RTPOperation instance is responsible for adding any
required inputs using the addInput() function. */
class LATTICEFX_EXPORT RTPOperation : public lfx::OperationBase
{
public:
    typedef enum {
        Mask,
        Filter,
        Channel
    } RTPOpType;
    /** \brief */
    RTPOpType getRTPOpType() const { return( _rtpOpType ); }


    RTPOperation( const RTPOpType rtpOpType );


    /** \brief Override to implement a mask operation. */
    virtual ChannelDataPtr mask( const ChannelDataPtr maskIn ) { return( ChannelDataPtr( ( ChannelData* )( NULL ) ) ); }

    /** \brief Override to implement a filter operation. */
    virtual void filter( const ChannelDataPtr maskIn ) { return; }

    /** \brief Override to implement a channel creation operation. */
    virtual ChannelDataPtr channel( const ChannelDataPtr maskIn ) { return( ChannelDataPtr( ( ChannelData* )( NULL ) ) ); }


    // 'public' required for plugin accedd?? TBD.
//protected:
    virtual ~RTPOperation();

protected:
    RTPOpType _rtpOpType;
};

typedef boost::shared_ptr< RTPOperation > RTPOperationPtr;
typedef std::list< RTPOperationPtr > RTPOperationList;


// lfx
}


// __LATTICEFX_RTP_OPERATION_H__
#endif
