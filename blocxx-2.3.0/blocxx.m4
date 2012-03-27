dnl ------------------------------------------------------------------------
dnl Try to find the blocxx headers and libraries.
dnl ------------------------------------------------------------------------
dnl   Usage:   CHECK_BLOCXX([REQUIRED-VERSION
dnl                      [,ACTION-IF-FOUND[,ACTION-IF-NOT-FOUND]]])
dnl
AC_DEFUN([CHECK_BLOCXX],
[AC_REQUIRE([AC_SYS_LARGEFILE])dnl
ac_BLOCXX_includes=NO
ac_BLOCXX_libraries=NO
BLOCXX_libraries=""
BLOCXX_includes=""
blocxx_error=""
AC_ARG_WITH(blocxx-dir,
	[  --with-blocxx-dir=DIR      where the root of blocxx is installed],
	[  if test -r "$withval"/src; then
			ac_BLOCXX_includes="$withval"/src
			ac_BLOCXX_libraries="$withval"/src/blocxx
		else
			ac_BLOCXX_includes="$withval"/include
			ac_BLOCXX_libraries="$withval"/lib${libsuff}
		fi
	])

want_BLOCXX=yes
if test $want_BLOCXX = yes; then

	AC_MSG_CHECKING(for blocxx)

	AC_CACHE_VAL(ac_cv_have_BLOCXX,
	[#try to guess blocxx locations

		BLOCXX_incdirs="/usr/include /usr/local/include /usr/blocxx/include /usr/local/blocxx/include /opt/local/include $prefix/include"
		BLOCXX_incdirs="$ac_BLOCXX_includes $BLOCXX_incdirs"
		AC_FIND_FILE(blocxx/BLOCXX_config.h, $BLOCXX_incdirs, BLOCXX_incdir)
		ac_BLOCXX_includes="$BLOCXX_incdir"

		BLOCXX_libdirs="${libdir} /usr/lib${libsuff} /usr/local/lib /usr/blocxx/lib /usr/local/blocxx/lib /opt/local/lib $prefix/lib $exec_prefix/lib $kde_extra_libs"
		if test ! "$ac_BLOCXX_libraries" = "NO"; then
			BLOCXX_libdirs="$ac_BLOCXX_libraries $BLOCXX_libdirs"
		fi

		test=NO
		BLOCXX_libdir=NO
		for dir in $BLOCXX_libdirs; do
			test -n "$dir" || continue
			try="ls -1 $dir/libblocxx*"
			if test=`eval $try 2> /dev/null`; then BLOCXX_libdir=$dir; break; else echo "tried $dir" >&AC_FD_CC ; fi
		done

		ac_BLOCXX_libraries="$BLOCXX_libdir"

		if test "$ac_BLOCXX_includes" = NO || test "$ac_BLOCXX_libraries" = NO; then
			have_BLOCXX=no
			blocxx_error="blocxx not found"
		else
			have_BLOCXX=yes;
		fi

	])

	eval "$ac_cv_have_BLOCXX"

	AC_MSG_RESULT([libraries $ac_BLOCXX_libraries, headers $ac_BLOCXX_includes])

	# Verify LFS support match
	BLOCXX_SAVE_FLAGS
	CXXFLAGS="-I$ac_BLOCXX_includes $CXXFLAGS"
	CFLAGS="-I$ac_BLOCXX_includes $CFLAGS"
	AC_COMPILE_IFELSE(
	    [AC_LANG_PROGRAM([[#include <blocxx/BLOCXX_config.h>]],
	                     [return BLOCXX_WITH_LARGEFILE])],
	    [blocxx_largefile=yes],[blocxx_largefile=no]
	)
	BLOCXX_RESTORE_FLAGS
	_enable_largefile="$enable_largefile"
	test "$_enable_largefile" != no && _enable_largefile=yes
	if test "$_enable_largefile" != "$blocxx_largefile" ; then
	    AC_MSG_ERROR([Large file support inconsistence with BloCxx detected!])
	fi

else
	have_BLOCXX=no
	blocxx_error="blocxx not found"
fi

if test "$ac_BLOCXX_includes" = "/usr/include" || \
   test  "$ac_BLOCXX_includes" = "/usr/local/include" || \
   test  "$ac_BLOCXX_includes" = "${includedir}" || \
   test -z "$ac_BLOCXX_includes"; then
	BLOCXX_INCLUDES="";
else
	BLOCXX_INCLUDES="-I$ac_BLOCXX_includes"
fi

if test "$ac_BLOCXX_libraries" = "/usr/lib" || \
   test "$ac_BLOCXX_libraries" = "/usr/local/lib" || \
   test "$ac_BLOCXX_libraries" = "${libdir}" || \
   test -z "$ac_BLOCXX_libraries"; then
	BLOCXX_LDFLAGS="-lblocxx"
	BLOCXX_LIB_DIR=""
else
	BLOCXX_LDFLAGS="-L$ac_BLOCXX_libraries -lblocxx"
	BLOCXX_LIB_DIR="$ac_BLOCXX_libraries"
fi

dnl If we have got a blocxx location, we can snag the required libs from its pc file.
dnl Note: This snags it directly from the pc file instead of from
dnl pkg-config, because blocxx installs the pc file on all platforms, but
dnl pkg-config is mostly linux-specific.
if test "x$ac_BLOCXX_libraries" != x && test -f "$ac_BLOCXX_libraries/pkgconfig/blocxx.pc"; then
	BLOCXX_LDFLAGS="${BLOCXX_LDFLAGS} `cat $ac_BLOCXX_libraries/pkgconfig/blocxx.pc | grep -i \"^libs:\" | sed \"s/^[[^:]]*://g\"`"
	BLOCXX_LIB_DIR="`echo ${BLOCXX_LDFLAGS} | sed -e \"s/-L\([[^ ]]*\) -l[[^ ]]*/\1/g\" | tr \" \" \":\"`"
	BLOCXX_CFLAGS="${BLOCXX_CFLAGS} `cat $ac_BLOCXX_libraries/pkgconfig/blocxx.pc | grep -i \"^cflags:\" | sed \"s/^[[^:]]*://g\"`"
fi

# Find needed compilation flags
if test "x$BLOCXX_CFLAGS" = "x"; then
	BLOCXX_SAVE_FLAGS
	CFLAGS="${CFLAGS} ${BLOCXX_INCLUDES}"
	CXXFLAGS="${CXXFLAGS} ${BLOCXX_INCLUDES}"
	# Some functions in blocxx depend on structures using the proper _FILE_OFFSET_BITS.  Set it if needed.
	BLOCXX_GET_OUTPUT([required file offset bits],
		[
			#include "blocxx/BLOCXX_config.h"
			#include <stdio.h>
			int main()
			{
				#ifdef _FILE_OFFSET_BITS
				printf("%d", _FILE_OFFSET_BITS);
				return 0;
				#else
				return 1;
				#endif
			}
		],
		[BLOCXX_CFLAGS="${BLOCXX_CFLAGS} -D_FILE_OFFSET_BITS=$conftest_output"],
		[]
	)
	BLOCXX_RESTORE_FLAGS
fi
dnl If we don't know the libs, we'll need to figure them out.
if test "x$BLOCXX_LDFLAGS" = "x"; then
	BLOCXX_SAVE_FLAGS
	CFLAGS="${CFLAGS} ${BLOCXX_INCLUDES}"
	CXXFLAGS="${CXXFLAGS} ${BLOCXX_INCLUDES}"
	dnl Find out if blocxx was built with iconv support.
	BLOCXX_USES_ICONV=0
	AC_MSG_CHECKING(if blocxx has iconv support)
	AC_RUN_IFELSE(
		AC_LANG_SOURCE(
			[
				#include <blocxx/BLOCXX_config.h>
				int main()
				{
				#ifdef BLOCXX_HAVE_ICONV_SUPPORT
					return 0;
				#else
					return 1;
				#endif
				}
			]
		),
		[
			AC_MSG_RESULT(yes)
			BLOCXX_USES_ICONV=1
		],
		[
			AC_MSG_RESULT(no)
			BLOCXX_USES_ICONV=0
		]
	)

	if test "x$BLOCXX_USES_ICONV" = x1; then
		dnl Check the libraries needed for iconv.  On many (most?) platforms, this will
		dnl be nothing, and the -lc will not be used.  On other platforms, -liconv is
		dnl needed.
		LIBS_icsaved="$LIBS"
		AC_CHECK_LIB(c, iconv_open,
			[
				dnl Use whatever was defined previously for ICONV_LIB.  This should be
				dnl empty, unless explicitly overridden.
				ICONV_LIB="$ICONV_LIB"
			],
			[
				AC_CHECK_LIB(iconv, iconv_open,
					[ ICONV_LIB="-liconv" ],
				)
			]
		)
		LIBS="$LIBS_icsaved"
	else
		dnl It was not linked with iconv, so we do not need it either.
		ICONV_LIB=""
	fi
	BLOCXX_LDFLAGS="${BLOCXX_LDFLAGS} $ICONV_LIB"
	BLOCXX_RESTORE_FLAGS
fi

dnl check REQUIRED-VERSION
ifelse([$1], , [], [

	# only check version if blocxx has been found
	if test "x$blocxx_error" = "x" ; then
		BLOCXX_REQUEST_VERSION="$1"
		AC_MSG_CHECKING(blocxx version)

		AC_REQUIRE([AC_PROG_EGREP])

		changequote(<<, >>)
		blocxx_version=`$EGREP "define BLOCXX_VERSION" $ac_BLOCXX_includes/blocxx/BLOCXX_config.h 2>&1 | sed 's/.* "\([^"]*\)".*/\1/p; d'`
		blocxx_major_ver=`expr $blocxx_version : '\([0-9]\+\)[0-9.]*'`
		blocxx_minor_ver=`expr $blocxx_version : '[0-9]\+\.\([0-9]\+\)[0-9.]*'`
		blocxx_micro_ver=`expr $blocxx_version : '[0-9]\+\.[0-9]\+\.\([0-9]\+\)' "|" 0`

		blocxx_major_req=`expr $BLOCXX_REQUEST_VERSION : '\([0-9]\+\)[0-9.]*'`
		blocxx_minor_req=`expr $BLOCXX_REQUEST_VERSION : '[0-9]\+\.\([0-9]\+\)[0-9.]*'`
		blocxx_micro_req=`expr $BLOCXX_REQUEST_VERSION : '[0-9]\+\.[0-9]\+\.\([0-9]\+\)' '|' 0`
		changequote([, ])
		AC_MSG_RESULT($blocxx_version)
		#echo "blocxx_major_ver=$blocxx_major_ver"
		#echo "blocxx_minor_ver=$blocxx_minor_ver"
		#echo "blocxx_micro_ver=$blocxx_micro_ver"
		#echo "blocxx_major_req=$blocxx_major_req"
		#echo "blocxx_minor_req=$blocxx_minor_req"
		#echo "blocxx_micro_req=$blocxx_micro_req"

		AC_MSG_CHECKING(requested blocxx version ($BLOCXX_REQUEST_VERSION))
		if test $blocxx_major_ver -gt $blocxx_major_req
		then
			AC_MSG_RESULT(yes)
			blocxx_version_ok=yes
		elif test $blocxx_major_ver -eq $blocxx_major_req &&
			test $blocxx_minor_ver -gt $blocxx_minor_req
		then
			AC_MSG_RESULT(yes)
			blocxx_version_ok=yes
		elif test $blocxx_major_ver -eq $blocxx_major_req &&
			test $blocxx_minor_ver -eq $blocxx_minor_req &&
			test $blocxx_micro_ver -ge $blocxx_micro_req
		then
			AC_MSG_RESULT(yes)
			blocxx_version_ok=yes
		else
			AC_MSG_RESULT(no)
			blocxx_error="Installed version of blocxx header files is too old"
		fi
	fi
]) dnl End of Ifdef REQUIRED-VERSION

if test "x$blocxx_error" = "x" ; then
	dnl Successfully found,
	dnl do ACTION-IF-FOUND
	ifelse([$2], , :, [$2])
else
	dnl do ACTION-IF-NOT-FOUND
	ifelse([$3], ,
		AC_MSG_ERROR($blocxx_error),
		[$3])
fi

AC_SUBST(ICONV_LIB)
AC_SUBST(BLOCXX_CFLAGS)
AC_SUBST(BLOCXX_INCLUDES)
AC_SUBST(BLOCXX_LDFLAGS)
AC_SUBST(BLOCXX_LIB_DIR)
])

dnl ------------------------------------------------------------------------
dnl Find a file (or one of more files in a list of dirs)
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN([AC_FIND_FILE],
[
$3=NO
for i in $2;
do
	for j in $1;
	do
		echo "configure: __oline__: $i/$j" >&AC_FD_CC
		if test -r "$i/$j"; then
			echo "taking that" >&AC_FD_CC
			$3=$i
			break 2
		fi
	done
done
])


AC_DEFUN([BLOCXX_PUSH_VARIABLE],
	[
		blocxx_stack_count_$1=${blocxx_stack_count_$1:-0}
		eval blocxx_stack_entries_$1_${blocxx_stack_count_$1}="\"${$1}\""
		blocxx_stack_count_$1=`expr ${blocxx_stack_count_$1} \+ 1`
	]
)

AC_DEFUN([BLOCXX_POP_VARIABLE],
	[
		blocxx_stack_count_$1=${blocxx_stack_count_$1:-0}
		if test ${blocxx_stack_count_$1} -gt 0; then
			blocxx_stack_count_$1=`expr ${blocxx_stack_count_$1} \- 1`
			eval $1="\"\${blocxx_stack_entries_$1_${blocxx_stack_count_$1}}\""
			unset blocxx_stack_entries_$1_${blocxx_stack_count_$1}
		else
			AC_MSG_ERROR(Attempt to pop empty variable $1)
		fi
	]
)

AC_DEFUN([BLOCXX_SAVE_FLAGS],
	[
		BLOCXX_PUSH_VARIABLE(CFLAGS)
		BLOCXX_PUSH_VARIABLE(CXXFLAGS)
		BLOCXX_PUSH_VARIABLE(AM_CXXFLAGS)
		BLOCXX_PUSH_VARIABLE(LDFLAGS)
	]
)

AC_DEFUN([BLOCXX_RESTORE_FLAGS],
	[
		BLOCXX_POP_VARIABLE(LDFLAGS)
		BLOCXX_POP_VARIABLE(CXXFLAGS)
		BLOCXX_POP_VARIABLE(AM_CXXFLAGS)
		BLOCXX_POP_VARIABLE(CFLAGS)
	]
)

dnl @synopsis BLOCXX_GET_OUTPUT(message, program, action_if_true, action_if_false)
dnl Compile and run the program $2.  If successful run $3, otherwise run
dnl $4.  In the true action ($3), the conftest_output variable will be
dnl set to the output of the program.
dnl
dnl @author Kevin Harris
AC_DEFUN([BLOCXX_GET_OUTPUT],
	[
		AC_MSG_CHECKING($1)
		AC_RUN_IFELSE($2,
			[
				# This uses some internals of autoconf.  Too much goes into
				# generating the executable command to repeat it.
				(eval "$ac_try") > conftest.output 2>&5
				conftest_output=[`]cat conftest.output[`]
				AC_MSG_RESULT([]) # The output has already been dumped out.
				$3
				rm -f conftest.output
				unset conftest_output
			],
			[
				AC_MSG_RESULT([]) # Execution error.
				$4
			]
		)
	]
)
