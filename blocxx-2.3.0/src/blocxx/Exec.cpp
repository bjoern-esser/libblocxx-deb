/*******************************************************************************
* Copyright (C) 2010, Quest Software, Inc. All rights reserved.
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
#include "blocxx/Exec.hpp"
#include "blocxx/Select.hpp"
#include "blocxx/ExceptionIds.hpp"
#include "blocxx/TimeoutTimer.hpp"
#include "blocxx/ExecMockObject.hpp"
#include "blocxx/GlobalPtr.hpp"
#include "blocxx/WaitpidThreadFix.hpp"

#if !defined(BLOCXX_WIN32)
#include "blocxx/PosixUnnamedPipe.hpp"
#include "blocxx/PosixExec.hpp"
#else
#include "blocxx/UnnamedPipe.hpp"
#include "blocxx/WinExec.hpp"
#endif

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
#ifndef BLOCXX_WIN32
#include <sys/wait.h>
#include <fcntl.h>
#endif
}

#ifdef BLOCXX_NCR
#if defined(sigaction)
#undef sigaction
#endif
#undef SIG_DFL
#define	SIG_DFL	(void(*)())0
#endif

namespace BLOCXX_NAMESPACE
{
BLOCXX_DEFINE_EXCEPTION_WITH_BASE_AND_ID(ExecTimeout, ExecErrorException);
BLOCXX_DEFINE_EXCEPTION_WITH_BASE_AND_ID(ExecBufferFull, ExecErrorException);
BLOCXX_DEFINE_EXCEPTION_WITH_ID(ExecError);


//////////////////////////////////////////////////////////////////////////////
namespace Exec
{
::BLOCXX_NAMESPACE::GlobalPtr<ExecMockObject, Impl::NullFactory> g_execMockObject = BLOCXX_GLOBAL_PTR_INIT;

//////////////////////////////////////////////////////////////////////////////
Process::Status
system(const Array<String>& command, const char* const envp[], const Timeout& timeout)
{

#ifndef BLOCXX_WIN32
	PosixExec::SystemPreExec spe;
#else
	WinExec::WinSystemPreExec spe;
#endif

	ProcessRef proc = Exec::spawn(command[0], command, envp, spe);

	proc->waitCloseTerm(Timeout::relative(0), timeout, Timeout::relative(0));
	return proc->processStatus();
}


//////////////////////////////////////////////////////////////////////////////
ProcessRef spawnImpl(
	char const * exec_path,
	char const * const argv[], char const * const envp[],
	PreExec & pre_exec
)
{
#ifdef BLOCXX_WIN32
	return WinExec::spawnImpl(exec_path, argv, envp, pre_exec);
#else
	return PosixExec::spawnImpl(exec_path, argv, envp, pre_exec);
#endif
}

//////////////////////////////////////////////////////////////////////////////
ProcessRef spawn(
	char const * exec_path,
	char const * const argv[], char const * const envp[],
	PreExec & pre_exec
)
{
	if (WaitpidThreadFix::shouldUseWaitpidThreadFix())
	{
		return WaitpidThreadFix::spawnProcess(exec_path, argv, envp, pre_exec);
	}
	return spawnImpl(exec_path, argv, envp, pre_exec);
}

/////////////////////////////////////////////////////////////////////////////
ProcessRef spawn(
	char const * const argv[], char const * const envp[]
)
{

#ifdef BLOCXX_WIN32
	WinExec::WinStandardPreExec pre_exec;
#else
	PosixExec::StandardPreExec pre_exec;
#endif

	return spawn(argv[0], argv, envp, pre_exec);
}

namespace Impl
{
void close_child_ends(UnnamedPipeRef ppipe[BLOCXX_NPIPE])
{
	// prevent the parent from using the child's end of the pipes.
	if (ppipe[BLOCXX_IN])
	{
		ppipe[BLOCXX_IN]->closeInputHandle();
	}
	if (ppipe[BLOCXX_OUT])
	{
		ppipe[BLOCXX_OUT]->closeOutputHandle();
	}
	if (ppipe[BLOCXX_SERR])
	{
		ppipe[BLOCXX_SERR]->closeOutputHandle();
	}
	ppipe[BLOCXX_EXEC_ERR]->closeOutputHandle();
}
} // end namespace Impl

namespace
{

#ifndef BLOCXX_MIN
#define BLOCXX_MIN(x, y) (x) < (y) ? (x) : (y)
#endif

/////////////////////////////////////////////////////////////////////////////
class StringOutputGatherer : public OutputCallback
{
public:
	StringOutputGatherer(String& stdoutput, String& erroutput, int outputLimit)
		: m_output(stdoutput)
		, m_erroutput(erroutput)
		, m_outputLimit(outputLimit)
	{
	}
	StringOutputGatherer(String& stdoutput, int outputLimit)
		: m_output(stdoutput)
		, m_erroutput(stdoutput)
		, m_outputLimit(outputLimit)
	{
	}
private:
	virtual void doHandleData(const char* data, size_t dataLen,
		EOutputSource outputSource, const ProcessRef& theProc,
		size_t streamIndex, Array<char>& inputBuffer)
	{
		String& output = (outputSource == E_STDOUT) ? m_output : m_erroutput;
		if (m_outputLimit >= 0 && output.length() + dataLen > static_cast<size_t>(m_outputLimit))
		{
			// the process output too much, so just copy what we can and return error
			int lentocopy = BLOCXX_MIN(m_outputLimit - output.length(), dataLen);
			if (lentocopy >= 0)
			{
				output += String(data, lentocopy);
			}
			BLOCXX_THROW(ExecBufferFullException,
				"Exec::StringOutputGatherer::doHandleData(): buffer full");
		}

		output += String(data, dataLen);
	}
	String& m_output;
	String& m_erroutput;
	int m_outputLimit;
};

/////////////////////////////////////////////////////////////////////////////
class SingleStringInputCallback : public InputCallback
{
public:
	SingleStringInputCallback(const String& s)
		: m_s(s)
	{
	}
private:
	virtual void doGetData(Array<char>& inputBuffer, const ProcessRef& theProc, size_t streamIndex)
	{
		if (m_s.length() > 0)
		{
			inputBuffer.insert(inputBuffer.end(), m_s.c_str(), m_s.c_str() + m_s.length());
			m_s.erase();
		}
		else if (theProc->in()->isOpen())
		{
			theProc->in()->close();
		}
	}
	String m_s;
};

}// end anonymous namespace

/////////////////////////////////////////////////////////////////////////////
Process::Status executeProcessAndGatherOutput(
	char const * const command[],
	String& output,
	char const * const envVars[],
	const Timeout& timeout,
	int outputLimit,
	char const * input)
{
	if (g_execMockObject.get())
	{
		return g_execMockObject.get()->executeProcessAndGatherOutput(command, output, envVars, timeout, outputLimit, input);
	}
	return feedProcessAndGatherOutput(spawn(command, envVars),
		output, timeout, outputLimit, input);
}

/////////////////////////////////////////////////////////////////////////////
Process::Status executeProcessAndGatherOutput(
	char const * const command[],
	String& output,
	String& erroutput,
	char const * const envVars[],
	const Timeout& timeout,
	int outputLimit,
	char const * input)
{
	if (g_execMockObject.get())
	{
		return g_execMockObject.get()->executeProcessAndGatherOutput2(command, output,
			erroutput, envVars, timeout, outputLimit, input);
	}

	return feedProcessAndGatherOutput(spawn(command, envVars),
		output, erroutput, timeout, outputLimit, input);
}

/////////////////////////////////////////////////////////////////////////////
Process::Status feedProcessAndGatherOutput(
	ProcessRef const & proc,
	String & output,
	Timeout const & timeout,
	int outputLimit,
	String const & input)
{
	Array<ProcessRef> procarr(1, proc);
	SingleStringInputCallback singleStringInputCallback(input);

	StringOutputGatherer gatherer(output, outputLimit);
	processInputOutput(gatherer, procarr, singleStringInputCallback, timeout);
	proc->waitCloseTerm();
	return proc->processStatus();
}

/////////////////////////////////////////////////////////////////////////////
Process::Status feedProcessAndGatherOutput(
	ProcessRef const & proc,
	String & output,
	String & erroutput,
	Timeout const & timeout,
	int outputLimit,
	String const & input)
{
	Array<ProcessRef> procarr(1, proc);
	SingleStringInputCallback singleStringInputCallback(input);

	StringOutputGatherer gatherer(output, erroutput, outputLimit);
	processInputOutput(gatherer, procarr, singleStringInputCallback, timeout);
	proc->waitCloseTerm();
	return proc->processStatus();
}

/////////////////////////////////////////////////////////////////////////////
void
gatherOutput(String& output, const ProcessRef& proc, int timeoutSecs, int outputLimit)
{
	gatherOutput(output, proc, Timeout::relativeWithReset(timeoutSecs), outputLimit);
}
/////////////////////////////////////////////////////////////////////////////
void
gatherOutput(String& output, const ProcessRef& proc, const Timeout& timeout, int outputLimit)
{
	Array<ProcessRef> procs(1, proc);

	StringOutputGatherer gatherer(output, outputLimit);
	SingleStringInputCallback singleStringInputCallback = SingleStringInputCallback(String());
	processInputOutput(gatherer, procs, singleStringInputCallback, timeout);
}

/////////////////////////////////////////////////////////////////////////////
OutputCallback::~OutputCallback()
{

}

/////////////////////////////////////////////////////////////////////////////
void
OutputCallback::handleData(const char* data, size_t dataLen, EOutputSource outputSource, const ProcessRef& theProc, size_t streamIndex, Array<char>& inputBuffer)
{
	doHandleData(data, dataLen, outputSource, theProc, streamIndex, inputBuffer);
}

/////////////////////////////////////////////////////////////////////////////
InputCallback::~InputCallback()
{
}

/////////////////////////////////////////////////////////////////////////////
void
InputCallback::getData(Array<char>& inputBuffer, const ProcessRef& theProc, size_t streamIndex)
{
	doGetData(inputBuffer, theProc, streamIndex);
}

namespace
{
	struct ProcessOutputState
	{
		bool inIsOpen;
		bool outIsOpen;
		bool errIsOpen;
		size_t availableDataLen;

		ProcessOutputState()
			: inIsOpen(true)
			, outIsOpen(true)
			, errIsOpen(true)
			, availableDataLen(0)
		{
		}
	};

}

/////////////////////////////////////////////////////////////////////////////
void
processInputOutput(OutputCallback& output, Array<ProcessRef>& procs, InputCallback& input, const Timeout& timeout)
{
	TimeoutTimer timer(timeout);

	Array<ProcessOutputState> processStates(procs.size());
	int numOpenPipes(procs.size() * 2); // count of stdout & stderr. Ignore stdin for purposes of algorithm termination.

	Array<Array<char> > inputs(processStates.size());
	for (size_t i = 0; i < processStates.size(); ++i)
	{
		input.getData(inputs[i], procs[i], i);
		processStates[i].availableDataLen = inputs[i].size();
		if (!procs[i]->out()->isOpen())
		{
			processStates[i].outIsOpen = false;
		}
		if (!procs[i]->err()->isOpen())
		{
			processStates[i].errIsOpen = false;
		}
		if (!procs[i]->in()->isOpen())
		{
			processStates[i].inIsOpen = false;
		}

	}

	timer.start();

	while (numOpenPipes > 0)
	{
		Select::SelectObjectArray selObjs;
		std::map<int, int> inputIndexProcessIndex;
		std::map<int, int> outputIndexProcessIndex;
		for (size_t i = 0; i < procs.size(); ++i)
		{
			if (processStates[i].outIsOpen)
			{
				Select::SelectObject selObj(procs[i]->out()->getReadSelectObj());
				selObj.waitForRead = true;
				selObjs.push_back(selObj);
				inputIndexProcessIndex[selObjs.size() - 1] = i;
			}
			if (processStates[i].errIsOpen)
			{
				Select::SelectObject selObj(procs[i]->err()->getReadSelectObj());
				selObj.waitForRead = true;
				selObjs.push_back(selObj);
				inputIndexProcessIndex[selObjs.size() - 1] = i;
			}
			if (processStates[i].inIsOpen && processStates[i].availableDataLen > 0)
			{
				Select::SelectObject selObj(procs[i]->in()->getWriteSelectObj());
				selObj.waitForWrite = true;
				selObjs.push_back(selObj);
				outputIndexProcessIndex[selObjs.size() - 1] = i;
			}

		}

		int selectrval = Select::selectRW(selObjs, timer.asRelativeTimeout());
		switch (selectrval)
		{
			case Select::SELECT_ERROR:
			{
				BLOCXX_THROW_ERRNO_MSG(ExecErrorException, "Exec::gatherOutput: error selecting on stdout and stderr");
			}
			break;
			case Select::SELECT_TIMEOUT:
			{
				timer.loop();
				if (timer.expired())
				{
					BLOCXX_THROW(ExecTimeoutException, "Exec::gatherOutput: timedout");
				}
			}
			break;
			default:
			{
				int availableToFind = selectrval;

				// reset the timeout counter
				timer.resetOnLoop();

				for (size_t i = 0; i < selObjs.size() && availableToFind > 0; ++i)
				{
					if (!selObjs[i].readAvailable)
					{
						continue;
					}
					else
					{
						--availableToFind;
					}
					int streamIndex = inputIndexProcessIndex[i];
					UnnamedPipeRef readstream;
					if (processStates[streamIndex].outIsOpen)
					{
						if (procs[streamIndex]->out()->getReadSelectObj() == selObjs[i].s)
						{
							readstream = procs[streamIndex]->out();
						}
					}

					if (!readstream && processStates[streamIndex].errIsOpen)
					{
						if (procs[streamIndex]->err()->getReadSelectObj() == selObjs[i].s)
						{
							readstream = procs[streamIndex]->err();
						}
					}

					if (!readstream)
					{
						continue; // for loop
					}

					char buff[1024];
					int readrc = readstream->read(buff, sizeof(buff) - 1);
					if (readrc == 0)
					{
						if (readstream == procs[streamIndex]->out())
						{
							processStates[streamIndex].outIsOpen = false;
							procs[streamIndex]->out()->close();
						}
						else
						{
							processStates[streamIndex].errIsOpen = false;
							procs[streamIndex]->err()->close();
						}
						--numOpenPipes;
					}
					else if (readrc == -1)
					{
						BLOCXX_THROW_ERRNO_MSG(ExecErrorException, "Exec::gatherOutput: read error");
					}
					else
					{
						buff[readrc] = '\0';
						output.handleData(
							buff,
							readrc,
							readstream == procs[streamIndex]->out() ? E_STDOUT : E_STDERR,
							procs[streamIndex],
							streamIndex, inputs[streamIndex]);
					}
				}

				// handle stdin for all processes which have data to send to them.
				for (size_t i = 0; i < selObjs.size() && availableToFind > 0; ++i)
				{
					if (!selObjs[i].writeAvailable)
					{
						continue;
					}
					else
					{
						--availableToFind;
					}
					int streamIndex = outputIndexProcessIndex[i];
					UnnamedPipeRef writestream;
					if (processStates[streamIndex].inIsOpen)
					{
						writestream = procs[streamIndex]->in();
					}

					if (!writestream)
					{
						continue; // for loop
					}

					size_t offset = inputs[streamIndex].size() - processStates[streamIndex].availableDataLen;
					int writerc = writestream->write(&inputs[streamIndex][offset], processStates[streamIndex].availableDataLen);
					if (writerc == -1 && errno == EPIPE)
					{
						processStates[streamIndex].inIsOpen = false;
						procs[streamIndex]->in()->close();
					}
					else if (writerc == -1)
					{
						BLOCXX_THROW_ERRNO_MSG(ExecErrorException, "Exec::gatherOutput: write error");
					}
					else if (writerc != 0)
					{
						inputs[streamIndex].erase(inputs[streamIndex].begin(), inputs[streamIndex].begin() + writerc);
						input.getData(inputs[streamIndex], procs[streamIndex], streamIndex);
						processStates[streamIndex].availableDataLen = inputs[streamIndex].size();
					}
				}
			}
			break;
		}
	}
}

void processInputOutput(const String& input, String& output, const ProcessRef& process,
	const Timeout& timeout, int outputLimit)
{
	Array<ProcessRef> procs;
	procs.push_back(process);

	StringOutputGatherer gatherer(output, outputLimit);
	SingleStringInputCallback singleStringInputCallback = SingleStringInputCallback(input);
	processInputOutput(gatherer, procs, singleStringInputCallback, timeout);
}


} // end namespace Exec

} // end namespace BLOCXX_NAMESPACE

