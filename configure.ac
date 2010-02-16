dnl Process this file with autoconf to produce a configure script.

SVN_REVISION="$Rev$"

AC_INIT(dimemas, 3.00, tools@bsc.es)
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE
AC_PREFIX_DEFAULT(${DIMEMASDIR:-/usr/local})

AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

AC_PROG_CC(icc xlc cc gcc)
AC_PROG_CXX(icpc xlC CC g++)
AC_PROG_CPP


dnl Transforming Program Names When Installing
dnl AC_ARG_PROGRAM
dnl Look for the install program
AC_PROG_INSTALL
dnl Look for the ranlib program
AC_PROG_RANLIB
dnl Look for yacc (bison) program
AC_PROG_YACC
AM_PROG_LEX
test -n "$YACC" && YACC="$YACC -d"
dnl If the package is not flat
AC_PROG_MAKE_SET

dnl =========================================================================
dnl Check if compiler hase __FUNCTION__ macro
dnl =========================================================================

dnl AC_CHECK__FUNCTION__

CFLAGS=""

dnl =========================================================================
dnl Architecture and OS detection
dnl =========================================================================

case "$host_cpu*" in
  i386*    ) Architecture="ia32"    ;;
  i486*    ) Architecture="ia32"    ;;
  i586*    ) Architecture="ia32"    ;;
  i686*    ) Architecture="ia32"    ;;
  powerpc* ) Architecture="powerpc" ;;
  ia64*    ) Architecture="ia64"    ;;
  alpha*   ) Architecture="alpha"   ;;
  mips*    ) Architecture="mips"    ;;
esac

case "x$host_os" in
  xlinux*  ) Operating_System="linux" ;;
  xaix*    ) Operating_System="aix"   ;;
  xosf*    ) Operating_System="dec"   ;;
  xirix*   ) Operating_System="irix"  ;;
  xdarwin* ) Operating_System="osx"   ;;
  xcygwin* ) Operating_System="cygwin";;
esac

dnl =========================================================================
dnl Check which compiler we are using to use the adequate parameter to 
dnl generate all the warnings in the source code
dnl =========================================================================

AC_MSG_CHECKING([for full warnings option in compiler])
if test "$ac_compiler_gnu" = "yes" ; then
  WARNINGS_IN_COMPILER="-Wall -W"
elif test "$CC" = "xlc" ; then
  WARNINGS_IN_COMPILER=""
elif test "$CC" = "icc" ; then
  WARNING_IN_COMPILER="-Wall"
elif test "$CC" = "cc" ; then
  if test "$Architecture" = "mips" -a "$Operating_System" = "irix" ; then
    WARNINGS_IN_COMPILER="-fullwarn"
  elif test "$Architecture" = "alpha" -a "$Operating_System" = "dec" ; then
    WARNINGS_IN_COMPILER="-warnprotos -w0"
  fi
fi
AC_MSG_RESULT([$WARNINGS_IN_COMPILER])
CFLAGS="$CFLAGS $WARNINGS_IN_COMPILER"
CXXFLAGS="$CXXFLAGS $WARNINGS_IN_COMPILER"

dnl =========================================================================
dnl Check whether the compilers need additional parameters
dnl =========================================================================

AC_MSG_CHECKING([for additional options in compilers C/C++])
if test "$ac_compiler_gnu" = "yes" ; then
  ADDITIONAL_FLAGS_CC=""
  ADDITIONAL_FLAGS_CXX=""
elif test "$CC" = "xlc" ; then
  ADDITIONAL_FLAGS_CC="-O -qcpluscmt"
  ADDITIONAL_FLAGS_CXX="-O -qstaticinline"
elif test "$CC" = "cc" ; then
  if test "$Architecture" = "mips" -a "$Operating_System" = "irix" ; then
    ADDITIONAL_FLAGS_CC=""
    ADDITIONAL_FLAGS_CXX=""
  elif test "$Architecture" = "alpha" -a "$Operating_System" = "dec" ; then
    ADDITIONAL_FLAGS_CC=""
    ADDITIONAL_FLAGS_CXX=""
  fi
fi
CFLAGS="$CFLAGS $ADDITIONAL_FLAGS_CC"
CXXFLAGS="$CXXFLAGS $ADDITIONAL_FLAGS_CXX"

dnl Just to write a nice message to the user 
if test "$ADDITIONAL_FLAGS_CC"  = "" ; then
  ADDITIONAL_FLAGS_CC="none"
fi
if test "$ADDITIONAL_FLAGS_CXX" = "" ; then
  ADDITIONAL_FLAGS_CXX="none"
fi

dnl Specific LDFLAGS for Intel C Compiler
if test "$CC" = "icc" ; then
  LDFLAGS="-i-static $LDFLAGS"
fi

dnl Do we have inline?
AC_C_INLINE

dnl =======================================================================
dnl Architecture and OS specific flags
dnl =======================================================================

