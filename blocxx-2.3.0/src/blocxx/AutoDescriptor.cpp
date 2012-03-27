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
* @author Kevin S. Van Horn
*/

#include "blocxx/BLOCXX_config.h"
#include "blocxx/AutoDescriptor.hpp"
#include "blocxx/System.hpp"

#if !defined(BLOCXX_NO_EXCEPTIONS)
#include "blocxx/Logger.hpp"
#include "blocxx/Format.hpp"
#endif

#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef BLOCXX_WIN32
#include <io.h>
#endif

#include <cerrno>

namespace BLOCXX_NAMESPACE
{
void AutoDescriptorPolicy::free(handle_type h)
{
#if defined(BLOCXX_WIN32)
	if (h != BLOCXX_INVALID_HANDLE)
	{
		if (!CloseHandle(h))
		{
#if !defined(BLOCXX_NO_EXCEPTIONS)
			String msg;
			System::lastErrorMsg("Closing handle", msg);

			Logger lgr("blocxx");
			BLOCXX_LOG_ERROR(lgr, msg);
#else
			// We can't do any logging without the possibility of throwing an exception.
#endif /* !defined(BLOCXX_NO_EXCEPTIONS) */
		}
	}
#else
	if (h >= 0)
	{
		int rc = 0;
		do
		{
			rc = ::close(h);
		} while (rc < 0 && errno == EINTR);

		if (rc == -1)
		{
#if !defined(BLOCXX_NO_EXCEPTIONS)
			int lerrno = errno;
			Logger lgr("blocxx");
			BLOCXX_LOG_ERROR(lgr, Format("Closing handle %1 failed: %2", h, lerrno));
#else
			// We can't do any logging without the possibility of throwing an exception.
#endif /* !defined(BLOCXX_NO_EXCEPTIONS) */
		}
	}
#endif /* defined(BLOCXX_WIN32) */
}
}
