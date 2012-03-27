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

#ifndef BLOCXX_EXEC_HPP_INCLUDE_GUARD_
#define BLOCXX_EXEC_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/EnvVars.hpp"
#include "blocxx/Process.hpp"
#ifdef BLOCXX_ENABLE_TEST_HOOKS
#include "blocxx/GlobalPtr.hpp"
#endif

namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_APIEXCEPTION(ExecError, BLOCXX_COMMON_API);
BLOCXX_DECLARE_APIEXCEPTION2(ExecTimeout, ExecErrorException, BLOCXX_COMMON_API);
BLOCXX_DECLARE_APIEXCEPTION2(ExecBufferFull, ExecErrorException, BLOCXX_COMMON_API);

/////////////////////////////////////////////////////////////////////////////
namespace Exec
{
	/**
	 * Execute a command.
	 * The command will inherit stdin, STDOUT, and STDERR from the parent
	 * process.  This function will not search the path for command[0], so
	 * the absolute path to the binary should be specified.  If the path needs
	 * to be searched, you can set command[0] = "/bin/sh"; command[1] = "-c";
	 * and then fill in the rest of the array with the command you wish to
	 * execute.
	 * This function blocks until the child process exits.  Use a timeout or
	 * be careful that the command you run doesn't hang.
	 *
	 * @param command
	 *  command[0] is the binary to be executed.
	 *  command[1] .. command[n] are the command line parameters to the command.
	 *
	 * @param envp an array of strings  of the form "key=value", which are passed
	 *  as environment to the new program. envp must be terminated by a null 
	 *  pointer. envp may be 0, in which case the current process's environment
	 *  variables will be used.
	 * 
	 * @param timeout How long to wait for the process to exit.
	 *
	 * @return The process's status
	 * @throws ExecErrorException on error
	 */
	BLOCXX_COMMON_API Process::Status system(const Array<String>& command,
		const char* const envp[] = 0, const Timeout& = Timeout::infinite);

	template <typename SA1, typename SA2>
	Process::Status system(const SA1& command,
		const SA2& envVars, const Timeout& timeout = Timeout::infinite)
	{
		Cstr::CstrArr<SA1> sa_command(command);
		Cstr::CstrArr<SA2> sa_envVars(envVars);
		return system(sa_command.sarr, sa_envVars.sarr, timeout);
	}

	/** This is deprecated.  Use system() instead */
	BLOCXX_COMMON_API int safeSystem(const Array<String>& command,
		const char* const envp[] = 0) BLOCXX_DEPRECATED;


	/**
	* This class is used to specify what spawn() should do between fork and
	* exec.
	*/
	class BLOCXX_COMMON_API PreExec
	{
	public:
		typedef ::BLOCXX_NAMESPACE::UnnamedPipe* pipe_pointer_t;

		PreExec(bool precompute_max_descriptors = false);

		virtual ~PreExec();

		/**
		* @return True if @c Process object returned by @c spawn() should
		* contain one end of a pipe connected to standard descriptor @a d
		* for the child process.  False if standard descriptor @a d of child
		* should be connected to <tt>/dev/null</tt>.
		*
		* @pre 0 <= @a d < 3.
		*/
		virtual bool keepStd(int d) const = 0;

		struct Error
		{
			enum { MAX_MSG_LEN = 64 };
			char message[MAX_MSG_LEN + 1]; // must be null-terminated
			int error_num;        // errno value; 0 means no errno value
		};

		/**
		* Use this class to allow @c call() to throw an exception
		*/
		struct DontCatch
		{
			virtual ~DontCatch();
		};

		/**
		* This function is called between @c fork and @c exec in the @c spawn()
		* function.  It must not allocate memory unless you can guarantee
		* that there is only one thread running -- on some platforms allocating
		* memory between fork and exec when there were other threads running
		* can result in a deadlock.
		*
		* Any exception thrown of type <tt>PreExec::Error</tt> or derived from
		* <tt>std::exception</tt> is reported in full back to the parent;
		* any exception derived from <tt>PreExec::DontCatch</tt> is allowed to
		* propagate out of <tt>Exec::spawn</tt>; and any other type of
		* exception is reported as an unknown exception.
		*
		* @param pparr an array pointer that can be passed to the static
		* functions @c close_pipes_on_exec() and @c setup_std_descriptors().
		* It includes pipes for each of the standard descriptors, plus any
		* additional pipes used by <tt>Exec::spawn</tt> for communicating
		* between child and parent before the @c execve occurs.
		*/
		virtual void call(pipe_pointer_t const pparr[]) = 0;

