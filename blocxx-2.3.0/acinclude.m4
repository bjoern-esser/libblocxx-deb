dnl @synopsis AC_CREATE_PREFIX_CONFIG_H [(OUTPUT-HEADER [,PREFIX [,ORIG-HEADER]])]
dnl
dnl This is a new variant from ac_prefix_config_
dnl This one will prefix all defines starting with an uppercase character with
dnl the supplied prefix.  Lowercase defines and defines starting with an
dnl underscore will be wrapped in an ifdef but will not be prefixed.
dnl
dnl An added bonus is a sanity check on all non-prefixed numeric constants.  If
dnl the value is used from a -D<value> option or comes from a system header, it
dnl is checked to ensure that the value has not changed, causing problems.
dnl An example of such a problem would be _FILE_OFFSET_BITS not matching what the
dnl library was compiled to use, causing structures to have incorrect sizes and
dnl bizarre runtime errors.
dnl
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
dnl   #ifndef testpkg_const
dnl   #define testpkg_const const
dnl   #endif
dnl   #ifndef _SOME_DEFINE
dnl   #define _SOME_DEFINE 1234
dnl   #elif _SOME_DEFINE != 1234
dnl   #error Sanity check fail, _SOME_DEFINE != 1234
dnl   #endif
dnl
dnl   and this "testpkg-config.h" can be installed along with other
dnl   header-files, which is most convenient when creating a shared
dnl   library (that has some headers) where some functionality is
dnl   dependent on the OS-features detected at compile-time. No
dnl   need to invent some "testpkg-confdefs.h.in" manually. :-)
dnl
dnl @author Guido Draheim <guidod@gmx.de>
dnl Modified by Kevin Harris

