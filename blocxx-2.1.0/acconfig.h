/*******************************************************************************
* Copyright (C) 2001-3 Vintela, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
*  - Neither the name of Vintela, Inc. nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Vintela, Inc. OR THE CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/**
 * @author Dan Nuffer
 */

/* autoconf set variables here */

@TOP@

/* Defined if --enable-debug was passed to configure */
#undef _GLIBCXX_DEBUG

#if defined(__cplusplus)
 #if defined(BLOCXX__GLIBCXX_DEBUG) && !defined(_GLIBCXX_DEBUG)
#error "This is a debug build of BloCxx: You must compile with -D_GLIBCXX_DEBUG"
 #endif
 #if !defined(BLOCXX__GLIBCXX_DEBUG) && defined(_GLIBCXX_DEBUG)
#error "This is a non-debug build of BloCxx: You must not compile with -D_GLIBCXX_DEBUG"
 #endif
#endif

/* The system has OpenSSL headers and libs */
#undef HAVE_OPENSSL

/* The system doesn't have OpenSSL */
#undef NO_SSL

/* Dirs from configure */
#undef DEFAULT_SYSCONF_DIR
#undef DEFAULT_STATE_DIR
#undef DEFAULT_LIB_DIR
#undef DEFAULT_LIBEXEC_DIR

/* Either "" or the argument to --with-package-prefix */
#undef PACKAGE_PREFIX

/* Defined if --enable-stack-trace was passed to configure */
#undef ENABLE_STACK_TRACE_ON_EXCEPTIONS

/* Defined if --enable-debug was passed to configure */
#undef DEBUG

/* Defined if --enable-profile was passed to configure */
#undef PROFILE

/* Defined if we are building gnu pthreads version */
#undef USE_PTHREAD

/* Defined if we want to use custom "new" operator to debug memory leaks */
#undef DEBUG_MEMORY

/* Define if we want to print "Entering"/"Leaving" <functionName> */
#undef PRINT_FUNC_DEBUG

/* Define if we want to check for NULL references and throw an exception */
#undef CHECK_NULL_REFERENCES

/* Define if we want to check for valid array indexing and throw an exception */
#undef CHECK_ARRAY_INDEXING

/* Define which one is the current platform */
#undef GNU_LINUX
#undef OPENUNIX
#undef SOLARIS
#undef OPENSERVER
#undef DARWIN
#undef HPUX
#undef FREEBSD
#undef NCR


#undef u_int8_t
#undef u_int16_t
#undef u_int32_t
#undef HAVE_SOCKLEN_T

#undef HAVE_PTHREAD_MUTEXATTR_SETTYPE
#undef HAVE_PTHREAD_SPIN_LOCK
#undef HAVE_PTHREAD_BARRIER

/* Define if system has dlfcn.h */
#undef USE_DL
/* Define if system has shl_*() <dl.h> */
#undef USE_SHL
/* Define if a system has dyld.h (Mac OS X) */
#undef USE_DYLD

/* Define to enable workarounds so we can work with valgrind */
#undef VALGRIND_SUPPORT

/* Define to enable workarounds for non-thread safe exception handling support */
#undef NON_THREAD_SAFE_EXCEPTION_HANDLING

@BOTTOM@

/**
 * Make sure, that the large file support flags are set.
 */
#if  defined(BLOCXX__FILE_OFFSET_BITS) && \
    !defined(_FILE_OFFSET_BITS)
#define \
	_FILE_OFFSET_BITS  BLOCXX__FILE_OFFSET_BITS
#endif

#if  defined(BLOCXX__LARGE_FILES) && \
    !defined(_LARGE_FILES)
#define \
	_LARGE_FILES BLOCXX__LARGE_FILES
#endif

/* end of autoconf set vars */