		/**
		* For calling from <tt>PreExec::call</tt>.
		* Sets every descriptor to close-on-exec, with the exception of
		* the standard descriptors 0, 1, and 2, and those descriptors d
		* for which d < keep.size() and keep[d] is true.
		*/
		void closeDescriptorsOnExec(std::vector<bool> const & keep);

		/**
		* For calling from <tt>PreExec::call</tt>.
		* Resets all signals to their default actions.
		*/
		static void resetSignals();

		/**
		* For calling from <tt>PreExec::call</tt>.
		* Sets all the pipe descriptors in pparr to close-on-exec.
		*
		* @param pparr The @a pparr parameter passed to <tt>PreExec::call</tt>.
		*/
		static void closePipesOnExec(pipe_pointer_t const pparr[]);

		/**
		* For calling from <tt>PreExec::call</tt>.
		* Connects standard descriptors to the appropriate pipes, or
		* opens them to <tt>/dev/null</tt>, as appropriate.
		*
		* @param pparr The @a pparr parameter passed to <tt>PreExec::call</tt>.
		*/
		static void setupStandardDescriptors(pipe_pointer_t const pparr[]);

		/**
		 * For calling from PreExec::call().
		 * Sets the process group ID of the process to be a new process group. In POSIX terms, setpgid(0, 0) is called.
		 * This allows signals to be send to the new process group, which will include the process any any descendents
		 * which have not joined a new process group.
		 */
		static void setNewProcessGroup();

	protected:
		long m_max_descriptors;
	};

	/// Intended to be used as a parameter to spawn() to indicate that the 
	/// current environment will be used for the child process.
	static char const * const * const currentEnvironment = 0;

	/**
	* Run the executable @a exec_path in a child process, with @a argv for
	* the program arguments and @a envp for the environment.
	*
	* @pre The standard descriptors (0, 1, and 2) are all open.
	*
	* @param exec_path Absolute path of the executable to run in the child
	* process.
	*
	* @param argv Null-terminated argument list for the child process.
	* If @a argv is null or the empty sequence (@a argv[0] is null),
	* then it is replaced with a sequence containing only @a exec_path.
	*
	* @param envp The null-terminated environment for the child process.  If
	* null then the caller's environment is used.  It is recommended that
	* envp be constructed by starting with <tt>Secure::minimal_environment()</tt>
	* and adding only those additional environment variables known to be
	* needed.
	*
	* @param pre_exec Specifies what action to take in the child process
	* between @c fork and @c exec.
	*/
	BLOCXX_COMMON_API ProcessRef spawn(
		char const * exec_path,
		char const * const argv[], char const * const envp[],
		PreExec & pre_exec
	);

	/**
	 * Variant of @c spawn that allows @a exec_path to have an arbitrary
	 * string-like type, and @a argv and @a envp to have arbitrary
	 * string-array-like types. If argv or envp are of type StringArray
	 * a terminating null is not necessary.
	 *
	 * @pre @a S is a type for which <tt>Cstr::to_char_ptr</tt> is defined.
	 *
	 * @pre Specializations of the <tt>Cstr::CstrArr</tt> class template are
	 * defined for both types @a SA1 and @a SA2.
	 */
	template <typename S, typename SA1, typename SA2>
	ProcessRef spawn(
		S const & exec_path, SA1 const & argv, SA2 const & envp,
		PreExec & pre_exec
	)
	{
		Cstr::CstrArr<SA1> sa_argv(argv);
		Cstr::CstrArr<SA2> sa_envp(envp);
		char const * s_exec_path = Cstr::to_char_ptr(exec_path);
		return spawn(s_exec_path, sa_argv.sarr, sa_envp.sarr, pre_exec);
	}

	/// Variant of spawn that uses @c StandardPreExec.
	//
	BLOCXX_COMMON_API ProcessRef spawn(
		char const * const argv[], char const * const envp[]
	);

