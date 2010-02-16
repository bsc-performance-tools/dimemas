/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  Dimemas                                  *
 *       Simulation tool for the parametric analysis of the behaviour of     *
 *       message-passing applications on a configurable parallel platform    *
 *                                                                           * 
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\

  $URL::                  $:  File
  $Rev::                  $:  Revision of last commit
  $Author::               $:  Author of last commit
  $Date::                 $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef MACROS_H
#define MACROS_H

/*#   include "utils/CallStack.h" */

#   include <stdio.h>
#   include <stdlib.h>
#   include <errno.h>

#   ifdef ERROR
#     undef ERROR
#   endif

#   ifdef CALLSTACK
#     include "CallStack.h"
#     define BEGINFN( x ) { CallStackEnter( #x ); }
#     define ENDFN( x )   { CallStackLeave( #x ); }
#   else
#     define BEGINFN( x )
#     define ENDFN( x )
#   endif

#   define SWAP( a, b, tmp ) { (tmp)= (a); (a) = (b); (b) = (tmp); }

/* << ERROR >> Not needed now!
#   define ERROR( x ) \
    {\
      fflush(NULL);\
      fprintf( stderr, "\n");\
      fprintf( stderr, "* Error: %s\n", x );\
      if (errno) fprintf( stderr, "* Error: %s\n", strerror(errno) );\
      fprintf( stderr, "* Error: %s (%d)\n",  __FILE__, __LINE__ );\
      fprintf( stderr, "*\n");\
      fflush(NULL);\
      exit( EXIT_FAILURE );\
      abort();\
    }
*/

/* << WARNING >> */
#   define WARNING( x ) \
    {\
      fprintf( stderr, "Warning message (process %d): %s, file %s (%d)\n", \
                       getpid(), x, __FILE__, __LINE__ );\
    }

/* << DEBUGMESSAGE >> */
#if 0

#   define DEBUGMESSAGE( x ) \
    {\
      fprintf( stderr, "Debug message : %s, file %s (%d)\n", \
                       x, __FILE__, __LINE__ );\
      fflush( stderr );\
    }

extern int StackNumber;
    
#   define BEGINFNS( x ) \
    {\
      int ii;\
      \
      StackNumber++;\
      for( ii=0; ii<StackNumber; ii++)\
      {\
        fprintf( stderr, " " );\
      }\
      fprintf( stderr, "-" );\
      fprintf( stderr, ">" );\
      fprintf( stderr, "%s, file %s (%d)\n", x, __FILE__, __LINE__ );\
      fflush( stderr );\
    }
    
#   define ENDFNS( x ) \
    {\
      int ii;\
      \
      for( ii=0; ii<StackNumber; ii++)\
      {\
        fprintf( stderr, " " );\
      }\
      fprintf( stderr, "<" );\
      fprintf( stderr, "-" );\
      StackNumber--;\
      fprintf( stderr, "%s, file %s (%d)\n", x, __FILE__, __LINE__ );\
      fflush( stderr );\
    }
    
#else

#   define DEBUGMESSAGE( x )

#   define BEGINFN1( x )

#   define ENDFN1( x )

#endif


#include <assert.h>

/* << ASSERT_ERROR >> */
#   define ASSERT_ERROR( x ) \
    {\
      fflush(NULL);\
      fprintf( stderr, "\n");\
      fprintf( stderr, "* Assertion failed: %s at %s (%d)\n", x, __FILE__, __LINE__ );\
      fprintf( stderr, "*\n");\
      fflush(NULL);\
      exit( EXIT_FAILURE );\
      abort();\
    }

/* << ASSERT_CONDITION >> */
#define ASSERT_CONDITION( condition )  assert( (condition) )

/* << ASSERT >> */
#define ASSERT( x ) if (!(x)) ASSERT_ERROR( #x )


#define BZERO( nelem, type, ptr ) \
   bzero ( ptr, ((size_t)(nelem)) * sizeof(type) )
      
#if defined( DEBUG_MEMORY_ALLOC )

#include <Memory.h>

#else
/* << ALLOC >> */
#   define ALLOC( nelem, type, var ) \
      malloc ( ((size_t)(nelem)) * sizeof(type) )
      
# define REALLOC( ptr, nelem, type, var )\
      realloc ( ptr, ((size_t)(nelem)) * sizeof(type) )

# define FREE( x )\
{\
  free( (void *)x );\
  x = NULL;\
}

#endif

#define DEBUG0( x )
#define DEBUG1( x, y )
#define DEBUG2( x, y, z ) 
#define DEBUG3( x, y, z, w )
#define DEBUG4( x, y, z, w, v )

#endif
