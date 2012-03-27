
dnl AC_AS_DIRNAME (PATH)
dnl this is the macro AS_DIRNAME from autoconf 2.4x
dnl defined here for use in autoconf 2.1x, remove the AC_ when you use 2.4x 
dnl
dnl @version $Id: acinclude.m4,v 1.15 2006/07/27 22:06:17 nuffer Exp $
dnl @author complain to <guidod@gmx.de>

AC_DEFUN([AC_ECHO_MKFILE],
[dnl
  case $2 in
    */*) P=` AC_AS_DIRNAME($2) ` ; AC_AS_MKDIR_P($P) ;;
  esac
  echo "$1" >$2
])

AC_DEFUN([AC_AS_DIRNAME],
[AC_AS_DIRNAME_EXPR([$1]) 2>/dev/null ||
AC_AS_DIRNAME_SED([$1])
])


# _AC_AS_EXPR_PREPARE
# ----------------
# Some expr work properly (i.e. compute and issue the right result),
# but exit with failure.  When a fall back to expr (as in AS_DIRNAME)
# is provided, you get twice the result.  Prevent this.
AC_DEFUN([_AC_AS_EXPR_PREPARE],
[if expr a : '\(a\)' >/dev/null 2>&1; then
  as_expr=expr
else
  as_expr=false
fi
])# _AC_AS_EXPR_PREPARE


# AS_DIRNAME(PATHNAME)
# --------------------
# Simulate running `dirname(1)' on PATHNAME, not all systems have it.
# This macro must be usable from inside ` `.
#
# Prefer expr to echo|sed, since expr is usually faster and it handles
# backslashes and newlines correctly.  However, older expr
# implementations (e.g. SunOS 4 expr and Solaris 8 /usr/ucb/expr) have
# a silly length limit that causes expr to fail if the matched
# substring is longer than 120 bytes.  So fall back on echo|sed if
# expr fails.
#
# FIXME: Please note the following m4_require is quite wrong: if the first
# occurrence of AS_DIRNAME_EXPR is in a backquoted expression, the
# shell will be lost.  We might have to introduce diversions for
# setting up an M4sh script: required macros will then be expanded there.

AC_DEFUN([AC_AS_DIRNAME_EXPR],
[AC_REQUIRE([_AC_AS_EXPR_PREPARE])dnl
$as_expr X[]$1 : 'X\(.*[[^/]]\)//*[[^/][^/]]*/*$' \| \
         X[]$1 : 'X\(//\)[[^/]]' \| \
         X[]$1 : 'X\(//\)$' \| \
         X[]$1 : 'X\(/\)' \| \
         .     : '\(.\)'])

AC_DEFUN([AC_AS_DIRNAME_SED],
[echo X[]$1 |
    sed ['/^X\(.*[^/]\)\/\/*[^/][^/]*\/*$/{ s//\1/; q; }
          /^X\(\/\/\)[^/].*/{ s//\1/; q; }
          /^X\(\/\/\)$/{ s//\1/; q; }
          /^X\(\/\).*/{ s//\1/; q; }
          s/.*/./; q']])


dnl @synopsis BX_PREFIX_CONFIG_H [(OUTPUT-HEADER [,PREFIX [,ORIG-HEADER]])]
dnl
dnl !! MODIFIED !! to avoid prefixing of lowecase names (e.g. const);
dnl All changes are marked with a CHANGED tag...
dnl
dnl This is a new variant from ac_prefix_config_ this one will use a
dnl lowercase-prefix if the config-define was starting with a
dnl lowercase-char, e.g. "#define const", "#define restrict", or
dnl "#define off_t", (and this one can live in another directory, e.g.
dnl testpkg/config.h therefore I decided to move the output-header to
dnl be the first arg)
dnl
dnl takes the usual config.h generated header file; looks for each of
dnl the generated "#define SOMEDEF" lines, and prefixes the defined
dnl name (ie. makes it "#define PREFIX_SOMEDEF". The result is written
dnl to the output config.header file. The PREFIX is converted to
dnl uppercase for the conversions.
dnl
dnl Defaults:
dnl
dnl   OUTPUT-HEADER = $PACKAGE-config.h
dnl   PREFIX = $PACKAGE
dnl   ORIG-HEADER, from AM_CONFIG_HEADER(config.h)
dnl
dnl Your configure.ac script should contain both macros in this order,
dnl and unlike the earlier variations of this prefix-macro it is okay
dnl to place the BX_PREFIX_CONFIG_H call before the AC_OUTPUT
dnl invokation.
dnl
dnl Example:
dnl
dnl   AC_INIT(config.h.in)        # config.h.in as created by "autoheader"
dnl   AM_INIT_AUTOMAKE(testpkg, 0.1.1)    # makes #undef VERSION and PACKAGE
dnl   AM_CONFIG_HEADER(config.h)          # prep config.h from config.h.in
dnl   BX_PREFIX_CONFIG_H(mylib/_config.h) # prep mylib/_config.h from it..
dnl   AC_MEMORY_H                         # makes "#undef NEED_MEMORY_H"
dnl   AC_C_CONST_H                        # makes "#undef const"
dnl   AC_OUTPUT(Makefile)                 # creates the "config.h" now
dnl                                       # and also mylib/_config.h
dnl
dnl if the argument to BX_PREFIX_CONFIG_H would have been omitted then
dnl the default outputfile would have been called simply
dnl "testpkg-config.h", but even under the name "mylib/_config.h" it
dnl contains prefix-defines like
dnl
dnl   #ifndef TESTPKG_VERSION
dnl   #define TESTPKG_VERSION "0.1.1"
dnl   #endif
dnl   #ifndef TESTPKG_NEED_MEMORY_H
dnl   #define TESTPKG_NEED_MEMORY_H 1
dnl   #endif
dnl   #ifndef _testpkg_const
dnl   #define _testpkg_const _const
dnl   #endif
dnl
dnl and this "mylib/_config.h" can be installed along with other
dnl header-files, which is most convenient when creating a shared
dnl library (that has some headers) where some functionality is
dnl dependent on the OS-features detected at compile-time. No need to
dnl invent some "mylib-confdefs.h.in" manually. :-)
dnl
dnl <CHANGED>
dnl
dnl Note that some AC_DEFINEs that end up in the config.h file are
dnl actually self-referential - e.g. AC_C_INLINE, AC_C_CONST, and the
dnl AC_TYPE_OFF_T say that they "will define inline|const|off_t if the
dnl system does not do it by itself". You might want to clean up about
dnl these - consider an extra mylib/conf.h that reads something like:
dnl
dnl    #include <mylib/_config.h>
dnl    #ifndef _testpkg_const
dnl    #define _testpkg_const const
dnl    #endif
dnl
dnl and then start using _testpkg_const in the header files. That is
dnl also a good thing to differentiate whether some library-user has
dnl starting to take up with a different compiler, so perhaps it could
dnl read something like this:
dnl
dnl   #ifdef _MSC_VER
dnl   #include <mylib/_msvc.h>
dnl   #else
dnl   #include <mylib/_config.h>
dnl   #endif
dnl   #ifndef _testpkg_const
dnl   #define _testpkg_const const
dnl   #endif
dnl </CHANGED>
dnl
dnl @category Misc
dnl @author Guido Draheim <guidod@gmx.de>
dnl @version 2003-11-04
dnl @license GPLWithACException

AC_DEFUN([BX_PREFIX_CONFIG_H],[AC_REQUIRE([AC_CONFIG_HEADER])
AC_CONFIG_COMMANDS([ifelse($1,,$PACKAGE-config.h,$1)],[dnl
AS_VAR_PUSHDEF([_OUT],[ac_prefix_conf_OUT])dnl
AS_VAR_PUSHDEF([_DEF],[ac_prefix_conf_DEF])dnl
AS_VAR_PUSHDEF([_PKG],[ac_prefix_conf_PKG])dnl
AS_VAR_PUSHDEF([_LOW],[ac_prefix_conf_LOW])dnl
AS_VAR_PUSHDEF([_UPP],[ac_prefix_conf_UPP])dnl
AS_VAR_PUSHDEF([_INP],[ac_prefix_conf_INP])dnl
m4_pushdef([_script],[conftest.prefix])dnl
m4_pushdef([_symbol],[m4_cr_Letters[]m4_cr_digits[]_])dnl
_OUT=`echo ifelse($1, , $PACKAGE-config.h, $1)`
_DEF=`echo _$_OUT | sed -e "y:m4_cr_letters:m4_cr_LETTERS[]:" -e "s/@<:@^m4_cr_Letters@:>@/_/g"`
_PKG=`echo ifelse($2, , $PACKAGE, $2)`
_LOW=`echo _$_PKG | sed -e "y:m4_cr_LETTERS-:m4_cr_letters[]_:"`
_UPP=`echo $_PKG | sed -e "y:m4_cr_letters-:m4_cr_LETTERS[]_:"  -e "/^@<:@m4_cr_digits@:>@/s/^/_/"`
_INP=`echo "ifelse($3,,,$3)" | sed -e 's/ *//'`
if test ".$_INP" = "."; then
   for ac_file in : $CONFIG_HEADERS; do test "_$ac_file" = _: && continue
     case "$ac_file" in
        *.h) _INP=$ac_file ;;
        *)
     esac
     test ".$_INP" != "." && break
   done
fi
if test ".$_INP" = "."; then
   case "$_OUT" in
      */*) _INP=`basename "$_OUT"`
      ;;
      *-*) _INP=`echo "$_OUT" | sed -e "s/@<:@_symbol@:>@*-//"`
      ;;
      *) _INP=config.h
      ;;
   esac
fi
if test -z "$_PKG" ; then
   AC_MSG_ERROR([no prefix for _PREFIX_PKG_CONFIG_H])
else
  if test ! -f "$_INP" ; then if test -f "$srcdir/$_INP" ; then
     _INP="$srcdir/$_INP"
  fi fi
  AC_MSG_NOTICE(creating $_OUT - prefix $_UPP for $_INP defines)
  if test -f $_INP ; then
    printf "%s\n" "s/@%:@undef  *\\(@<:@m4_cr_LETTERS[]_@:>@\\)/@%:@undef $_UPP""_\\1/" > _script
    printf "%s\n" "s/@%:@undef  *\\(@<:@m4_cr_letters@:>@\\)/@%:@undef \\1/" >> _script
    printf "%s\n" "s/@%:@def[]ine  *\\(@<:@m4_cr_LETTERS[]_@:>@@<:@_symbol@:>@*\\)\\(.*\\)/@%:@ifndef $_UPP""_\\1 \\" >> _script
    printf "%s\n" "@%:@def[]ine $_UPP""_\\1\\2 \\" >> _script
    printf "%s\n" "@%:@endif/" >>_script
    printf "%s\n" "s/@%:@def[]ine  *\\(@<:@m4_cr_letters@:>@@<:@_symbol@:>@*\\)\\(.*\\)/@%:@ifndef \\1 \\" >> _script
    printf "%s\n" "@%:@define \\1\\2 \\" >> _script
    printf "%s\n" "@%:@endif/" >> _script
    dnl Remove any duplicates (pre-prefixed defines)
    printf "%s\n" "s/\(@%:@\)\(ifndef\|undef\|define\) *$_UPP""_$_UPP""_/\1\2 $_UPP""_/g" >> _script

    # now executing _script on _DEF input to create _OUT output file
    printf "%s\n" "@%:@ifndef $_DEF"      >$tmp/pconfig.h
    printf "%s\n" "@%:@def[]ine $_DEF 1" >>$tmp/pconfig.h
    printf "%s\n" ' ' >>$tmp/pconfig.h
    printf "%s\n" /'*' $_OUT. Generated automatically at end of configure. '*'/ >>$tmp/pconfig.h

    sed -f _script $_INP >>$tmp/pconfig.h
    printf "%s\n" ' ' >>$tmp/pconfig.h
    printf "%s\n" '/* once:' $_DEF '*/' >>$tmp/pconfig.h
    printf "%s\n" "@%:@endif" >>$tmp/pconfig.h
    if cmp -s $_OUT $tmp/pconfig.h 2>/dev/null; then
      AC_MSG_NOTICE([$_OUT is unchanged])
    else
      ac_dir=`AS_DIRNAME(["$_OUT"])`
      AS_MKDIR_P(["$ac_dir"])
      rm -f "$_OUT"
      mv -f $tmp/pconfig.h "$_OUT"
    fi
    cp _script _configs.sed
  else
    AC_MSG_ERROR([input file $_INP does not exist - skip generating $_OUT])
  fi
  rm -f conftest.*
fi
m4_popdef([_symbol])dnl
m4_popdef([_script])dnl
AS_VAR_POPDEF([_INP])dnl
AS_VAR_POPDEF([_UPP])dnl
AS_VAR_POPDEF([_LOW])dnl
AS_VAR_POPDEF([_PKG])dnl
AS_VAR_POPDEF([_DEF])dnl
AS_VAR_POPDEF([_OUT])dnl
],[PACKAGE="$PACKAGE"])])


dnl @synopsis AC_CREATE_PREFIX_CONFIG_H [(OUTPUT-HEADER [,PREFIX [,ORIG-HEADER]])]
dnl
dnl this is a new variant from ac_prefix_config_
dnl   this one will use a lowercase-prefix if
dnl   the config-define was starting with a lowercase-char, e.g. 
dnl   #define const or #define restrict or #define off_t
dnl   (and this one can live in another directory, e.g. testpkg/config.h
dnl    therefore I decided to move the output-header to be the first arg)
dnl
dnl takes the usual config.h generated header file; looks for each of
dnl the generated "#define SOMEDEF" lines, and prefixes the defined name
dnl (ie. makes it "#define PREFIX_SOMEDEF". The result is written to
dnl the output config.header file. The PREFIX is converted to uppercase 
dnl for the conversions. 
dnl
dnl default OUTPUT-HEADER = $PACKAGE-config.h
dnl default PREFIX = $PACKAGE
dnl default ORIG-HEADER, derived from OUTPUT-HEADER
dnl         if OUTPUT-HEADER has a "/", use the basename
dnl         if OUTPUT-HEADER has a "-", use the section after it.
dnl         otherwise, just config.h
dnl
dnl In most cases, the configure.in will contain a line saying
dnl         AC_CONFIG_HEADER(config.h) 
dnl somewhere *before* AC_OUTPUT and a simple line saying
dnl        AC_PREFIX_CONFIG_HEADER
dnl somewhere *after* AC_OUTPUT.
dnl
dnl example:
dnl   AC_INIT(config.h.in)        # config.h.in as created by "autoheader"
dnl   AM_INIT_AUTOMAKE(testpkg, 0.1.1)   # "#undef VERSION" and "PACKAGE"
dnl   AM_CONFIG_HEADER(config.h)         #                in config.h.in
dnl   AC_MEMORY_H                        # "#undef NEED_MEMORY_H"
dnl   AC_C_CONST_H                       # "#undef const"
dnl   AC_OUTPUT(Makefile)                # creates the "config.h" now
dnl   AC_CREATE_PREFIX_CONFIG_H          # creates "testpkg-config.h"
dnl         and the resulting "testpkg-config.h" contains lines like
dnl   #ifndef TESTPKG_VERSION 
dnl   #define TESTPKG_VERSION "0.1.1"
dnl   #endif
dnl   #ifndef TESTPKG_NEED_MEMORY_H 
dnl   #define TESTPKG_NEED_MEMORY_H 1
dnl   #endif
dnl   #ifndef _testpkg_const 
dnl   #define _testpkg_const const
dnl   #endif
dnl
dnl   and this "testpkg-config.h" can be installed along with other
dnl   header-files, which is most convenient when creating a shared
dnl   library (that has some headers) where some functionality is
dnl   dependent on the OS-features detected at compile-time. No
dnl   need to invent some "testpkg-confdefs.h.in" manually. :-)
dnl
dnl @version $Id: acinclude.m4,v 1.15 2006/07/27 22:06:17 nuffer Exp $
dnl @author Guido Draheim <guidod@gmx.de>

AC_DEFUN([AC_CREATE_PREFIX_CONFIG_H],
[changequote({, })dnl 
ac_prefix_conf_OUT=`echo ifelse($1, , $PACKAGE-config.h, $1)`
ac_prefix_conf_OUTTMP="$ac_prefix_conf_OUT"tmp
ac_prefix_conf_DEF=`echo $ac_prefix_conf_OUT | sed -e 'y:abcdefghijklmnopqrstuvwxyz./,-:ABCDEFGHIJKLMNOPQRSTUVWXYZ____:'`
ac_prefix_conf_PKG=`echo ifelse($2, , $PACKAGE, $2)`
ac_prefix_conf_LOW=`echo _$ac_prefix_conf_PKG | sed -e 'y:ABCDEFGHIJKLMNOPQRSTUVWXYZ-:abcdefghijklmnopqrstuvwxyz_:'`
ac_prefix_conf_UPP=`echo $ac_prefix_conf_PKG | sed -e 'y:abcdefghijklmnopqrstuvwxyz-:ABCDEFGHIJKLMNOPQRSTUVWXYZ_:'  -e '/^[0-9]/s/^/_/'`
ac_prefix_conf_INP=`echo ifelse($3, , _, $3)`
if test "$ac_prefix_conf_INP" = "_"; then
   case $ac_prefix_conf_OUT in
      */*) ac_prefix_conf_INP=`basename $ac_prefix_conf_OUT` 
      ;;
      *-*) ac_prefix_conf_INP=`echo $ac_prefix_conf_OUT | sed -e 's/[a-zA-Z0-9_]*-//'`
      ;;
      *) ac_prefix_conf_INP=config.h
      ;;
   esac
fi
changequote([, ])dnl
if test -z "$ac_prefix_conf_PKG" ; then
   AC_MSG_ERROR([no prefix for _PREFIX_PKG_CONFIG_H])
else
  AC_MSG_RESULT(creating $ac_prefix_conf_OUT - prefix $ac_prefix_conf_UPP for $ac_prefix_conf_INP defines)
  if test -f $ac_prefix_conf_INP ; then
#    AC_AS_DIRNAME([/* automatically generated */], $ac_prefix_conf_OUTTMP)
    echo '/* automatically generated */' > $ac_prefix_conf_OUTTMP
changequote({, })dnl 
    echo '#ifndef '$ac_prefix_conf_DEF >>$ac_prefix_conf_OUTTMP
    echo '#define '$ac_prefix_conf_DEF' 1' >>$ac_prefix_conf_OUTTMP
    echo ' ' >>$ac_prefix_conf_OUTTMP
    echo /'*' $ac_prefix_conf_OUT. Generated automatically at end of configure. '*'/ >>$ac_prefix_conf_OUTTMP

    echo 's/#undef  *\([A-Z_]\)/#undef '$ac_prefix_conf_UPP'_\1/' >conftest.sed
#    echo 's/#undef  *\([a-z]\)/#undef '$ac_prefix_conf_LOW'_\1/' >>conftest.sed
    echo 's/#define  *\([A-Z_][A-Z0-9_]*\)\(.*\)/#ifndef '$ac_prefix_conf_UPP"_\\1 \\" >>conftest.sed
    echo '#define '$ac_prefix_conf_UPP"_\\1\\2 \\" >>conftest.sed
    echo '#endif/' >>conftest.sed
#    echo 's/#define  *\([A-Z0-9_]*\)\(.*\)/#ifndef '$ac_prefix_conf_LOW"_\\1 \\" >>conftest.sed
    echo '#define '$ac_prefix_conf_LOW"_\\1\\2 \\" >>conftest.sed
    echo '#endif/' >>conftest.sed
    sed -f conftest.sed $ac_prefix_conf_INP >>$ac_prefix_conf_OUTTMP
    echo ' ' >>$ac_prefix_conf_OUTTMP
    echo '/*' $ac_prefix_conf_DEF '*/' >>$ac_prefix_conf_OUTTMP
    echo '#endif' >>$ac_prefix_conf_OUTTMP
    if cmp -s $ac_prefix_conf_OUT $ac_prefix_conf_OUTTMP 2>/dev/null ; then
#      AC_MSG_RESULT([$ac_prefix_conf_OUT is unchanged])
#      doesn't work for some reason
      echo "$ac_prefix_conf_OUT is unchanged" 
      rm -f $ac_prefix_conf_OUTTMP
    else
      rm -f $ac_prefix_conf_OUT
      mv $ac_prefix_conf_OUTTMP $ac_prefix_conf_OUT
    fi
changequote([, ])dnl
  else
    AC_MSG_ERROR([input file $ac_prefix_conf_IN does not exist, dnl
    skip generating $ac_prefix_conf_OUT])
  fi
  rm -f conftest.* 
fi])
           


dnl @synopsis TYPE_SOCKLEN_T
dnl
dnl Check whether sys/socket.h defines type socklen_t. Please note
dnl that some systems require sys/types.h to be included before
dnl sys/socket.h can be compiled.
dnl
dnl @version $Id: acinclude.m4,v 1.15 2006/07/27 22:06:17 nuffer Exp $
dnl @author Lars Brinkhoff <lars@nocrew.org>
dnl
AC_DEFUN([TYPE_SOCKLEN_T],
[AC_CACHE_CHECK([for socklen_t], ac_cv_type_socklen_t,
[
  AC_TRY_COMPILE(
  [#include <stdlib.h>
   #include <sys/types.h>
   #include <sys/socket.h>],
  [socklen_t len = 42; return 0;],
  ac_cv_type_socklen_t=yes,
  ac_cv_type_socklen_t=no)
])
  if test $ac_cv_type_socklen_t = yes; then
    AC_DEFINE(HAVE_SOCKLEN_T)
  fi
])

#
# Stolen from postgres
#

# PGAC_PATH_PERL
# --------------
AC_DEFUN([PGAC_PATH_PERL],
[AC_PATH_PROG(PERL, perl)])


# PGAC_CHECK_PERL_CONFIG(NAME)
# ----------------------------
AC_DEFUN([PGAC_CHECK_PERL_CONFIG],
[AC_REQUIRE([PGAC_PATH_PERL])
AC_MSG_CHECKING([for Perl $1])
perl_$1=`$PERL -MConfig -e 'print $Config{$1}'`
AC_SUBST(perl_$1)dnl
AC_MSG_RESULT([$perl_$1])])


# PGAC_CHECK_PERL_CONFIGS(NAMES)
# ------------------------------
AC_DEFUN([PGAC_CHECK_PERL_CONFIGS],
[m4_foreach([pgac_item], [$1], [PGAC_CHECK_PERL_CONFIG(pgac_item)])])


# PGAC_CHECK_PERL_EMBED_LDFLAGS
# -----------------------------
AC_DEFUN([PGAC_CHECK_PERL_EMBED_LDFLAGS],
[AC_REQUIRE([PGAC_PATH_PERL])
AC_MSG_CHECKING(for flags to link embedded Perl)
pgac_tmp1=`$PERL -MExtUtils::Embed -e ldopts`
pgac_tmp2=`$PERL -MConfig -e 'print $Config{ccdlflags}'`
perl_embed_ldflags=`echo X"$pgac_tmp1" | sed "s/^X//;s%$pgac_tmp2%%"`
AC_SUBST(perl_embed_ldflags)dnl
AC_MSG_RESULT([$perl_embed_ldflags])])



# PGAC_CHECK_PERL_EMBED_CCFLAGS
# -----------------------------
AC_DEFUN([PGAC_CHECK_PERL_EMBED_CCFLAGS],
[AC_REQUIRE([PGAC_PATH_PERL])
AC_MSG_CHECKING(for flags to link embedded Perl)
perl_embed_ccflags=`$PERL -MExtUtils::Embed -e ccopts`
AC_SUBST(perl_embed_ccflags)dnl
AC_MSG_RESULT([$perl_embed_ccflags])])

dnl BLOCXX_HELP_STRING(LHS, RHS)
dnl This is a replace for AC_HELP_STRING, which doesn't exist in 2.13
dnl Autoconf 2.50 can not handle substr correctly.  It does have 
dnl AC_HELP_STRING, so let's try to call it if we can.
dnl Note: this define must be on one line so that it can be properly returned
dnl as the help string.
AC_DEFUN([BLOCXX_HELP_STRING],[ifelse(regexp(AC_ACVERSION, 2\.1), -1, AC_HELP_STRING($1,$2),[  ]$1 substr([                       ],len($1))$2)])dnl

AC_DEFUN([BLOCXX_CHECK_COMPILER_FLAG],
[
AC_MSG_CHECKING(whether $CXX supports -$1)
blocxx_cache=`echo $1 | sed 'y% .=/+-%____p_%'`
AC_CACHE_VAL(blocxx_cv_prog_cxx_$blocxx_cache,
[
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -$1"
  AC_TRY_LINK([],[ return 0; ], [eval "blocxx_cv_prog_cxx_$blocxx_cache=yes"], [])
  CXXFLAGS="$save_CXXFLAGS"
  AC_LANG_RESTORE
])
if eval "test \"`echo '$blocxx_cv_prog_cxx_'$blocxx_cache`\" = yes"; then
 AC_MSG_RESULT(yes)
 :
 $2
else
 AC_MSG_RESULT(no)
 :
 $3
fi
])

dnl Needed for monitor.
dnl Check for UNIX95 style cmsghdr types.
dnl Solaris doesn't turn them on by default, so if the quick code
dnl snippet fails, use define _XPG4_2.  That's the define that surrounds
dnl the changes in the msghdr structure.
AC_DEFUN([LO_CHECK_CMSGHDR], [
	AC_MSG_CHECKING(if we need _XPG4_2)
	AC_TRY_COMPILE([
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <sys/socket.h>

void foo(void) {
	struct cmsghdr c;
	struct msghdr m;
	m.msg_control = &c;
}
		],[
		],ac_check_cmsghdr=no,
		  ac_check_cmsghdr=yes
		  AC_DEFINE(_XPG4_2, 1,
			[Define for to get UNIX95 cmsghdr structures.])
	)
	AC_MSG_RESULT($ac_check_cmsghdr)
])

dnl Needed for monitor.
dnl Check for UNIX95 style cmsghdr "CMSG_LEN" macro
dnl define it if it doesn't exist. (Solaris)
AC_DEFUN([LO_CHECK_CMSG_LEN], [
	AC_MSG_CHECKING(if CMSG_LEN is defined)
	AC_TRY_COMPILE([
#define _XPG4_2 1
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <sys/socket.h>

const int size = CMSG_LEN(sizeof(int));
		],[
		],ac_check_cmsg_len=yes,
		  ac_check_cmsg_len=no
		  AC_DEFINE(LO_CMSG_DEF, 1,
			[Define if sys/socket.h doesn't.])
	)
	AC_MSG_RESULT($ac_check_cmsg_len)
])

dnl Needed for monitor.
dnl Check for the existance of pam_fail_delay and the PAM_FAIL_DELAY
dnl type for pam_get_item/pam_set_item.  If the platform doesn't support
dnl PAM_FAIL_DELAY, then we define the type macro to -1, and don't define
dnl the wrapper functions for pam_fail_delay().
AC_DEFUN([LO_CHECK_PAM_FAIL_DELAY], [
	AC_MSG_CHECKING(if there is support for pam_fail_delay)
	ac_func_search_save_LIBS=$LIBS
	LIBS="-lpam"
	AC_TRY_LINK_FUNC(pam_fail_delay,[
		lo_cv_search_pam_fail_delay=yes
		AC_DEFINE(LO_HAVE_PAM_FAIL_DELAY, 1,
			[Define if pam_fail_delay exists.])],
		[lo_cv_search_pam_fail_delay=no
		AC_DEFINE(PAM_FAIL_DELAY,-1,	
			[Define if the system headers do not.])]
	)
	LIBS=$ac_func_search_save_LIBS
	AC_MSG_RESULT($lo_cv_search_pam_fail_delay)
])

dnl Needed for monitor.
dnl Solaris and Linux/BSD differ on the "const"ness of various methods.
dnl Here we test for the second arg of the conversion function being const
dnl and define a macro to either "const" or "" as needed.
AC_DEFUN([LO_PAM_CONST_CONV_FUNC], [
	AC_MSG_CHECKING(if the PAM conversion function takes const messages)
	AC_LANG_SAVE
	AC_LANG_CPLUSPLUS
	AC_TRY_COMPILE([
#if   defined(HAVE_SECURITY_PAM_APPL_H)
#include <security/pam_appl.h>
#elif defined(HAVE_PAM_PAM_APPL_H)
#include <pam/pam_appl.h>
#endif
		],[
int c(int, const struct pam_message **m, struct pam_response **r, void *p);

struct pam_conv pc = { c };

		],lo_pam_const_conv_func=yes , lo_pam_const_conv_func=no
	)
	if test $lo_pam_const_conv_func = "yes" ; then
		foo=const
	else
		foo=
	fi
	AC_LANG_RESTORE
	AC_DEFINE_UNQUOTED(PAM_CONV_FUNC_CONST,$foo,
			[are pam_messages const in the conv func?])
	AC_MSG_RESULT($lo_pam_const_conv_func)
])

dnl Needed for monitor.
dnl Solaris and Linux/BSD differ on the "const"ness of various methods.
dnl Here we test for the third arg of pam_get_item being const
dnl and define a macro to either "const" or "" as needed.
AC_DEFUN([LO_PAM_CONST_GET_ITEM], [
	AC_MSG_CHECKING(if the PAM get_item function takes const items)
	AC_LANG_SAVE
	AC_LANG_CPLUSPLUS
	AC_TRY_COMPILE([
#if   defined(HAVE_SECURITY_PAM_APPL_H)
#include <security/pam_appl.h>
#elif defined(HAVE_PAM_PAM_APPL_H)
#include <pam/pam_appl.h>
#endif
		],[
	const void **item;
	pam_get_item(0,0,item);
		],lo_pam_const_get_item=yes , lo_pam_const_get_item=no
	)
	if test $lo_pam_const_get_item = "yes" ; then
		foo=const
	else
		foo=
	fi
	AC_LANG_RESTORE
	AC_DEFINE_UNQUOTED(PAM_GET_ITEM_CONST,$foo,
			[are items const in the get_item func?])
	AC_MSG_RESULT($lo_pam_const_get_item)
])


dnl
dnl This function will "undefine" a value by removing it from the confdefs.h,
dnl which will become config.h.
dnl
dnl @author Kevin Harris
AC_DEFUN([BLOCXX_UNDEFINE],
	[
		for blocxx_undefine_value_name in $1; do
			if grep "^#define[ 	]*${blocxx_undefine_value_name}[ 	]*.*" confdefs.h >/dev/null 2>&1; then
				dnl Warn that we are undefining something that was defined.
				AC_MSG_WARN(Undefining ${blocxx_undefine_value_name})
				sed "/^#define[ 	]*${blocxx_undefine_value_name}[ 	]*.*$/d" confdefs.h > confdefs.h.undef
				cat confdefs.h.undef > confdefs.h
				rm confdefs.h.undef
			else
				dnl AC_MSG_WARN(Not already defined: ${blocxx_undefine_value_name})
				:
			fi
		done
	]
)


dnl
dnl
dnl These are uncached header checking functions.
dnl
dnl Should these functions detect that a header is not possbile to include, it
dnl will attempt to undefine the value saying that it can be included.
dnl
dnl

dnl A helper function for checking headers.
dnl This helper creates a template for use with autoheader and tries to compile
dnl a simple program that only includes the requested file.
dnl
dnl On success, HAVE_<HEADER_NAME_TWEAKED> is defined.
dnl On failure, HAVE_<HEADER_NAME_TWEAKED>, if already defined is undefined.
dnl
dnl $1 = header file name
dnl $2 = test type
dnl $3 = translitted (tweaked) header name
dnl $4 = true action (optional)
dnl $5 = false action (optional)
dnl
dnl @author Kevin Harris
AC_DEFUN([BLOCXX_GENERIC_CHECK_HEADERS_HELPER],
	[
		AH_TEMPLATE(
			[HAVE_$3],
			[Defined to 1 if the <$1> header is available $2]
		)
		AC_MSG_CHECKING(for $1 $2)
		AC_TRY_COMPILE(
			[
				#include "./blocxx_header_test_file.h"
				#include <$1>
			],
			[],
			[
				AC_MSG_RESULT(yes)
				AC_DEFINE_UNQUOTED(HAVE_$3, 1, [The <$1> header is available $2])
				if test "x$6" != x; then
					echo "#include <$1>" >> blocxx_header_test_file.h
				fi
				$4
			],
			[
				AC_MSG_RESULT(no)
				BLOCXX_UNDEFINE(HAVE_$3)
				$5
			]
		)
	]
)

dnl Check for headers
dnl $1 = list of headers (whitespace separated)
dnl $2 = test type (usually "(c)" or "(c++)") -- Just gets dumped as output and in the config.h
dnl $3 = success action (optional)
dnl $4 = failure action (optional)
dnl
dnl @author Kevin Harris
AC_DEFUN([BLOCXX_GENERIC_CHECK_HEADERS],
	[
		rm -f blocxx_header_test_file.h
		touch blocxx_header_test_file.h
		m4_foreach([HeaderName],
			m4_split(m4_normalize([$1])),
			[
				BLOCXX_GENERIC_CHECK_HEADERS_HELPER(HeaderName, $2, translit(HeaderName, [a-z./], [A-Z__]), $3, $4, $5)
			]
		)
		rm -f blocxx_header_test_file.h
	]
)

dnl Check for headers that work in C++. Each header listed is tested 
dnl independently from any others listed.
dnl $1 = list of headers (whitespace separated)
dnl $2 = success action (optional)
dnl $3 = failure action (optional)
dnl
dnl @author Kevin Harris
AC_DEFUN([BLOCXX_INDEPENDENT_CHECK_CXX_HEADERS],
	[
		AC_LANG_SAVE
		AC_LANG_CPLUSPLUS
		BLOCXX_GENERIC_CHECK_HEADERS($1, (c++), $2, $3)
		AC_LANG_RESTORE
	]
)

dnl Check for headers that work in C.  Each header listed is tested
dnl independently from any others listed.
dnl $1 = list of headers (whitespace separated)
dnl $2 = success action (optional)
dnl $3 = failure action (optional)
dnl
dnl @author Kevin Harris
AC_DEFUN([BLOCXX_INDEPENDENT_CHECK_C_HEADERS],
	[
		AC_LANG_SAVE
		AC_LANG_C
		BLOCXX_GENERIC_CHECK_HEADERS($1, (c), $2, $3)
		AC_LANG_RESTORE
	]
)

dnl Check for headers that work in C++.  If the list includes more than one 
dnl header, all headers that exist will be included with each subsequent test.
dnl Note that for long lists this is MUCH slower than an independent header test.
dnl $1 = list of headers (whitespace separated)
dnl $2 = success action (optional)
dnl $3 = failure action (optional)
dnl
dnl @author Kevin Harris
AC_DEFUN([BLOCXX_CHECK_CXX_HEADERS],
	[
		AC_LANG_SAVE
		AC_LANG_CPLUSPLUS
		BLOCXX_GENERIC_CHECK_HEADERS($1, (c++), $2, $3, 1)
		AC_LANG_RESTORE
	]
)

dnl Check for headers that work in C.  If the list includes more than one 
dnl header, all headers that exist will be included with each subsequent test.
dnl Note that for long lists this is MUCH slower than an independent header test.
dnl $1 = list of headers (whitespace separated)
dnl $2 = success action (optional)
dnl $3 = failure action (optional)
dnl
dnl @author Kevin Harris
AC_DEFUN([BLOCXX_CHECK_C_HEADERS],
	[
		AC_LANG_SAVE
		AC_LANG_C
		BLOCXX_GENERIC_CHECK_HEADERS($1, (c), $2, $3, 1)
		AC_LANG_RESTORE
	]
)

dnl BLOCXX_ENABLE_SUPPORT -- Check for the --enable-FOO or --disable-FOO argument
dnl
dnl Allows up to 4 arguments (should have at least one)
dnl $1 = the name of the item to support
dnl $2 = action when "force-enabled"
dnl $3 = action when "force-disabled"
dnl $4 = action when unspecified (autodetect)
AC_DEFUN([BLOCXX_ENABLE_SUPPORT],
	[
		enable_support_variable=`echo $1 | tr '[-]' '[_]'`
		AC_MSG_CHECKING([if support for $1 is enabled])
		AC_ARG_ENABLE($1,
			BLOCXX_HELP_STRING([--enable-$1],[enable support for $1]),
			[
				if eval test "x\$enable_${enable_support_variable}" = "xyes"; then
					eval enable_${enable_support_variable}=force_enable
					AC_MSG_RESULT([force-enabled])
					$2
				else
					eval enable_${enable_support_variable}=force_disable
					AC_MSG_RESULT([force-disabled])
					$3
				fi
			],
			[
				eval enable_${enable_support_variable}=autodetect
				AC_MSG_RESULT([(autodetect)])
				$4
			]
		)
	]
)

dnl Check for support of some feature with autodetection.  This may
dnl seem complex, but for most use, it should be rather simple.
dnl
dnl $1 = Name of the option
dnl $2 = <program> (see AC_LANG_PROGRAM)
dnl $3 = executed when force-enabled and available.
dnl $4 = executed when force-disabled
dnl $5 = executed when autodetect available
dnl $6 = executed when autodetect unavailable.
dnl
dnl Most of the time, $3 through $6 should not be needed.
AC_DEFUN([BLOCXX_VERIFY_ENABLE_SUPPORT],
	[
		enable_support_variable=`echo $1 | tr '[-]' '[_]'`
		BLOCXX_ENABLE_SUPPORT([$1],
			[
				AC_MSG_CHECKING([if support for $1 is possible])
				AC_LINK_IFELSE(
					[ $2 ],
					[
						AC_MSG_RESULT(yes)
						eval enable_${enable_support_variable}=yes
						$3
					],
					AC_MSG_ERROR(Support for $1 is not possible)
				)
			],
			[
				eval enable_${enable_support_variable}=no
				$4
			],
			[
				AC_MSG_CHECKING([(autodetecting) possible support for $1])
				AC_LINK_IFELSE(
					[ $2 ],
					[
						AC_MSG_RESULT(yes)
						eval enable_${enable_support_variable}=yes
						$5
					],
					[
						AC_MSG_RESULT(no)
						eval enable_${enable_support_variable}=no
						$6
					]
				)
			]
		)
	]
)

dnl Check for support of some feature with autodetection (execution
dnl required).  This may seem complex, but for most use, it should
dnl be rather simple.
dnl
dnl $1 = Name of the option
dnl $2 = <program> (see AC_LANG_PROGRAM)
dnl $3 = executed when force-enabled and available.
dnl $4 = executed when force-disabled
dnl $5 = executed when autodetect available
dnl $6 = executed when autodetect unavailable.
dnl
dnl Most of the time, $3 through $6 should not be needed.
AC_DEFUN([BLOCXX_EXEC_VERIFY_ENABLE_SUPPORT],
	[
		enable_support_variable=`echo $1 | tr '[-]' '[_]'`
		BLOCXX_ENABLE_SUPPORT([$1],
			[
				AC_MSG_CHECKING([if support for $1 is possible])
				AC_RUN_IFELSE(
					[ $2 ],
					[
						AC_MSG_RESULT(yes)
						eval enable_${enable_support_variable}=yes
						$3
					],
					AC_MSG_ERROR(Support for $1 is not possible)
				)
			],
			[
				eval enable_${enable_support_variable}=no
				$4
			],
			[
				AC_MSG_CHECKING([(autodetecting) possible support for $1])
				AC_RUN_IFELSE(
					[ $2 ],
					[
						AC_MSG_RESULT(yes)
						eval enable_${enable_support_variable}=yes
						$5
					],
					[
						AC_MSG_RESULT(no)
						eval enable_${enable_support_variable}=no
						$6
					]
				)
			]
		)
	]
)

dnl @synopsis BLOCXX_REPLACEMENT_SCRIPT(output_file)
dnl @author Kevin Harris
dnl
dnl Create replacement sed scripts for all substituted variables
dnl
dnl There is a lot of junk in here to work around quoting.
dnl
dnl Since some of the variables being substituted will be makefile variables
dnl (cannot be evaluated now), some processing must be done to prevent the
dnl shell from interpreting them and puking (hence the ugly sed expression).
dnl
dnl Some variables have backslashes in them.  These need to be preserved.
AC_DEFUN([BLOCXX_REPLACEMENT_SCRIPT],
	[
		AC_MSG_NOTICE([Generating replacement script "$1"])

		rm -f $1
		touch $1

		replacement_makefile_expression='s/\$(\([[^)]]*\))/MAKEFILE_VARIABLE(\1)/g'
		replacement_backslash_expression='s/\\/\\\\/g'

		for replacement_variable in ${ac_subst_vars}; do
			# Evaluate (repeatedly) the variable value until it no longer changes.
			replacement_value1=
			replacement_value=`eval printf '%s' "\"\\\${$replacement_variable}\"" | sed -e "${replacement_makefile_expression}" -e "${replacement_backslash_expression}"`
			while test "x${replacement_value1}" != "x${replacement_value}"; do
				replacement_value1="${replacement_value}"
				replacement_value=`eval printf '%s' "\"${replacement_value}\"" | sed -e "${replacement_makefile_expression}" -e "${replacement_backslash_expression}"`
			done
			# Find a valid sed separator that is not present in the replacement expression.
			replacement_separator=
			for sep in ',' '/' '|' '?' '!'; do
				# If you ever find horrible shell breakage while executing the
				# configure script, check this next line first.
				replacement_without_separator=`eval printf '%s' "\"\\\${replacement_value%${sep}*}\""`
				if test "x${replacement_value}" = "x${replacement_without_separator}"; then
					replacement_separator=${sep}
					break
				fi
			done
			dnl AC_MSG_NOTICE([Substituting variable "${replacement_variable}" as "${replacement_value}"])
			printf '%s\n' "s${replacement_separator}@${replacement_variable}@${replacement_separator}${replacement_value}${replacement_separator}g" >> $1
		done
	]
)