/**
 * The BLOCXX_DEPRECATED macro can be used to trigger compile-time warnings
 * with gcc >= 3.2 when deprecated functions are used.
 *
 * For non-inline functions, the macro is used at the very end of the
 * function declaration, right before the semicolon, unless it's pure
 * virtual:
 *
 * int deprecatedFunc() const BLOCXX_DEPRECATED;
 * virtual int deprecatedPureVirtualFunc() const BLOCXX_DEPRECATED = 0;
 *
 * Functions which are implemented inline are handled differently:
 * the BLOCXX_DEPRECATED macro is used at the front, right before the return
 * type, but after "static" or "virtual":
 *
 * BLOCXX_DEPRECATED void deprecatedFuncA() { .. }
 * virtual BLOCXX_DEPRECATED int deprecatedFuncB() { .. }
 * static BLOCXX_DEPRECATED bool deprecatedFuncC() { .. }
 *
 * You can also mark whole structs or classes as deprecated, by inserting the
 * BLOCXX_DEPRECATED macro after the struct/class keyword, but before the
 * name of the struct/class:
 *
 * class BLOCXX_DEPRECATED DeprecatedClass { };
 * struct BLOCXX_DEPRECATED DeprecatedStruct { };
 * 
 * However, deprecating a struct/class doesn't create a warning for gcc 
 * versions <= 3.3 (haven't tried 3.4 yet).  If you want to deprecate a class,
 * also deprecate all member functions as well (which will cause warnings).
 *
 */
#if __GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2)
#define DEPRECATED __attribute__ ((deprecated))
#else
#define DEPRECATED
#endif

#ifdef BLOCXX_WIN32
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
#else
#define EXPORT
#define IMPORT
#endif

#ifdef BLOCXX_COMMON_BUILD
#define COMMON_API BLOCXX_EXPORT
#else
#define COMMON_API BLOCXX_IMPORT
#endif

#ifdef BLOCXX_WIN32
#define EXPORT_TEMPLATE(API, TMPL, X) template class API TMPL< X >
#else
#define EXPORT_TEMPLATE(API, TMPL, X)
#endif

/* C++ specific stuff here */
#ifdef __cplusplus

/* Need this first macro because ## doesn't expand vars, and we need an intermediate step */
#define NAMESPACE_CAT(bx, ver) bx ## ver
#define NAMESPACE_AUX(ver) BLOCXX_NAMESPACE_CAT(blocxx, ver)
#define NAMESPACE BLOCXX_NAMESPACE_AUX(BLOCXX_LIBRARY_VERSION)

/* need this to set up an alias. */
namespace BLOCXX_NAMESPACE
{
}

namespace blocxx = BLOCXX_NAMESPACE;

#ifdef BLOCXX_DEBUG_MEMORY
#include "blocxx/MemTracer.hpp"
#endif

/* For printing function names during debug */
#ifdef BLOCXX_PRINT_FUNC_DEBUG
#include "blocxx/FuncNamePrinter.hpp"

#define PRINT_FUNC_NAME BLOCXX_FuncNamePrinter fnp##__LINE__ (__PRETTY_FUNCTION__)

#define PRINT_FUNC_NAME_ARGS1(a) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a)
#define PRINT_FUNC_NAME_ARGS2(a, b) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b)
#define PRINT_FUNC_NAME_ARGS3(a, b, c) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c)
#define PRINT_FUNC_NAME_ARGS4(a, b, c, d) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c, d)
#define PRINT_FUNC_NAME_ARGS5(a, b, c, d, e) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c, d, e)
#define PRINT_FUNC_NAME_ARGS6(a, b, c, d, e, f) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c, d, e, f)
#define PRINT_FUNC_NAME_ARGS7(a, b, c, d, e, f, g) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c, d, e, f, g)
#define PRINT_FUNC_NAME_ARGS8(a, b, c, d, e, f, g, h) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c, d, e, f, g, h)
#define PRINT_FUNC_NAME_ARGS9(a, b, c, d, e, f, g, h, i) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c, d, e, f, g, h, i)
#define PRINT_FUNC_NAME_ARGS10(a, b, c, d, e, f, g, h, i, j) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c, d, e, f, g, h, i, j)
#else
#define PRINT_FUNC_NAME_ARGS1(a)
#define PRINT_FUNC_NAME_ARGS2(a, b)
#define PRINT_FUNC_NAME_ARGS3(a, b, c)
#define PRINT_FUNC_NAME_ARGS4(a, b, c, d)
#define PRINT_FUNC_NAME_ARGS5(a, b, c, d, e)
#define PRINT_FUNC_NAME_ARGS6(a, b, c, d, e, f)
#define PRINT_FUNC_NAME_ARGS7(a, b, c, d, e, f, g)
#define PRINT_FUNC_NAME_ARGS8(a, b, c, d, e, f, g, h)
#define PRINT_FUNC_NAME_ARGS9(a, b, c, d, e, f, g, h, i)
#define PRINT_FUNC_NAME_ARGS10(a, b, c, d, e, f, g, h, i, j)
#define PRINT_FUNC_NAME
#endif /* #ifdef BLOCXX_PRINT_FUNC_DEBUG */

#endif /* #ifdef __cplusplus */

