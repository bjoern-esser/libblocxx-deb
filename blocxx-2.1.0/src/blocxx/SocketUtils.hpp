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
 * @author Dan Nuffer
 */

#ifndef BLOCXX_SOCKETUTILS_HPP_INCLUDE_GUARD_
#define BLOCXX_SOCKETUTILS_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Types.hpp"
#include "blocxx/NetworkTypes.hpp"
#include "blocxx/SocketFlags.hpp"
#include "blocxx/CommonFwd.hpp"

#ifndef ETIMEDOUT
#define ETIMEDOUT 110
#endif

#if defined(BLOCXX_NCR)
/* MP-RAS had no ECANCELED error code.
   We've done it like in bits/errno.h on Linux */
#ifndef ECANCELED
#define ECANCELED	160
#endif
#endif


namespace BLOCXX_NAMESPACE
{

namespace SocketUtils
{
#if defined(BLOCXX_WIN32)
	/**
	 * Wait for input or output on a socket.
	 * @param fd the handle of the socket to wait on.
	 * @param event The event associated with the socket that will
	 *		be signaled when I/O is available
	 * @param timeOutSecs the number of seconds to wait.
	 * @param networkEvents netWork events to wait for
	 * @return zero if we got input before the timeout expired,
	 *  -1 on error, and ETIMEDOUT on timeout.
	 */
	BLOCXX_COMMON_API int waitForIO(SocketHandle_t fd, HANDLE event,
		int timeOutSecs, long networkEvents=-1L) BLOCXX_DEPRECATED; // in 4.0.0

	/**
	 * Wait for input or output on a socket.
	 * @param fd the handle of the socket to wait on.
	 * @param event The event associated with the socket that will
	 *		be signaled when I/O is available
	 * @param timeOutSecs the number of seconds to wait.
	 * @param networkEvents netWork events to wait for
	 * @return zero if we got input before the timeout expired,
	 *  -1 on error, and ETIMEDOUT on timeout.
	 */
	BLOCXX_COMMON_API int waitForIO(SocketHandle_t fd, HANDLE event,
		const Timeout& timeout, long networkEvents=-1L);

#else
	/**
	 * Wait for input or output on a socket.
	 * @param fd the handle of the socket to wait on.
	 * @param timeOutSecs the number of seconds to wait.
	 * @param forInput true if we are waiting for input.
	 * @return zero if we got input before the timeout expired,
	 *  -1 on error, and ETIMEDOUT on timeout.
	 */
	int waitForIO(SocketHandle_t fd, int timeOutSecs,
		SocketFlags::EWaitDirectionFlag forInput) BLOCXX_DEPRECATED; // in 4.0.0
	/**
	 * Wait for input or output on a socket.
	 * @param fd the handle of the socket to wait on.
	 * @param timeOutSecs the number of seconds to wait.
	 * @param forInput true if we are waiting for input.
	 * @return zero if we got input before the timeout expired,
	 *  -1 on error, and ETIMEDOUT on timeout.
	 */
	int waitForIO(SocketHandle_t fd, const Timeout& timeout,
		SocketFlags::EWaitDirectionFlag forInput);
#endif

	BLOCXX_COMMON_API String inetAddrToString(UInt64 addr);
	/**
	 * Get the fully qualified host name.
	 * This function can be expensive performance-wise.  It may query multiple DNS servers.
	 * If the network is not working correctly, it will fail and throw an exception.
	 * @throws SocketException on failure.
	 */
	BLOCXX_COMMON_API String getFullyQualifiedHostName();
}

} // end namespace BLOCXX_NAMESPACE

namespace BLOCXX_SocketUtils = BLOCXX_NAMESPACE::SocketUtils;

#endif
