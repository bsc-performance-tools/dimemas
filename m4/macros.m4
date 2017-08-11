# AX_FLAGS_SAVE
# -------------
AC_DEFUN([AX_FLAGS_SAVE],
[
   saved_LIBS="${LIBS}"
   saved_CC="${CC}"
   saved_CFLAGS="${CFLAGS}"
   saved_CXXFLAGS="${CXXFLAGS}"
   saved_CPPFLAGS="${CPPFLAGS}"
   saved_LDFLAGS="${LDFLAGS}"
])


# AX_FLAGS_RESTORE
# ----------------
AC_DEFUN([AX_FLAGS_RESTORE],
[
   LIBS="${saved_LIBS}"
   CC="${saved_CC}"
   CFLAGS="${saved_CFLAGS}"
   CXXFLAGS="${saved_CXXFLAGS}"
   CPPFLAGS="${saved_CPPFLAGS}"
   LDFLAGS="${saved_LDFLAGS}"
])


# AX_FIND_INSTALLATION
# --------------------
AC_DEFUN([AX_FIND_INSTALLATION],
[
	AC_REQUIRE([AX_SELECT_BINARY_TYPE])

	dnl Search for home directory
	AC_MSG_CHECKING([for $1 installation])
    for home_dir in [$2 "not found"]; do
        if test -d "$home_dir/$BITS" ; then
            home_dir="$home_dir/$BITS"
            break
        elif test -d "$home_dir" ; then
            break
        fi
    done
	AC_MSG_RESULT([$home_dir])
	$1_HOME="$home_dir"
	if test "$$1_HOME" = "not found" ; then
		$1_HOME=""
	else
		dnl Search for includes directory
		AC_MSG_CHECKING([for $1 includes directory])
		for incs_dir in [$$1_HOME/include$BITS $$1_HOME/include "not found"] ; do
			if test -d "$incs_dir" ; then
				break
			fi
		done
		AC_MSG_RESULT([$incs_dir])
		$1_INCLUDES="$incs_dir"
		if test "$$1_INCLUDES" = "not found" ; then
       $1_INCLUDES=""
       $1_CFLAGS=""
       $1_CXXFLAGS=""
       $1_CPPFLAGS=""
		else
       $1_CFLAGS="-I$$1_INCLUDES"
       $1_CXXFLAGS="-I$$1_INCLUDES"
       $1_CPPFLAGS="-I$$1_INCLUDES"
		fi

		dnl Search for libs directory
		AC_MSG_CHECKING([for $1 libraries directory])
		for libs_dir in [$$1_HOME/lib$BITS $$1_HOME/lib "not found"] ; do
			if test -d "$libs_dir" ; then
				break
			fi
		done
		AC_MSG_RESULT([$libs_dir])
		$1_LIBSDIR="$libs_dir"
		if test "$$1_LIBSDIR" = "not found" ; then
       $1_LIBSDIR=""
       $1_LDFLAGS=""
       $1_SHAREDLIBSDIR=""
		else
       $1_LDFLAGS="-L$$1_LIBSDIR"
       if test -d "$$1_LIBSDIR/shared" ; then
          $1_SHAREDLIBSDIR="$$1_LIBSDIR/shared"
       else
          $1_SHAREDLIBSDIR=$$1_LIBSDIR
       fi
		fi
	fi

	dnl Everything went OK?
	if test "$$1_HOME" != "" -a "$$1_INCLUDES" != "" -a "$$1_LIBSDIR" != "" ; then
		$1_INSTALLED="yes"

		AC_SUBST($1_HOME)
		AC_SUBST($1_INCLUDES)

    AC_SUBST($1_CFLAGS)
    AC_SUBST($1_CXXFLAGS)
    AC_SUBST($1_CPPFLAGS)

    AC_SUBST($1_LDFLAGS)
    AC_SUBST($1_SHAREDLIBSDIR)
    AC_SUBST($1_LIBSDIR)

    dnl Update the default variables so the automatic checks will take into account the new directories
    CFLAGS="$CFLAGS $$1_CFLAGS"
    CXXFLAGS="$CXXFLAGS $$1_CXXFLAGS"
    CPPFLAGS="$CPPFLAGS $$1_CPPFLAGS"
    LDFLAGS="$LDFLAGS $$1_LDFLAGS"
	else	
		$1_INSTALLED="no"
	fi
])


# AX_CHECK_POINTER_SIZE
# ---------------------
AC_DEFUN([AX_CHECK_POINTER_SIZE],
[
   AC_TRY_RUN(
      [
         int main()
         {
            return sizeof(void *)*8;
         }
      ],
      [ POINTER_SIZE="0" ],
      [ POINTER_SIZE="$?"]
   )
])


# AX_SELECT_BINARY_TYPE
# ---------------------
# Check the binary type the user wants to build and verify whether it can be successfully built
AC_DEFUN([AX_SELECT_BINARY_TYPE],
[
	AC_ARG_WITH(binary-type,
		AC_HELP_STRING(
			[--with-binary-type@<:@=ARG@:>@],
			[choose the binary type between: 32, 64, default @<:@default=default@:>@]
		),
		[Selected_Binary_Type="$withval"],
		[Selected_Binary_Type="default"]
	)

	if test "$Selected_Binary_Type" != "default" -a "$Selected_Binary_Type" != "32" -a "$Selected_Binary_Type" != "64" ; then
		AC_MSG_ERROR([--with-binary-type: Invalid argument '$Selected_Binary_Type'. Valid options are: 32, 64, default.])
	fi

	C_compiler="$CC"
	CXX_compiler="$CXX"

	AC_LANG_SAVE([])
	m4_foreach([language], [[C], [C++]], [
		AC_LANG_PUSH(language)

		AC_CACHE_CHECK(
			[for $_AC_LANG_PREFIX[]_compiler compiler default binary type], 
			[[]_AC_LANG_PREFIX[]_ac_cv_compiler_default_binary_type],
			[
				AX_CHECK_POINTER_SIZE
				Default_Binary_Type="$POINTER_SIZE"
				[]_AC_LANG_PREFIX[]_ac_cv_compiler_default_binary_type="$Default_Binary_Type""-bit"
			]
		)

		if test "$Default_Binary_Type" != "32" -a "$Default_Binary_Type" != 64 ; then
			AC_MSG_ERROR([Unknown default binary type (pointer size is $POINTER_SIZE!?)])
		fi

		if test "$Selected_Binary_Type" = "default" ; then
			Selected_Binary_Type="$Default_Binary_Type"
		fi

		if test "$Selected_Binary_Type" != "$Default_Binary_Type" ; then

			force_bit_flags="-m32 -q32 -32 -m64 -q64 -64 -maix64 none"

			AC_MSG_CHECKING([for $_AC_LANG_PREFIX[]_compiler compiler flags to build a $Selected_Binary_Type-bit binary])
			for flag in [$force_bit_flags]; do
				old_[]_AC_LANG_PREFIX[]FLAGS="$[]_AC_LANG_PREFIX[]FLAGS"
				[]_AC_LANG_PREFIX[]FLAGS="$[]_AC_LANG_PREFIX[]FLAGS $flag"

				AX_CHECK_POINTER_SIZE()
				if test "$POINTER_SIZE" = "$Selected_Binary_Type" ; then
					BINARY_TYPE_FLAGS="$flag"
					AC_MSG_RESULT([$flag])
					break
				else
					[]_AC_LANG_PREFIX[]FLAGS="$old_[]_AC_LANG_PREFIX[]FLAGS"
					if test "$flag" = "none" ; then
						AC_MSG_RESULT([unknown])
						AC_MSG_NOTICE([$Selected_Binary_Type-bit binaries not supported])
						AC_MSG_ERROR([Please use '--with-binary-type' to select an appropriate binary type.])

					fi
				fi
			done
		fi
		AC_LANG_POP(language)
	])
	AC_LANG_RESTORE([])
	BITS="$Selected_Binary_Type"
])


# AX_CHECK_ENDIANNESS
# -------------------
# Test if the architecture is little or big endian
AC_DEFUN([AX_CHECK_ENDIANNESS],
[
   AC_CACHE_CHECK([for the architecture endianness], [ac_cv_endianness],
   [
      AC_LANG_SAVE()
      AC_LANG([C])
      AC_TRY_RUN(
      [
         int main()
         {
            short s = 1;
            short * ptr = &s;
            unsigned char c = *((char *)ptr);
            return c;
         }
      ],
      [ac_cv_endianness="big endian" ],
      [ac_cv_endianness="little endian" ]
      )
      AC_LANG_RESTORE()
   ])
   if test "$ac_cv_endianness" = "big endian" ; then
      AC_DEFINE(IS_BIG_ENDIAN, 1, [Define to 1 if architecture is big endian])
   fi
   if test "$ac_cv_endianness" = "little endian" ; then
      AC_DEFINE(IS_LITTLE_ENDIAN, 1, [Define to 1 if architecture is little endian])
   fi
])