	/**
	* Variant of @c spawn that uses @c StandardPreExec and
	* @a argv and @a envp to have arbitrary string-array-like types.
	*
	* @pre Specializations of the <tt>Cstr::CstrArr</tt> class template are
	* defined for both types @a SA1 and @a SA2.
	*/
	template <typename SA1, typename SA2>
	ProcessRef spawn(
		SA1 const & argv, SA2 const & envp
	)
	{
		Cstr::CstrArr<SA1> sa_argv(argv);
		Cstr::CstrArr<SA2> sa_envp(envp);
		return spawn(sa_argv.sarr, sa_envp.sarr);
	}

	template <typename SA1>
	ProcessRef spawn(
		SA1 const & argv
	)
	{
		return spawn(argv, Exec::currentEnvironment);
	}

	/**
	 * Wait for output from a child process.  The function returns when the
	 * process exits. In the case that the child process doesn't exit, if a
	 * timout is specified, then an ExecTimeoutException is thrown.
	 * If the process outputs more bytes than outputlimit, an
	 * ExecBufferFullException is thrown.
	 *
	 * @param output An out parameter, the process output will be appended to
	 *  this string.
	 * 
	 * @param proc The connection to the child process.
	 * 
	 * @param timeout Specifies the interval to wait for the
	 *  process to exit. If the process hasn't exited after the timeout,
	 *  an ExecTimeoutException will be thrown.  
	 *  Output from a process can reset a relative with reset timeout.
	 * 
	 * @param outputlimit Specifies the maximum size of the parameter output,
	 *  in order to constrain possible memory usage.  If the process outputs
	 *  more data than will fit into output, then an ExecBufferFullException
	 *  is thrown. If outputlimit < 0, the limit will be infinite, and an
	 *  ExecBufferFullException will never be thrown.
	 *
	 * @throws ProcessError on error.
	 * @throws ProcessTimeout if the process hasn't finished within timeoutsecs.
	 * @throws ProcessBufferFull if the process output exceeds outputlimit bytes.
	 */
	BLOCXX_COMMON_API void gatherOutput(String& output, const ProcessRef& proc, const Timeout& timeout = Timeout::infinite, int outputlimit = -1);
	
	enum EOutputSource
	{
		E_STDOUT,
		E_STDERR
	};

	class BLOCXX_COMMON_API OutputCallback
	{
	public:
		virtual ~OutputCallback();
		void handleData(const char* data, size_t dataLen, EOutputSource outputSource, const ProcessRef& theProc, size_t streamIndex, Array<char>& inputBuffer);
	private:
		/**
		 * @param data The data output from the process identified by theStream. Will be NULL terminated.  However, if the process
		 * output 0 bytes, those will be contained in data.
		 */
		virtual void doHandleData(const char* data, size_t dataLen, EOutputSource outputSource, const ProcessRef& theProc, size_t streamIndex, Array<char>& inputBuffer) = 0;
	};

	class BLOCXX_COMMON_API InputCallback
	{
	public:
		virtual ~InputCallback();
		void getData(Array<char>& inputBuffer, const ProcessRef& theProc, size_t streamIndex);
	private:
		virtual void doGetData(Array<char>& inputBuffer, const ProcessRef& theProc, size_t streamIndex) = 0;
	};
#if 0
	enum EProcessRunning
	{
		E_PROCESS_RUNNING,
		E_PROCESS_EXITED
	};

	// class invariant: if m_running == E_PROCESS_RUNNING, then m_status == 0.
	class ProcessStatus
	{
	public:
		ProcessStatus()
		: m_running(E_PROCESS_RUNNING)
		, m_status(0)
		{
		}

		explicit ProcessStatus(int status)
		: m_running(E_PROCESS_EXITED)
		, m_status(status)
		{
		}

		bool hasExited() const
		{
			return m_running == E_PROCESS_EXITED;
		}

		const int& getStatus() const
		{
			return m_status;
		}
	private:
		EProcessRunning m_running;
		int m_status;
	};
#endif

