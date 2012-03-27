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


/**
 * @author Dan Nuffer
 */

#ifndef BLOCXX_PATHS_HPP_INCLUDE_GUARD_
#define BLOCXX_PATHS_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"

// The purpose of this file is to portably provide system paths from paths.h (BSD derived)

#ifdef BLOCXX_HAVE_PATHS_H
#include <paths.h>

// Standard paths for busted systems that have paths.h but don't define all the
// neccessary paths.
#if !defined(BLOCXX_HAVE_DECL__PATH_DEFPATH) || (BLOCXX_HAVE_DECL__PATH_DEFPATH == 0)
#define _PATH_DEFPATH   "/usr/bin:/bin"
#endif
#if !defined(BLOCXX_HAVE_DECL__PATH_STDPATH) || (BLOCXX_HAVE_DECL__PATH_STDPATH == 0)
#define _PATH_STDPATH   "/usr/bin:/bin:/usr/sbin:/sbin"
#endif

#else

#ifndef BLOCXX_WIN32
#define _PATH_DEFPATH   "/usr/bin:/bin"
#define _PATH_STDPATH   "/usr/bin:/bin:/usr/sbin:/sbin"
#else
#define _PATH_DEFPATH   "%WINDIR%;%WINDIR%/system32"
#define _PATH_STDPATH   _PATH_DEFPATH
#endif

#define _PATH_BSHELL    "/bin/sh"
#define _PATH_CONSOLE   "/dev/console"
#define _PATH_CSHELL    "/bin/csh"
#define _PATH_DEVDB     "/var/run/dev.db"
#define _PATH_DEVNULL   "/dev/null"
#define _PATH_DRUM      "/dev/drum"
#define _PATH_KLOG      "/proc/kmsg"
#define _PATH_KMEM      "/dev/kmem"
#define _PATH_LASTLOG   "/var/log/lastlog"
#define _PATH_MAILDIR   "/var/mail"
#define _PATH_MAN       "/usr/share/man"
#define _PATH_MEM       "/dev/mem"
#define _PATH_MNTTAB    "/etc/fstab"
#define _PATH_MOUNTED   "/etc/mtab"
#define _PATH_NOLOGIN   "/etc/nologin"
#define _PATH_PRESERVE  "/var/lib"
#define _PATH_RWHODIR   "/var/spool/rwho"
#define _PATH_SENDMAIL  "/usr/sbin/sendmail"
#define _PATH_SHADOW    "/etc/shadow"
#define _PATH_SHELLS    "/etc/shells"
#define _PATH_TTY       "/dev/tty"
#define _PATH_UNIX      "/boot/vmlinux"
#define _PATH_UTMP      "/var/run/utmp"
#define _PATH_VI        "/bin/vi"
#define _PATH_WTMP      "/var/log/wtmp"
#define _PATH_DEV       "/dev/"
#define _PATH_TMP       "/tmp/"
#define _PATH_VARDB     "/var/db/"
#define _PATH_VARRUN    "/var/run/"
#define _PATH_VARTMP    "/var/tmp/"



#endif

#endif