AC_DEFUN([AC_CREATE_PREFIX_CONFIG_H],
[
	changequote({, })dnl
	ac_prefix_conf_OUT=`echo ifelse($1, , $PACKAGE-config.h, $1)`
	ac_prefix_conf_OUTTMP="$ac_prefix_conf_OUT"tmp
	ac_prefix_conf_DEF=`echo $ac_prefix_conf_OUT | sed -e 'y:abcdefghijklmnopqrstuvwxyz./,-:ABCDEFGHIJKLMNOPQRSTUVWXYZ____:'`
	ac_prefix_conf_PKG=`echo ifelse($2, , $PACKAGE, $2)`
	ac_prefix_conf_UPP=`echo $ac_prefix_conf_PKG | sed -e 'y:abcdefghijklmnopqrstuvwxyz-:ABCDEFGHIJKLMNOPQRSTUVWXYZ_:'  -e '/^[0-9]/s/^/_/'`
	ac_prefix_conf_INP=`echo ifelse($3, , _, $3)`
	ac_prefix_conf_SED=conftest.sed
	if test "$ac_prefix_conf_INP" = "_"; then
		case $ac_prefix_conf_OUT in
			*/*) _INP=`basename $ac_prefix_conf_OUT` ;;
			*-*) _INP=`echo $ac_prefix_conf_OUT | sed -e 's/[a-zA-Z0-9_]*-//'` ;;
			*) _INP=config.h ;;
		esac
	fi
	changequote([, ])dnl
	if test -z "$ac_prefix_conf_PKG" ; then
		AC_MSG_ERROR([no prefix for _PREFIX_PKG_CONFIG_H])
	else
		AC_MSG_RESULT(creating $ac_prefix_conf_OUT - prefix $ac_prefix_conf_UPP for $ac_prefix_conf_INP defines)
		if test -f $ac_prefix_conf_INP ; then
			changequote({, })dnl
			printf '%s\n' '/* automatically generated */' > $ac_prefix_conf_OUTTMP
			printf '%s\n' '#ifndef '$ac_prefix_conf_DEF >>$ac_prefix_conf_OUTTMP
			printf '%s\n' '#define '$ac_prefix_conf_DEF' 1' >>$ac_prefix_conf_OUTTMP
			printf '\n' >>$ac_prefix_conf_OUTTMP
			printf '%s\n' "/* $ac_prefix_conf_OUT. Generated automatically at end of configure. */" >>$ac_prefix_conf_OUTTMP

			printf '%s\n' 's/#undef  *\([A-Z]\)/#undef '$ac_prefix_conf_UPP'_\1/' >$ac_prefix_conf_SED

			#
			# Prefixed #define tweaking (no lowercase or leading underscores)
			#
			printf '%s\n' 's/#define  *\([A-Z][A-Z0-9_]*\)\(.*\)/#ifndef '"$ac_prefix_conf_UPP"'_\1\' >>$ac_prefix_conf_SED
			printf '%s\n' '#define '"$ac_prefix_conf_UPP"'_\1\2\' >>$ac_prefix_conf_SED
			printf '%s\n' '#endif/' >>$ac_prefix_conf_SED

			#
			# Non-prefixed #define tweaking (for lowercase identifiers or leading underscores)
			#

			# Wrapped, no defined value.
			printf '%s\n' 's/#define[ 	]*\([a-z_][a-zA-Z0-9_]*\)[ 	]*$/#ifndef \1\' >> $ac_prefix_conf_SED
			printf '%s\n' '#define \1\' >> $ac_prefix_conf_SED
			printf '%s\n' '#endif/' >> $ac_prefix_conf_SED

			# Wrapped in #ifndef, but no sanity check (non-numeric)
			printf '%s\n' 's/#define[ 	]*\([a-z_][a-zA-Z0-9_]*\)[ 	]\+\([^0-9].*\)/#ifndef \1\' >> $ac_prefix_conf_SED
			printf '%s\n' '#define \1 \2\' >> $ac_prefix_conf_SED
			printf '%s\n' '#endif/' >> $ac_prefix_conf_SED

			# Wrap with a sanity checked version
			printf '%s\n' 's/#define[ 	]*\([a-z_][a-zA-Z0-9_]*\)[ 	]\+\([0-9]\+\)/#ifndef \1\' >> $ac_prefix_conf_SED
			printf '%s\n' '#define \1 \2\' >> $ac_prefix_conf_SED
			printf '%s\n' '#elif \1 != \2\' >> $ac_prefix_conf_SED
			printf '%s\n' '#error Sanity check fail, \1 != \2\' >> $ac_prefix_conf_SED
			printf '%s\n' '#endif/' >> $ac_prefix_conf_SED

			sed -f $ac_prefix_conf_SED $ac_prefix_conf_INP >>$ac_prefix_conf_OUTTMP
			printf '\n' >>$ac_prefix_conf_OUTTMP
			printf '%s\n' "/* $ac_prefix_conf_DEF */" >>$ac_prefix_conf_OUTTMP
			printf '%s\n' '#endif' >>$ac_prefix_conf_OUTTMP
			if cmp -s $ac_prefix_conf_OUT $ac_prefix_conf_OUTTMP 2>/dev/null ; then
				echo "$ac_prefix_conf_OUT is unchanged"
				rm -f $ac_prefix_conf_OUTTMP
			else
				rm -f $ac_prefix_conf_OUT
				mv $ac_prefix_conf_OUTTMP $ac_prefix_conf_OUT
			fi
			changequote([, ])dnl
		else # ! -f $ac_prefix_conf_INP
			AC_MSG_ERROR([input file $ac_prefix_conf_IN does not exist, skip generating $ac_prefix_conf_OUT])
		fi
		rm -f conftest.*
	fi # ! -z $ac_prefix_conf_PKG
])

dnl
AC_DEFUN([BLOCXX_APPLY_PREFIX_TO_CONFIG_H],
	[
		AC_CONFIG_COMMANDS(
			[$1],
			[AC_CREATE_PREFIX_CONFIG_H($1,$2,$3)],
			[PACKAGE=$PACKAGE]
		)
	]
)

dnl @synopsis TYPE_SOCKLEN_T
dnl
dnl Check whether sys/socket.h defines type socklen_t. Please note
dnl that some systems require sys/types.h to be included before
dnl sys/socket.h can be compiled.
dnl
dnl @version $Id: acinclude.m4,v 1.1.1.14.4.3 2010/02/03 22:46:38 kharris Exp $
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
    AC_DEFINE(HAVE_SOCKLEN_T, 1, [socklen_t is available])
  fi
])

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
AC_DEFUN([LO_CHECK_MSGHDR_NEEDS_XPG4_2], [
	AC_MSG_CHECKING(if we need _XPG4_2)
	AC_TRY_COMPILE(
	[
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
	],[],
	[ac_check_cmsghdr=no],
	[
		ac_check_cmsghdr=yes
		AC_DEFINE(_XPG4_2,,[Define for to get UNIX95 cmsghdr structures.])
	]
	)
	AC_MSG_RESULT($ac_check_cmsghdr)
])