	/**
	 * Send input and wait for output from child processes.  The function returns when the
	 * processes have exited. In the case that a child process doesn't exit, if a
	 * timout is specified, then an ExecTimeoutException is thrown.
	 * If an exception is thrown by the OutputCallback or InputCallback, it will not be
	 * caught.
	 *
	 * @param output A callback, whenever data is received from a process, it will
	 *  be passed to output.handleData().
	 *
	 * @param procs The connections to the child processes.
	 *
	 * @param timeout Specifies the interval to wait for all the
	 *  processes to exit. If the timeout expires, an ExecTimeoutException will
	 *  be thrown. Output from a process can reset a relative with reset timeout.
	 *
	 * @param input Callback to provide data to be written to the process(es) standard input.
	 *  input.getData() will be called once for each stream, and subsequently once every time
	 *  data has been written to a process. output.handleData() may also provide input data
	 *  via the inputBuffer parameter, it is called every time data is read from a process.
	 *
	 * @throws ExecErrorException on error.
	 * @throws ExecTimeoutException if the process hasn't finished within timeoutSecs.
	 */
	BLOCXX_COMMON_API void processInputOutput(OutputCallback& output, Array<ProcessRef>& procs,
		InputCallback& input, const Timeout& timeout = Timeout::infinite);
	

	BLOCXX_COMMON_API void processInputOutput(const String& input, String& output, const ProcessRef& process, 
		const Timeout& timeout = Timeout::infinite, int outputlimit = -1);

	/**
	 * Send input to a process, collect the output, and wait for it to exit.
	 * The function returns when the process exits. In the case that the child
	 * process doesn't exit, if a timout is specified, then an
	 * @c ExecTimeoutException is thrown. If the process outputs more bytes
	 * than outputlimit, an @c ExecBufferFullException is thrown.
	 *
	 * If the process does not terminate by itself, or if an exception is
	 * thrown because a limit has been reached (time or output), then
	 * Process::waitCloseTerm() is called with default arguments to force
	 * termination.
	 *
	 * @param proc The object managing the process.
	 *
	 * @param output Process output is appended to this string.
	 *
	 * @param timeout The interval to wait for the
	 *  process to exit. If the process hasn't exited after the timeout,
	 *  an @c ExecTimeoutException will be thrown, and the process will be
	 *  killed. Output from a process can reset a relative with reset timeout.
	 *
	 * @param outputlimit The maximum size of the parameter output,
	 *  in order to constrain possible memory usage.  If the process outputs
	 *  more data than will fit into output, then an @c ExecBufferFullException
	 *  is thrown, and the process will be killed.
	 *  If @c outputlimit < 0, the limit will be infinite, and an
	 *  @c ExecBufferFullException will not be thrown.
	 *
	 * @param input Data to write to the child's stdin. After the data
	 *  has been written, stdin is closed.
	 *
	 * @throws ExecErrorException on error.
	 * @throws ExecTimeoutException if the process hasn't finished within timeoutsecs.
	 * @throws ExecBufferFullException if the process output exceeds outputlimit bytes.
	 */
	BLOCXX_COMMON_API Process::Status feedProcessAndGatherOutput(
		ProcessRef const & proc, String & output,
		Timeout const & timeout = Timeout::infinite, int outputlimit = -1,
		String const & input = String());

	/**
	 * Send input to a process, collect STDOUT and STDERR, and wait for it
	 * to exit. The function returns when the process exits. In the case 
	 * that the child process doesn't exit, if a timout is specified, then an
	 * @c ExecTimeoutException is thrown. If the process outputs more bytes
	 * than outputlimit on STDOUT or STDERR, an @c ExecBufferFullException
	 * is thrown.
	 *
	 * If the process does not terminate by itself, or if an exception is
	 * thrown because a limit has been reached (time or output), then
	 * Process::waitCloseTerm() is called with default arguments to force
	 * termination.
	 *
	 * @param proc The object managing the process.
	 *
	 * @param output Process STDOUT is appended to this string.
	 *
	 * @param erroutut Process STDERR is appended to this string.
	 *
	 * @param timeout The interval to wait for the
	 *  process to exit. If the process hasn't exited after the timeout,
	 *  an @c ExecTimeoutException will be thrown, and the process will be
	 *  killed. Output from a process can reset a relative with reset timeout.
	 *
	 * @param outputlimit The maximum size of the parameter output,
	 *  in order to constrain possible memory usage.  If the process outputs
	 *  more data than will fit into output, then an @c ExecBufferFullException
	 *  is thrown, and the process will be killed.
	 *  If @c outputlimit < 0, the limit will be infinite, and an
	 *  @c ExecBufferFullException will not be thrown.
	 *
	 * @param input Data to write to the child's stdin. After the data
	 *  has been written, stdin is closed.
	 *
	 * @throws ExecErrorException on error.
	 * @throws ExecTimeoutException if the process hasn't finished within timeoutsecs.
	 * @throws ExecBufferFullException if the process output exceeds outputlimit bytes.
	 */
	BLOCXX_COMMON_API Process::Status feedProcessAndGatherOutput(
		ProcessRef const & proc, String & output,
		String & erroutput, Timeout const & timeout = Timeout::infinite,
		int outputLimit = -1, 
		String const & input = String());

