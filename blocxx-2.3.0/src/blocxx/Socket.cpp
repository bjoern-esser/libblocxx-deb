/*******************************************************************************
* Copyright (C) 2005, 2010, Quest Software, Inc. All rights reserved.
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
 * @author Bart Whiteley
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Socket.hpp"
#include "blocxx/UnnamedPipe.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/MutexLock.hpp"
#include "blocxx/Exception.hpp"
#include "blocxx/IOException.hpp"
#include "blocxx/ExceptionIds.hpp"
#include "blocxx/SocketImpl.hpp"
#include "blocxx/GlobalMutex.hpp"


namespace BLOCXX_NAMESPACE
{

BLOCXX_DEFINE_EXCEPTION_WITH_ID(Socket);
BLOCXX_DEFINE_EXCEPTION_WITH_ID(SocketTimeout);

LazyGlobal<Socket::ShutDownMechanism_t, int, Socket::ShutDownMechanismFactory> Socket::s_shutDownMechanism = BLOCXX_LAZY_GLOBAL_INIT(0);

//////////////////////////////////////////////////////////////////////////////
Socket::Socket()
	: m_impl(new SocketImpl)
{
}

//////////////////////////////////////////////////////////////////////////////
Socket::~Socket()
{
}

static bool b_gotShutDown = false;
static GlobalMutex shutdownMutex = BLOCXX_GLOBAL_MUTEX_INIT();

//////////////////////////////////////////////////////////////////////////////
// STATIC
void
Socket::shutdownAllSockets()
{
	MutexLock mlock(shutdownMutex);

	BLOCXX_ASSERT(s_shutDownMechanism.get() != 0);
	if (!s_shutDownMechanism.get())
	{
		return;
	}

	b_gotShutDown = true;
#if defined(BLOCXX_WIN32)
	::SetEvent(s_shutDownMechanism);
#else
	if ((s_shutDownMechanism.get())->writeString("die!") == -1)
	{
		BLOCXX_THROW_ERRNO_MSG(IOException, "Failed writing to socket shutdown pipe");
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////
// STATIC
void
Socket::createShutDownMechanism()
{
	MutexLock mlock(shutdownMutex);
	if (!s_shutDownMechanism.get())
	{
#if defined(BLOCXX_WIN32)
		s_shutDownMechanism = ::CreateEvent(0, TRUE, FALSE, 0);
		BLOCXX_ASSERT(s_shutDownMechanism.get() != 0);
#else
		s_shutDownMechanism = UnnamedPipe::createUnnamedPipe();
		(s_shutDownMechanism.get())->setBlocking(UnnamedPipe::E_NONBLOCKING);
#endif
		b_gotShutDown = false;
	}
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
void
Socket::deleteShutDownMechanism()
{
	MutexLock mlock(shutdownMutex);
	if (s_shutDownMechanism.get())
	{
#if defined(BLOCXX_WIN32)
		::CloseHandle(s_shutDownMechanism);
#endif
		s_shutDownMechanism = static_cast<Socket::ShutDownMechanism_t>(0);
	}
}

} // end namespace BLOCXX_NAMESPACE

