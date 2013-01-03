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

#ifndef __LFX_CORE_RTP_OPERATION_H__
#define __LFX_CORE_RTP_OPERATION_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/OperationBase.h>
#include <latticefx/core/ChannelData.h>

#include <boost/smart_ptr/shared_ptr.hpp>

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization//base_object.hpp>

#include <list>



namespace lfx {
namespace core {


/** \class RTPOperation RTPOperation.h <latticefx/core/RTPOperation.h>
\brief Base class for all Run-Time Processing operations.
\details Examples of operations include Filter, Mask,
and CreateChannel. See the example mask operation MyMask.

An implementation of an RTPOperation must override one of
the mask(), filter(), or channel() functions. The app creating
the RTPOperation instance is responsible for adding any
required inputs using the addInput() function. */
class LATTICEFX_EXPORT RTPOperation : public OperationBase
{
public:
    typedef enum {
        Undefined,
        Mask,
        Filter,
        Channel
    } RTPOpType;
    /** \brief */
    RTPOpType getRTPOpType() const { return( _rtpOpType ); }


    // This is to remove compile errors caused by boost serializers requiring
    // either a default constructor or special code for constructing.
    RTPOperation() : _rtpOpType( Undefined ) {}

    RTPOperation( const RTPOpType rtpOpType );
    RTPOperation( const RTPOperation& rhs );


    /** \brief Override to implement a mask operation. */
    virtual ChannelDataPtr mask( const ChannelDataPtr maskIn ) { return( ChannelDataPtr( ( ChannelData* )( NULL ) ) ); }

    /** \brief Override to implement a filter operation. */
    virtual void filter( const ChannelDataPtr maskIn ) { return; }

    /** \brief Override to implement a channel creation operation. */
    virtual ChannelDataPtr channel( const ChannelDataPtr maskIn ) { return( ChannelDataPtr( ( ChannelData* )( NULL ) ) ); }


    // 'public' required for plugin access?? TBD.
//protected:
    virtual ~RTPOperation();

protected:
    RTPOpType _rtpOpType;


private:
    friend class boost::serialization::access;

    template< class Archive >
    void serialize( Archive& ar, const unsigned int version )
    {
        ar & boost::serialization::base_object< OperationBase >( *this );
        ar & _rtpOpType;
    }
};


typedef boost::shared_ptr< RTPOperation > RTPOperationPtr;
typedef std::list< RTPOperationPtr > RTPOperationList;


// core
}
// lfx
}


BOOST_CLASS_VERSION( lfx::core::RTPOperation, 0 );


// __LFX_CORE_RTP_OPERATION_H__
#endif