dnl Check if msghdr{} has msg_control member.
AC_DEFUN([LO_CHECK_MSGHDR_MSG_CONTROL],
	[
		AC_CHECK_MEMBER([struct msghdr.msg_control],
		[
			AC_DEFINE(HAVE_MSGHDR_MSG_CONTROL, 1, [define if struct msghdr contains the msg_control member])
		],
		[],
		[
			#include <sys/types.h>
			#include <sys/socket.h>
		])
	]
)

AC_DEFUN([LO_GET_MSG_SIZE],
	BLOCXX_GET_OUTPUT([size of integer message],
		[
			#include <sys/types.h>
			#include <sys/socket.h>
			#include <stdio.h>

			#ifndef CMSG_SPACE
			#define CMSG_SPACE(size) (sizeof(struct cmsghdr) + (size))
			#endif

			int main()
			{
				int space = CMSG_SPACE(sizeof(int));
				printf("%d", space);
				return 0;
			}
		],
		[
			AC_DEFINE_UNQUOTED([MSGHDR_MSG_SIZE_INT], $conftest_output, [size of an integer message])
		],
		[
			AC_MSG_WARN([Unable to determine size of an integer message])
		]
	)
)

AC_DEFUN([BLOCXX_CHECK_MSGHDR],
	[
		LO_CHECK_MSGHDR_NEEDS_XPG4_2
		LO_CHECK_MSGHDR_MSG_CONTROL
		LO_GET_MSG_SIZE
	]
)

