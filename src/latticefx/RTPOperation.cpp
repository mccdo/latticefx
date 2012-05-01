
#include <latticefx/RTPOperation.h>


namespace lfx {


RTPOperation::RTPOperation( const RTPOpType rtpOpType )
  : OperationBase( OperationBase::RunTimeProcessingType ),
    _rtpOpType( rtpOpType )
{
}
RTPOperation::RTPOperation( const RTPOperation& rhs )
  : OperationBase( rhs ),
    _rtpOpType( rhs._rtpOpType )
{
}
RTPOperation::~RTPOperation()
{
}


// lfx
}
