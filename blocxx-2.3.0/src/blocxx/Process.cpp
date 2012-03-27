/*******************************************************************************
* Copyright (C) 2009, Quest Software, Inc. All rights reserved.
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
*       nor Network Associates,
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
* @author Dan Nuffer (code modified from Exec.cpp)
*/

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Process.hpp"
#include "blocxx/DateTime.hpp"
#include "blocxx/Exec.hpp"
  // To get ExecErrorException declaration
#include "blocxx/Format.hpp"
#include "blocxx/SafeCString.hpp"
#include "blocxx/String.hpp"
#include "blocxx/Thread.hpp"
#include "blocxx/UnnamedPipe.hpp"
#include "blocxx/Paths.hpp"
#include "blocxx/TimeoutTimer.hpp"
#include "blocxx/SignalUtils.hpp"
#include "blocxx/ThreadPool.hpp"
#include "blocxx/Runnable.hpp"
#include "blocxx/LazyGlobal.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/GlobalString.hpp"
#include "blocxx/WaitpidThreadFix.hpp"
#include "blocxx/System.hpp"

#ifdef BLOCXX_WIN32
#include "blocxx/WinProcessUtils.hpp"
#else
#include <sys/wait.h>
#endif

#include <fcntl.h>
#include <signal.h>
#include <cerrno>
#include <cmath>
#include <algorithm>
#include <limits>
#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(sigemptyset)
// We want to use the function instead of the macro (for scoping reasons).
#undef sigemptyset
#endif // sigemptyset

namespace BLOCXX_NAMESPACE
{

static const char* TERM_MESSAGE = "Terminate Process";

namespace
{
	GlobalString COMPONENT_NAME = BLOCXX_GLOBAL_STRING_INIT("blocxx.Process");
}

// This function is called by both
// ProcessChildImpl::pollStatus and WaitpidThreadFix::waitPid
Process::Status pollStatusImpl(ProcId pid);

BLOCXX_DEFINE_EXCEPTION(ProcessError);

// -------------------- Process::Status ---------------------------
//-----------------------------------------------------------------

Process::Status::Status(ProcId pid, int status)
: m_status_available(pid > 0),
  m_status(status)
{
}

Process::Status::Status(int rep1, int rep2, Repr)
: m_status_available(static_cast<bool>(rep1)),
  m_status(rep2)
{
}

#ifdef BLOCXX_WIN32

Process::Status::Status() : m_status_available(false), m_status(STILL_ACTIVE)
{
}

bool Process::Status::running() const
{
	return m_status == STILL_ACTIVE;
}

bool Process::Status::terminated() const
{
	return m_status != STILL_ACTIVE;
}

bool Process::Status::exitTerminated() const
{
	return m_status != STILL_ACTIVE;
}

int Process::Status::exitStatus() const
{
	return m_status;
}

int Process::Status::getPOSIXwaitpidStatus() const
{
	return m_status;
}

bool Process::Status::signalTerminated() const
{
	return false;
}

int Process::Status::termSignal() const
{
	return -1;
}

bool Process::Status::stopped() const
{
	return false;
}

int Process::Status::stopSignal() const
{
	return -1;
}

#else

Process::Status::Status() : m_status_available(false), m_status(0)
{
}

bool Process::Status::running() const
{
	return !m_status_available;
}

bool Process::Status::terminated() const
{
	return m_status_available && (WIFEXITED(m_status) || WIFSIGNALED(m_status));
}

bool Process::Status::exitTerminated() const
{
	return m_status_available && WIFEXITED(m_status);
}

int Process::Status::exitStatus() const
{
	return WEXITSTATUS(m_status);
}

int Process::Status::getPOSIXwaitpidStatus() const
{
	return m_status;
}

bool Process::Status::signalTerminated() const
{
	return m_status_available && WIFSIGNALED(m_status);
}

int Process::Status::termSignal() const
{
	return WTERMSIG(m_status);
}

bool Process::Status::stopped() const
{
	return m_status_available && WIFSTOPPED(m_status);
}

int Process::Status::stopSignal() const
{
	return WSTOPSIG(m_status);
}

#endif

void Process::Status::repr(int & rep1, int & rep2) const
{
	rep1 = static_cast<int>(m_status_available);
	rep2 = m_status;
}

bool Process::Status::terminatedSuccessfully() const
{
	return exitTerminated() && exitStatus() == 0;
}

String Process::Status::toString() const
{
	if (running())
	{
		return "running";
	}
	else if (stopped())
	{
		return Format("stopped by %1", SignalUtils::signalName(stopSignal()));
	}
	else if (terminated())
	{
		if (exitTerminated())
		{
			return Format("exited with status %1", String(exitStatus()));
		}
		else if (signalTerminated())
		{
			return Format("terminated by signal %1", SignalUtils::signalName(termSignal()));
		}
	}
	return "Unknown";
}

//-----------------------------------------------------------------------------

ProcessImpl::~ProcessImpl()
{
}

namespace
{

class ChildProcessImpl : public ProcessImpl
{
public:
	virtual int kill(ProcId pid, int sig)
	{
#ifdef BLOCXX_WIN32
		return -1;
#else
		return ::kill(pid, sig) == 0 ? 0 : errno;
#endif
	}

