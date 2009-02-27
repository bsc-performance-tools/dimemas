/*****************************************************************************
 *                                                                           *
 *  PALLAS software tools environment                                        *
 *                                                                           *
 *  Copyright (c) PALLAS GmbH 1994, 1995, 1996                               *
 *                                                                           *
 *  Tool-independent include file                                            *
 *                                                                           *
 *  $Revision: 1.2 $
 *                                                                           *
 *  $Date: 2005/05/24 15:41:40 $
 *                                                                           *
 *****************************************************************************/

/*****************************************************************************
 *                                                                           *
 * $Log: pal_env.h,v $
 * Revision 1.2  2005/05/24 15:41:40  paraver
 * Modificacion global para retomar la version Stable como version de desarrollo
 *
 * Revision 1.6  1998/07/27 10:45:09  astein
 * added new platforms
 *
 * Revision 1.5  1998/05/29 09:38:54  astein
 * new platform ScaMPI x86 (Solaris), added 2nd Fortran binding
 *
 * Revision 1.4  1997/07/07 15:00:53  astein
 * Release 1.5 for SX4 (OS7.1)
 *
 * Revision 1.3  1997/07/01 12:47:37  astein
 * new version for RS6K-OS4 and HPPA-OS10
 *
 * Revision 1.2  1997/06/25 14:16:27  astein
 * mofified for NEC--CJ3
 *
 * Revision 1.1.1.1  1997/06/02 08:22:47  astein
 * Initial revision
 *
 *****************************************************************************/

#ifndef __PAL_ENV_H
#define __PAL_ENV_H

/****************************************************************************
 * Platform-specific defines for the license checker
 ****************************************************************************/

/*----------------------------------------------------------------
 *              DEC Alpha CPU, OSF/1 3.x
 *----------------------------------------------------------------*/
#if defined(PAL_PLATFORM_AXP) && defined(PAL_ARCH_OS3)
/* --> DEC Alpha CPU, OSF/1 3.x <-- */
#include <netdb.h>
#define LC_ARCHSTRING "AXP--OS3"
#define LC_TYPESTRING "W"
#define LITTLEENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef signed long    Int64;
typedef unsigned long  Card64;
typedef unsigned int c_32bit_t;

/*----------------------------------------------------------------
 *              DEC Alpha CPU, OSF/1 4.x
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_AXP) && defined(PAL_ARCH_OS4)
/* --> DEC Alpha CPU, OSF/1 4.x <-- */
#include <netdb.h>
#define LC_ARCHSTRING "AXP--OS4"
#define LC_TYPESTRING "W"
#define LITTLEENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef signed long    Int64;
typedef unsigned long  Card64;
typedef unsigned int c_32bit_t;

/*----------------------------------------------------------------
 *              Cray YMP CPU, Unicos 9.x
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_CRAY) && defined(PAL_ARCH_YMP)
/* --> Cray YMP CPU, Unicos 9.x  */
/* no extra include files */
#define LC_ARCHSTRING "CRAY-YMP"
#define LC_TYPESTRING "V"
#error "missing typedefs, ENDIAN & IEEE definition"
typedef unsigned short c_32bit_t;

/*----------------------------------------------------------------
 *              Cray J90 CPU, Unicos 9.x
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_CRAY) && defined(PAL_ARCH_J90)
/* --> Cray J90 CPU, Unicos 9.x  */
/* no extra include files */
#define LC_ARCHSTRING "CRAY-J90"
#define LC_TYPESTRING "V"
#error "missing typedefs, ENDIAN & IEEE definition"
typedef unsigned short c_32bit_t;

/*----------------------------------------------------------------
 *              Cray T3D
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_CRAY) && defined(PAL_ARCH_T3D)
/* --> Cray MPP  */
/* no extra include files */
#define LC_ARCHSTRING "CRAY-T3D"
#define LC_TYPESTRING "S"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int32;
typedef unsigned short Card32;
typedef signed long    Int64;
typedef unsigned long  Card64;
typedef unsigned short c_32bit_t;

/*----------------------------------------------------------------
 *              Cray T3E
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_CRAY) && defined(PAL_ARCH_T3E)
/* no extra include files */
#define LC_ARCHSTRING "CRAY-T3E"
#define LC_TYPESTRING "S"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int32;
typedef unsigned short Card32;
typedef signed long    Int64;
typedef unsigned long  Card64;
typedef unsigned short c_32bit_t;

/*----------------------------------------------------------------
 *              HP PA-RISC CPU, HPUX 9.x
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_HPPA) && defined(PAL_ARCH_OS9)
/* --> HP PA-RISC CPU, HPUX 9.x <-- */
#include <sys/utsname.h>
#define LC_ARCHSTRING "HPPA-OS9"
#define LC_TYPESTRING "W"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;   /* sizeof(int) == sizeof(long) */

