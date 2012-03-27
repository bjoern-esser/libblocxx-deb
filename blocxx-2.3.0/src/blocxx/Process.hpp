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


#ifndef BLOCXX_PROCESS_HPP_INCLUDE_GUARD_
#define BLOCXX_PROCESS_HPP_INCLUDE_GUARD_

/**
* @author Kevin S. Van Horn
*/

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Exception.hpp"
#include "blocxx/Types.hpp"
#include "blocxx/IntrusiveCountableBase.hpp"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/Timeout.hpp"

namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_EXCEPTION(ProcessError);

class ProcessImpl;
typedef IntrusiveReference<ProcessImpl> ProcessImplRef;

/// Class for communicating with and managing a child process.
//
class BLOCXX_COMMON_API Process : public IntrusiveCountableBase
{
public:
	/// @pre: @a pid is the process ID for a child process whose standard input
	/// is @a in, standard output is @a out, and standard error is @a err.
	//
	Process(
		UnnamedPipeRef const & in, UnnamedPipeRef const & out,
		UnnamedPipeRef const & err, ProcId pid
	);

protected:
	/// Constructor for derived classes that allow them to override the implementation.
	/// This is necessary because destructors shouldn't call virtual functions (it's
	/// undefined behavior), so instead of this class having virtual functions, that is
	/// moved to ProcessImpl.
	/// @pre: @a pid is the process ID for a child process whose standard input
	/// is @a in, standard output is @a out, and standard error is @a err.
	//
	Process(
		const ProcessImplRef& impl, UnnamedPipeRef const & in, UnnamedPipeRef const & out,
		UnnamedPipeRef const & err, ProcId pid
	);

public:
	/// @pre: @a pid is the process ID for a child process.
	/// @post: @c in(), @c out(), and @c err() are all null.
	Process(ProcId pid);

	/**
	* Releases ownership of the ProcId and UnnamedPipes held by this object.
	*
	* @post @c in(), @c out(), and @c err() are all null; @c pid() < 0;
	* the dtor does nothing.  Only the above-mentioned methods may be called
	* on this object.
	*/
	void release();

	/**
	* If @c release has been called on this object, does nothing.  Otherwise,
	* closes pipes and waits for process to die, killing it if necessary,
	*/
	virtual ~Process();

	/// Stdin for the child process.
	/// The default timeout is set to 10 minutes.
	UnnamedPipeRef in() const;

	/// Stdout for the child process.
	/// The default timeout is set to 10 minutes.
	UnnamedPipeRef out() const;

	/// Stderr for the child process.
	/// The default timeout is set to 10 minutes.
	UnnamedPipeRef err() const;

	/// Process ID for the child process.
	ProcId pid() const;

	/// Portable process status.
	//
	class BLOCXX_COMMON_API Status
	{
	public:
		struct Repr { };

		/// @invariant Exactly one of <tt>running()</tt>, <tt>stopped()</tt>
		/// and <tt>terminated()</tt> is true.

		/// @param wpid return value from @c waitpid
		/// @param status status value assigned by @c waitpid.
		//
		Status(ProcId wpid, int status);

		/// @pre @a rep1 and @rep2 were obtained by a prior call to @c repr()
		/// on some @c Status object.
		//
		Status(int rep1, int rep2, Repr);

		/// @post <tt>running()</tt> is true
		//
		Status();

		/// @return Is the process still running?
		//
		bool running() const;

		/// @return Has the process terminated normally?
		//
		bool exitTerminated() const;

		/**
		* @pre @c exitTerminated()
		* @return exit status of process, as passed to @c exit() or
		* returned from @c main().
		*/
		int exitStatus() const;

		/// @return exitTerminated() && exitStatus() == 0
		//
		bool terminatedSuccessfully() const;

		/// @return Has the process terminated because of an uncaught signal?
		//
		bool signalTerminated() const;

		/// @return Has the process terminated?
		//
		bool terminated() const;

		/// @pre @c signalTerminated()
		/// @return signal that caused the process to terminate.
		//
		int termSignal() const;

		/// @return Has the process stopped?
		//
		bool stopped() const;

		/// @pre @c stopped()
		/// @return signal that caused the child to stop
		//
		int stopSignal() const;

		/**
		 * Get a string representation of the status suitable for debugging or logging.
		 */
		String toString() const;

		/**
		 * Get the result from waitpid()
		 */
		int getPOSIXwaitpidStatus() const;

		void repr(int & rep1, int & rep2) const;

	private:
		bool m_status_available;
		int m_status;
	};

