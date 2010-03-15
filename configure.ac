dnl Process this file with autoconf to produce a configure script.

SVN_REVISION="$Rev$"

AC_INIT(dimemas, 3.00, tools@bsc.es)
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE
AC_PREFIX_DEFAULT(${DIMEMASDIR:-/usr/local})

AC_SUBST(PACKAGE)
AC_SUBST(VERSION)


dnl Check if CFLAGS/CXXFLAGS have been defined by user
AC_MSG_CHECKING([whether configure should try to set CFLAGS])
if test "x$CFLAGS" = "x"; then
  enable_cflags_setting=yes
else
  enable_cflags_setting=no
fi
AC_MSG_RESULT($enable_cflags_setting)

AC_MSG_CHECKING([whether configure should try to set CXXFLAGS])
if test "x$CXXFLAGS" = "x"; then
  enable_cxxflags_setting=yes
else
  enable_cxxflags_setting=no
fi
AC_MSG_RESULT($enable_cxxflags_setting)


AC_PROG_CC(icc xlc cc gcc)
AC_PROG_CXX(icpc xlC CC g++)
AC_PROG_CPP

AM_PROG_CC_C_O

SVNREV=`svnversion $srcdir | sed 's/:.*//'`
which svnversion > /dev/null 
if test $? -ne 0  -o  "x$SVNREV" = "xexported";
  then SVNREV=`cat $srcdir/SVNREV`
  else echo -n $SVNREV>$srcdir/SVNREV
fi
AC_SUBST(SVNREV)

AM_CONFIG_HEADER(config.h)

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

LIBS="$LIBS $LEXLIB"

dnl If the package is not flat
AC_PROG_MAKE_SET

AC_DEFINE_UNQUOTED(DATE, ["`date`"], [Current date of compilation] )

dnl =========================================================================
dnl Set default xxxFLAGS if not defined
dnl =========================================================================

if test "x$enable_cflags_setting" = "xyes"; then
  CFLAGS="-O3 -Os"
fi

if test "x$enable_cxxflags_setting" = "xyes"; then
  CXXFLAGS="-O3 -Os"
fi

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
dnl Check whether the compilers need additional parameters
dnl =========================================================================

AC_MSG_CHECKING([for additional options in compilers C/C++])
if test "x$enable_cflags_setting" = "yes"; then
  if test "$ac_compiler_gnu" = "yes" ; then
    ADDITIONAL_FLAGS_CC=""
    ADDITIONAL_FLAGS_CXX=""
  elif test "$CC" = "xlc" ; then
    ADDITIONAL_FLAGS_CC="-qcpluscmt"
    ADDITIONAL_FLAGS_CXX="-qstaticinline"
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
fi

dnl Do we have inline?
AC_C_INLINE

AM_CONDITIONAL(ARCH_IA32,    test "${Architecture}"    = "ia32"    )
AM_CONDITIONAL(ARCH_POWERPC, test "${Architecture}"    = "powerpc" )
AM_CONDITIONAL(ARCH_IA64,    test "${Architecture}"    = "ia64"    )
AM_CONDITIONAL(ARCH_ALPHA,   test "${Architecture}"    = "alpha"   )
AM_CONDITIONAL(ARCH_MIPS,    test "${Architecture}"    = "mips"    )

AM_CONDITIONAL(OS_LINUX,     test "${OperatingSystem}" = "linux"   )
AM_CONDITIONAL(OS_AIX,       test "${OperatingSystem}" = "aix"     )
AM_CONDITIONAL(OS_DEC,       test "${OperatingSystem}" = "dec"     )
AM_CONDITIONAL(OS_IRIX,      test "${OperatingSystem}" = "irix"    )
AM_CONDITIONAL(OS_FREEBSD,   test "${OperatingSystem}" = "freebsd" )
AM_CONDITIONAL(OS_SOLARIS,   test "${OperatingSystem}" = "solaris" )
AM_CONDITIONAL(OS_MACOSX,    test "${OperatingSystem}" = "osx"     )
AM_CONDITIONAL(OS_CYGWIN,    test "${OperatingSystem}" = "cygwin"  )

dnl =======================================================================
dnl Conditional defines
dnl =======================================================================