case "$Operating_System" in
  linux  ) arch_defines="-DARCH_LINUX" ;;
  aix    ) arch_defines="-DARCH_AIX";;
  dec    ) arch_defines="-DARCH_ALPHA";;
  irix   ) arch_defines="-DARCH_SGI" ;;
  osx    ) arch_defines="-DARCH_MACOSX" ;;
  cygwin ) arch_defines="-DARCH_CYGWIN";;
esac

case "Architecture" in
  ia32    ) ;;
  powerpc ) ;;
  ia64    ) arch_defines="$arch_defines -DARCH_ITANIUM";;
  alpha   ) ;;
  mips    ) ;;
esac

AC_SUBST(arch_defines)

dnl =======================================================================
dnl Optional defines
dnl =======================================================================

AC_DEFINE(PAL_PLATFORM_LINU)

AC_DEFINE(PAL_ARCH_OS2)

AC_DEFINE(PALLAS)

AC_DEFINE(DIMEMAS)

AC_DEFINE(VT_LCNETID)

AC_DEFINE_UNQUOTED(DATE, "`date`")

dnl Needed to compile 'ParaverBinaryTrace' from 'common-files'
AC_DEFINE(PARAVER_TOOL_COMPILATION)

dnl =======================================================================
dnl Conditional compilation flags
dnl =======================================================================

dnl == Code Coverage compile ==

AC_ARG_ENABLE(
  code-coverage,
  AS_HELP_STRING(
    [--enable-code-coverage],
    [compile with code coverage information (only GCC)]
  ),
  [if test "$ac_compiler_gnu" = "yes"; then
     CFLAGS="$CFLAGS -fprofile-arcs -ftest-coverage"
   else
     AC_WARN(Code coverage is only supported with GNU compilers)
   fi
  ]
)

dnl == GPROF Debug ==

AC_ARG_ENABLE(
  gprof-debug,
  AS_HELP_STRING(
    [--enable-gprof-debug],
    [compile with gprof profiling information]
  ),
  [CFLAGS="$CFLAGS -pg"]
)

dnl == General Debug ==

AC_ARG_ENABLE(
  debug,
  AS_HELP_STRING(
    [--enable-debug],
    [compile with general debug information]
  ),
  [CFLAGS="$CFLAGS -g"]
)

dnl == High optimization ==

AC_ARG_ENABLE(
  optimization,
  AS_HELP_STRING(
    [--enable-optimization],
    [compile with high optimization]
  ),
  [
    if test "$CC" = "xlc" ; then
      CFLAGS="$CFLAGS -O3"
    elif test "$CC" = "icc" ; then
      CFLAGS="$CFLAGS -ipo5 -O3 -Os"
    else
      CFLAGS="$CFLAGS -O3 -Os -funroll-loops"
    fi

    if test "$CXX" = "xlC"; then
      CXXFLAGS="$CXXFLAGS -O3"
    else
      CXXFLAGS="$CXXFLAGS -O3 -Os -funroll-loops"
    fi
  ]
)

dnl =======================================================================
dnl Conditional defines
dnl =======================================================================

dnl LICENSE_CHECK: if defined Dimemas checks license on startup
AC_ARG_ENABLE(
  no-license,
  AS_HELP_STRING(
    [--enable-no-license],
    [compile without license check]
  ),
  AC_DEFINE(NO_LICENSE)
)

dnl OLD_PRV_FORMAT: if defined Dimemas generates Paraver traces in old format

AC_ARG_ENABLE(
  old-prv-format,
  AS_HELP_STRING(
    [--enable-old-prv-format],
    [version with old Paraver format output]
  ),
  AC_DEFINE(OLD_PRV_FORMAT)
)

dnl FORMAT_VELL_RECORDS_4_PRV: if defined Dimemas generates Paraver traces with
dnl the old Record 4 format

AC_ARG_ENABLE(
  old-type4-records,
  AS_HELP_STRING(
    [--enable-old-type4-records],
    [version with old Paraver type 4 records]
  ),
  AC_DEFINE(FORMAT_VELL_RECORDS_4_PRV)
)

dnl DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED
dnl GENERATE_VAMPIR_ALLOWED: if defined Dimemas can generate Vampir traces
dnl AC_DEFINE(GENERATE_VAMPIR_ALLOWED)
dnl DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED

dnl STARTUP_ALS_IRECV: if defined Dimemas considers initial startups on 
dnl MPI_Irecv functions

AC_ARG_ENABLE(
  irecv-startup, 
  AS_HELP_STRING(
    [--enable-irecv-startup],
    [version with startup time during IRECV primitives]
  ),
  AC_DEFINE(STARTUP_ALS_IRECV)
)

dnl LOGICAL_RECEIVE_ALS_WAIT: if defined Dimemas puts logical receive time
dnl when MPI_Wait is called (not on MPI_Irecv)

AC_ARG_ENABLE(
  wait-logical-recv,
  AS_HELP_STRING(
    [--enable-wait-logical-recv],
    [version with logical recv time on WAIT primitives]
  ),
  AC_DEFINE(LOGICAL_RECEIVE_ALS_WAIT)
)

dnl ENABLE_LARGE_TRACES: if defined Dimemas can use traces larger than 2Gb

