
#ifndef __LATTICEFX_TRANSFER_FUNCTION_UTILS_H__
#define __LATTICEFX_TRANSFER_FUNCTION_UTILS_H__ 1


#include <latticefx/Export.h>

#include <string>


namespace osg {
    class Image;
}


namespace lfx {


/** \defgroup TransferFunctionUtils Utilities for transfer functions
*/
/**@{*/


/** \def TBD */
#define LFX_ALPHA_CONSTANT 0
#define LFX_ALPHA_RAMP_0_TO_1 1
#define LFX_ALPHA_RAMP_1_TO_0 2

/** \brief Load a transfer function from a .dat file.
\details The .dat file format is described here:
http://local.wasp.uwa.edu.au/~pbourke/texture_colour/colourramp/
It is essentially a text file consisting of an unsigned char index
followed by an unsigned char rgb triple. Multiple unsigned char quadruples
may appear on a line.

Lines that begin with '#' are skipped. They may be used as comments.

This function reads the .dat file into an osg::Image with one dimension. */
LATTICEFX_EXPORT osg::Image* loadImageFromDat( const std::string& fileName,
    const unsigned int alphaPolicy=LFX_ALPHA_CONSTANT, float alpha=1.f );


/**@}*/


// lfx
}


// __LATTICEFX_TRANSFER_FUNCTION_UTILS_H__
#endif
