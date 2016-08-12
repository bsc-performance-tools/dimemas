#ifndef _COMMON_MACROS_H
#define _COMMON_MACROS_H

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define BEGINFN( x )
#define ENDFN( x )


#   define SWAP( a, b, tmp ) { (tmp)= (a); (a) = (b); (b) = (tmp); }

/* << WARNING >> */
#   define WARNING( x ) \
    {\
      fprintf( stderr, "Warning message (process %d): %s, file %s (%d)\n", \
                       getpid(), x, __FILE__, __LINE__ );\
    }

#   define DEBUGMESSAGE( x )

#   define BEGINFN1( x )

#   define ENDFN1( x )

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

/* << ERROR >> */
/*#   define ERROR( x ) \
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
    }*/

/*#define BZERO( nelem, type, ptr ) \
   memset ( ptr, '\0', ((size_t)(nelem)) * sizeof(type) )*/
      
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

#define DEBUG0( x )
#define DEBUG1( x, y )
#define DEBUG2( x, y, z ) 
#define DEBUG3( x, y, z, w )
#define DEBUG4( x, y, z, w, v )

#endif
