#ifndef _TYPES_H_
#define _TYPES_H_

#include <config.h>

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#  include <inttypes.h>
#endif

#if defined( HAVE_INT64_T ) && defined( HAVE_UINT64_T )
/* If system has uint64_t/int64_t use them first */
typedef int64_t INT64;
typedef uint64_t UINT64;
#else
/* If system does not have UINT64/INT64, check which type can be used instead */
#  if defined( HAVE_LONG_LONG ) && ( SIZEOF_LONG_LONG == 8 )
/* long long occupies 8 bytes!, use it as UINT64/INT64 */
typedef long long INT64;
typedef unsigned long long UINT64;
#  elif defined( HAVE_LONG ) && ( SIZEOF_LONG == 8 )
/* long occupies 8 bytes!, use it as UINT64/INT64 */
typedef long INT64;
typedef unsigned long UINT64;
#  else
#    error "No 64-bits data type found"
#  endif
#endif

#if defined( HAVE_INT8_T ) && defined( HAVE_UINT8_T )
/* If system has uint8/int8 use them first */
typedef int8_t INT8;
typedef uint8_t UINT8;
#else
/* If system does not have UINT8/INT8, check which type can be used instead */
#  if defined( HAVE_CHAR ) && ( SIZEOF_CHAR == 1 )
/* char occupies 1 byte!, use it as UINT8/INT8 */
typedef char INT8;
typedef unsigned char UINT8;
#  else
#    error "No 8-bits data type found"
#  endif
#endif

#if defined( HAVE_INT32_T ) && defined( HAVE_UINT32_T )
/* If system has uint32/int32 use them first */
typedef int32_t INT32;
typedef uint32_t UINT32;
#else
/* If system does not have UINT32/INT32, check which type can be used instead */
#  if defined( HAVE_INT ) && ( SIZEOF_INT == 4 )
/* int occupies 4 bytes!, use it as UINT32/INT32 */
typedef int INT32;
typedef unsigned int UINT32;
#  else
#    error "No 32-bits data type found"
#  endif
#endif

typedef UINT64 STACK_ADDRESS;

#endif /* _TYPES_H_ */