AC_ARG_ENABLE(
  large-traces,
  AS_HELP_STRING(
    [--enable-large-traces],
    [version with support for traces larger than 2GiB]
  ),
  if test "$Operating_System" = "cygwin"; then
    AC_DEFINE(__LARGE64_FILES)
  elif test "$Operating_System" = "linux" ; then
    AC_DEFINE(_FILE_OFFSET_BITS, 64)
    AC_DEFINE(_LARGEFILE64_SOURCE)
  fi

  if test "$Operating_System" != "cygwin"; then
    AC_DEFINE(ENABLE_LARGE_TRACES)
  fi
)

dnl READ_TRACE_SEEKS: if defined Dimemas can use lseek shortcuts from traces

AC_ARG_ENABLE(
  read-trace-seeks,
  AS_HELP_STRING(
    [--enable-read-trace-seeks],
    [version with on-trace seek read support]
  ),
  AC_DEFINE(READ_TRACE_SEEKS)
)

dnl WRITE_TRACE_SEEKS: if defined Dimemas adds lseek information on traces

AC_ARG_ENABLE(
  write-trace-seeks,
  AS_HELP_STRING(
    [--enable-write-trace-seeks],
    [version with on-trace seek write support]
  ),
  AC_DEFINE(WRITE_TRACE_SEEKS)
)

dnl GESTIO_LINKS_ORIGINAL: if defined Dimemas uses the original protocol to
dnl manage links

AC_ARG_ENABLE(
  original-links,
  AS_HELP_STRING(
    [--enable-original-links],
    [version with original link management]
  ),
  AC_DEFINE(GESTIO_LINKS_ORIGINAL)
)

dnl NO_PARAVER_TYPE4: if defined Dimemas doesn't produce Paraver Type 4 
dnl records

AC_ARG_ENABLE(
  no-paraver-type4,
  AS_HELP_STRING(
    [--enable-no-paraver-type4],
    [version without Paraver type 4 records] 
  ),
  AC_DEFINE(NO_PARAVER_TYPE4)
)

dnl TIME_LIMIT: this variable is used on 'define.h' and indicates the maximum
dnl clock value on simulations. By default, it assumes 7h (2.52e10 us) of 
dnl maximum clock time

AC_ARG_WITH(
  time_limit,
  AS_HELP_STRING(
    [--with-time-limit=TIME-LIMIT], 
    [give the simulation time limit, in cientific notation]
  ),
  time_limit="$withval",
  time_limit="2.52e10"
)

AC_DEFINE_UNQUOTED(TIME_LIMIT, $time_limit)

dnl NO_COMPILATION_INFO: if defined Dimemas doesn't show compilation 
dnl information on help message
AC_ARG_ENABLE(
  no-compilation-info,
  AS_HELP_STRING(
    [--enable-no-compilation-info],
    [compile without compilation information]
  ),
  AC_DEFINE(NO_COMPILATION_INFO)
)

dnl IDLE_ACCOUNTING: if defined, idle CPU regions are computed as CPU
dnl consumption in the accouting stats
AC_ARG_ENABLE(
  idle-accounting,
  AC_HELP_STRING(
    [--enable-idle-accounting],
    [compile with idle blocks accouting]
  ),
  AC_DEFINE(IDLE_ACCOUNTING)
)


dnl VENUS_ENABLED: if defined, venus client are compiled and included in Dimemas
AC_ARG_ENABLE(
  venus,
  AC_HELP_STRING(
    [--enable-venus],
    [compile with Venus simulator]
  ),
  [ CFLAGS="$CFLAGS -DVENUS_ENABLED",
    venus_enabled="yes" ]
)

AM_CONDITIONAL(VENUS_ENABLED, test "x$venus_enabled" = "xyes")


dnl =======================================================================
dnl Additional checks
dnl =======================================================================

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h limits.h malloc.h strings.h sys/time.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

dnl Checks for library functions.
AC_FUNC_MMAP
AC_FUNC_SETVBUF_REVERSED
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(gethostname gettimeofday strdup strerror strstr strtoul uname)

AC_CHECK_LIB(z, [gzopen,gzputs],[
    AC_DEFINE(HAVE_LIBZ)
      if test "x${Z_DIR}" != "x"; then
        Z_CFLAGS="-I${Z_DIR}/include"
        Z_LIBS="-L${Z_DIR}/lib -lz"
      else
        Z_LIBS="-lz"
      fi])

AC_SUBST(Z_CFLAGS)
AC_SUBST(Z_LIBS)

dnl =======================================================================
dnl Final Makefile generation
dnl =======================================================================

AC_OUTPUT([
  Makefile
  include/Makefile
  src/Makefile
])
dnl	src/Makefile \
dnl	src/binfmt/Makefile \
dnl	src/input/Makefile \
dnl	src/license/Makefile \
dnl	src/model/Makefile \
dnl	src/prv_utils/Makefile \
dnl	src/sched/Makefile \
dnl	src/utils/Makefile \
dnl	src/vampir/Makefile \
dnl     ])