	/**
	 * Execute a command and run @c feedProcessAndGatherOutput() on the process.
	 * This function will not search the path for command[0], so
	 * the absolute path to the binary should be specified.  If the path needs
	 * to be searched, you can set command[0] = "/bin/sh"; command[1] = "-c";
	 * and then fill in the rest of the array with the command you wish to
	 * execute. Exercise caution when doing this, as you may be creating a
	 * security hole.
	 *
	 * @param command
	 *  command[0] is the binary to be executed.
	 *  command[1] .. command[n] are the command line parameters to the command.
	 *
	 * @param output @see feedProcessAndGatherOutput().
	 *
	 * @param envVars An @c EnvVars object that contains the environment variables
	 *	to pass as the environment to the new process. If @a envVars
	 *	doesn't contain any environment variables, then the current process's
	 *	environment variables will be used.
	 *
	 * @param timeout @see feedProcessAndGatherOutput().
	 *
	 * @param outputlimit @see feedProcessAndGatherOutput().
	 *
	 * @param input @see feedProcessAndGatherOutput().
	 *
	 * @throws ExecErrorException on error.
	 * @throws ExecTimeoutException if the process hasn't finished within timeoutsecs.
	 * @throws ExecBufferFullException if the process output exceeds outputlimit bytes.
	 */
	BLOCXX_COMMON_API Process::Status executeProcessAndGatherOutput(
		char const * const command[], String& output, char const * const envVars[],
		const Timeout& timeout = Timeout::infinite, int outputlimit = -1,
		char const * input = 0);

	/**
	 * Execute a command and run @c feedProcessAndGatherOutput() on the process.
	 * This function will not search the path for command[0], so
	 * the absolute path to the binary should be specified.  If the path needs
	 * to be searched, you can set command[0] = "/bin/sh"; command[1] = "-c";
	 * and then fill in the rest of the array with the command you wish to
	 * execute. Exercise caution when doing this, as you may be creating a
	 * security hole.
	 *
	 * @param command
	 *  command[0] is the binary to be executed.
	 *  command[1] .. command[n] are the command line parameters to the command.
	 *
	 * @param output @see feedProcessAndGatherOutput().
	 *
	 * @param erroutput @see feedProcessAndGatherOutput().
	 *
	 * @param envVars An @c EnvVars object that contains the environment variables
	 *	to pass as the environment to the new process. If @a envVars
	 *	doesn't contain any environment variables, then the current process's
	 *	environment variables will be used.
	 *
	 * @param timeout @see feedProcessAndGatherOutput().
	 *
	 * @param outputlimit @see feedProcessAndGatherOutput().
	 *
	 * @param input @see feedProcessAndGatherOutput().
	 *
	 * @throws ExecErrorException on error.
	 * @throws ExecTimeoutException if the process hasn't finished within timeoutsecs.
	 * @throws ExecBufferFullException if the process output exceeds outputlimit bytes.
	 */
	BLOCXX_COMMON_API Process::Status executeProcessAndGatherOutput(
		char const * const command[], String& output, String& erroutput,
		char const * const envVars[],
		const Timeout& timeout = Timeout::infinite, int outputLimit = -1,
		char const * input = 0);
	