dnl STARTUP_ALS_IRECV: if defined Dimemas considers initial startups on 
dnl MPI_Irecv functions
AC_ARG_ENABLE(
  irecv-startup, 
  AS_HELP_STRING(
    [--enable-irecv-startup],
    [version with startup time during IRECV primitives]
  ),
  AC_DEFINE(STARTUP_ALS_IRECV,
            [1],
            [Dimemas will consider intial startup times on MPI_ircv functions])
)

dnl LOGICAL_RECEIVE_ALS_WAIT: if defined Dimemas puts logical receive time
dnl when MPI_Wait is called (not on MPI_Irecv)
AC_ARG_ENABLE(
  wait-logical-recv,
  AS_HELP_STRING(
    [--enable-wait-logical-recv],
    [version with logical recv time on WAIT primitives]
  ),
  AC_DEFINE(LOGICAL_RECEIVE_ALS_WAIT,
            [1],
            [Paraver 'logical recv' events will we located when MPI_wait is
            called])
)

dnl ENABLE_LARGE_TRACES: if defined Dimemas can use traces larger than 2Gb
AC_ARG_ENABLE(
  large-traces,
  AS_HELP_STRING(
    [--enable-large-traces],
    [version with support for traces larger than 2GiB]
  ),
  if test "$Operating_System" = "cygwin"; then
    CFLAGS="$CFLAGS -D__LARGE64_FILES"
    CXXFLAGS="$CXXFLAGS -D__LARGE64_FILES"
  elif test "$Operating_System" = "linux" ; then
    CFLAGS="$CFLAGS -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE"
    CXXFLAGS="$CXXFLAGS -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE"
  fi

  if test "$Operating_System" != "cygwin"; then
    AC_DEFINE(ENABLE_LARGE_TRACES, [1], [Support for 64 bit file access])
  fi
)

dnl READ_TRACE_SEEKS: if defined Dimemas can use lseek shortcuts from traces
AC_ARG_ENABLE(
  read-trace-seeks,
  AS_HELP_STRING(
    [--enable-read-trace-seeks],
    [version with on-trace seek read support]
  ),
  AC_DEFINE(READ_TRACE_SEEKS, [1], [Read trace lseek information])
)

dnl WRITE_TRACE_SEEKS: if defined Dimemas adds lseek information on traces
AC_ARG_ENABLE(
  write-trace-seeks,
  AS_HELP_STRING(
    [--enable-write-trace-seeks],
    [version with on-trace seek write support]
  ),
  write_trace_seeks_enable=yes
  AC_DEFINE(WRITE_TRACE_SEEKS, [1], [Add trace lseek information])
)

dnl GESTIO_LINKS_ORIGINAL: if defined Dimemas uses the original protocol to
dnl manage links
AC_ARG_ENABLE(
  original-links,
  AS_HELP_STRING(
    [--enable-original-links],
    [version with original link management]
  ),
  AC_DEFINE(GESTIO_LINKS_ORIGINAL, [1], [Use first version of link managing protocol])
)

dnl TIME_LIMIT: this variable is used on 'define.h' and indicates the maximum
dnl clock value on simulations. By default, it assumes 7h (2.52e10 us) of 
dnl maximum clock time
AC_ARG_WITH(
  time_limit,
  AS_HELP_STRING(
    [--with-time-limit=TIME-LIMIT], 
    [give the simulation time limit in microseconds (scientific notation can  
     be used)]
  ),
  time_limit="$withval",
  time_limit="2.52e10"
)

AC_DEFINE_UNQUOTED(TIME_LIMIT,
                   $time_limit,
                   [Maximum simulation time in microseconds])


dnl COMMON_FILES_DIR: point the compiling system where the tools 'common-files'
dnl directory is located

AC_ARG_WITH(
  [common-files-dir],
  AS_HELP_STRING(
    [--with-common-files-dir=LIB_DIR],
    [force given directory as BSC Performance Tools common-files location]
  ),
  [
  if test -d $withval; then
    common_filesdir="$withval"
  else
    AC_MSG_ERROR(--with-common-files-dir expected directory name)
  fi
  ],
  [
    if test -d ../common-files; then
      common_filesdir="`pwd`/../common-files"
    else
      AC_MSG_ERROR(
        Unable to find 'common-files' directory. Please point it using 
        '--with-common-files-dir' option or copying it to the parent directory)
    fi
  ]
)
AC_SUBST(common_filesdir)

