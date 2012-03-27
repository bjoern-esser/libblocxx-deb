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
 * @author Jon Carey
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/File.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/Format.hpp"

#ifdef BLOCXX_WIN32
	#include <io.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <memory.h>
#else
	#include <fcntl.h>
	#ifdef BLOCXX_HAVE_UNISTD_H
		#include <unistd.h>
	#endif
#endif


namespace BLOCXX_NAMESPACE
{
#ifdef BLOCXX_WIN32
namespace
{
/////////////////////////////////////////////////////////////////////////////
// implementation of lock functions
int
doLock(HANDLE hFile, bool doWait, DWORD lockType)
{
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	DWORD flags = lockType;
	if (!doWait)
	{
		flags |= LOCKFILE_FAIL_IMMEDIATELY;
	}

	OVERLAPPED ov;
	memset(&ov, 0, sizeof(ov));
	if (!LockFileEx(hFile, flags, 0, 0xffffffff,
		0xffffffff, &ov))
	{
		return -1;
	}

	return 0;
}

} // end unnamed namespace

/////////////////////////////////////////////////////////////////////////////
File::File(const File& x) : m_hdl(BLOCXX_INVALID_FILEHANDLE)
{
	if( x.m_hdl != BLOCXX_INVALID_FILEHANDLE )
	{
		DuplicateHandle(GetCurrentProcess(), x.m_hdl, GetCurrentProcess(),
			&m_hdl , 0, FALSE, DUPLICATE_SAME_ACCESS);
	}
}
/////////////////////////////////////////////////////////////////////////////
int 
File::getLock(ELockType type)
{
	return doLock(m_hdl, true,  type == E_WRITE_LOCK ?
	                            LOCKFILE_EXCLUSIVE_LOCK : 0);
}
/////////////////////////////////////////////////////////////////////////////
int
File::tryLock(ELockType type)
{
	return doLock(m_hdl, false, type == E_WRITE_LOCK ?
	                            LOCKFILE_EXCLUSIVE_LOCK : 0);
}
/////////////////////////////////////////////////////////////////////////////
int
File::unlock()
{
	if (m_hdl == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	OVERLAPPED ov;
	memset(&ov, 0, sizeof(ov));
	if (!UnlockFileEx(m_hdl, 0, 0xffffffff, 0xffffffff, &ov))
	{
		return -1;
	}

	return 0;
}

#else	// NOT WIN32

/////////////////////////////////////////////////////////////////////////////
File::File(const File& x)
	: m_hdl(x.m_hdl != BLOCXX_INVALID_FILEHANDLE ?
		dup(x.m_hdl) : BLOCXX_INVALID_FILEHANDLE)
{
}

namespace {
/////////////////////////////////////////////////////////////////////////////
// implementation of lock functions
int
doLock(int hdl, int cmd, short int type)
{
	struct flock lck;
	::memset (&lck, '\0', sizeof (lck));
	lck.l_type = type;          // type of lock
	lck.l_whence = 0;           // 0 offset for l_start
	lck.l_start = 0L;           // lock starts at BOF
	lck.l_len = 0L;             // extent is entire file
	return ::fcntl(hdl, cmd, &lck);
}
} // end unnamed namespace
/////////////////////////////////////////////////////////////////////////////
int 
File::getLock(ELockType type)
{
	return doLock(m_hdl, F_SETLKW, type == E_WRITE_LOCK ?
	                               F_WRLCK : F_RDLCK);
}
/////////////////////////////////////////////////////////////////////////////
int
File::tryLock(ELockType type)
{
	return doLock(m_hdl, F_SETLK, type == E_WRITE_LOCK ?
	                              F_WRLCK : F_RDLCK);
}
/////////////////////////////////////////////////////////////////////////////
int
File::unlock()
{
	return doLock(m_hdl, F_SETLK, F_UNLCK);
}
#endif

/////////////////////////////////////////////////////////////////////////////
File::~File()
{
	if (close() == -1)
	{
		int lerrno = errno;
		Logger lgr("blocxx.common");
		BLOCXX_LOG_ERROR(lgr,
			Format("Closing file handle %1 failed: %2",
				m_hdl, lerrno)
		);
		errno = lerrno;
	}
}

/////////////////////////////////////////////////////////////////////////////
#ifndef BLOCXX_WIN32
AutoDescriptor
File::releaseDescriptor()
{
	AutoDescriptor result( m_hdl );
	m_hdl = BLOCXX_INVALID_FILEHANDLE;
	return result;
}
#endif


} // end namespace BLOCXX_NAMESPACE

