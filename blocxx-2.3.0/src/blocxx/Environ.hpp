/*******************************************************************************
* Copyright (C) 2005, Quest Software, Inc. All rights reserved.
* Copyright (C) 2006, Novell, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of
*       Quest Software, Inc.,
*       nor Novell, Inc.,
*       nor the names of its contributors or employees may be used to
*       endorse or promote products derived from this software without
*       specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/


#ifndef BLOCXX_ENVIRON_HPP_INCLUDE_GUARD_
#define BLOCXX_ENVIRON_HPP_INCLUDE_GUARD_

/**
 * @author Kevin Harris
 * @author Jon Carey
 */

#include "blocxx/BLOCXX_config.h"

#if defined(BLOCXX_DARWIN)
// On MacOSX, environ is not defined in shared libraries.  It is only provided
// for executables.  Since the linker must resolve all symbols at link time,
// the normal extern declaration of environ won't work.  As a substitute, we
// need to use this:
#include <crt_externs.h>
#define environ (*_NSGetEnviron())

// For more details on the subject, follow one of these links:
// http://lists.gnu.org/archive/html/bug-guile/2004-01/msg00013.html
// http://lists.apple.com/archives/darwin-dev/2005/Mar/msg00132.html
// http://www.metapkg.org/wiki/15 (under the linking problems)

#else /* !DARWIN */
// according to man environ:
// This variable must be declared in the user program, but is
// declared in the header file unistd.h in case the header files came from
// libc4 or libc5, and in case they came from glibc and _GNU_SOURCE was
// defined.
BLOCXX_IMPORT extern char** environ;
#endif /* DARWIN */

#endif /* INCLUDE_GUARD */
