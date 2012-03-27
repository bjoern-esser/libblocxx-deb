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

#include "blocxx/BLOCXX_config.h"
#include "blocxx/SignalUtils.hpp"
#include "blocxx/String.hpp"
#include <algorithm>

#include <signal.h>

namespace BLOCXX_NAMESPACE
{
	namespace SignalUtils
	{
		namespace // anonymous
		{
			struct signalNameMapping
			{
				int sig;
				const char* name;
			};

			// Simple comparison functions so that searching (and sorting, if
			// desired) can be done on a set of signal name mappings.
			bool operator <(const signalNameMapping& s1, const signalNameMapping& s2)
			{
				return s1.sig < s2.sig;
			}
			bool operator ==(const signalNameMapping& s1, const signalNameMapping& s2)
			{
				return s1.sig == s2.sig;
			}

			// An array (not neccessarily sorted) of signal mappings.  This is
			// really ugly, but various platforms duplicate signal numbers with
			// different constants.  Not all platforms duplicate them in the same
			// way, so a general case statement became difficult to manage.
			const signalNameMapping supportedSignalMapping[] =
				{
					{ SIGINT, "SIGINT" },
					{ SIGILL, " SIGILL" },
					{ SIGABRT, "SIGABRT" },
					{ SIGFPE, " SIGFPE" },
					{ SIGSEGV, "SIGSEGV" },
					{ SIGTERM, "SIGTERM" },
#ifdef BLOCXX_WIN32
					{ SIGBREAK, "SIGBREAK" }
#else
					{ SIGHUP, "SIGHUP" },
					{ SIGQUIT, "SIGQUIT" },
					{ SIGKILL, "SIGKILL" },
					{ SIGPIPE, "SIGPIPE" },
					{ SIGALRM, "SIGALRM" },
					{ SIGUSR1, "SIGUSR1" },
					{ SIGUSR2, "SIGUSR2" },
					{ SIGCHLD, "SIGCHLD" },
					{ SIGCONT, "SIGCONT" },
					{ SIGSTOP, "SIGSTOP" },
					{ SIGTSTP, "SIGTSTP" },
					{ SIGTTIN, "SIGTTIN" },
					{ SIGTTOU, "SIGTTOU" },
					{ SIGBUS, "SIGBUS" },
#ifdef SIGPOLL
					{ SIGPOLL, "SIGPOLL" },
#endif
					{ SIGPROF, "SIGPROF" },
					{ SIGSYS, "SIGSYS" },
					{ SIGTRAP, "SIGTRAP" },
					{ SIGURG, "SIGURG" },
					{ SIGVTALRM, "SIGVTALRM" },
					{ SIGXCPU, "SIGXCPU" },
					{ SIGXFSZ, "SIGXFSZ" },
#ifdef SIGEMT
					{ SIGEMT, "SIGEMT" },
#endif
#ifdef SIGSTKFLT
					{ SIGSTKFLT, "SIGSTKFLT" },
#endif
#ifdef SIGPWR
					{ SIGPWR, "SIGPWR" },
#endif
#ifdef SIGLOST
					{ SIGLOST, "SIGLOST" },
#endif
					{ SIGWINCH, "SIGWINCH" },
#ifdef SIGINFO
					{ SIGINFO, "SIGINFO" },
#endif
					{ SIGIOT, "SIGIOT" },
					{ SIGIO, "SIGIO" },
#ifdef SIGCLD
					{ SIGCLD, "SIGCLD" },
#endif
#ifdef SIGUNUSED
					{ SIGUNUSED, "SIGUNUSED" }
#endif
#endif // for	#ifdef BLOCXX_WIN32....#else...
				};
		} // end anonymous namespace

		const char* signalName(int sig)
		{
			signalNameMapping targetSignal = { sig, "" };

			const signalNameMapping* begin = supportedSignalMapping;
			const signalNameMapping* end = begin + sizeof(supportedSignalMapping) / sizeof(*supportedSignalMapping);

			const signalNameMapping* location = std::find(begin, end, targetSignal);

			if( location != end )
			{
				return location->name;
			}
			return "UNKNOWN";
		}
	} // end namespace SignalUtils
} // end namespace BLOCXX_NAMESPACE