/*----------------------------------------------------------------
 *              HP PA-RISC CPU, HPUX 10.x
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_HPPA) && defined(PAL_ARCH_OS10)
/* --> HP PA-RISC CPU, HPUX 10.x <-- */
#include <sys/utsname.h>
#define LC_ARCHSTRING "HPPA-OS10"
#define LC_TYPESTRING "W"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;   /* sizeof(int) == sizeof(long) */

/*----------------------------------------------------------------
 *              HP PA-RISC CPU, SPPUX 5.x
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_HPPA) && defined(PAL_ARCH_SPP5)
/* --> HP PA-RISC CPU, HPUX 10.x <-- */
#include <sys/utsname.h>
#define LC_ARCHSTRING "HPPA-SPP5"
#define LC_TYPESTRING "S"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;   /* sizeof(int) == sizeof(long) */

/*----------------------------------------------------------------
 *              NEC SX-4, Super-UX [678].x 
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_SX4) && \
      (defined(PAL_ARCH_OS6) || defined(PAL_ARCH_OS7) || defined(PAL_ARCH_OS8))
/* NEC SX-4, Super-UX [678].x */
#ifndef RDCLK                /* avoid multiple include of syssx */
#include <sys/types.h>
#include <sys/syssx.h>
#endif
#define LC_ARCHSTRING "NEC--SX4"
#define LC_TYPESTRING "V"
#define BIGENDIAN
#define HAS_16
#define HAS_32
#ifdef _FLOAT0
/* float0 format is IEEE conformant */
#define HAS_IEEE
#else
/* conversion macros for float1 and float2 format */
#include <math.h>
#ifdef _FLOAT1
#define fl_mode "float1" 
#define ie3_cnv ie3_fl1
#define cnv_ie3 fl1_ie3
#else
#define ie3_cnv ie3_fl2
#define cnv_ie3 fl2_ie3
#define fl_mode "float2" 
#endif
#define DOUBLE_FROM_IEEE(valp, from) \
  /* convert IEEE value *from to double */ \
  { int rc; \
    rc = ie3_cnv ((char *)from, (char *)valp, 8, 8, 1); \
    if (rc != 1) { \
      fprintf(stderr, "Can't convert IEEE double-precision value to " \
              fl_mode " format.\n"); \
      *valp = 0.0; \
    } \
  }
#define IEEE_FROM_DOUBLE(to, val) \
  /* convert double value val to IEEE value *to */ \
  { Double d = val; int rc; \
    rc = cnv_ie3 ((char *)&d, (char *)to, 8, 8, 1); \
    if (rc != 1) { \
      fprintf(stderr, "Can't convert " fl_mode \
              " double precision value to IEEE format.\n"); \
      memset(to, 0, 8); \
    } \
  }
#endif /* _FLOAT0 */
#ifdef _INT64
typedef signed char         Int8;
typedef unsigned char       Card8;
typedef signed short        Int16;
typedef unsigned short      Card16;
typedef signed _int32       Int32;
typedef unsigned _int32     Card32;
typedef signed long long    Int64;
typedef unsigned long long  Card64;
typedef unsigned _int32 c_32bit_t;
#else
typedef signed char         Int8;
typedef unsigned char       Card8;
typedef signed short        Int16;
typedef unsigned short      Card16;
typedef signed int          Int32;
typedef unsigned int        Card32;
typedef signed long long    Int64;
typedef unsigned long long  Card64;
typedef unsigned int c_32bit_t;
#endif  /* _INT64 SX4 */

/*----------------------------------------------------------------
 *              NEC Cenju 3 (R4400)
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_NEC) && defined(PAL_ARCH_CJ3)
#define LC_ARCHSTRING "NEC--CJ3"
#define LC_TYPESTRING "S"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_IEEE
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;     /* sizeof(int) == sizeof(long) */

/*----------------------------------------------------------------
 *              SGI MPIS R4000 compatible CPU, IRIX 5
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_R4K) && defined(PAL_ARCH_OS5)
/* --> SGI MPIS R4000 compatible CPU, IRIX 5 */
#include <sys/systeminfo.h>
#define LC_ARCHSTRING "R4K--OS5"
#define LC_TYPESTRING "W"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;     /* sizeof(int) == sizeof(long) */

/*----------------------------------------------------------------
 *              SGI MPIS R4000 compatible CPU, IRIX 6
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_R4K) && defined(PAL_ARCH_OS6)
/* --> SGI MPIS R4000 compatible CPU, IRIX 6 */
#include <sys/systeminfo.h>
#define LC_ARCHSTRING "R4K--OS6"
#define LC_TYPESTRING "W"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;

