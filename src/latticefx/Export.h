
#ifndef __LATTICEFX_EXPORT_H__
#define __LATTICEFX_EXPORT_H__ 1


#if defined( _MSC_VER ) || defined( __CYGWIN__ ) || defined( __MINGW32__ ) || defined( __BCPLUSPLUS__ ) || defined( __MWERKS__ )
    #if defined( LATTICEFX_STATIC )
        #define LATTICEFX_EXPORT
    #elif defined( LATTICEFX_LIBRARY )
        #define LATTICEFX_EXPORT __declspec( dllexport )
    #else
        #define LATTICEFX_EXPORT __declspec( dllimport )
    #endif
#else
    #define LATTICEFX_EXPORT
#endif


// __LATTICEFX_EXPORT_H__
#endif
