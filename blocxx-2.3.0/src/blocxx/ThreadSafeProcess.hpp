/*******************************************************************************
* Copyright (C) 2005, 2009, Quest Software, Inc. All rights reserved.
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

#ifndef BLOCXX_THREAD_SAFE_PROCESS_HPP_INCLUDE_GUARD_
#define BLOCXX_THREAD_SAFE_PROCESS_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Process.hpp"
#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/UnnamedPipe.hpp"

namespace BLOCXX_NAMESPACE
{

/**
 * Class for serializing access to a Process instance.
 * All member function calls are serialized.
 */
class BLOCXX_COMMON_API ThreadSafeProcess : public IntrusiveCountableBase
{
public:
	ThreadSafeProcess(const ProcessRef& procToWrap);

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
	virtual ~ThreadSafeProcess();

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

	/**
	* @return Status of child process.
	* @note Does not wait for child to terminate.
	* @throw ProcessErrorException
	*/
	Process::Status processStatus();

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
		Process::ETerminationSelectionFlag terminationSelectionFlag = Process::E_TERMINATE_PROCESS_GROUP);

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

	/// Copying not allowed (private)
	ThreadSafeProcess(ThreadSafeProcess const &);

	/// Assignment not allowed (private)
	void operator=(ThreadSafeProcess const &);

	ProcessRef m_proc;
	mutable NonRecursiveMutex m_guard;
};


} // namespace BLOCXX_NAMESPACE

#endif