dnl NO_COMPILATION_INFO: if defined Dimemas doesn't show compilation 
dnl information on help message
AC_ARG_ENABLE(
  no-compilation-info,
  AS_HELP_STRING(
    [--enable-no-compilation-info],
    [compile without compilation information]
  ),
  AC_DEFINE(NO_COMPILATION_INFO,
            [1],
            [Dimemas won't show compilation information])
)

dnl IDLE_ACCOUNTING: if defined, idle CPU regions are computed as CPU
dnl consumption in the accouting stats
AC_ARG_ENABLE(
  idle-accounting,
  AC_HELP_STRING(
    [--enable-idle-accounting],
    [compile with idle blocks accouting]
  ),
  AC_DEFINE(IDLE_ACCOUNTING,
            [1],
            [Idle CPU regions are computed as CPU consumption])
)

dnl VENUS_ENABLED: if defined, venus client are compiled and included in Dimemas
AC_ARG_ENABLE(
  venus,
  AC_HELP_STRING(
    [--enable-venus],
    [compile with Venus simulator]
  ),
  [ CFLAGS="$CFLAGS -DVENUS_ENABLED"
    CXXFLAGS="$CXXFLAGS -DVENUS_ENABLED"
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

AC_CHECK_LIB(z, [gzopen,gzputs],[
    AC_DEFINE(HAVE_LIBZ,[1],[libz present on the system])
      if test "x${Z_DIR}" != "x"; then
        Z_CFLAGS="-I${Z_DIR}/include"
        Z_LIBS="-L${Z_DIR}/lib -lz"
      else
        Z_LIBS="-lz"
      fi])

CFLAGS="$CFLAGS $Z_FLAGS"
CXXFLAGS="$CXXFLAGS $Z_FLAGS"
LIBS="$LIBS $Z_LIBS"


dnl =======================================================================
dnl Java checks
dnl =======================================================================
AC_CHECK_CLASSPATH
AX_PROG_JAVAC
AX_PROG_JAVA
AX_PROG_JAR

have_java=yes 
if test "x$ac_cv_prog_java_works" = "x"; then
  echo "NOT JAVA"
  have_java=no;
fi

if test "x$ac_cv_prog_javac_works" = "x"; then
  echo "NOT JAVAC"
  have_java=no;
fi 

if test "x$ac_cv_prog_jar_exists" = "x"; then
  echo "NOT JAR"
  have_java=no;
fi 

if test "x$have_java" = "xno"; then
  echo "------------------------------------------------------------"
  echo " Some Java tools not found - continuing without GUI support"
  echo "------------------------------------------------------------"
fi

AM_CONDITIONAL([HAVE_JAVA], [test "x$have_java" = xyes])

dnl if test "x$have_java" = xyes; then
dnl  AC_DEFINE(HAVE_JAVA, 1, Define to 1 if JAVA is installed in the system)
dnl fi

dnl =======================================================================
dnl Mandatory FLAGS
dnl =======================================================================

CFLAGS="$CFLAGS -DDIMEMAS_COMPILATION"
CXXFLAGS="$CXXFLAGS -DDIMEAMS_COMPILATION"
LDFLAGS="$LDFLAGS"

dnl =======================================================================
dnl Final Makefile generation
dnl =======================================================================

AC_CONFIG_FILES([
  Makefile
  DimemasGUI/Makefile
  include/Makefile
  src/Makefile
])

AC_OUTPUT

dnl =======================================================================
dnl Compiling options resume
dnl =======================================================================
echo "
  $PACKAGE_NAME version $PACKAGE_VERSION.$SVNREV

  Prefix.................: $prefix
  Debug Build............: $debug
  C Compiler Flags.......: $CFLAGS
  C++ Compiler Flags.....: $CXXFLAGS $CPPFLAGS
  Linker flags...........: $LDFLAGS $LIBS
  Java VM................: ${JAVA:-NONE}
  Java Compiler..........: ${JAVAC:-NONE} $JAVACFLAGS
  Jar Utility............: ${JAR:-NONE} $JARFLAGS 
  Common files directory.: ${common_filesdir:-default}
"

