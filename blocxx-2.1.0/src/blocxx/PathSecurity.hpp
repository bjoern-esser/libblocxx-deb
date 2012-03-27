/*******************************************************************************
* Copyright (C) 2005, Vintela, Inc. All rights reserved.
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
*       Vintela, Inc., 
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
 * @author Anton Afanasiev
 */

#ifndef BLOCXX_PATHSECURITY_HPP_INCLUDE_GUARD_
#define BLOCXX_PATHSECURITY_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/String.hpp"
#ifdef BLOCXX_HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

namespace BLOCXX_NAMESPACE
{

enum EFileStatusReturn
{
	E_FILE_OK,
	E_FILE_BAD_OWNER,
	E_FILE_BAD_OTHER
};

#ifdef BLOCXX_WIN32

#define S_IRUSR 400       /* Read by owner.  */
#define S_IWUSR 200      /* Write by owner.  */
#define S_IXUSR 100       /* Execute by owner.  */
/* Read, write, and execute by owner.  */
#define S_IRWXU (S_IRUSR|S_IWUSR|S_IXUSR)

#define S_IRGRP (S_IRUSR / 10)  /* Read by group.  */
#define S_IWGRP (S_IWUSR / 10)  /* Write by group.  */
#define S_IXGRP (S_IXUSR / 10)  /* Execute by group.  */
/* Read, write, and execute by group.  */
#define S_IRWXG (S_IRGRP|S_IWGRP|S_IXGRP)

#define S_IROTH (S_IRGRP / 10)  /* Read by others.  */
#define S_IWOTH (S_IWGRP / 10)  /* Write by others.  */
#define S_IXOTH (S_IXGRP / 10)  /* Execute by others.  */
/* Read, write, and execute by others.  */
#define S_IRWXO (S_IROTH|S_IWOTH|S_IXOTH)

#define S_ISVTX 0x0001000			/* sticky bit (see below) */
#define S_ISDIR(__PARM__) (_S_IFDIR & __PARM__)
#define S_ISREG(__PARM__) (_S_IFREG & __PARM__)
#define S_ISLNK(__PARM__) (false)	/*TODO: implement hardlinks */

#ifndef ELOOP
#define ELOOP           40			/* Too many symbolic links encountered */
#endif

static const unsigned long BLOCXX_WIN32_ACCESSMASK_FILE_READ_DATA    = 1;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_FILE_WRITE_DATA   = 1<<1;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_FILE_APPEND_DATA  = 1<<2;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_FILE_READ_EA      = 1<<3;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_FILE_WRITE_EA     = 1<<4;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_FILE_EXEC         = 1<<5;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_FILE_DELETE_CHILD = 1<<6;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_FILE_READ_ATTRS   = 1<<7;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_FILE_WRITE_ATTRS  = 1<<8;

static const unsigned long BLOCXX_WIN32_ACCESSMASK_DELETE            = 1<<16;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_READ_CONTROL      = 1<<17;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_WRITE_DAC         = 1<<18;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_WRITE_OWNER       = 1<<19;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_SYNCHRONIZE       = 1<<20;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_SYSSECURITY       = 1<<24;

static const unsigned long BLOCXX_WIN32_ACCESSMASK_GENERIC_ALL       = 1<<28;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_GENERIC_EXEC      = 1<<29;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_GENERIC_WRITE     = 1<<30;
static const unsigned long BLOCXX_WIN32_ACCESSMASK_GENERIC_READ      = 1<<31;

static const unsigned long BLOCXX_WIN32_ACCESSMASK_ALLOW_ANY_CHANGE  = \
			BLOCXX_WIN32_ACCESSMASK_GENERIC_ALL | \
			BLOCXX_WIN32_ACCESSMASK_GENERIC_WRITE | \
			BLOCXX_WIN32_ACCESSMASK_WRITE_DAC | \
			BLOCXX_WIN32_ACCESSMASK_FILE_WRITE_ATTRS | \
			BLOCXX_WIN32_ACCESSMASK_FILE_WRITE_DATA | \
			BLOCXX_WIN32_ACCESSMASK_FILE_APPEND_DATA | \
			BLOCXX_WIN32_ACCESSMASK_FILE_WRITE_EA ;


#endif //ifdef BLOCXX_WIN32

#if defined(BLOCXX_NETWARE) || defined(BLOCXX_WIN32)
  #define LSTAT ::stat
  #define S_ISLNK(x) false
  #define READLINK(path, buf, size) 0
  #define READLINK_ALLOWED false
#else
  #define LSTAT ::lstat
  #define READLINK(path, buf, size) ::readlink((path), (buf), (size))
  #define READLINK_ALLOWED true
#endif


BLOCXX_COMMON_API bool isPathAbsolute(String const & path);
/** GetFileStatus() - just to unify the call of file_ok() for Win and xNix */
EFileStatusReturn getFileStatus(struct stat const & x, uid_t uid, bool is_full_path, const String& path);

} // end namespace BLOCXX_NAMESPACE

#endif