	/**
	 * Version of @c executeProcessAndGatherOutput() that passes the current
	 * process's environment to the child process.
	 */
	template <typename SA1, typename S1, typename S2>
	Process::Status executeProcessAndGatherOutput(
		SA1 const & command, S1& output,
		const Timeout& timeout, int outputlimit, S2 const& input)
	{
		Cstr::CstrArr<SA1> sa_command(command);
		String tmpOutput;
		char const * sInput = Cstr::to_char_ptr(input);
		Process::Status res;
		try
		{
			res = executeProcessAndGatherOutput(sa_command.sarr, tmpOutput,
				currentEnvironment,	timeout, outputlimit, sInput);
		}
		catch(...)
		{
			output = tmpOutput.c_str();
			throw;
		}
		output = tmpOutput.c_str();
		return res;
	}

	/**
	 * Version of @c executeProcessAndGatherOutput() that passes the current
	 * process's environment to the child process and separates STDOUT
	 * and STDERR.
	 */
	template <typename SA1, typename S1, typename S2>
	Process::Status executeProcessAndGatherOutput(
		SA1 const & command, S1& output, S1& erroutput,
		const Timeout& timeout, int outputlimit, S2 const& input)
	{
		Cstr::CstrArr<SA1> sa_command(command);
		String tmpOutput, tmpErrOut;
		char const * sInput = Cstr::to_char_ptr(input);
		Process::Status res;
		try
		{
			res = executeProcessAndGatherOutput(sa_command.sarr, tmpOutput,
				tmpErrOut, currentEnvironment,	timeout, outputlimit, sInput);
		}
		catch(...)
		{
			output = tmpOutput.c_str();
			erroutput = tmpErrOut.c_str();
			throw;
		}
		output = tmpOutput.c_str();
		erroutput = tmpErrOut.c_str();
		return res;
	}

	/**
	 * Version of @c executeProcessAndGatherOutput() that passes the current
	 * process's environment to the child process.
	 */
	template <typename SA1, typename S1>
	Process::Status executeProcessAndGatherOutput(
		SA1 const & command, S1& output,
		const Timeout& timeout = Timeout::infinite, int outputlimit = -1)
	{
		Cstr::CstrArr<SA1> sa_command(command);
		String tmpOutput;
		Process::Status res;
		try
		{
			res = executeProcessAndGatherOutput(sa_command.sarr, tmpOutput,
				currentEnvironment,	timeout, outputlimit, (char const*)0);
		}
		catch(...)
		{
			output = tmpOutput.c_str();
			throw;
		}
		output = tmpOutput.c_str();
		return res;
	}

	/**
	 * Version of @c executeProcessAndGatherOutput() that passes the current
	 * process's environment to the child process and collects STDOUT and
	 * STDERR separately.
	 */
	template <typename SA1, typename S1>
	Process::Status executeProcessAndGatherOutput(
		SA1 const & command, S1& output, S1& erroutput,
		const Timeout& timeout = Timeout::infinite, int outputlimit = -1)
	{
		Cstr::CstrArr<SA1> sa_command(command);
		String tmpOutput, tmpErrOut;
		Process::Status res;
		try
		{
			res = executeProcessAndGatherOutput(sa_command.sarr, tmpOutput,
				tmpErrOut, currentEnvironment,	timeout, outputlimit,
				(char const*)0);
		}
		catch(...)
		{
			output = tmpOutput.c_str();
			erroutput = tmpErrOut.c_str();
			throw;
		}
		output = tmpOutput.c_str();
		erroutput = tmpErrOut.c_str();
		return res;
	}

	/**
	 * Variant of @c executeProcessAndGatherOutput that allows @a command and @a envp
	 * to have arbitrary string-array-like types and @a output to have an arbitrary
	 * string-like type. If command or envp are of type StringArray a terminating
	 * null is not necessary.
	 * 
	 * @pre @a S1 supports assignment from a const char*.
	 * 
	 * @pre @a S2 is a type for which <tt>Cstr::to_char_ptr</tt> is defined.
	 * 
	 * @pre Specializations of the <tt>Cstr::CstrArr</tt> class template are
	 * defined for both types @a SA1 and @a SA2.
	 */
	template <typename SA1, typename S1, typename SA2, typename S2>
	Process::Status executeProcessAndGatherOutput(
		SA1 const & command, S1& output, SA2 const & envp,
		const Timeout& timeout, int outputlimit, S2 const& input)
	{
		Cstr::CstrArr<SA1> sa_command(command);
		Cstr::CstrArr<SA2> sa_envp(envp);
		String tmpOutput;
		char const * sInput = Cstr::to_char_ptr(input);
		Process::Status res;
		try
		{
			res = executeProcessAndGatherOutput(sa_command.sarr, tmpOutput, sa_envp.sarr,
				timeout, outputlimit, sInput);
		}
		catch(...)
		{
			output = tmpOutput.c_str();
			throw;
		}
		output = tmpOutput.c_str();
		return res;
	}

