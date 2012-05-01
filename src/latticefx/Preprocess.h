
#ifndef __LATTICEFX_PREPROCESS_H__
#define __LATTICEFX_PREPROCESS_H__ 1


#include <latticefx/Export.h>
#include <latticefx/OperationBase.h>

#include <boost/smart_ptr/shared_ptr.hpp>

#include <list>



namespace lfx {


/** \class Preprocess Preprocess.h <latticefx/Preprocess.h>
\brief Base class for Preprocessing & Caching operations.
\details TBD
*/
class LATTICEFX_EXPORT Preprocess : public lfx::OperationBase
{
public:
    Preprocess();
    Preprocess( const Preprocess& rhs );
    virtual ~Preprocess();

protected:
};

typedef boost::shared_ptr< Preprocess > PreprocessPtr;
typedef std::list< PreprocessPtr > PreprocessList;


// lfx
}


// __LATTICEFX_PREPROCESS_H__
#endif