	virtual Process::Status pollStatus(ProcId pid)
	{
		if (WaitpidThreadFix::shouldUseWaitpidThreadFix())
		{
			return WaitpidThreadFix::waitPid(pid);
		}
		return pollStatusImpl(pid);
	}
};


struct ZombieReaperPoolCreator
{
	static ThreadPool* create(int dummy)
	{
		return new ThreadPool(ThreadPool::DYNAMIC_SIZE, (std::numeric_limits<UInt32>::max)(), 0);
	}
};
LazyGlobal<ThreadPool, int, ZombieReaperPoolCreator> g_zombieReaperPool = BLOCXX_LAZY_GLOBAL_INIT(0);

class ZombieReaper : public Runnable
{
public:
	ZombieReaper(ProcId pid, const ProcessImplRef& impl, const String& initialReason)
	: m_pid(pid)
	, m_impl(impl)
	, m_reason(initialReason)
	{
	}
	virtual void run()
	{
		Logger lgr(COMPONENT_NAME);
		BLOCXX_LOG_DEBUG(lgr, Format("ZombieReaper getting status for %1.", m_pid));
		Process::Status status = m_impl->pollStatus(m_pid);
		size_t attempts = 0;
		const size_t MAX_ZOMBIE_POLL_ATTEMPTS = 6;
		while (!status.terminated())
		{
			// Ask the process to quit.
			try
			{
				int signum;
				if( attempts++ < MAX_ZOMBIE_POLL_ATTEMPTS )
				{
					BLOCXX_LOG_DEBUG(lgr, Format("ZombieReaper sending polite term request to %1 (attempt #%2) -- Original reason: %3", m_pid, attempts, m_reason));
					signum = SIGTERM;
				}
				else
				{
					BLOCXX_LOG_DEBUG(lgr, Format("ZombieReaper sending immediate death request to %1 (attempt #%2) -- Original reason: %3", m_pid, attempts, m_reason));
					signum = SIGKILL;
				}

				m_impl->kill(m_pid, signum);
			}
			catch(const Exception& e)
			{
				BLOCXX_LOG_DEBUG(lgr, Format("ZombieReaper failed to send signal to %1: %2", m_pid, e));
			}
			catch(...)
			{
				BLOCXX_LOG_DEBUG(lgr, Format("ZombieReaper failed to send signal to %1", m_pid));
			}

			Thread::sleep(Timeout::relative(10));
			BLOCXX_LOG_DEBUG(lgr, Format("ZombieReaper getting status for %1.", m_pid));
			status = m_impl->pollStatus(m_pid);
		}
		BLOCXX_LOG_DEBUG(lgr, Format("ZombieReaper got status for %1: %2.", m_pid, status.toString()));
	}
private:
	ProcId m_pid;
	ProcessImplRef m_impl;
	String m_reason;
};

} // end unnamed namespace


// --- Process ---

Process::Process(
	UnnamedPipeRef const & in, UnnamedPipeRef const & out,
	UnnamedPipeRef const & err, ProcId pid
)
: m_impl(new ChildProcessImpl())
, m_in(in)
, m_out(out)
, m_err(err)
, m_pid(pid)
, m_status()
{
}

Process::Process(
	const ProcessImplRef& impl, UnnamedPipeRef const & in, UnnamedPipeRef const & out,
	UnnamedPipeRef const & err, ProcId pid
)
: m_impl(impl)
, m_in(in)
, m_out(out)
, m_err(err)
, m_pid(pid)
, m_status()
{
}


Process::Process(ProcId pid)
: m_impl(new ChildProcessImpl())
, m_in()
, m_out()
, m_err()
, m_pid(pid)
, m_status()
{
}

Process::~Process()
{
	if (m_pid < 0)
	{
		return;
	}
	try
	{
		this->waitCloseTerm(Timeout::relative(0.0), Timeout::relative(1.0), Timeout::relative(5.0));
	}
	catch (Exception& e)
	{
		Logger lgr(COMPONENT_NAME);
		BLOCXX_LOG_DEBUG(lgr, Format("Process::~Process caught %1 from waitCloseTerm()", e));
		// Make a last ditch attempt to prevent zombies.
		if (!m_status.terminated())
		{
			BLOCXX_LOG_DEBUG(lgr, Format("Process %1 didn't exit cleanly. Creating a ZombieReaper for it.", m_pid));
			static_cast<ThreadPool>(g_zombieReaperPool).addWork(new ZombieReaper(m_pid, m_impl, Format("Exception caught: %1", e)));
		}
	}
	catch (...)
	{
		// Make a last ditch attempt to prevent zombies.
		if (!m_status.terminated())
		{
			Logger lgr(COMPONENT_NAME);
			BLOCXX_LOG_DEBUG(lgr, Format("Process %1 didn't exit cleanly. Creating a ZombieReaper for it.", m_pid));
			static_cast<ThreadPool>(g_zombieReaperPool).addWork(new ZombieReaper(m_pid, m_impl, "Unknown exception caught"));
		}
	}
}

void Process::release()
{
	m_in = 0;
	m_out = 0;
	m_err = 0;
	m_pid = BLOCXX_INVALID_HANDLE;
}

UnnamedPipeRef Process::in() const
{
	return m_in;
}

UnnamedPipeRef Process::out() const
{
	return m_out;
}

UnnamedPipeRef Process::err() const
{
	return m_err;
}

ProcId Process::pid() const
{
	return m_pid;
}

Process::Status Process::processStatus()
{
	// m_pid tested in case this method is called inappropriately
	if (m_pid >= 0 && !m_status.terminated())
	{
		m_status = m_impl->pollStatus(m_pid);
	}
	return m_status;
}

namespace
{
	inline void upr_close(UnnamedPipeRef & x)
	{
		if (x)
		{
			x->close();
		}
	}
}

void Process::waitCloseTerm(float wait_initial, float wait_close, float wait_term)
{
	waitCloseTerm(Timeout::relative(wait_initial), Timeout::relative(wait_close), Timeout::relative(wait_term));
}

void Process::waitCloseTerm(const Timeout& wait_initial, const Timeout& wait_close, const Timeout& wait_term,
							ETerminationSelectionFlag terminationSelectionFlag)
{
	if (m_pid < 0) // safety check in case called inappropriately
	{
		return;
	}

	processStatus(); // update m_status

	if (m_status.terminated())
	{
		return;
	}

	if (m_pid == getCurProcessId())
	{
		BLOCXX_THROW(ProcessErrorException, "Process::m_pid == the current process id");
	}

	TimeoutTimer initialTimer(wait_initial);
	TimeoutTimer closeTimer(wait_close);
	TimeoutTimer termTimer(wait_term);

	if (wait_initial.getType() == Timeout::E_RELATIVE && wait_initial.getRelative() > 0 && this->terminatesWithin(initialTimer.asAbsoluteTimeout()))
	{
		return;
	}

	if (wait_close.getType() == Timeout::E_RELATIVE && wait_close.getRelative() > 0)
	{
		// Close the streams. If the child process is blocked waiting to output,
		// then this will cause it to get a SIGPIPE (or ERROR_BROKEN_PIPE on Windows),
		// and it may be able to clean up after itself.  Likewise, if the child process
		// is blocked waiting for input, it will now detect EOF.
		upr_close(m_in);
		upr_close(m_out);
		upr_close(m_err);

		if (this->terminatesWithin(closeTimer.asAbsoluteTimeout()))
		{
			return;
		}
	}

#ifdef BLOCXX_WIN32

	if (wait_term.getType() == Timeout::E_RELATIVE && wait_term.getRelative() > 0 && this->terminateByMessage(termTimer.asAbsoluteTimeout()))
	{
		return;
	}

	// Give it a full minute to make sure we don't leave zombies hanging around
	// if the system is heavily loaded
	Timeout const killTimeout = Timeout::relative(60.0);
	if (!killProcess(killTimeout, terminationSelectionFlag))
	{
		BLOCXX_THROW(ProcessErrorException, "Child process has not terminated after killProcess().");
	}

#else

	if (wait_term.getType() == Timeout::E_RELATIVE && wait_term.getRelative() > 0 && this->killWait(termTimer.asAbsoluteTimeout(), SIGTERM, "SIGTERM", terminationSelectionFlag))
	{
		return;
	}
	// Give it a full minute to make sure we don't leave zombies hanging around
	// if the system is heavily loaded
	Timeout const sigkillTimeout = Timeout::relative(60.0);
	if (!killWait(sigkillTimeout, SIGKILL, "SIGKILL", terminationSelectionFlag))
	{
		BLOCXX_THROW(ProcessErrorException, "Child process has not terminated after sending it a SIGKILL.");
	}

#endif
}

// Waits wait_time at most wait_time seconds for process to terminate, setting
// m_status.
// RETURNS: whether or not process terminated.
//
bool Process::terminatesWithin(const Timeout& wait_time)
{
	float const mult = 1.20;
	float const max_period = 5000.0; // milliseconds
	float period = 100.0; // milliseconds
	TimeoutTimer timer(wait_time);
	while (!timer.expired() && !m_status.terminated())
	{
		Thread::sleep(static_cast<UInt32>(period));
		period = (std::min)(max_period, period * mult);
		m_status = m_impl->pollStatus(m_pid);
		timer.loop();
	}
	return m_status.terminated();
}

//------------------ Platform-dependent methods --------------------------
//------------------------------------------------------------------------
#ifdef BLOCXX_WIN32

Process::Status pollStatusImpl(ProcId pid)
{
	DWORD exitCode;

	DWORD rc1 = WaitForSingleObject(pid, 0);
	if(rc1 == WAIT_FAILED)
	{
		String msg;
		System::lastErrorMsg("pollStatusImpl() 1: ", msg);
		BLOCXX_THROW_ERRNO_MSG(ProcessErrorException, msg);
	}

	BOOL rc = GetExitCodeProcess(pid, &exitCode);

	if (!rc)
	{
		String msg;
		System::lastErrorMsg("pollStatusImpl() 2: ", msg);
		BLOCXX_THROW_ERRNO_MSG(ProcessErrorException, msg);
	}

	return Process::Status(pid, exitCode);
}

// Sends a defined message to a process hoping that the process knows it and
// will be able to terminate itself
bool Process::terminateByMessage(const Timeout& waitTime)
{
	DWORD bsmApp = BSM_APPLICATIONS;
	UINT termMsg = RegisterWindowMessage(TERM_MESSAGE);
	BOOL bSucceed = BroadcastSystemMessage(BSF_IGNORECURRENTTASK, &bsmApp, termMsg, NULL, NULL);

	if (bSucceed == -1)
	{
		if (this->processStatus().terminated())
		{
			return true;
		}
		else
		{
			String msg;
			System::lastErrorMsg("Process::terminateByMessage()", msg);
			BLOCXX_THROW_ERRNO_MSG(ProcessErrorException, msg);
		}
	}

	return this->terminatesWithin(waitTime);
}

bool Process::killProcess(const Timeout& waitTime, ETerminationSelectionFlag terminationSelectionFlag)
{
	DWORD result = ERROR_SUCCESS;

	DWORD pId = WinUtils::getProcessIdNT(m_pid);
	if (terminationSelectionFlag == E_TERMINATE_PROCESS_GROUP)
	{
		result = WinUtils::killProcessGroup(pId);
	}
	else
	{
		result = WinUtils::killProcess(pId);
	}

	if (result != ERROR_SUCCESS)
	{
		if (this->processStatus().terminated())
		{
			return true;
		}
		else
		{
			String msg;
			System::lastErrorMsg("Process::killProcess()", msg);
			BLOCXX_THROW_ERRNO_MSG(ProcessErrorException, msg);
		}
	}

	return this->terminatesWithin(waitTime);
}

ProcId Process::getCurProcessId()
{
	return GetCurrentProcess();
}

#else

Process::Status pollStatusImpl(ProcId pid)
{
	ProcId wpid;
	int status;

	do
	{
		// Use WUNTRACED so that we can detect if process stopped
		wpid = ::waitpid(pid, &status, WNOHANG | WUNTRACED);

	} while (wpid < 0 && errno == EINTR);

	if (wpid < 0)
	{
		BLOCXX_THROW_ERRNO_MSG(ProcessErrorException, "waitpid() failed");
	}
	return Process::Status(wpid, status);
}

// Sends signal sig to child process and waits wait_time seconds for it
// to terminate.  If an error occurs, signame is used in constructing the
// error message.
//
bool Process::killWait(const Timeout& wait_time, int sig, char const * signame, ETerminationSelectionFlag terminationSelectionFlag)
{
	ProcId killArg = terminationSelectionFlag == E_TERMINATE_PROCESS_GROUP ? -m_pid : m_pid;
	int errnum = m_impl->kill(killArg, sig);
	if (errnum != 0)
	{
		// maybe kill() failed because child terminated first
		if (this->processStatus().terminated())
		{
			return true;
		}
		else
		{
			Format fmt("Failed sending %1 to process %2.", signame, m_pid);
			char const * msg = fmt.c_str();
			errno = errnum;
			BLOCXX_THROW_ERRNO_MSG(ProcessErrorException, msg);
		}
	}
	return this->terminatesWithin(wait_time);
}

ProcId Process::getCurProcessId()
{
	return ::getpid();
}

#endif

} // namespace BLOCXX_NAMESPACE