	/**
	* @return Status of child process.
	* @note Does not wait for child to terminate.
	* @throw ProcessErrorException
	*/
	Status processStatus();

	enum ETerminationSelectionFlag
	{
		/// The process and any descendent processes which are in the process group will be terminated.
		E_TERMINATE_PROCESS_GROUP,
		/// The process will be terminated.
		E_TERMINATE_PROCESS_ONLY
	};
	/**
	* Waits for the child process to terminate, taking increasingly severe
	* measures to ensure that this happens. All timeouts are measured
	* from the start of the function.
	* The following steps are taken in order until termination is detected:
	* -# If @a wait_initial.getRelative() > 0, waits until @a wait_initial expires for the
	*    process to terminate on its own.
	* -# If @a wait_close.getRelative() > 0, closes the input and output FileHandles and
	*    then waits until @a wait_close expires for the process to die.
	* -# If @a wait_term.getRelative() > 0, sends process a @c SIGTERM signal and waits
	*    until @a wait_term expires for it to die.
	* -# Sends the process a @c SIGKILL signal.
	*
	* In steps 1-3 the function returns as soon as termination is detected.
	* If this function is called a second time it is a no-op, because it
	* immediately sees that the process has already terminated.
	*
	* @note If @a wait_close <= 0 then the FileHandles are NOT closed, and
	* if @a wait_term <= 0 then no @c SIGTERM signal is sent.
	*
	* @throw ProcessErrorException
	*/
	void waitCloseTerm(
		const Timeout& wait_initial = Timeout::relative(5.0),
		const Timeout& wait_close =   Timeout::relative(10.0),
		const Timeout& wait_term =    Timeout::relative(15.0),
		ETerminationSelectionFlag terminationSelectionFlag = E_TERMINATE_PROCESS_GROUP);

	/**
	 * Waits for the child process to terminate, taking increasingly severe
	 * measures to ensure that this happens. All times are measured
	 * from the start of the function.
	 * The following steps are taken in order until termination is detected:
	 * -# If @a wait_initial > 0, waits @a wait_initial seconds for the
	 *    process to terminate on its own.
	 * -# If @a wait_close > 0, closes the input and output FileHandles and
	 *    then waits until @a wait_close seconds have passed for the process to die.
	 * -# If @a wait_term > 0, sends process a @c SIGTERM signal and waits
	 *    until @a wait_term seconds have passed for it to die.
	 * -# Sends the process a @c SIGKILL signal.
	 *
	 * In steps 1-3 the function returns as soon as termination is detected.
	 * If this function is called a second time it is a no-op, because it
	 * immediately sees that the process has already terminated.
	 *
	 * @note If @a wait_close <= 0 then the FileHandles are NOT closed, and
	 * if @a wait_term <= 0 then no @c SIGTERM signal is sent.
	 *
	 * @throw ProcessErrorException
	 */
	void waitCloseTerm(float wait_initial, float wait_close, float wait_term);

private:
	bool terminatesWithin(const Timeout& wait_time);
	ProcId getCurProcessId();

#ifdef BLOCXX_WIN32
	bool terminateByMessage(const Timeout& waitTime);
	bool killProcess(const Timeout& waitTime, ETerminationSelectionFlag terminationSelectionFlag);
#else
	bool killWait(const Timeout& wait_time, int sig, char const * signame, ETerminationSelectionFlag terminationSelectionFlag);
#endif

	/// Copying not allowed (private)
	Process(Process const &);

	/// Assignment not allowed (private)
	void operator=(Process const &);

	ProcessImplRef m_impl;
	UnnamedPipeRef m_in;
	UnnamedPipeRef m_out;
	UnnamedPipeRef m_err;
	ProcId m_pid;
	Status m_status;
};

/**
 * Abstract interface for abstracting details of dealing with a process
 */
class BLOCXX_COMMON_API ProcessImpl : public IntrusiveCountableBase
{
public:
	virtual ~ProcessImpl();

	/**
	 * Sends signal @a sig to process @a pid. If pid is less than -1, then sig is sent to every process in the process
	 * group -pid.
	 *
	 * @return 0 on success and @c errno on failure.
	 */
	virtual int kill(ProcId pid, int sig) = 0;

	/**
	 * @return Current status for child process.
	 * @pre <tt>!s.terminated()</t> for the return value <tt>s</tt> of any
	 * previous call to @c pollStatus for @pid.
	 * @throws ProcessErrorException on error.
	 */
	virtual Process::Status pollStatus(ProcId pid) = 0;
};



} // namespace BLOCXX_NAMESPACE

#endif