/*----------------------------------------------------------------
 *              SGI MIPS R8000 compatible CPU, IRIX 6
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_SGI) && defined(PAL_ARCH_POWC)
/* --> SGI MIPS R8000 compatible CPU, IRIX 6 */
#include <sys/systeminfo.h>
#define LC_ARCHSTRING "SGI--POWC"
#define LC_TYPESTRING "S"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef signed long    Int64;
typedef unsigned long  Card64;
typedef unsigned int c_32bit_t;

/*----------------------------------------------------------------
 *              SGI MIPS R10000 compatible CPU, IRIX 6
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_SGI) && defined(PAL_ARCH_ORI2)
/* --> SGI MIPS R8000 compatible CPU, IRIX 6 */
#include <sys/systeminfo.h>
#define LC_ARCHSTRING "SGI--ORI2"
#define LC_TYPESTRING "S"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef signed long    Int64;
typedef unsigned long  Card64;
typedef unsigned int c_32bit_t;


/*----------------------------------------------------------------
 *              IBM Power CPU, AIX 3.x
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_RS6K) && defined(PAL_ARCH_OS3)
/* --> IBM Power CPU, AIX 3.x */
#include <sys/utsname.h>
#define LC_ARCHSTRING "RS6K-OS3"
#define LC_TYPESTRING "W"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;

/*----------------------------------------------------------------
 *              IBM Power CPU, AIX 4.x
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_RS6K) && defined(PAL_ARCH_OS4)
/* --> IBM Power CPU, AIX 3.x */
#include <sys/utsname.h>
#define LC_ARCHSTRING "RS6K-OS4"
#define LC_TYPESTRING "W"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;

/*----------------------------------------------------------------
 *              Sparc CPU, SunOS 4.x 
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_SPRC) && defined(PAL_ARCH_OS4)
/* --> Sparc CPU, SunOS 4.x <-- */
/* no extra include files */
#define LC_ARCHSTRING "SPRC-OS4"
#define LC_TYPESTRING "W"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;


/*----------------------------------------------------------------
 *              Sparc CPU, SunOS 5.x
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_SPRC) && defined (PAL_ARCH_OS5)
/* --> Sparc CPU, SunOS 5.x <-- */
#include <sys/systeminfo.h>
/*  Solaris 2.5 doesn't have a declaration for gethostname() */
int gethostname(char *name, int namelen);
#define LC_ARCHSTRING "SPRC-OS5"
#define LC_TYPESTRING "W"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;


/*----------------------------------------------------------------
 *              Intel x86 CPU, SunOS 5.x
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_I86) && defined (PAL_ARCH_OS5)
/* --> Sparc CPU, SunOS 5.x <-- */
#include <sys/systeminfo.h>
#define LC_ARCHSTRING "I86--OS5"
#define LC_TYPESTRING "W"
#define LITTLEENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;

/*----------------------------------------------------------------
 *              ScaMPI Intel x86 CPU, SunOS 5.x
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_SCAI) && defined (PAL_ARCH_OS5)
/* --> Sparc CPU, SunOS 5.x <-- */
#include <sys/systeminfo.h>
#define LC_ARCHSTRING "SCAI-OS5"
#define LC_TYPESTRING "S"
#define LITTLEENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;

/*----------------------------------------------------------------
 *              ScaMPI Intel x86 CPU, SunOS 5.x
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_SCAI) && defined (PAL_ARCH_OS6)
/* --> Sparc CPU, SunOS 5.x <-- */
#include <sys/systeminfo.h>
#define LC_ARCHSTRING "SCAI-OS6"
#define LC_TYPESTRING "S"
#define LITTLEENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;

/*----------------------------------------------------------------
 *              Intel x86 CPU, Linux 2.x
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_LINU) && defined (PAL_ARCH_OS2)

/* --> Intel x86 CPU, Linux 2.x <-- */
#include <netinet/in.h>
#include <netdb.h>
#define LC_ARCHSTRING "LINU-OS2"
#define LC_TYPESTRING "W"
#define LITTLEENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;

/*----------------------------------------------------------------
 *              MIPS R4000 CPU, SVr4.2
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_EWS) && defined (PAL_ARCH_OS4)
/* --> MIPS CPU, SVR4 <-- */
#include <sys/systeminfo.h>
#define LC_ARCHSTRING "EWS--OS4"
#define LC_TYPESTRING "W"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;


/*----------------------------------------------------------------
 *              IBM Power CPU, AIX 4.x
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_IBM) && defined (PAL_ARCH_SP2)
#include <sys/utsname.h>
#define LC_ARCHSTRING "IBM--SP2"
#define LC_TYPESTRING "S"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;


/*----------------------------------------------------------------
 *              Intel i860 CPU, OSF/1
 *----------------------------------------------------------------*/
