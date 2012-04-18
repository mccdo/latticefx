
#ifndef __LATTICEFX_MASK_UTILS_H__
#define __LATTICEFX_MASK_UTILS_H__ 1


#include <latticefx/Export.h>
#include <latticefx/ChannelData.h>



namespace lfx {


/** \defgroup MaskUtils Utilities for masking data on the host CPU
*/
/**@{*/


/** \brief Return a copy of the channel containing only enabled elements.
\details Obtains a copy of masked data. As an example, consider a Renderer designed for efficient
GPU memory usage. Rather than send a complete copy of the data to the GPU and use shaders to
skip rendering masked objects, the Renderer would invoke getMaskedChannel() and send down only
the unmasked data to the GPU.
\returns NULL if the named channel doesn't exist at the specified time. */
LATTICEFX_EXPORT ChannelDataPtr getMaskedChannel( const ChannelDataPtr source, const ChannelDataPtr mask );


/**@}*/


// lfx
}


// __LATTICEFX_MASK_UTILS_H__
#endif
