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

dnl ------------------------------------------------------------------------
dnl Try to find the blocxx headers and libraries.
dnl ------------------------------------------------------------------------
dnl   Usage:   CHECK_BLOCXX([REQUIRED-VERSION
dnl                      [,ACTION-IF-FOUND[,ACTION-IF-NOT-FOUND]]])
dnl
AC_DEFUN([CHECK_BLOCXX],
[AC_REQUIRE([AC_SYS_LARGEFILE])dnl
ac_BLOCXX_includes=NO ac_BLOCXX_libraries=NO
BLOCXX_libraries=""
BLOCXX_includes=""
blocxx_error=""
AC_ARG_WITH(blocxx-dir,
	[  --with-blocxx-dir=DIR      where the root of blocxx is installed],
	[  ac_BLOCXX_includes="$withval"/include
		ac_BLOCXX_libraries="$withval"/lib${libsuff}
	])

want_BLOCXX=yes
if test $want_BLOCXX = yes; then

	AC_MSG_CHECKING(for blocxx)

	AC_CACHE_VAL(ac_cv_have_BLOCXX,
	[#try to guess blocxx locations

		BLOCXX_incdirs="/usr/include /usr/local/include /usr/blocxx/include /usr/local/blocxx/include $prefix/include"
		BLOCXX_incdirs="$ac_BLOCXX_includes $BLOCXX_incdirs"
		AC_FIND_FILE(blocxx/BLOCXX_config.h, $BLOCXX_incdirs, BLOCXX_incdir)
		ac_BLOCXX_includes="$BLOCXX_incdir"

		BLOCXX_libdirs="${libdir} /usr/lib${libsuff} /usr/local/lib /usr/blocxx/lib /usr/local/blocxx/lib $prefix/lib $exec_prefix/lib $kde_extra_libs"
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
	AC_COMPILE_IFELSE(
	    [AC_LANG_PROGRAM([[#include <blocxx/BLOCXX_config.h>]],
	                     [return BLOCXX_WITH_LARGEFILE])],
	    [blocxx_largefile=yes],[blocxx_largefile=no]
	)
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

AC_SUBST(BLOCXX_INCLUDES)
AC_SUBST(BLOCXX_LDFLAGS)
AC_SUBST(BLOCXX_LIB_DIR)
])



