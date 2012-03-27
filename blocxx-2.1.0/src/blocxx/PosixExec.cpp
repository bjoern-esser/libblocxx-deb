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

#include "blocxx/BLOCXX_config.h"

#if !defined(BLOCXX_WIN32) 

#include "blocxx/PosixExec.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/SafeCString.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/PosixUnnamedPipe.hpp"
#include "blocxx/Paths.hpp"
#include "blocxx/TimeoutTimer.hpp"
#include "blocxx/Select.hpp"

extern "C"
{
#ifdef BLOCXX_HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef BLOCXX_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h> // for perror
#include <signal.h>
}

// NSIG may be defined by signal.h, otherwise 64 should be plenty.
#ifndef NSIG
#define NSIG 64
#endif

#if defined(sigemptyset)
// We want to use the function instead of the macro (for scoping reasons).
#undef sigemptyset
#endif // sigemptyset

#ifdef BLOCXX_NCR
#if defined(sigaction)
#undef sigaction
#endif
#undef SIG_DFL
#define	SIG_DFL	(void(*)())0
#endif

namespace BLOCXX_NAMESPACE
{

namespace // anonymous
{

	void throw_child_error(Exec::PreExec::Error const & err, const String& process_path)
	{
		Format msg("Exec::spawn(%1): child startup failed: %2", process_path, err.message);
		if (err.error_num != 0)
		{
			BLOCXX_THROW_ERRNO_MSG1(
				ExecErrorException,	msg.c_str(), err.error_num);
		}
		else
		{
			BLOCXX_THROW(ExecErrorException, msg.c_str());
		}
	}

	void check(bool b, char const * message, bool use_errno = true)
	{
		if (!b)
		{
			Exec::PreExec::Error x;
			SafeCString::strcpy_trunc(x.message, message);
			x.error_num = use_errno ? errno : 0;
			throw x;
		}
	}

	void parent_check(bool b, char const * msg)
	{
		if (!b)
		{
			BLOCXX_THROW(ExecErrorException, msg);
		}
	}

	void close_on_exec(Descriptor descr, bool may_be_bad)
	{
		int e = ::fcntl(descr, F_SETFD, FD_CLOEXEC);
		check(e == 0 || may_be_bad && errno == EBADF, "fcntl");
	}

	void handle_child_error(int rc, Exec::PreExec::Error const & ce, Process & proc, const String& process_path)
	{
		if (rc < 0) // read of error status from child failed
		{
			int errnum = errno;
			// For some reason child initialization failed; kill it.
			proc.waitCloseTerm(Timeout::relative(0.0), Timeout::relative(0.0), Timeout::relative(0.0));
			if (errnum == ETIMEDOUT)
			{
				BLOCXX_THROW(ExecErrorException,
					Format("Exec::spawn(%1): timed out waiting for child to exec()",process_path).c_str());
			}
			BLOCXX_THROW_ERRNO_MSG1(ExecErrorException,
				Format("Exec::spawn(%1): error reading init status from child",process_path).c_str(), errnum);
		}
		if (rc > 0) // child sent an initialization error message
		{
			throw_child_error(ce, process_path);
		}
		// If rc == 0, initialization succeeded
	}

	long getMaxOpenFiles()
	{
		long sysconfValue = sysconf(_SC_OPEN_MAX);
		long maxOpen = sysconfValue;
		rlimit rl;
		rl.rlim_cur = rlim_t(0);
		if( getrlimit(RLIMIT_NOFILE, &rl) != -1 )
		{
			if( sysconfValue < 0 )
			{
				maxOpen = rl.rlim_cur;
			}
			else
			{
				maxOpen = std::min<rlim_t>(rl.rlim_cur, sysconfValue);
			}
		}
		// Check for a value of maxOpen that really is reasonable.
		// This checks the maximum value to make sure it will fit in an int
		// (required for close).
		BLOCXX_ASSERT( (maxOpen > 2) && (maxOpen <= long(std::numeric_limits<int>::max())) );
		return maxOpen;
	}

