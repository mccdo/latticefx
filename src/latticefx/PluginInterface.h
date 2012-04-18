
#ifndef __LATTICEFX_PLUGIN_INTERFACE_H__
#define __LATTICEFX_PLUGIN_INTERFACE_H__ 1


#include <latticefx/Export.h>

#include <osg/ref_ptr>
#include <osg/Referenced>

#include <list>



namespace lfx {


/** \class PluginInterface PluginInterface.h <latticefx/PluginInterface.h>
\brief Common interface between LatticeFX and all plugin shared libraries.
\details TBD
*/
class LATTICEFX_EXPORT PluginInterface
{
public:
    PluginInterface();
    virtual ~PluginInterface();
};


// lfx
}


// __LATTICEFX_PLUGIN_INTERFACE_H__
#endif