# AX_CHECK__FUNCTION__MACRO
# -------------------------
# Check whether the compiler defines the __FUNCTION__ macro
AC_DEFUN([AX_CHECK__FUNCTION__MACRO],
[
   AC_CACHE_CHECK([whether the compiler defines the __FUNCTION__ macro], [ac_cv_have__function__],
      [
         AC_LANG_SAVE()
         AC_LANG([C])
         AC_TRY_COMPILE(
            [#include <stdio.h>],
            [
               char *s = __FUNCTION__;
               return 0;
            ],
            [ac_cv_have__function__="yes"],
            [ac_cv_have__function__="no"]
         )
         AC_LANG_RESTORE()
      ]
   )
   if test "$ac_cv_have__function__" = "yes" ; then
      AC_DEFINE([HAVE__FUNCTION__], 1, [Define to 1 if __FUNCTION__ macro is supported])
   fi
])

# AX_PROG_XML2
# -----------
AC_DEFUN([AX_PROG_XML2],
[
   XML2_HOME_BIN="`dirname ${XML2_CONFIG}`"
   XML2_HOME="`dirname ${XML2_HOME_BIN}`"

   XML2_INCLUDES1="${XML2_HOME}/include/libxml2"
   XML2_INCLUDES2="${XML2_HOME}/include"
   XML2_CFLAGS="-I${XML2_INCLUDES1} -I${XML2_INCLUDES2}"
   XML2_CPPFLAGS=${XML2_CFLAGS}
   XML2_CXXFLAGS=${XML2_CFLAGS}

   XML2_LIBS="-lxml2"
   if test -f ${XML2_HOME}/lib${BITS}/libxml2.so -o -f ${XML2_HOME}/lib${BITS}/libxml2.a ; then
      XML2_LIBSDIR="${XML2_HOME}/lib${BITS}"
   else
      XML2_LIBSDIR="${XML2_HOME}/lib"
   fi
   XML2_LDFLAGS="-L${XML2_LIBSDIR}"

   if test -d ${XML2_LIBSDIR}/shared ; then 
      XML2_SHAREDLIBSDIR="${XML2_LIBSDIR}/shared"
   else
      XML2_SHAREDLIBSDIR=${XML2_LIBSDIR}
   fi

   XML_LIBS="${XML2_LDFLAGS} -lxml2 -lz -lpthread -lm"

   AC_SUBST(XML2_HOME)
   AC_SUBST(XML2_CFLAGS)
   AC_SUBST(XML2_CPPFLAGS)
   AC_SUBST(XML2_CXXFLAGS)
   AC_SUBST(XML2_INCLUDES)
   AC_SUBST(XML2_LIBSDIR)
   AC_SUBST(XML2_SHAREDLIBSDIR)
   AC_SUBST(XML2_LIBS)
   AC_SUBST(XML2_LDFLAGS)
])

# AX_PROG_BFD
# -----------
AC_DEFUN([AX_PROG_BFD],
[
   if test "${IS_BGL_MACHINE}" = "yes" ; then
      bfd_default_paths="${BGL_HOME}/blrts-gnu"
      libiberty_default_paths="${BGL_HOME}/blrts-gnu"
   else
      bfd_default_paths="/usr /usr/local"
      libiberty_default_paths="/usr /usr/local"
   fi

   AC_MSG_CHECKING([for libbfd])
   AC_ARG_WITH(bfd,
      AC_HELP_STRING(
         [--with-bfd@<:@=DIR@:>@],
         [specify where to find BFD libraries and includes]
      ),
      [bfd_paths="${withval}"],
      [bfd_paths="${bfd_default_paths}"]
   )
   for bfd_home_dir in [${bfd_paths} "not found"]; do
      if test -r "${bfd_home_dir}/lib${BITS}/libbfd.so" ; then
         BFD_LIBSDIR="${bfd_home_dir}/lib${BITS}"
         break
      elif test -r "${bfd_home_dir}/lib${BITS}/libbfd.a" ; then
         BFD_LIBSDIR="${bfd_home_dir}/lib${BITS}"
         break
      elif test -r "${bfd_home_dir}/lib/libbfd.so" ; then
         BFD_LIBSDIR="${bfd_home_dir}/lib"
         break
      elif test -r "${bfd_home_dir}/lib/libbfd.a" ; then
         BFD_LIBSDIR="${bfd_home_dir}/lib"
         break
      fi
   done
   AC_MSG_RESULT(${bfd_home_dir})

   AX_FLAGS_SAVE()
   CFLAGS="-I${bfd_home_dir}/include"
   AC_CHECK_HEADERS([bfd.h], [], [BFD_INSTALLED="no"])
   AX_FLAGS_RESTORE()

   if test "${bfd_home_dir}" != "not found" -a "${BFD_INSTALLED}" != "no" ; then

      AC_MSG_CHECKING([for libiberty])
      AC_ARG_WITH(liberty,
         AC_HELP_STRING(
            [--with-liberty@<:@=DIR@:>@],
            [specify where to find LIBERTY libraries and includes]
         ),
         [liberty_paths="${withval}"],
         [liberty_paths="${libiberty_default_paths}"]
      )
      for liberty_home_dir in [${liberty_paths} "not found"]; do
         if test -r "${liberty_home_dir}/lib${BITS}/libiberty.so" ; then
            LIBERTY_LIBSDIR="${liberty_home_dir}/lib${BITS}"
            break
         elif test -r "${liberty_home_dir}/lib${BITS}/libiberty.a" ; then
            LIBERTY_LIBSDIR="${liberty_home_dir}/lib${BITS}"
            break
         elif test -r "${liberty_home_dir}/lib/libiberty.so" ; then
            LIBERTY_LIBSDIR="${liberty_home_dir}/lib"
            break
         elif test -r "${liberty_home_dir}/lib/libiberty.a" ; then
            LIBERTY_LIBSDIR="${liberty_home_dir}/lib"
            break
         fi
      done
      AC_MSG_RESULT(${liberty_home_dir})
      
      dnl Check if they work
      
      if test "${liberty_home_dir}" != "not found" ; then
         AC_MSG_CHECKING([if libbfd and libiberty works])
	 
         AX_FLAGS_SAVE()
         LIBS="-L${bfd_home_dir}/lib -lbfd -L${liberty_home_dir}/lib -liberty"
         CFLAGS="-I${bfd_home_dir}/include"
         AC_TRY_LINK(
            [ #include <bfd.h> ], 
            [ bfd *abfd = bfd_openr ("", ""); ],
            [ bfd_liberty_works="yes" ]
         )

         if test "${bfd_liberty_works}" != "yes" ; then
            dnl On some machines BFD/LIBERTY need an special symbol (e.g BGL)
            AC_TRY_LINK(
               [ #include <bfd.h> 
                 int *__errno_location(void) { return 0; }
               ], 
               [ bfd *abfd = bfd_openr ("", ""); ],
               [ bfd_liberty_works="yes" ]
            )
            if test "${bfd_liberty_works}" = "yes" ; then
               AC_DEFINE([NEED_ERRNO_LOCATION_PATCH], 1, [Define to 1 if system requires __errno_location and does not provide it])
            fi
         fi

         if test "${bfd_liberty_works}" = "yes" ; then
            AC_MSG_RESULT([yes])

            BFD_HOME="${bfd_home_dir}"
            BFD_INCLUDES="${BFD_HOME}/include"
            BFD_CFLAGS="-I${BFD_INCLUDES}"
            BFD_CXXFLAGS=${BFD_CFLAGS}
            BFD_CPPFLAGS=${BFD_CFLAGS}
            BFD_LIBS="-lbfd"
            BFD_LDFLAGS="-L${BFD_LIBSDIR}"
            AC_SUBST(BFD_HOME)
            AC_SUBST(BFD_INCLUDES)
            AC_SUBST(BFD_CFLAGS)
            AC_SUBST(BFD_CXXFLAGS)
            AC_SUBST(BFD_CPPFLAGS)
            AC_SUBST(BFD_LIBS)
            AC_SUBST(BFD_LIBSDIR)
            if test -d ${BFD_LIBSDIR}/shared ; then
               BFD_SHAREDLIBSDIR="${BFD_LIBSDIR}/shared"
            else
               BFD_SHAREDLIBSDIR=${BFD_LIBSDIR}
            fi
            AC_SUBST(BFD_SHAREDLIBSDIR)
            AC_SUBST(BFD_LDFLAGS)

            LIBERTY_HOME="${liberty_home_dir}"
            LIBERTY_INCLUDES="${LIBERTY_HOME}/include"
            LIBERTY_CFLAGS="-I${LIBERTY_INCLUDES}"
            LIBERTY_CXXFLAGS=${LIBERTY_CFLAGS}
            LIBERTY_CPPFLAGS=${LIBERTY_CFLAGS}
            LIBERTY_LIBS="-liberty"
            LIBERTY_LDFLAGS="-L${LIBERTY_LIBSDIR}"
            AC_SUBST(LIBERTY_HOME)
            AC_SUBST(LIBERTY_INCLUDES)
            AC_SUBST(LIBERTY_CFLAGS)
            AC_SUBST(LIBERTY_CXXFLAGS)
            AC_SUBST(LIBERTY_CPPFLAGS)
            AC_SUBST(LIBERTY_LIBS)
            AC_SUBST(LIBERTY_LIBSDIR)
            if test -d ${LIBERTY_LIBSDIR}/shared ; then
               BFD_SHAREDLIBSDIR="${LIBERTY_LIBSDIR}/shared"
            else
               BFD_SHAREDLIBSDIR=${LIBERTY_LIBSDIR}
            fi
            AC_SUBST(LIBERTY_SHAREDLIBSDIR)
            AC_SUBST(LIBERTY_LDFLAGS)

            AC_DEFINE([HAVE_BFD], 1, [Define to 1 if BFD is installed in the system])

            AC_MSG_CHECKING([whether bfd_get_section_size is defined in bfd.h])
            AC_TRY_LINK(
              [ #include <bfd.h> ],
              [ 
                  asection *section;
                  int result = bfd_get_section_size(section); 
              ],
              [ bfd_get_section_size_found="yes"]
            )
            if test "${bfd_get_section_size_found}" = "yes" ; then
               AC_DEFINE(HAVE_BFD_GET_SECTION_SIZE, [], [Defined to 1 if bfd.h defines bfd_get_section_size])
               AC_MSG_RESULT([yes])
            else
               AC_MSG_RESULT([no])
            fi

            AC_MSG_CHECKING([whether bfd_get_section_size_before_reloc is defined in bfd.h])
            AC_TRY_LINK(
              [ #include <bfd.h> ],
              [ 
                  asection *section;
                  int result = bfd_get_section_size_before_reloc(section); 
              ],
              [ bfd_get_section_size_before_reloc_found="yes"]
            )
            if test "${bfd_get_section_size_before_reloc_found}" = "yes" ; then
               AC_DEFINE(HAVE_BFD_GET_SECTION_SIZE_BEFORE_RELOC, [], [Defined to 1 if bfd.h defines bfd_get_section_size_before_reloc])
               AC_MSG_RESULT([yes])
            else
               AC_MSG_RESULT([no])
            fi

         else
            AC_MSG_RESULT([no, see config.log for further details])
         fi
         AX_FLAGS_RESTORE()
      fi
   fi
])

# AX_PROG_MPI
# -----------
AC_DEFUN([AX_PROG_MPI],
[
   AX_FLAGS_SAVE()

   if test "${IS_BGL_MACHINE}" = "yes" ; then
      mpi_default_paths="${BGL_HOME}/bglsys"
   else
      mpi_default_paths="none"
   fi

   AC_ARG_WITH(mpi,
      AC_HELP_STRING(
         [--with-mpi@<:@=DIR@:>@],
         [specify where to find MPI libraries and includes]
      ),
      [mpi_paths=${withval}],
      [mpi_paths=${mpi_default_paths}] dnl List of possible default paths
   )

   dnl Search for MPI installation
   AX_FIND_INSTALLATION([MPI], [$mpi_paths])

   if test "${MPI_INSTALLED}" = "yes" ; then

      if test -d "$MPI_INCLUDES/mpi" ; then
         MPI_INCLUDES="$MPI_INCLUDES/mpi"
         MPI_CFLAGS="-I$MPI_INCLUDES"
         CFLAGS="$MPI_CFLAGS $CFLAGS"
      fi

      dnl Check for the MPI header files.
      AC_CHECK_HEADERS([mpi.h], [], [MPI_INSTALLED="no"])

      dnl Check for the MPI library.
      dnl We won't use neither AC_CHECK_LIB nor AC_TRY_LINK because this library may have unresolved references to other libs (i.e: libgm).
      AC_MSG_CHECKING([for MPI library])
      if test -f "${MPI_LIBSDIR}/libmpi.a" ; then
         MPI_LIBS="-lmpi"
      elif test -f "${MPI_LIBSDIR}/libmpi.so" ; then
         MPI_LIBS="-lmpi"
      elif test -f "${MPI_LIBSDIR}/libmpich.a" ; then
         MPI_LIBS="-lmpich"
      elif test -f "${MPI_LIBSDIR}/libmpich.so" ; then
         MPI_LIBS="-lmpich"
      else
         MPI_LIBS="not found"
      fi
      if test -f "${MPI_LIBSDIR}/libmpi.so"; then
         MPI_SHARED_LIB_FOUND="yes"
      elif test -f "${MPI_LIBSDIR}/libmpich.so"; then
         MPI_SHARED_LIB_FOUND="yes"
      elif test -f "${MPI_LIBSDIR}/shared/libmpi.so"; then
         MPI_SHARED_LIB_FOUND="yes"
      elif test -f "${MPI_LIBSDIR}/shared/libmpich.so"; then
         MPI_SHARED_LIB_FOUND="yes"
      fi
      AC_MSG_RESULT([${MPI_LIBSDIR}])

      if test "${MPI_LIBSDIR}" = "not found" ; then
         MPI_INSTALLED="no"
      else
         MPI_LDFLAGS="${MPI_LDFLAGS}"
         AC_SUBST(MPI_LDFLAGS)
         AC_SUBST(MPI_LIBS)
      fi

      dnl If $MPICC is not set, check for mpicc under $MPI_HOME/bin. We don't want to mix multiple MPI installations.
      AC_MSG_CHECKING([for MPI C compiler])
      if test "${MPICC}" = "" ; then
         mpicc_compilers="mpicc hcc mpxlc_r mpxlc mpcc cmpicc"
         for mpicc in [$mpicc_compilers]; do
            if test -f "${MPI_HOME}/bin/${mpicc}" ; then
               MPICC="${MPI_HOME}/bin/${mpicc}"
               AC_MSG_RESULT([${MPICC}])
               break
            fi
         done
         if test "${MPICC}" = "" ; then
            AC_MSG_RESULT([not found])
            AC_MSG_NOTICE([Cannot find \${MPI_HOME}/bin/mpicc -or similar- using \${CC} instead])
            MPICC_DOES_NOT_EXIST="yes"
            MPICC=${CC}
         else
            MPICC_DOES_NOT_EXIST="no"
         fi
      else
         AC_MSG_RESULT([${MPICC}])
      fi
   fi
   AC_SUBST(MPICC)

   # If the system do not have MPICC (or similar) be sure to add -lmpi and -Impi
   AM_CONDITIONAL(NEED_MPI_LIB_INCLUDE, test "${CC}" = "${MPICC}" )

   dnl Did the checks pass?
   AM_CONDITIONAL(HAVE_MPI, test "${MPI_INSTALLED}" = "yes")
   AC_DEFINE([HAVE_MPI], 1, [Determine if MPI in installed])

   if test "$MPI_INSTALLED" = "no" ; then
       AC_MSG_WARN([MPI tracing support has been disabled])
   fi

   AX_FLAGS_RESTORE()
])

# AX_CHECK_MPI_STATUS_SIZE
# ---------------------
AC_DEFUN([AX_CHECK_MPI_STATUS_SIZE],
[
   AC_MSG_CHECKING([for size of the MPI_Status struct])
   AX_FLAGS_SAVE()
   CFLAGS="${CFLAGS} -I${MPI_INCLUDES}"
   AC_TRY_RUN(
      [
         #include <mpi.h>
         int main()
         {
            return sizeof(MPI_Status)/sizeof(int);
         }
      ],
      [ SIZEOF_MPI_STATUS="0" ],
      [ SIZEOF_MPI_STATUS="$?"]
   )
   AC_MSG_RESULT([${SIZEOF_MPI_STATUS}])
   AC_DEFINE_UNQUOTED([SIZEOF_MPI_STATUS], ${SIZEOF_MPI_STATUS}, [Size of the MPI_Status structure in "sizeof-int" terms])
   AX_FLAGS_RESTORE()
])

# AX_CHECK_MPI_SOURCE_OFFSET
#------------------------
AC_DEFUN([AX_CHECK_MPI_SOURCE_OFFSET],
[
   AX_FLAGS_SAVE()
   CFLAGS="${CFLAGS} -I${MPI_INCLUDES}"

   AC_CHECK_MEMBER(MPI_Status.MPI_SOURCE,,
                [AC_MSG_ERROR([We need MPI_Status.MPI_SOURCE!])],
                [#include <mpi.h>])

   AC_MSG_CHECKING([for offset of SOURCE field in MPI_Status])
   AC_TRY_RUN(
      [
         #include <mpi.h>
         int main()
         {
            MPI_Status s;
            long addr1 = (long) &s;
            long addr2 = (long) &(s.MPI_SOURCE);

            return (addr2 - addr1)/sizeof(int);
         }
      ],
      [ MPI_SOURCE_OFFSET="0" ],
      [ MPI_SOURCE_OFFSET="$?"]
   )
   AC_MSG_RESULT([${MPI_SOURCE_OFFSET}])
   AC_DEFINE_UNQUOTED([MPI_SOURCE_OFFSET], ${MPI_SOURCE_OFFSET}, [Offset of the SOURCE field in MPI_Status in sizeof-int terms])
   AX_FLAGS_RESTORE()
])

# AX_CHECK_MPI_TAG_OFFSET
#------------------------
AC_DEFUN([AX_CHECK_MPI_TAG_OFFSET],
[
   AX_FLAGS_SAVE()
   CFLAGS="${CFLAGS} -I${MPI_INCLUDES}"

   AC_CHECK_MEMBER(MPI_Status.MPI_TAG,,
                [AC_MSG_ERROR([We need MPI_Status.MPI_TAG!])],
                [#include <mpi.h>])

   AC_MSG_CHECKING([for offset of TAG field in MPI_Status])
   AC_TRY_RUN(
      [
         #include <mpi.h>
         int main()
         {
            MPI_Status s;
            long addr1 = (long) &s;
            long addr2 = (long) &(s.MPI_TAG);

            return (addr2 - addr1)/sizeof(int);
         }
      ],
      [ MPI_TAG_OFFSET="0" ],
      [ MPI_TAG_OFFSET="$?"]
   )
   AC_MSG_RESULT([${MPI_TAG_OFFSET}])
   AC_DEFINE_UNQUOTED([MPI_TAG_OFFSET], ${MPI_TAG_OFFSET}, [Offset of the TAG field in MPI_Status in sizeof-int terms])
   AX_FLAGS_RESTORE()
])


# AX_CHECK_PERUSE
# ---------------------------
AC_DEFUN([AX_CHECK_PERUSE],
[
   AC_REQUIRE([AX_PROG_MPI])

   PERUSE_AVAILABILITY="no"
   AC_ARG_ENABLE(peruse,
      AC_HELP_STRING(
         [--enable-peruse],
         [Enable gathering information with PerUse]
      ),
      [enable_peruse="${enableval}"],
      [enable_peruse="auto"]
   )

   if test "${MPI_INSTALLED}" = "yes"; then
      if test "${enable_peruse}" = "auto" ; then
         AC_MSG_CHECKING(for peruse.h)
         if test -r ${MPI_INCLUDES}/peruse.h ; then
            AC_MSG_RESULT([available])
            enable_peruse="yes"
         else
            AC_MSG_RESULT([not available])
            enable_peruse="no"
         fi
      elif test "${enable_peruse}" = "yes" ; then
            AC_MSG_CHECKING(for peruse.h)
         if test -r ${MPI_INCLUDES}/peruse.h ; then
            AC_MSG_RESULT([available])
         else
            AC_MSG_NOTICE([Can not find the peruse header inside the MPI include directory.])
            AC_MSG_ERROR([Feature requested by the user but not available!])
         fi
      fi
   else
      enable_peruse="no"
   fi

   if test "${enable_peruse}" = "yes" ; then
      AC_MSG_CHECKING(for PERUSE_SUCCESS in peruse.h)
      AX_FLAGS_SAVE()
      CFLAGS="-I${MPI_INCLUDES}"
      AC_LANG_SAVE()
      AC_LANG([C])
      AC_TRY_COMPILE(
         [#include <peruse.h>],
         [
            int i = PERUSE_SUCCESS;
            return 0;
         ],
         [peruse_success="yes"],
         [peruse_success="no"]
      )
      AX_FLAGS_RESTORE()
      AC_LANG_RESTORE()

      if test "${peruse_success}" = "yes"; then
         AC_MSG_RESULT([available])
         AC_DEFINE([PERUSE_ENABLED], 1, [Determine if the PerUse API can be used])
         PERUSE_AVAILABILITY="yes"
      else
         AC_MSG_NOTICE([Can not find PERUSE_SUCCESS in peruse.h])
         AC_MSG_ERROR([Feature requested by the user but not available!])
      fi
   fi
])

# AX_CHECK_PMPI_NAME_MANGLING
# ---------------------------
AC_DEFUN([AX_CHECK_PMPI_NAME_MANGLING],
[
   AC_REQUIRE([AX_PROG_MPI])

   AC_ARG_WITH(name-mangling,
      AC_HELP_STRING(
         [--with-name-mangling@<:@=ARG@:>@], 
         [choose the name decoration scheme for external Fortran symbols from: 0u, 1u, 2u, upcase, auto @<:@default=auto@:>@]
      ),
      [name_mangling="$withval"],
      [name_mangling="auto"]
   )

   if test "$name_mangling" != "0u" -a "$name_mangling" != "1u" -a "$name_mangling" != "2u" -a "$name_mangling" != "upcase" -a "$name_mangling" != "auto" ; then
      AC_MSG_ERROR([--with-name-mangling: Invalid argument '$name_mangling'. Valid options are: 0u, 1u, 2u, upcase, auto.])
   fi

   AC_MSG_CHECKING(for Fortran PMPI symbols name decoration scheme)

   if test "$name_mangling" != "auto" ; then
      if test "$name_mangling" = "2u" ; then
         AC_DEFINE([PMPI_DOUBLE_UNDERSCORE], 1, [Defined if name decoration scheme is of type pmpi_routine__])
         FORTRAN_DECORATION="2 underscores"
      elif test "$name_mangling" = "1u" ; then
         AC_DEFINE([PMPI_SINGLE_UNDERSCORE], 1, [Defined if name decoration scheme is of type pmpi_routine_])
         FORTRAN_DECORATION="1 underscore"
      elif test "$name_mangling" = "upcase" ; then
         AC_DEFINE([PMPI_UPPERCASE], 1, [Defined if name decoration scheme is of type PMPI_ROUTINE])
         FORTRAN_DECORATION="UPPER CASE"
      elif test "$name_mangling" = "0u" ; then
         AC_DEFINE([PMPI_NO_UNDERSCORES], 1, [Defined if name decoration scheme is of type pmpi_routine])
         FORTRAN_DECORATION="0 underscores"
      fi
      AC_MSG_RESULT([${FORTRAN_DECORATION}])
   else

      AC_LANG_SAVE()
      AC_LANG([C])
      AX_FLAGS_SAVE()

      dnl If we've previously set MPICC to CC then we don't have MPICC
      dnl Add the default includes and libraries
      if test "${MPICC_DOES_NOT_EXIST}" = "yes" ; then
         CFLAGS="${MPI_CFLAGS}"
         LIBS="${MPI_LIBS}"
         LDFLAGS="${MPI_LDFLAGS}"
      fi

      CC="$MPICC"

      for ac_cv_name_mangling in \
         PMPI_DOUBLE_UNDERSCORE  \
         PMPI_SINGLE_UNDERSCORE  \
         PMPI_UPPERCASE          \
         PMPI_NO_UNDERSCORES ;
      do
         CFLAGS="-D$ac_cv_name_mangling"
   
         AC_TRY_LINK(
            [#include <mpi.h>], 
            [
               #if defined(PMPI_NO_UNDERSCORES)
               #define MY_ROUTINE pmpi_finalize
               #elif defined(PMPI_UPPERCASE)
               #define MY_ROUTINE PMPI_FINALIZE
               #elif defined(PMPI_SINGLE_UNDERSCORE)
               #define MY_ROUTINE pmpi_finalize_
               #elif defined(PMPI_DOUBLE_UNDERSCORE)
               #define MY_ROUTINE pmpi_finalize__
               #endif
   
               int ierror;
               MY_ROUTINE (&ierror);
            ],
            [
               break 
            ]
         )
      done

      AX_FLAGS_RESTORE()
      AC_LANG_RESTORE()

      if test "$ac_cv_name_mangling" = "PMPI_DOUBLE_UNDERSCORE" ; then
         AC_DEFINE([PMPI_DOUBLE_UNDERSCORE], 1, [Defined if name decoration scheme is of type pmpi_routine__])
         FORTRAN_DECORATION="2 underscores"
      elif test "$ac_cv_name_mangling" = "PMPI_SINGLE_UNDERSCORE" ; then
         AC_DEFINE([PMPI_SINGLE_UNDERSCORE], 1, [Defined if name decoration scheme is of type pmpi_routine_])
         FORTRAN_DECORATION="1 underscore"
      elif test "$ac_cv_name_mangling" = "PMPI_UPPERCASE" ; then
         AC_DEFINE([PMPI_UPPERCASE], 1, [Defined if name decoration scheme is of type PMPI_ROUTINE])
         FORTRAN_DECORATION="UPPER CASE"
      elif test "$ac_cv_name_mangling" = "PMPI_NO_UNDERSCORES" ; then
         AC_DEFINE([PMPI_NO_UNDERSCORES], 1, [Defined if name decoration scheme is of type pmpi_routine])
         FORTRAN_DECORATION="0 underscores"
      else
         FORTRAN_DECORATION="[unknown]"
         AC_MSG_NOTICE([Can not determine the name decoration scheme for external Fortran symbols])
         AC_MSG_ERROR([Please use '--with-name-mangling' to select an appropriate decoration scheme.])
      fi
      AC_MSG_RESULT([${FORTRAN_DECORATION}])
   fi
])


# AX_PROG_GM
# ----------
AC_DEFUN([AX_PROG_GM],
[
   AX_FLAGS_SAVE()

   AC_ARG_WITH(gm,
      AC_HELP_STRING(
         [--with-gm@<:@=DIR@:>@],
         [specify where to find GM libraries and includes]
      ),
      [gm_paths="$withval"],
      [gm_paths="/opt/osshpc/gm"] dnl List of possible default paths
   )

   dnl Search for GM installation
   AX_FIND_INSTALLATION([GM], [$gm_paths])

   if test "$GM_INSTALLED" = "yes" ; then
      dnl Check for GM header files.
      AC_CHECK_HEADERS([gm.h], [], [GM_INSTALLED="no"])

      dnl Check for libgm
      AC_CHECK_LIB([gm], [_gm_get_globals], 
         [ 
           GM_LDFLAGS="$GM_LDFLAGS -lgm"
           AC_SUBST(GM_LDFLAGS)
         ],
         [ GM_INSTALLED="no"]
      )
   fi

   dnl Did the checks pass?
   AM_CONDITIONAL(HAVE_GM, test "$GM_INSTALLED" = "yes")

   if test "$GM_INSTALLED" = "no" ; then
      AC_MSG_WARN([Myrinet GM counters tracing has been disabled])
   fi

   AX_FLAGS_RESTORE()
])


# AX_PROG_MX
# ----------
AC_DEFUN([AX_PROG_MX],
[
   AX_FLAGS_SAVE()

   AC_ARG_WITH(mx,
      AC_HELP_STRING(
         [--with-mx@<:@=DIR@:>@],
         [specify where to find MX libraries and includes]
      ),
      [mx_paths="$withval"],
      [mx_paths="/gpfs/apps/MX /opt/osshpc/mx"] dnl List of possible default paths
   )

   dnl Search for MX installation
   AX_FIND_INSTALLATION([MX], [$mx_paths])

   if test "$MX_INSTALLED" = "yes" ; then
      AC_CHECK_HEADERS([myriexpress.h], [], [MX_INSTALLED="no"])
      AC_CHECK_LIB([myriexpress], [mx_get_info], 
         [ 
           MX_LDFLAGS="$MX_LDFLAGS -lmyriexpress"
           AC_SUBST(MX_LDFLAGS)
         ], 
         [ MX_INSTALLED="no" ]
      )
      AC_CHECK_HEADERS([mx_dispersion.h], [mx_dispersion_h_found="yes"], [mx_dispersion_h_found="no"])
      AC_CHECK_LIB([myriexpress], [mx_get_dispersion_counters], 
         [mx_get_dispersion_counters_found="yes"], 
         [mx_get_dispersion_counters="no"]
      )
      if test "$mx_dispersion_h_found" = "yes" -a "$mx_get_dispersion_counters_found" = "yes" ; then
         MX_CFLAGS="$MX_CFLAGS -DMX_MARENOSTRUM_API"
         AC_SUBST(MX_CFLAGS)
         MX_CXXFLAGS="$MX_CFLAGS -DMX_MARENOSTRUM_API"
         AC_SUBST(MX_CXXFLAGS)
      fi
   fi

   dnl Did the checks pass?
   AM_CONDITIONAL(HAVE_MX, test "$MX_INSTALLED" = "yes")

   if test "$MX_INSTALLED" = "no" ; then
      AC_MSG_WARN([Myrinet MX counters tracing has been disabled])
   fi

   AX_FLAGS_RESTORE()
])


# AX_PROG_PAPI
# ------------
AC_DEFUN([AX_PROG_PAPI],
[
   AX_FLAGS_SAVE()

   papi_default_paths="none"

   AC_ARG_WITH(papi,
      AC_HELP_STRING(
         [--with-papi@<:@=DIR@:>@],
         [specify where to find PAPI libraries and includes]
      ),
      [papi_paths="${withval}"],
      [papi_paths=${papi_default_paths}] dnl List of possible default paths
   )
   AC_ARG_ENABLE(sampling,
      AC_HELP_STRING(
         [--enable-sampling],
         [Enable PAPI sampling support]
      ),
      [enable_sampling="${enableval}"],
      [enable_sampling="auto"]
   )
   PAPI_SAMPLING_ENABLED="no"

   dnl Search for PAPI installation
   AX_FIND_INSTALLATION([PAPI], [$papi_paths])

   if test "${PAPI_INSTALLED}" = "yes" ; then
      AC_CHECK_HEADERS([papi.h], [], [PAPI_INSTALLED="no"])

      if test "${IS_BGL_MACHINE}" = "yes" ; then
         LIBS="-static ${LIBS} -L${BGL_HOME}/bglsys/lib -lbgl_perfctr.rts -ldevices.rts -lrts.rts"
      else
         if test "${OperatingSystem}" = "freebsd" ; then
            LIBS="-lpapi -lpmc"
         elif test "${OperatingSystem}" = "linux" -a "${Architecture}" = "powerpc" ; then
            LIBS="-lpapi -lperfctr"
         else
            LIBS="-lpapi"
         fi
      fi

      AC_CHECK_LIB([papi], [PAPI_start],
         [ 
           if test "${IS_BGL_MACHINE}" = "yes" ; then
              PAPI_LIBS="-static -lpapi -L${BGL_HOME}/bglsys/lib -lbgl_perfctr.rts -ldevices.rts -lrts.rts"
           else
              if test "${OperatingSystem}" = "freebsd" ; then
                 PAPI_LIBS="-lpapi -lpmc"
              elif test "${OperatingSystem}" = "linux" -a "${Architecture}" = "powerpc" ; then
                 PAPI_LIBS="-lpapi -lperfctr"
              else
                 PAPI_LIBS="-lpapi"
              fi
           fi
           AC_SUBST(PAPI_LIBS)
         ],
         [PAPI_INSTALLED="no"]
      )
   fi

   AM_CONDITIONAL(HAVE_PAPI, test "${PAPI_INSTALLED}" = "yes")

   if test "${PAPI_INSTALLED}" = "no" ; then
      AC_MSG_WARN([PAPI counters tracing support has been disabled])
   else
      AC_DEFINE([NEW_HWC_SYSTEM], [1], [Enable HWC support])
      AC_DEFINE([USE_HARDWARE_COUNTERS], [1], [Enable HWC support])
      if test "${enable_sampling}" = "yes" ; then
         AC_CHECK_MEMBER([PAPI_substrate_info_t.supports_hw_overflow],[support_hw_overflow="yes"],[support_hw_overflow="no"],[#include <papi.h>])
         if test "${support_hw_overflow}" = "yes" ; then
            AC_DEFINE([HAVE_SUPPORT_HW_OVERFLOW], [1], [Use supports_hw_overflow field])
            AC_DEFINE([SAMPLING_SUPPORT], [1], [Enable PAPI sampling support])
            PAPI_SAMPLING_ENABLED="yes"
         else
            AC_CHECK_MEMBER([PAPI_substrate_info_t.hardware_intr_sig],[hardware_intr_sig="yes"],[hardware_intr_sig="no"],[#include <papi.h>])
            if test "${hardware_intr_sig}" = "yes" ; then
               AC_DEFINE([HAVE_HARDWARE_INTR_SIG], [1], [Use hardware_intr_sig field])
               AC_DEFINE([SAMPLING_SUPPORT], [1], [Enable PAPI sampling support])
               PAPI_SAMPLING_ENABLED="yes"
            else
               AC_MSG_ERROR([Cannot determine how to check whether PAPI supports HW overflows!])
            fi
         fi
      fi
   fi

   AX_FLAGS_RESTORE()
])

# AX_IS_ALTIX_MACHINE
# ----------------
AC_DEFUN([AX_IS_ALTIX_MACHINE],
[
   AC_MSG_CHECKING([if this is an Altix machine])
   if test -r /etc/sgi-release ; then 
      AC_MSG_RESULT([yes])
      IS_ALTIX_MACHINE="yes"
			AC_DEFINE([IS_ALTIX], 1, [Defined if this machine is a SGI Altix])
   else
      AC_MSG_RESULT([no])
      IS_ALTIX_MACHINE="no"
   fi
])


# AX_HAVE_MMTIMER_DEVICE
# ----------------
AC_DEFUN([AX_HAVE_MMTIMER_DEVICE],
[
   AC_REQUIRE([AX_IS_ALTIX_MACHINE])

   if test "${IS_ALTIX_MACHINE}" = "yes" ; then
      AC_MSG_CHECKING([if this is an Altix machine has MMTimer device])
      if test -r /dev/mmtimer ; then 
         AC_MSG_RESULT([yes])
         AC_DEFINE([HAVE_MMTIMER_DEVICE], 1, [Defined if this machine has a MMTimer device and it is readable])
         HAVE_MMTIMER_DEVICE="yes"
      else
         AC_MSG_RESULT([no])
         HAVE_MMTIMER_DEVICE="no"
      fi
   else
      HAVE_MMTIMER_DEVICE="no"
   fi
])

# AX_SHOW_CONFIGURATION
# --------------------
AC_DEFUN([AX_SHOW_CONFIGURATION],
[
   if test "${host}" != "${target}" ; then
      CROSSC="${host} to ${target}"
   else
      if test "${IS_BGL_MACHINE}" = "yes" ; then
         CROSSC="${host} with BG/L system support"
      elif test "${IS_CELL_MACHINE}" = "yes" ; then
         CROSSC="${host} with Cell Broadband Engine support - SDK ${CELL_SDK}.x"
      else
         CROSSC="no"
      fi
   fi

   echo ""
   echo "Package configuration :"
   echo "-----------------------"
   echo "Cross compilation  : ${CROSSC}"
   echo "CC                 : ${CC}"
   echo "CXX                : ${CXX}"
   echo "Binary type        : ${BITS} bits"
   echo "Fortran decoration : ${FORTRAN_DECORATION}"
   echo "MPI                : ${MPI_HOME} - enabled? ${MPI_INSTALLED} - peruse? ${PERUSE_AVAILABILITY}"
   echo "pThread            : ${enable_pthread}"
   echo "PAPI               : ${PAPI_HOME} - sampling? ${PAPI_SAMPLING_ENABLED}"
   echo "BFD                : ${BFD_HOME}"
   echo "liberty            : ${LIBERTY_HOME}"
   echo "XML2 config        : ${XML2_CONFIG}"
   echo "DynInst            : ${DYNINST_HOME}"
   echo ""
   echo "Machine information :"
   echo "---------------------"
   echo "is Altix           : ${IS_ALTIX_MACHINE}"
   echo "/dev/mmtimer       : ${HAVE_MMTIMER_DEVICE}"
   echo "is BG/L            : ${IS_BGL_MACHINE}"
   echo ""
   echo "Optional features:"
   echo "------------------"
   echo "Heterogeneous support : ${enable_hetero}"
   echo ""
])

# AX_IS_CELL_MACHINE
# --------------------
AC_DEFUN([AX_IS_CELL_MACHINE],
[
   AC_MSG_CHECKING([if this is a CELL machine])
   grep "Cell Broadband Engine" /proc/cpuinfo 2> /dev/null > /dev/null
   IS_CELL_MACHINE=$?
   if test "${IS_CELL_MACHINE}" = "0" ; then
      AC_MSG_RESULT([yes])
      AC_DEFINE([IS_CELL_MACHINE], 1, [Defined if this machine is a CELL machine])
      if test -f /usr/include/libspe2.h ; then
         AC_DEFINE([CELL_SDK], 2, [Defined if this machine has SDK x installed])
         CELL_SDK="2"
      elif test -f /usr/include/libspe.h ; then
         AC_DEFINE([CELL_SDK], 1, [Defined if this machine has SDK x installed])
         CELL_SDK="1"
      else
         AC_MSG_ERROR([Cannot determine which CELL SDK is installed])
         CELL_SDK="0"
      fi
      IS_CELL_MACHINE="yes"
   else
      AC_MSG_RESULT([no])
      IS_CELL_MACHINE="no"
   fi
   AM_CONDITIONAL(IS_CELL_MACHINE, test "${IS_CELL_MACHINE}" = "yes")
   AM_CONDITIONAL(CELL_SDK_2, test "${CELL_SDK}" = "2")
   AM_CONDITIONAL(CELL_SDK_1, test "${CELL_SDK}" = "1")

   if test "${CELL_SDK}" = "1" ; then
      AC_MSG_NOTICE([CBEA SDK installed support 1.x])
   elif test "${CELL_SDK}" = "2" ; then
      AC_MSG_NOTICE([CBEA SDK installed support 2.x/3.x])
   fi
])

# AX_IS_BGL_MACHINE
# ---------------------
AC_DEFUN([AX_IS_BGL_MACHINE],
[
   AC_MSG_CHECKING([if this is a BG/L machine])
   if test -d /bgl/BlueLight/ppcfloor/bglsys ; then
     IS_BGL_MACHINE="yes"
     BGL_HOME="/bgl/BlueLight/ppcfloor"
     CFLAGS="${CFLAGS} -I${BGL_HOME}/bglsys/include -I${BGL_HOME}/blrts-gnu/include"
     AC_SUBST(BGL_HOME)
     AC_MSG_RESULT([yes])
     AC_DEFINE([IS_BGL_MACHINE], 1, [Defined if this machine is a BG/L machine])
   else
     IS_BGL_MACHINE="no"
     AC_MSG_RESULT([no])
   fi
   AM_CONDITIONAL(IS_BGL_MACHINE, test "${IS_BGL_MACHINE}" = "yes")
])

# AX_IS_MN_MACHINE
#---------------------
AC_DEFUN([AX_IS_MN_MACHINE],
[
   AC_MSG_CHECKING([if this is MN machine])
   grep "Welcome to MareNostrum" /etc/motd 2> /dev/null > /dev/null
   GREP_RESULT=$?
   if test "${GREP_RESULT}" = "0" ; then
      AC_MSG_RESULT([yes])
      AC_DEFINE([IS_MN_MACHINE], 1, [Defined if this machine is MN])
   else
      AC_MSG_RESULT([no])
   fi
])

# AX_OPENMP
#-----------------
AC_DEFUN([AX_OPENMP],
[
   AC_PREREQ(2.59)

   AC_CACHE_CHECK([for OpenMP flag of _AC_LANG compiler],
      ax_cv_[]_AC_LANG_ABBREV[]_openmp,
      [save[]_AC_LANG_PREFIX[]FLAGS=$[]_AC_LANG_PREFIX[]FLAGS ax_cv_[]_AC_LANG_ABBREV[]_openmp=unknown
      # Flags to try:  -fopenmp (gcc), -openmp (icc), -mp (SGI &amp; PGI),
      #                -xopenmp (Sun), -omp (Tru64), -qsmp=omp (AIX), none
      ax_openmp_flags="-fopenmp -openmp -mp -xopenmp -omp -qsmp=omp none"
      if test "x$OPENMP_[]_AC_LANG_PREFIX[]FLAGS" != x; then
         ax_openmp_flags="$OPENMP_[]_AC_LANG_PREFIX[]FLAGS $ax_openmp_flags"
      fi
      for ax_openmp_flag in $ax_openmp_flags; do
         case $ax_openmp_flag in
            none) []_AC_LANG_PREFIX[]FLAGS=$save[]_AC_LANG_PREFIX[] ;;
            *) []_AC_LANG_PREFIX[]FLAGS="$save[]_AC_LANG_PREFIX[]FLAGS $ax_openmp_flag" ;;
         esac
         AC_TRY_LINK_FUNC(omp_set_num_threads,
   	       [ax_cv_[]_AC_LANG_ABBREV[]_openmp=$ax_openmp_flag; break])
      done
      []_AC_LANG_PREFIX[]FLAGS=$save[]_AC_LANG_PREFIX[]FLAGS])
      if test "x$ax_cv_[]_AC_LANG_ABBREV[]_openmp" = "xunknown"; then
         m4_default([$2],:)
      else
         if test "x$ax_cv_[]_AC_LANG_ABBREV[]_openmp" != "xnone"; then
            OPENMP_[]_AC_LANG_PREFIX[]FLAGS=$ax_cv_[]_AC_LANG_ABBREV[]_openmp
         fi
         m4_default([$1], [AC_DEFINE(HAVE_OPENMP,1,[Define if OpenMP is enabled])])
      fi
])

# AX_CHECK_UNWIND
# ------------
AC_DEFUN([AX_CHECK_UNWIND],
[
   AX_FLAGS_SAVE()

   AC_MSG_CHECKING([if libunwind works])

   AC_ARG_WITH(libunwind,
      AC_HELP_STRING(
         [--with-libunwind@<:@=DIR@:>@],
         [specify where to find libunwind libraries and includes]
      ),
      [libunwind_paths="$withval"],
      [libunwind_paths="/usr/local /usr"] dnl List of possible default paths
   )

   for home_dir in [${libunwind_paths} "not found"]; do
      if test -f "${home_dir}/${BITS}/include/libunwind.h" -a \
              -f "${home_dir}/${BITS}/lib/libunwind.a" -a \
              -f "${home_dir}/${BITS}/lib/libunwind-ia64.a" ; then
         UNWIND_HOME="${home_dir}/${BITS}"
         UNWIND_LIBSDIR="${home_dir}/${BITS}/lib"
         break
      elif test -f "${home_dir}/include/libunwind.h" -a \
                -f "${home_dir}/lib${BITS}/libunwind.a" -a \
                -f "${home_dir}/lib${BITS}/libunwind-ia64.a" ; then
         UNWIND_HOME="${home_dir}"
         UNWIND_LIBSDIR="${home_dir}/lib${BITS}"
      elif test -f "${home_dir}/include/libunwind.h" -a \
                -f "${home_dir}/lib/libunwind.a" -a \
                -f "${home_dir}/lib/libunwind-ia64.a" ; then
         UNWIND_HOME="${home_dir}"
         UNWIND_LIBSDIR="${home_dir}/lib"
         break
      fi
    done

   UNWIND_INCLUDES="${UNWIND_HOME}/include"
   UNWIND_CFLAGS="-I${UNWIND_INCLUDES}"
   UNWIND_CPPFLAGS=${UNWIND_CFLAGS}
   UNWIND_CXXFLAGS=${UNWIND_CFLAGS}
   UNWIND_LIBS="-lunwind -lunwind-ia64"
   UNWIND_LDFLAGS="-L${UNWIND_LIBSDIR}"
   if test -d ${UNWIND_LIBSDIR}/shared ; then 
      UNWIND_SHAREDLIBSDIR="${UNWIND_LIBSDIR}/shared"
   else
      UNWIND_SHAREDLIBSDIR=${UNWIND_LIBSDIR}
   fi

   AC_SUBST(UNWIND_HOME)
   AC_SUBST(UNWIND_CFLAGS)
   AC_SUBST(UNWIND_CPPFLAGS)
   AC_SUBST(UNWIND_CXXFLAGS)
   AC_SUBST(UNWIND_INCLUDES)
   AC_SUBST(UNWIND_LIBSDIR)
   AC_SUBST(UNWIND_SHAREDLIBSDIR)
   AC_SUBST(UNWIND_LIBS)
   AC_SUBST(UNWIND_LDFLAGS)

   CFLAGS="${CFLAGS} ${UNWIND_CFLAGS}"
   LIBS="${LIBS} ${UNWIND_LIBS}"
   LDFLAGS="${LDFLAGS} ${UNWIND_LDFLAGS}"

   AC_TRY_LINK(
      [ #define UNW_LOCAL_ONLY
        #include <libunwind.h> ], 
      [ unw_cursor_t cursor;
        unw_context_t uc;
        unw_word_t ip;

        unw_getcontext(&uc);
        unw_init_local(&cursor, &uc);
        unw_step(&cursor);
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
      ],
      [ libunwind_works="yes" ],
      [ libunwind_works="no" ]
   )

   if test "${libunwind_works}" = "yes"; then
      AC_DEFINE([UNWIND_SUPPORT], [1], [Unwinding support enabled for IA64])
      AC_DEFINE([HAVE_LIBUNWIND_H], [1], [Define to 1 if you have <libunwind.h> header file])
   fi

   AC_MSG_RESULT([${libunwind_works}])
   AX_FLAGS_RESTORE()
])

# AX_CHECK_LIBZ
# ------------
AC_DEFUN([AX_CHECK_LIBZ],
[
   AX_FLAGS_SAVE()

   AC_ARG_WITH(libz,
      AC_HELP_STRING(
         [--with-libz@<:@=DIR@:>@],
         [specify where to find libz libraries and includes]
      ),
      [libz_paths="${withval}"],
      [libz_paths="/usr/local /usr"] dnl List of possible default paths
   )

   for home_dir in [${libz_paths} "not found"]; do
      if test -f "${home_dir}/${BITS}/include/zlib.h" -a \
              -f "${home_dir}/${BITS}/lib/libz.a" ; then
         LIBZ_HOME="${home_dir}/${BITS}"
         LIBZ_LIBSDIR="${home_dir}/${BITS}/lib"
         break
      elif test -f "${home_dir}/include/zlib.h" -a \
                -f "${home_dir}/lib${BITS}/libz.a" ; then
         LIBZ_HOME="${home_dir}"
         LIBZ_LIBSDIR="${home_dir}/lib${BITS}"
      elif test -f "${home_dir}/include/zlib.h" -a \
                -f "${home_dir}/lib/libz.a" ; then
         LIBZ_HOME="${home_dir}"
         LIBZ_LIBSDIR="${home_dir}/lib"
         break
      fi
    done

   LIBZ_INCLUDES="${LIBZ_HOME}/include"
   LIBZ_CFLAGS="-I${LIBZ_INCLUDES}"
   LIBZ_CPPFLAGS=${LIBZ_CFLAGS}
   LIBZ_CXXFLAGS=${LIBZ_CFLAGS}
   LIBZ_LIBS="-lz"
   LIBZ_LDFLAGS="-L${LIBZ_LIBSDIR}"
   if test -d ${LIBZ_LIBSDIR}/shared ; then 
      LIBZ_SHAREDLIBSDIR="${LIBZ_LIBSDIR}/shared"
   else
      LIBZ_SHAREDLIBSDIR=${LIBZ_LIBSDIR}
   fi

   AC_SUBST(LIBZ_HOME)
   AC_SUBST(LIBZ_CFLAGS)
   AC_SUBST(LIBZ_CPPFLAGS)
   AC_SUBST(LIBZ_CXXFLAGS)
   AC_SUBST(LIBZ_INCLUDES)
   AC_SUBST(LIBZ_LIBSDIR)
   AC_SUBST(LIBZ_SHAREDLIBSDIR)
   AC_SUBST(LIBZ_LIBS)
   AC_SUBST(LIBZ_LDFLAGS)

   CFLAGS="${CFLAGS} ${LIBZ_CFLAGS}"
   LIBS="${LIBS} ${LIBZ_LIBS}"
   LDFLAGS="${LDFLAGS} ${LIBZ_LDFLAGS}"

   AC_CHECK_LIB(z, inflateEnd, [zlib_cv_libz=yes], [zlib_cv_libz=no])
   AC_CHECK_HEADER(zlib.h, [zlib_cv_zlib_h=yes], [zlib_cv_zlib_h=no])

   if test "${zlib_cv_libz}" = "yes" -a "${zlib_cv_zlib_h}" = "yes" ; then
      AC_DEFINE([HAVE_ZLIB], [1], [Zlib available])
			ZLIB_INSTALLED="yes"
   else
      ZLIB_INSTALLED="no"
   fi

   AM_CONDITIONAL(HAVE_ZLIB, test "${ZLIB_INSTALLED}" = "yes")

   AX_FLAGS_RESTORE()
])

# AX_PROG_DYNINST
# -------------
AC_DEFUN([AX_PROG_DYNINST],
[
   AX_FLAGS_SAVE()

   AC_ARG_WITH(dyninst,
      AC_HELP_STRING(
         [--with-dyninst@<:@=DIR@:>@],
         [specify where to find DynInst libraries and includes]
      ),
      [dyninst_paths="$withval"],
      [dyninst_paths="no"] dnl List of possible default paths
   )

   dnl Search for MRNet installation
   AX_FIND_INSTALLATION([DYNINST], [${dyninst_paths}])

   if test "${DYNINST_INSTALLED}" = "yes" ; then
      AC_LANG_SAVE()

      AC_LANG_PUSH([C++])

      dnl Check for MRNet header files.
      CXXFLAGS="${CXXFLAGS} -I${DYNINST_INCLUDES}"
      CPPFLAGS="${CPPFLAGS} -I${DYNINST_INCLUDES}"
      AC_CHECK_HEADERS([BPatch.h], [], [DYNINST_INSTALLED="no"])

      AC_LANG_RESTORE()
   fi

   dnl Did the checks pass?
   AM_CONDITIONAL(HAVE_DYNINST, test "${DYNINST_INSTALLED}" = "yes")

   if test "${DYNINST_INSTALLED}" = "no" ; then
      AC_MSG_WARN([Dyninst support has been disabled])
   else
      AC_DEFINE([HAVE_DYNINST], 1, [Define to 1 if DYNINST is installed in the system])
   fi

   AX_FLAGS_RESTORE()
])
# AX_PROG_MRNET
# -------------
AC_DEFUN([AX_PROG_MRNET],
[
   AX_FLAGS_SAVE()

   AC_ARG_WITH(mrnet,
      AC_HELP_STRING(
         [--with-mrnet@<:@=DIR@:>@],
         [specify where to find MRNet libraries and includes]
      ),
      [mrnet_paths="$withval"],
      [mrnet_paths="/home/bsc41/bsc41127/mrnet_last"] dnl List of possible default paths
   )

   dnl Search for MRNet installation
   AX_FIND_INSTALLATION([MRNET], [$mrnet_paths])

   if test "$MRNET_INSTALLED" = "yes" ; then
      AC_LANG_SAVE()

      AC_LANG_PUSH([C++])

      dnl Check for MRNet header files.
      CXXFLAGS="${CXXFLAGS} -I${MRNET_INCLUDES}/mrnet"
      CPPFLAGS="${CPPFLAGS} -I${MRNET_INCLUDES}/mrnet"
      AC_CHECK_HEADERS([MRNet.h], [], [MRNET_INSTALLED="no"])

      dnl Check for libraries.
      AC_MSG_CHECKING([for libmrnet and libxplat])

      if test -f ${MRNET_LIBSDIR}/libmrnet.a -a -f ${MRNET_LIBSDIR}/libxplat.a ; then
         MRNET_LDFLAGS="${MRNET_LDFLAGS} -lmrnet -lxplat -ldl -lpthread"
         AC_SUBST(MRNET_LDFLAGS)
         AC_MSG_RESULT([yes])
      else
         MRNET_INSTALLED="no"
         AC_MSG_RESULT([no])
      fi

      AC_LANG_RESTORE()
   fi

   dnl Did the checks pass?
   AM_CONDITIONAL(HAVE_MRNET, test "${MRNET_INSTALLED}" = "yes")

   if test "${MRNET_INSTALLED}" = "no" ; then
      AC_MSG_WARN([MRNet support has been disabled])
   else
      AC_DEFINE([HAVE_MRNET], 1, [Define to 1 if MRNET is installed in the system])
   fi

   AX_FLAGS_RESTORE()
])

AC_DEFUN([AX_CHECK_WEAK_ALIAS_ATTRIBUTE],
[
  # Test whether compiler accepts __attribute__ form of weak aliasing
  AC_CACHE_CHECK([whether ${CC} accepts function __attribute__((weak,alias()))],
  [ax_cv_weak_alias_attribute], [

    # We add -Werror if it's gcc to force an error exit if the weak attribute
    # isn't understood

    save_CFLAGS=${CFLAGS}
    
    if test "${GCC}" = "yes" ; then
       CFLAGS="-Werror"
    elif test "`basename ${CC}`" = "xlc" ; then
       CFLAGS="-qhalt=i"
    fi

    # Try linking with a weak alias...
    AC_LINK_IFELSE(
    [
      AC_LANG_PROGRAM(
      [
         void __weakf(int c) {}
         void weakf(int c) __attribute__((weak, alias("__weakf")));],
         [weakf(0)])],
      [ax_cv_weak_alias_attribute="yes"],
      [ax_cv_weak_alias_attribute="no"])

    # Restore original CFLAGS
    CFLAGS=${save_CFLAGS}
  ])

  # What was the result of the test?
  AS_IF([test "${ax_cv_weak_alias_attribute}" = "yes"],
  [
    AC_DEFINE([HAVE_WEAK_ALIAS_ATTRIBUTE], 1,
              [Define this if weak aliases may be created with __attribute__])
  ])
])

AC_DEFUN([AX_CHECK_ALIAS_ATTRIBUTE],
[
  # Test whether compiler accepts __attribute__ form of aliasing
  AC_CACHE_CHECK([whether ${CC} accepts function __attribute__((alias()))],
  [ax_cv_alias_attribute], [

    # We add -Werror if it's gcc to force an error exit if the weak attribute
    # isn't understood

    save_CFLAGS=${CFLAGS}
    
    if test "${GCC}" = "yes" ; then
       CFLAGS="-Werror"
    elif test "`basename ${CC}`" = "xlc" ; then
       CFLAGS="-qhalt=i"
    fi

    # Try linking with a weak alias...
    AC_LINK_IFELSE(
    [
      AC_LANG_PROGRAM(
      [
         void __alias(int c) {}
         void alias(int c) __attribute__((alias("__alias")));],
         [alias(0)])],
      [ax_cv_alias_attribute="yes"],
      [ax_cv_alias_attribute="no"])

    # Restore original CFLAGS
    CFLAGS=${save_CFLAGS}
  ])

  # What was the result of the test?
  AS_IF([test "${ax_cv_alias_attribute}" = "yes"],
  [
    AC_DEFINE([HAVE_ALIAS_ATTRIBUTE], 1,
              [Define this if aliases may be created with __attribute__])
  ])
])

AC_DEFUN([AX_CHECK_UNUSED_ATTRIBUTE],
[
  # Test whether compiler accepts __attribute__ form of setting unused 
  AC_CACHE_CHECK([whether ${CC} accepts function __attribute__((unused))],
  [ax_cv_unused_attribute], [

    # We add -Werror if it's gcc to force an error exit if the weak attribute
    # isn't understood

    save_CFLAGS=${CFLAGS}
    
    if test "${GCC}" = "yes" ; then
       CFLAGS="-Werror"
    elif test "`basename ${CC}`" = "xlc" ; then
       CFLAGS="-qhalt=i"
    fi

    # Try linking with a weak alias...
    AC_LINK_IFELSE(
    [
      AC_LANG_PROGRAM(
      [
         static char var __attribute__((unused));],
         [])],
      [ax_cv_unused_attribute="yes"],
      [ax_cv_unused_attribute="no"])

    # Restore original CFLAGS
    CFLAGS=${save_CFLAGS}
  ])

  # What was the result of the test?
  AS_IF([test "${ax_cv_unused_attribute}" = "yes"],
  [
    AC_DEFINE([HAVE_UNUSED_ATTRIBUTE], 1,
              [Define this if variables/functions can be marked as unused])
  ])
])

AC_DEFUN([AX_CHECK_LOAD_BALANCING],
[
	AC_MSG_CHECKING([for load-balancing installation])
	AC_ARG_WITH(load-balancing,
		AC_HELP_STRING(
			[--with-load-balancing@<:@=DIR@:>@],
			[specify where to find "load balancing" libraries and includes]
		),
		[lb_paths="$withval"],
		[lb_paths="none"] dnl List of possible default paths
	)
	if test -r "${lb_paths}/include/load_balancing.h" ; then
		AC_MSG_RESULT([$lb_paths])
		LOAD_BALANCING_HOME=${lb_paths}
		AC_SUBST([LOAD_BALANCING_HOME])
		lb_found="yes"
	else
		AC_MSG_RESULT([not found])
		lb_found="no"
	fi
	AM_CONDITIONAL(GENERATE_LOAD_BALANCING, test "${lb_found}" = "yes" )
])

AC_DEFUN([AX_OFF_T_64BIT],
[
	AC_MSG_CHECKING([how to get 64-bit off_t])
	if test "${OperatingSystem}" = "linux" ; then
		AC_DEFINE([_FILE_OFFSET_BITS],[64],[Define the bits for the off_t structure])
		AC_MSG_RESULT([define _FILE_OFFSET_BITS=64])
	elif test "${OperatingSystem}" = "freebsd" ; then
		AC_MSG_RESULT([nothing required])
	else
		AC_MSG_RESULT([unknown])
	fi
])