	void init_child(char const * exec_path, 
		char const * const argv[], char const * const envp[],
		Exec::PreExec & pre_exec, UnnamedPipe* ppipe[Exec::Impl::BLOCXX_NPIPE])
	{
		// This code must be careful not to allocate memory, as this can
		// cause a deadlock on some platforms when there are multiple
		// threads running at the time of the fork().

		int exec_err_desc = -1;
		Exec::PreExec::Error err;
		err.error_num = 0;      // should be unnecessary, but just in case...
		err.message[0] = '\0';  // should be unnecessary, but just in case...
		try
		{
			int rc;
			exec_err_desc = ppipe[Exec::Impl::BLOCXX_EXEC_ERR]->getOutputDescriptor();
			pre_exec.call(ppipe);

			int rval = 0;
			char * const * cc_argv = const_cast<char * const *>(argv);
			char * const * cc_envp = const_cast<char * const *>(envp);
			if (envp)
			{
				check(::execve(exec_path, cc_argv, cc_envp) != -1, "execve");
			}
			else
			{
				check(::execv(exec_path, cc_argv) != -1, "execv");
			}
		}
		catch (Exec::PreExec::Error & e)
		{
			err = e;
		}
		catch (std::exception & e)
		{
			SafeCString::strcpy_trunc(err.message, e.what());
			err.error_num = 0;
		}
		catch (Exec::PreExec::DontCatch & e)
		{
			throw;
		}
		catch (...)
		{
			SafeCString::strcpy_trunc(err.message, "unknown exception");
			err.error_num = 0;
		}
		ssize_t rv = ::write(exec_err_desc, &err, sizeof(err));
		::_exit(127);
	}

} // end anonymous namespace

namespace Exec
{

using namespace Impl;

/////////////////////////////////////////////////////////////////////////////////////////////
//  PreExec methods
//
void PreExec::resetSignals()
{
	/*
	according to susv3:

	This  volume  of  IEEE Std 1003.1-2001  specifies  that signals set to
	SIG_IGN remain set to SIG_IGN, and that  the  process  signal  mask be
	unchanged across an exec. This is consistent with historical implemen-
	tations, and it permits some useful functionality, such  as  the nohup
	command.  However,  it should be noted that many existing applications
	wrongly assume that they start with certain signals set to the default
	action  and/or  unblocked.  In particular, applications written with a
	simpler signal model that does not include blocking of signals, such as
	the one in the ISO C standard, may not behave properly if invoked with
	some signals blocked. Therefore, it is best not to block or ignore sig-
	nals across execs without explicit reason to do so, and especially not
	to block signals across execs of arbitrary (not  closely co-operating)
	programs.

	so we'll reset the signal mask and all signal handlers to SIG_DFL.
	We set them all just in case the current handlers may misbehave now
	that we've fork()ed.
	*/
	int rc;
	::sigset_t emptymask;
	check(::sigemptyset(&emptymask) == 0, "sigemptyset");
	check(::sigprocmask(SIG_SETMASK, &emptymask, 0) == 0, "sigprocmask");

	for (std::size_t sig = 1; sig <= NSIG; ++sig)
	{
		if (sig == SIGKILL || sig == SIGSTOP)
		{
			continue;
		}
		struct sigaction temp;
		int e = ::sigaction(sig, 0, &temp);
		check(e == 0 || errno == EINVAL, "sigaction [1]");
		if (e == 0 && temp.sa_handler != SIG_DFL) // valid signal
		{
			temp.sa_handler = SIG_DFL;
			// note that we don't check the return value because there are signals 
			// (e.g. SIGGFAULT on HP-UX), which are gettable, but not settable.
			::sigaction(sig, &temp, 0);
		}
	}
}

void PreExec::closeDescriptorsOnExec(std::vector<bool> const & keep)
{
	long numd = m_max_descriptors ? m_max_descriptors : getMaxOpenFiles();
	for (int d = 3; d < int(numd); ++d) // Don't close standard descriptors
	{
		if (size_t(d) >= keep.size() || !keep[d])
		{
			close_on_exec(d, true);
		}
	}
}

void PreExec::setupStandardDescriptors(pipe_pointer_t const ppipe[])
{
	int nulld = 0;
	if (!(ppipe[0] && ppipe[1] && ppipe[2]))
	{
		nulld = ::open(_PATH_DEVNULL, O_RDWR);
		check(nulld >= 0, "open");
		close_on_exec(nulld, false);
	}
	for (unsigned d = 0; d < 3; ++d)
	{
		PosixUnnamedPipe * p = dynamic_cast<PosixUnnamedPipe*>(ppipe[d]);
		int ddup =
			!p ? nulld : d==BLOCXX_IN ? p->getInputHandle() : p->getOutputHandle();
		check(::dup2(ddup, d) != -1, "dup2");
	}
}

void PreExec::closePipesOnExec(pipe_pointer_t const ppipe[])
{
	for (unsigned d = 0; d < BLOCXX_NPIPE; ++d)
	{
		UnnamedPipe* p = ppipe[d];
		if (p)
		{
			close_on_exec(p->getInputDescriptor(), false);
			close_on_exec(p->getOutputDescriptor(), false);
		}
	}
}

void PreExec::setNewProcessGroup()
{
	int pgidrv = setpgid(0, 0);
	BLOCXX_ASSERT(pgidrv == 0);
}

PreExec::PreExec(bool precompute_max_descriptors)
	: m_max_descriptors(precompute_max_descriptors ? getMaxOpenFiles() : 0)
{
}

PreExec::~PreExec()
{
}

PreExec::DontCatch::~DontCatch()
{
}

} // end Exec namespace

namespace PosixExec
{
//----------- Standard PreExec -------------
//------------------------------------------
StandardPreExec::StandardPreExec() : PreExec(true)
{
}

bool StandardPreExec::keepStd(int) const
{
	return true;
}

void StandardPreExec::call(pipe_pointer_t const pparr[])
{
	std::vector<bool> empty;
	PreExec::resetSignals();
	PreExec::setNewProcessGroup();
	PreExec::setupStandardDescriptors(pparr);
	PreExec::closeDescriptorsOnExec(empty);
}

//----------- System PreExec ---------------
//------------------------------------------
SystemPreExec::SystemPreExec() : PreExec(true) 
{ 
}

bool SystemPreExec::keepStd(int d) const 
{
	return true; // want them all unchanged
}

void SystemPreExec::call(pipe_pointer_t const pparr[])
{
	std::vector<bool> empty;
	PreExec::resetSignals();
	PreExec::setNewProcessGroup();
	PreExec::closeDescriptorsOnExec(empty);
}

ProcessRef spawnImpl(char const * exec_path, char const * const argv[], char const * const envp[],
					 Exec::PreExec & pre_exec)
{
	// It's important that this code be exception-safe (proper release
	// of resources when exception thrown), as at least one caller
	// (the monitor code) relies on being able to throw a DontCatch-derived
	// exception from pre_exec.call() in the child process and have
	// it propagate out of the spawn call.
	//
	parent_check(exec_path, "Exec::spawn: null exec_path");
	char const * default_argv[2] = { exec_path, 0 };
	if (!argv || !*argv)
	{
		argv = default_argv;
	}

	// Check this here so that any exceptions or core files caused by it can
	// be traced to a real problem instead of the child processes just
	// failing for an unreportable reason.
	getMaxOpenFiles();

	UnnamedPipeRef upipe[Exec::BLOCXX_NPIPE];
	UnnamedPipe* ppipe[Exec::BLOCXX_NPIPE] = {0};

	for (unsigned i = 0; i < Exec::BLOCXX_NPIPE; ++i)
	{
		if (i == Exec::BLOCXX_EXEC_ERR || pre_exec.keepStd(i))
		{
			upipe[i] = UnnamedPipe::createUnnamedPipe();
			ppipe[i] = upipe[i].getPtr();
		}
	}


	::pid_t child_pid = ::fork();
	if (child_pid == 0) // child process
	{
		init_child(exec_path, argv, envp, pre_exec, ppipe); // never returns
	}

	parent_check(child_pid >= 0, Format("Exec::spawn(%1): fork() failed", exec_path).c_str());

	Exec::close_child_ends(upipe);

	// 10 seconds should be plenty for the child to go from fork() to execv()
	const Timeout SECONDS_TO_WAIT_FOR_CHILD_TO_EXEC = Timeout::relative(10);
	upipe[Exec::BLOCXX_EXEC_ERR]->setReadTimeout(SECONDS_TO_WAIT_FOR_CHILD_TO_EXEC);

	ProcessRef retval(new Process(upipe[0], upipe[1], upipe[2], child_pid));


	Exec::PreExec::Error child_error;
	int nread = upipe[Exec::BLOCXX_EXEC_ERR]->read(&child_error, sizeof(child_error));
	handle_child_error(nread, child_error, *retval, exec_path);

	return retval;
}

} // end PosixExec namespace

} // end BLOCXX_NAMESPACE

#endif