dnl
dnl This function will "undefine" a value by removing it from the confdefs.h,
dnl which will become config.h.
dnl
dnl @author Kevin Harris
AC_DEFUN([BLOCXX_UNDEFINE],
	[
		for blocxx_undefine_value_name in $1; do
			if grep "^#define[ 	]*${blocxx_undefine_value_name}[ 	]*.*" confdefs.h >/dev/null 2>/dev/null; then
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

dnl @synopsis OW_CHECK_MACRO(name, [true], [false])
dnl @author Kevin Harris
dnl check for the macro defined by $1.
dnl if defined, $2 will be executed.
dnl if not, $3 will
AC_DEFUN([BLOCXX_CHECK_MACRO],
	[
		AC_RUN_IFELSE(
			[AC_LANG_SOURCE([
				int main(int argc, const char** argv)
				{
				#if defined $1
					return 0;
				#else
					return 1;
				#endif
				}
			])],
			[
				$2
			],
			[
				$3
			]
		)
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
		AC_RUN_IFELSE([AC_LANG_SOURCE([$2])],
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

		replacement_at_expression='s/[[$]]@/\\$\@/g'
		replacement_makefile_expression='s/[[$]][[(]]/MAKEFILE_VARIABLE(/g'
		replacement_backslash_expression='s/\\/\\\\/g'

		for replacement_variable in ${ac_subst_vars}; do
			# Evaluate (repeatedly) the variable value until it no longer changes.
			replacement_value1=
			replacement_value=`eval printf '%s' "\"\\\[$]{[$]replacement_variable}\"" | sed -e "[$]{replacement_makefile_expression}" -e "[$]{replacement_backslash_expression}" -e "[$]{replacement_at_expression}"`
			while test "x[$]{replacement_value1}" != "x[$]{replacement_value}"; do
				replacement_value1="[$]{replacement_value}"
				replacement_value=`eval printf '%s' "\"[$]{replacement_value}\"" | sed -e "[$]{replacement_makefile_expression}" -e "[$]{replacement_backslash_expression}" -e "[$]{replacement_at_expression}"`
			done
			# Find a valid sed separator that is not present in the replacement expression.
			replacement_separator=
			for sep in ',' '/' '|' '?' '!'; do
				# If you ever find horrible shell breakage while executing the
				# configure script, check this next line first.
				replacement_without_separator=`eval printf '%s' "\"\\\[$]{replacement_value}\"" | tr -d "${sep}"`
				if test "x[$]{replacement_value}" = "x[$]{replacement_without_separator}"; then
					replacement_separator=${sep}
					break
				fi
			done
			dnl AC_MSG_NOTICE([Substituting variable "[$]{replacement_variable}" as "[$]{replacement_value}"])
			printf '%s\n' "s[$]{replacement_separator}@[$]{replacement_variable}@[$]{replacement_separator}[$]{replacement_value}[$]{replacement_separator}g" >> $1
		done
	]
)


dnl @synopsis BLOCXX_CHECK_MKTIME_FULL(year, month, day, hour, minute, second, [action if valid], [action if invalid])
dnl @author Kevin Harris
AC_DEFUN([BLOCXX_CHECK_MKTIME_FULL],
[
	dnl AC_MSG_CHECKING([if $1-$2-$3 $4:$5:$6 is a convertable time])
	AC_TRY_RUN(
		[
			#include <time.h>
			#include <string.h>
			#include <errno.h>
			#include <stdio.h>
			#include <stdlib.h>
			main()
			{
				putenv("TZ=UTC");
				tzset();
				struct tm timestruct;
				memset(&timestruct, 0, sizeof(struct tm));
				timestruct.tm_year = $1 - 1900;
				timestruct.tm_mon = ($2 - 1);
				timestruct.tm_mday = $3;
				timestruct.tm_hour = $4;
				timestruct.tm_min = $5;
				timestruct.tm_sec = $6;

				errno = 0;
				time_t value = mktime(&timestruct);
				if( value < 0 && errno != 0 )
				{
					printf("Failed converting to time_t: error=%s\n", strerror(errno));
					exit(1);
				}

				// Not thread safe but we could care less for this test.
				struct tm* timestruct2 = gmtime(&value);

				errno = 0;
				if( !timestruct2 || errno != 0 )
				{
					printf("Failed converting back to struct tm: error=%s\n", strerror(errno));
					exit(2);
				}


				if( timestruct2->tm_year != ($1 - 1900) )
				{
					printf("The year does not match: Expected=%04d-%02d-%02d, created=%04d-%02d-%02d\n", $1,$2,$3, timestruct2->tm_year + 1900, timestruct2->tm_mon + 1, timestruct2->tm_mday);
					exit(3);
				}
				else if( timestruct2->tm_mon != ($2 - 1) )
				{
					printf("The month does not match: Expected=%04d-%02d-%02d, created=%04d-%02d-%02d\n", $1,$2,$3, timestruct2->tm_year + 1900, timestruct2->tm_mon + 1, timestruct2->tm_mday);
					exit(4);
				}
				else if( timestruct2->tm_mday != $3 )
				{
					printf("The day does not match: Expected=%04d-%02d-%02d, created=%04d-%02d-%02d\n", $1,$2,$3, timestruct2->tm_year + 1900, timestruct2->tm_mon + 1, timestruct2->tm_mday);
					exit(5);
				}

				// The hour minute and second can not reliably be converted back and forth because of leap seconds and other adjustments.

				exit(0);
			}
		],
		[
			dnl AC_MSG_RESULT([yes])
			$7
		],
		[
			dnl AC_MSG_RESULT([no])
			$8
		]
	)
])

dnl @synopsis BLOCXX_CHECK_MKTIME_FULL(year, month, day, [action if valid], [action if invalid])
AC_DEFUN([BLOCXX_CHECK_MKTIME],
	[
		BLOCXX_CHECK_MKTIME_FULL($1,$2,$3,0,0,0,$4,$5)
	]
)

dnl BLOCXX_AC_PATH_BC -- Check for bc command line calculator
AC_DEFUN([BLOCXX_AC_PATH_BC],
[AC_PATH_PROG(BC, bc, [])])

dnl @synopsis BLOCXX_FIND_TIME_BOUNDARY(min_year, max_year, success_impl)
dnl where success_impl can use $calculated_years and $calculated_time
AC_DEFUN([BLOCXX_FIND_TIME_BOUNDARY],
	[
		AC_REQUIRE([BLOCXX_AC_PATH_BC])
		if test "x$BC" = x ; then
			AC_MSG_ERROR([no bc command line calculator found])
		fi

		time_min=$1
		time_max=$2

		time_mday=31
		time_mon=12
		end_of_year_shift=1

		if test $time_min -gt $time_max; then
			time_mday=1
			time_mon=1
			end_of_year_shift=0
		fi

		BLOCXX_CHECK_MKTIME($time_max, $time_mon, $time_mday,
			[
				calculated_year=$time_max
			],
			[
				while test x$time_min != x$time_max; do
					time_mid=[`]expr \( $time_min + $time_max \) / 2[`]
					if test x$time_mid = x$time_min -o x$time_mid = x$time_max; then
						break
					fi
					# echo "Mid=$time_mid"
					BLOCXX_CHECK_MKTIME($time_mid, $time_mon, $time_mday,
						[
							time_min=$time_mid
							# echo "Changed min to $time_min"
						],
						[
							time_max=$time_mid
							# echo "Changed max to $time_max"
						]
					)
				done
				calculated_year=$time_min
			]
		)

		# Include the leap day in 1972, if it is needed.
		leap_day_shift=0
		if test $calculated_year -gt 1972 || test $calculated_year -ge 1972 -a $time_mon -gt 2 ; then
			leap_day_shift=1
		fi

		leap_days_since_1970=[`]expr \( $calculated_year - 1972 \) / 4 - \( $calculated_year - 2000 \) / 100 + \( $calculated_year - 2000 \) / 400 + $leap_day_shift[`]

		# Calculate the largest (or smallest) possible date based on
		# the calculated year.  This should be usable as a time_t.
		calculated_days=[`]expr \( $calculated_year + $end_of_year_shift - 1970 \) \* 365 + $leap_days_since_1970[`]
		calculated_time=[`]echo "$calculated_days * 24 * 60 * 60 - $end_of_year_shift" | $BC[`]
		calculated_time=[`]printf "%s" $calculated_time[`]

		$3
	]
)


dnl @synopsis BLOCXX_CHECK_TIME_SUPPORT
dnl @author Kevin Harris
dnl Check for time functions and find the maximum and minimum times that can be
dnl used on this platform.
AC_DEFUN([BLOCXX_TIME_SUPPORT],
	[
		AC_CHECK_SIZEOF(time_t, 4, [#include <time.h>])
		AC_CHECK_FUNCS([asctime_r gmtime_r localtime_r gettimeofday timegm])
		max_year=0
		min_year=0
		max_time=0
		min_time=0
		AC_MSG_CHECKING([for latest possible year in time conversion functions])
		# Check the largest 4-digit year.
		BLOCXX_FIND_TIME_BOUNDARY(1970, 9999,
			[
				max_year=$calculated_year
				max_time=$calculated_time
			]
		)
		AC_MSG_RESULT([$max_year])

		AC_DEFINE_UNQUOTED([DATETIME_MAXIMUM_YEAR], [${max_year}], [The largest complete year accepted by DateTime])
		AC_DEFINE_UNQUOTED([DATETIME_MAXIMUM_TIME], [${max_time}ll], [The largest acceptable distance in seconds from the epoch])


		AC_MSG_CHECKING([for earliest possible year in time conversion functions])
		# Check the minimum time value and minimum year.
		BLOCXX_FIND_TIME_BOUNDARY(1970, 0,
			[
				min_year=$calculated_year
				min_time=$calculated_time
			]
		)
		AC_MSG_RESULT([$min_year])

		AC_DEFINE_UNQUOTED([DATETIME_MINIMUM_YEAR], [${min_year}], [The smallest complete year accepted by DateTime])
		AC_DEFINE_UNQUOTED([DATETIME_MINIMUM_TIME], [${min_time}ll], [The largest acceptable (negative) distance in seconds from the epoch])
	]
)

# Execute the arguments given by $2..$n until ${$1} is nonzero.
# @author Kevin Harris
#
# This is the same as a bunch of nested ifs, but makes writing the
# tests easier.
#
# How it works:
# It will repeatedly check the value of the variable given by $1.  If
# this is zero, then the $2 will be executed and it will recurse with
# $3...$n still remaining.
#
# All of the builtin([foo]) junk is used to prevent m4 from
# entering an infinite loop and consuming all available memory.
#
# $1=variable name
# $2...$n = actions to perform until the variable specified in $1 is non-zero.
#
# Example:
# value=0
# BLOCXX_SELECT_ONE([value], [echo 1], [echo 2; value=1], [echo 3])
#
# would result in:
#   echo 1
#   echo 2
# at which point no further evaluation would be done.
AC_DEFUN([BLOCXX_SELECT_ONE],
	[
		builtin([ifelse],
			[$2], [], [],
			[
				if eval test [$]{$1:-0} = 0; then
					$2
				fi
				BLOCXX_SELECT_ONE($1, builtin([shift],builtin([shift], $@)))
			]
		)
	]
)


dnl @synopsis BLOCXX_COMPILER_TEST
dnl @author Kevin Harris
dnl Check for the compiler type and version.
dnl
dnl This will define one of (names before transform):
dnl   USING_ACC
dnl   USING_XLC
dnl   USING_SUNC
dnl   USING_GCC
dnl And will define:
dnl   COMPILER_VERSION
dnl   COMPILER_MAJOR
dnl   COMPILER_MINOR
dnl And will set the variables:
dnl   BLOCXX_COMPILER_VERSION
dnl   BLOCXX_COMPILER_MAJOR
dnl   BLOCXX_COMPILER_MINOR
dnl   BLOCXX_COMPILER_TYPE
dnl   BLOCXX_COMPILER_LIBDIR
dnl And will export to Makefiles:
dnl   BLOCXX_COMPILER_LIBDIR
dnl
dnl Among the variables it sets, this could be used (in the future) to
dnl store in the BLOCXX_config.h the compiler type that was used to build
dnl BLOCXX.  This could be important when building something that is using
dnl a binary-incompatible compiler.

AC_DEFUN([BLOCXX_COMPILER_TEST],
	[
		BLOCXX_COMPILER_LIBDIR=
		found_compiler_type=0
		BLOCXX_SELECT_ONE([found_compiler_type],
			# Check for aCC
			[
				AC_MSG_CHECKING([C++ compiler is aCC])
				BLOCXX_CHECK_MACRO([__HP_aCC],
					[
						AC_MSG_RESULT([yes])
						AC_DEFINE(USING_ACC, [], [BLOCXX was built with aCC])
						BLOCXX_COMPILER_TYPE=aCC
						found_compiler_type=1
					],
					[
						AC_MSG_RESULT([no])
					]
				)
			],
			# Check for xlC
			[
				AC_MSG_CHECKING([C++ compiler is xlC])
				BLOCXX_CHECK_MACRO([__xlC__],
					[
						AC_MSG_RESULT([yes])
						AC_DEFINE(USING_XLC, [], [BLOCXX was built with xlC])
						BLOCXX_COMPILER_TYPE=xlC
						found_compiler_type=1
					],
					[
						AC_MSG_RESULT([no])
					]
				)
			],
			# Check for Sun C++
			[
				AC_MSG_CHECKING([C++ compiler is Sun C++])
				# Sun C++ apparently defines no macros that could be used
				# to identify it.  Use the -V option and a grep instead.
				if $CXX -V 2>&1 | grep 'Sun C++' >/dev/null 2>&1; then
					AC_MSG_RESULT([yes])
					AC_DEFINE(USING_SUNC, [], [BLOCXX was built with Sun C++])
					BLOCXX_COMPILER_TYPE=sunc
					found_compiler_type=1
				else
					AC_MSG_RESULT([no])
				fi
			],
			# Check for gcc
			[
				AC_MSG_CHECKING([C++ compiler is gcc])
				BLOCXX_CHECK_MACRO([__GNUC__],
					[
						AC_MSG_RESULT([yes])
						AC_DEFINE(USING_GCC, [], [BLOCXX was built with gcc (g++)])
						BLOCXX_COMPILER_TYPE=gcc
						found_compiler_type=1
					],
					[
						AC_MSG_RESULT([no])
					]
				)
			],
			[
				AC_MSG_WARN(Unable to detect the compiler type)
				BLOCXX_COMPILER_TYPE=unknown
			]
		)

		# Get the version...
		# gcc:
		# $ g++ --version
		# g++ (GCC) 4.1.2 (Ubuntu 4.1.2-0ubuntu4)
		# or
		# i686-apple-darwin10-g++-4.2.1 (GCC) 4.2.1 (Apple Inc. build 5646)
		# ...
		# -->  g++ --version 2>&1 | grep '[0-9][0-9]*\.[0-9][0-9]*' | sed -e 's/^[^(]*[(][^)]*[)]//g' -e 's/[(][^)]*[)]//g' -e 's/[^0-9.]//g'
		#
		# xlC_r:
		# $ xlC_r -qversion
		# IBM XL C/C++ Enterprise Edition V8.0 for AIX
		# Version: 08.00.0000.0010
		# --> xlC_r -qversion 2>&1 | sed -e 's/[^0-9.]//g' | head -1
		#
		# aCC:
		# $ aCC --version
		# aCC: HP ANSI C++ B3910B A.03.67
		# --> aCC --version 2>&1 | sed 's/[^.]*\.//'
		#
		# Sun C:
		# $ CC -V
		# CC: Sun C++ 5.9 SunOS_i386 2007/05/03
		# --> CC -V 2>&1 | grep 'Sun C++' | sed -e 's/.*C++[ ]*//' -e 's/ .*//'
		case $BLOCXX_COMPILER_TYPE in
			aCC)
				BLOCXX_COMPILER_VERSION=`$CXX --version 2>&1 | sed 's/[[^.]]*\.//'`
				;;
			xlC)
				BLOCXX_COMPILER_VERSION=`$CXX -qversion 2>&1 | sed -e 's/[[^0-9.]]//g' | head -1`
				;;
			sunc)
				BLOCXX_COMPILER_VERSION=`$CXX -V 2>&1 | grep 'Sun C++' | sed -e 's/.*C++[[ ]]*//' -e 's/ .*//'`
				;;
			gcc)
				# Everything before the (gcc) needs to be removed.  Any suffixed vendor information (such as from apple) in parens also needs to be removed.  Then all non-numeric characters are stripped.
				BLOCXX_COMPILER_VERSION=`$CXX --version 2>&1 | grep '[[0-9]][[0-9]]*\.[[0-9]][[0-9]]*' | sed -e 's/^[[^(]]*[[(]][[^)]]*[[)]]//g' -e 's/[[(]][[^)]]*[[)]]//g' -e 's/[[^0-9.]]//g'`
				BLOCXX_COMPILER_LIBDIR=`$CXX --print-file libstdc++.$LIB_EXT`
				BLOCXX_CLEANUP_DIRECTORY_NAME(BLOCXX_COMPILER_LIBDIR, `dirname $BLOCXX_COMPILER_LIBDIR`)
				;;
			*)
				AC_MSG_WARN(Unable to detect compiler version)
				;;
		esac

		if test "x$BLOCXX_COMPILER_VERSION" != x; then
			AC_DEFINE(COMPILER_VERSION, [$BLOCXX_COMPILER_VERSION], [The compiler version used to compile OpenWBEM])
			BLOCXX_COMPILER_MAJOR=`printf '%s' "${BLOCXX_COMPILER_VERSION}" | sed 's/[[^0-9]].*//g'`
			BLOCXX_COMPILER_MINOR=`printf '%s' "${BLOCXX_COMPILER_VERSION}" | sed -e 's/^[[^0-9]]*[[0-9]]*\.//' -e 's/[[^0-9]].*//'`
			if test "x$BLOCXX_COMPILER_MINOR" = x; then
				BLOCXX_COMPILER_MINOR=0
			fi
			AC_DEFINE_UNQUOTED(COMPILER_MAJOR, [$BLOCXX_COMPILER_MAJOR], [Major version of the compiler])
			AC_DEFINE_UNQUOTED(COMPILER_MINOR, [$BLOCXX_COMPILER_MINOR], [Minor version of the compiler])
		fi
		AC_SUBST(BLOCXX_COMPILER_LIBDIR)
	]
)
