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
#ifndef LATTICEFX_COMPILER_GUARDS_H
#define LATTICEFX_COMPILER_GUARDS_H

//From: 
//https://svn.boost.org/trac/boost/wiki/Guidelines/WarningsGuidelines
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
#define GCC_DIAG_STR(s) #s
#define GCC_DIAG_JOINSTR(x,y) GCC_DIAG_STR(x ## y)
# define GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
# define DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
# if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
#  define DIAG_OFF(x) DIAG_PRAGMA(push) \
DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W,x))
#  define DIAG_ON(x) DIAG_PRAGMA(pop)
# else
#  define DIAG_OFF(x) DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W,x))
#  define DIAG_ON(x)  DIAG_PRAGMA(warning GCC_DIAG_JOINSTR(-W,x))
# endif
#elif defined( __clang__ )
#  define CLANG_DIAG_STR(s) # s
// stringize s to "no-unused-variable"
#  define CLANG_DIAG_JOINSTR(x,y) CLANG_DIAG_STR(x ## y)
//  join -W with no-unused-variable to "-Wno-unused-variable"
#  define CLANG_DIAG_DO_PRAGMA(x) _Pragma (#x)
// _Pragma is unary operator  #pragma ("")
#  define DIAG_PRAGMA(x) CLANG_DIAG_DO_PRAGMA(clang diagnostic x)
#    define DIAG_OFF(x) DIAG_PRAGMA(push) \
DIAG_PRAGMA(ignored CLANG_DIAG_JOINSTR(-W,x))
// For example: #pragma clang diagnostic ignored "-Wno-unused-variable"
#   define DIAG_ON(x) DIAG_PRAGMA(pop)
// For example: #pragma clang diagnostic warning "-Wno-unused-variable"
#else // Ensure these macros so nothing for other compilers.
# define DIAG_OFF(x)
# define DIAG_ON(x)
# define DIAG_PRAGMA(x)
#endif

/* Usage:
 DIAG_OFF(unused-variable)
 DIAG_OFF(unused-parameter)
 DIAG_OFF(uninitialized)
 */

#endif //LATTICEFX_COMPILER_GUARDS_H