#elif defined(PAL_PLATFORM_INTL) && defined (PAL_ARCH_PGON)
/* Intel PARAGON */
#define LC_ARCHSTRING "INTL-PGON"
#define LC_TYPESTRING "S"
#define LITTLEENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;

/*----------------------------------------------------------------
 *              Hitachi SR2201
 *----------------------------------------------------------------*/
#elif defined (PAL_PLATFORM_HITA) && defined (PAL_ARCH_SR2201)
#define LC_ARCHSTRING "HITA-MPP2"
#define LC_TYPESTRING "V"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;


/*----------------------------------------------------------------
 *              Fujitsu VPP300
 *----------------------------------------------------------------*/
#elif defined (PAL_PLATFORM_FUJI) && defined (PAL_ARCH_VPP3)
#define LC_ARCHSTRING "FUJI-VPP3"
#define LC_TYPESTRING "V"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;


/*----------------------------------------------------------------
 *              Fujitsu VPP700
 *----------------------------------------------------------------*/
#elif defined (PAL_PLATFORM_FUJI) && defined (PAL_ARCH_VPP7)
#define LC_ARCHSTRING "FUJI-VPP7"
#define LC_TYPESTRING "V"
#define BIGENDIAN
#define HAS_IEEE
#define HAS_16
#define HAS_32
typedef signed char    Int8;
typedef unsigned char  Card8;
typedef signed short   Int16;
typedef unsigned short Card16;
typedef signed int     Int32;
typedef unsigned int   Card32;
typedef unsigned long c_32bit_t;

#else
#error Invalid platform/architecture specification
#endif /* PAL_PLATFORM_AXP */

/*****************************************************************************
 * Typedefs for KFA binlib
 *****************************************************************************/
typedef long   FilePos;
typedef char   Char;
typedef double Double;
typedef float  Single;

typedef Card8  Byte;
typedef Int8   ShortInt;
typedef Byte   Booli; 
#ifdef HAS_16
typedef Card16 Word;
typedef Int16  Integer;
#endif

#ifdef HAS_32
typedef Card32 LongWord;
typedef Int32  LongInt;
#else
#error "32 Bit Datatypes must be defined !"
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*****************************************************************************
 * Constants and macros for the PALLAS licensechecker
 *****************************************************************************/
/**
 * Scope specifiers
 **/
#define PUBLIC  extern
#define PRIVATE static
#define PROC_LOCAL
#define PROC_GLOBAL
/**
 * PARMACS integer type
 **/
typedef int pm_int;
/**
 * String length
 **/
#define PM_STRLEN 256
/**
 * Abort function
 **/
#define PM_ABORT(msg) \
/* abort the application, print message */ \
do {fprintf(stderr, "%s internal ERROR: %s, terminating.\n", \
            msg, PAL_TOOLNAME);\
    fflush(stderr); \
    exit(EXIT_FAILURE); } while (0)
/**
 * Safe allocation funtions
 **/
#define PM_ALLOC(ptr,typ,n,str) \
/* Allocate space for <n> copies of <typ> */ \
do {if (NULL==(ptr=(typ *)malloc((n)*sizeof(typ)))) \
    PM_ABORT("could not allocate " #typ " in " str);} while (0)

#define PM_REALLOC(ptr,typ,n,str) \
/* Allocate space for <n> copies of <typ> */ \
do {if (NULL==(ptr=(typ *)realloc(ptr,(n)*sizeof(typ)))) \
    PM_ABORT("could not allocate " #typ " in " str);} while (0)

#define PM_FREE(ptr,str) \
/* Free space referenced by ptr */ \
free(ptr)
/**
 * Debugging macros
 **/
#if DEBUG >= 1
#ifndef DEBUG1
#define DEBUG1(a)       printf a; fflush(stdout)
#endif
#else
#ifndef DEBUG1
#define DEBUG1(a)
#endif
#endif
#if DEBUG >= 2
#ifndef DEBUG2
#define DEBUG2(a)       printf a; fflush(stdout)
#endif
#else
#ifndef DEBUG2
#define DEBUG2(a)
#endif
#endif
#if DEBUG >= 3
#ifndef DEBUG3
#define DEBUG3(a)       printf a; fflush(stdout)
#endif
#else
#ifndef DEBUG3
#define DEBUG3(a)
#endif
#endif
/**
 * On some platforms, crucial definitions are missing ...
 **/
#ifndef EXIT_FAILURE
#define EXIT_FAILURE -1
#endif
#ifndef MAX
#define MAX(a,b)        ((a) >= (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)        ((a) >= (b) ? (b) : (a))
#endif

#endif /* __PAL_ENV_H */
