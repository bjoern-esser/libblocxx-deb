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

#ifndef BLOCXX_SIGNALSCOPE_HPP_INCLUDE_GUARD_
#define BLOCXX_SIGNALSCOPE_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include <signal.h>

#ifdef BLOCXX_NCR
#undef SIG_IGN
#define	SIG_IGN	(void (*)())1
#endif

// The classes and functions defined in this file are not meant for general
// use, they are internal implementation details.  They may change at any time.

namespace BLOCXX_NAMESPACE
{

const int BLOCXX_SIGSEGV = SIGSEGV;
const int BLOCXX_SIGABRT = SIGABRT;
#ifdef SIGBUS // NetWare doesn't have this signal
const int BLOCXX_SIGBUS = SIGBUS;
#endif
const int BLOCXX_SIGFPE = SIGFPE;

extern "C" {
#ifdef BLOCXX_NCR
typedef void (*sighandler_t)(void);
#else
typedef void (*sighandler_t)(int);
#endif
}

class BLOCXX_COMMON_API SignalScope
{
public:
	SignalScope( int sig, sighandler_t handler )
			: m_sig( sig )
	{
#ifndef BLOCXX_WIN32
		struct sigaction saNew;
		saNew.sa_handler = handler;
		sigemptyset(&saNew.sa_mask);
		saNew.sa_flags = 0;
		sigaction(m_sig, &saNew, &m_oldHandler);
#endif
	}
	~SignalScope()
	{
#ifndef BLOCXX_WIN32
		sigaction(m_sig, &m_oldHandler, 0);
#endif
	}
private:
	SignalScope(const SignalScope&);
	const SignalScope& operator=(const SignalScope&);
	int m_sig;
#ifndef BLOCXX_WIN32
	struct sigaction m_oldHandler;
#endif
};

} // end namespace BLOCXX_NAMESPACE

#endif
