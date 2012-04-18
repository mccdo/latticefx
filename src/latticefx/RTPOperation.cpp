
#include <latticefx/RTPOperation.h>


namespace lfx {


RTPOperation::RTPOperation( const RTPOpType rtpOpType )
    : OperationBase( OperationBase::RunTimeProcessingType ),
      _rtpOpType( rtpOpType )
{
}

RTPOperation::~RTPOperation()
{
}


// lfx
}