	/**
	 * Variant of @c executeProcessAndGatherOutput that allows @a command and @a envp
	 * to have arbitrary string-array-like types and @a output to have an arbitrary
	 * string-like type. It also collects STDOUT and STDERR separately.
	 * If command or envp are of type StringArray a terminating
	 * null is not necessary.
	 * 
	 * @pre @a S1 supports assignment from a const char*.
	 * 
	 * @pre @a S2 is a type for which <tt>Cstr::to_char_ptr</tt> is defined.
	 * 
	 * @pre Specializations of the <tt>Cstr::CstrArr</tt> class template are
	 * defined for both types @a SA1 and @a SA2.
	 */
	template <typename SA1, typename S1, typename SA2, typename S2>
	Process::Status executeProcessAndGatherOutput(
		SA1 const & command, S1& output, S1& erroutput, SA2 const & envp,
		const Timeout& timeout, int outputlimit, S2 const& input)
	{
		Cstr::CstrArr<SA1> sa_command(command);
		Cstr::CstrArr<SA2> sa_envp(envp);
		String tmpOutput, tmpErrOut;
		char const * sInput = Cstr::to_char_ptr(input);
		Process::Status res;
		try
		{
			res = executeProcessAndGatherOutput(sa_command.sarr, tmpOutput,
				tmpErrOut, sa_envp.sarr, timeout, outputlimit, sInput);
		}
		catch(...)
		{
			output = tmpOutput.c_str();
			erroutput = tmpErrOut.c_str();
			throw;
		}
		output = tmpOutput.c_str();
		erroutput = tmpErrOut.c_str();
		return res;
	}

	/**
	 * Version of @c executeProcessAndGatherOutput() that passes no input to the process.
	 * STDOUT and STDERR are collected separately.
	 */
	template <typename SA1, typename S1, typename SA2>
	Process::Status executeProcessAndGatherOutput(
		SA1 const & command, S1& output, S1& erroutput, SA2 const & envp,
		const Timeout& timeout = Timeout::infinite, int outputlimit = -1)
	{
		return executeProcessAndGatherOutput(command, output, erroutput, envp,
			timeout, outputlimit, String());
	}


	BLOCXX_COMMON_API void executeProcessAndGatherOutput(
		const Array<String>& command,
		String& output, int& processstatus,
		int timeoutsecs = -1, int outputlimit = -1, 
		const String& input = String()) BLOCXX_DEPRECATED;
	
	namespace Impl
	{
		// internal implementation details, not meant for use outside of *Exec.?pp files.
		unsigned const BLOCXX_IN		= 0;
		unsigned const BLOCXX_OUT		= 1;
		unsigned const BLOCXX_SERR		= 2;
		unsigned const BLOCXX_EXEC_ERR	= 3;
		unsigned const BLOCXX_NPIPE		= 4;
		void close_child_ends(UnnamedPipeRef ppipe[BLOCXX_NPIPE]);

		struct NullFactory
		{
			static void* create()
			{
				return 0;
			}
		};
	} // end namespace Impl

#ifdef BLOCXX_ENABLE_TEST_HOOKS
	typedef GlobalPtr<ExecMockObject, Impl::NullFactory> ExecMockObject_t;
	/**
	 * If this object is non-null, the default functionality of the Exec class will be replaced by calls to
	 * g_execMockObject's member functions. This is meant to be used for unit tests. Not all functions may be
	 * implemented, if you need one that isn't, then please implement it! Modifying this variable will affect all
	 * threads, it should not be used in a threaded program.
	 */
	extern ExecMockObject_t g_execMockObject;
#endif
} // end namespace Exec

} // end namespace BLOCXX_NAMESPACE

#endif
