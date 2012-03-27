/*******************************************************************************
* Copyright (C) 2005,2009, Quest Software, Inc. All rights reserved.
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
#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"

#include "blocxx/Exec.hpp"
#include "blocxx/UnnamedPipe.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/FileSystem.hpp"

#include <utility> // for pair
#include <cassert>
#include <csignal>

#if defined(BLOCXX_HAVE_SYS_WAIT_H) && defined(BLOCXX_WIFEXITED_NEEDS_WAIT_H)
#include <sys/wait.h>
#endif

using namespace blocxx;
using namespace std;

#ifdef BLOCXX_WIN32
static const char* TRUE_APP = "ChildProcess.exe true ";
static const char* FALSE_APP = "ChildProcess.exe false ";
static const char* ECHO_APP = "ChildProcess.exe echo ";
static const char* CAT_APP = "ChildProcess.exe cat ";
#else
static const char* TRUE_APP = BLOCXX_TRUE_PATHNAME;
static const char* FALSE_APP = BLOCXX_FALSE_PATHNAME;
static const char* ECHO_APP = "/bin/echo";
static const char* CAT_APP = "/bin/cat";
#endif

AUTO_UNIT_TEST(ExecTestCases_testExecuteProcessAndGatherOutput)
{
	{
		String output;
		Process::Status status = Exec::executeProcessAndGatherOutput(StringArray(1, TRUE_APP), output);
		unitAssert(output.empty());
		unitAssert(status.terminatedSuccessfully());
	}

	{
		String output;
		Process::Status status = Exec::executeProcessAndGatherOutput(StringArray(1, FALSE_APP), output);
		unitAssert(output.empty());
		unitAssert(status.exitTerminated());
		// the return value of false is not always 1.
		unitAssert(status.exitStatus() != 0);
	}

	{
		String output;
		Process::Status status = Exec::executeProcessAndGatherOutput(StringArray(1, ECHO_APP) += "false", output);
		StringArray out_array = output.tokenize();
		unitAssertEquals(size_t(1), out_array.size());
		unitAssertEquals("false", *out_array.begin());
		unitAssert(status.terminatedSuccessfully());
	}

	{
		String output;
		Process::Status status = Exec::executeProcessAndGatherOutput(StringArray(1, CAT_APP), output, Timeout::relative(10.0), -1, "hello to world\n");
		unitAssert(output == "hello to world\n");
		unitAssert(status.terminatedSuccessfully());
	}

	// only do timeout tests if we're doing the long test, since it's slowwww
	if (getenv("BLOCXXLONGTEST"))
	{
		// do a timeout
		String output;
		try
		{
			StringArray cmd;
			cmd.push_back("/bin/sh");
			cmd.push_back("-c");

			// We want a delay in this test before any output occurs.  This is
			// important because on some platforms (like Linux), the output could be
			// received even if the timeout was set as 0.  With a sleep at the
			// beginning of the test, this should hopefully avoid any scheduling
			// issues with a child process going too fast on an unloaded machine.

			// The sequence should go like this:
			// 1. sleep 1 - test blocks
			// 2. echo before - test gets it then resets timeout to 2 seconds.
			// 3. sleep 4 starts
			// 4. test times out after 2 seconds and throws.
			cmd.push_back("sleep 1; echo before; sleep 4; echo after");
			Process::Status status = Exec::executeProcessAndGatherOutput(cmd, output, Timeout::relativeWithReset(2.0));
			unitAssert(0);
		}
		catch (const ExecTimeoutException& e)
		{
		}
		unitAssert(output == "before\n");
	}

	{
		// test output limit
		int processstatus = 0;
		String output;
		try
		{
			Process::Status status = Exec::executeProcessAndGatherOutput(StringArray(1, ECHO_APP) += "12345", output, Timeout::infinite, 4);
			unitAssert(0);
		}
		catch (const ExecBufferFullException& e)
		{
		}
		unitAssert(output == "1234");
	}

#ifndef BLOCXX_WIN32 //in Windows signals are not supported
	{
		// test a process that dies from a signal. SIGTERM == 15
		String output;
		Process::Status status = Exec::executeProcessAndGatherOutput(StringArray(1, FileSystem::Path::getCurrentWorkingDirectory() + "/exitWithSignal") += "15", output);
		unitAssert(!status.exitTerminated());
		unitAssert(status.signalTerminated());
		unitAssert(status.termSignal() == 15);
	}
#endif
}

class TestOutputGatherer : public Exec::OutputCallback
{
public:
	TestOutputGatherer(Array<pair<ProcessRef, String> >& outputs)
		: m_outputs(outputs)
	{
	}
private:
	virtual void doHandleData(const char* data, size_t dataLen, Exec::EOutputSource outputSource, const ProcessRef& theProc, size_t streamIndex, Array<char>& inputBuffer)
	{
		assert(m_outputs[streamIndex].first == theProc); // too bad we can't do unitAssert...
		assert(outputSource == Exec::E_STDOUT);
		m_outputs[streamIndex].second += String(data, dataLen);
	}

	Array<pair<ProcessRef, String> >& m_outputs;
};

class TestInputCallback : public Exec::InputCallback
{
public:
	TestInputCallback(const Array<Array<char> >& inputs)
		: m_inputs(inputs)
	{
	}
	TestInputCallback()
	{
	}
private:
	virtual void doGetData(Array<char>& inputBuffer, const ProcessRef& theProc, size_t streamIndex)
	{
		if (streamIndex < m_inputs.size() && m_inputs[streamIndex].size() > 0)
		{
			inputBuffer.insert(inputBuffer.end(), m_inputs[streamIndex].begin(), m_inputs[streamIndex].end());
			m_inputs[streamIndex].clear();
		}
		else if (theProc->in()->isOpen())
		{
			theProc->in()->close();
		}
	}
	Array<Array<char> > m_inputs;
};

AUTO_UNIT_TEST(ExecTestCases_testgatherOutput)
{
	{
		Array<ProcessRef> procs;
		Array<pair<ProcessRef, String> > outputs;
		TestInputCallback inputs;
		const int TEST_PROC_COUNT = 5;
		for (int i = 0; i < TEST_PROC_COUNT; ++i)
		{
			ProcessRef curStream(Exec::spawn(StringArray(1, ECHO_APP) += String(i)));
			procs.push_back(curStream);
			outputs.push_back(make_pair(curStream, String()));
		}

		TestOutputGatherer testOutputGatherer(outputs);
		processInputOutput(testOutputGatherer, procs, inputs, Timeout::relative(10.0));
		for (int i = 0; i < TEST_PROC_COUNT; ++i)
		{
			procs[i]->waitCloseTerm();
			Process::Status status = procs[i]->processStatus();
			unitAssert(status.terminatedSuccessfully());
			unitAssert(outputs[i].second == String(i) + "\n");
		}
	}

	{
		Array<ProcessRef> procs;
		Array<pair<ProcessRef, String> > outputs;
		Array<Array<char> > inputData;
		const int TEST_PROC_COUNT = 5;
		for (int i = 0; i < TEST_PROC_COUNT; ++i)
		{
			ProcessRef curStream(Exec::spawn(StringArray(1, CAT_APP)));
			procs.push_back(curStream);
			outputs.push_back(make_pair(curStream, String()));
			String num(i);
			num += '\n';
			inputData.push_back(Array<char>(num.c_str(), num.c_str() + num.length()));
		}
		TestInputCallback inputs(inputData);

		TestOutputGatherer testOutputGatherer(outputs);
		processInputOutput(testOutputGatherer, procs, inputs, Timeout::relative(10.0));
		for (int i = 0; i < TEST_PROC_COUNT; ++i)
		{
			procs[i]->waitCloseTerm();
			Process::Status status = procs[i]->processStatus();
			unitAssert(status.terminatedSuccessfully());
			unitAssert(outputs[i].second == String(i) + "\n");
		}
	}

	// only do timeout tests if we're doing the long test, since it's slowwww
	if (getenv("BLOCXXLONGTEST"))
	{
		Array<ProcessRef> streams;
		Array<pair<ProcessRef, String> > outputs;
		const int TEST_PROC_COUNT = 4;
		const Timeout TEST_TIMEOUT = Timeout::relativeWithReset(2.0);
		TestInputCallback inputs;
		for (int i = 0; i < TEST_PROC_COUNT; ++i)
		{
			StringArray cmd;
			cmd.push_back("/bin/sh");
			cmd.push_back("-c");
			cmd.push_back(Format("sleep %1; echo before; sleep %2; echo after", i, i * i));
			ProcessRef curStream(Exec::spawn(cmd));
			streams.push_back(curStream);
			outputs.push_back(make_pair(curStream, String()));
		}

		TestOutputGatherer testOutputGatherer(outputs);
		try
		{
			processInputOutput(testOutputGatherer, streams, inputs, TEST_TIMEOUT);
			unitAssert(0);
		}
		catch (ExecTimeoutException& e)
		{
		}
		for (int i = 0; i < TEST_PROC_COUNT; ++i)
		{
			streams[i]->waitCloseTerm(Timeout::relative(0.0), Timeout::relative(0.0), Timeout::relative(0.001));
		}
		for (int i = 0; i < TEST_PROC_COUNT; ++i)
		{
			Process::Status status = streams[i]->processStatus();
			if (i * i + i < 2 + TEST_PROC_COUNT) // all the ones that finished before the timout
			{
				unitAssert(status.terminatedSuccessfully());
			}
			else // these ones got killed
			{
#ifdef BLOCXX_SOLARIS
				// The solaris shell eats SIGTERM and returns 143 because the sleep gets killed
				unitAssert(status.exitTerminated());
				unitAssertEquals(status.exitStatus(), 143);

#else
				unitAssert(status.signalTerminated());
#ifdef BLOCXX_WIN32 //in Windows SIGPIPE doesn't exist
				unitAssert(status.termSignal() == SIGTERM );
#else
				unitAssert(status.termSignal() == SIGTERM || status.termSignal() == SIGPIPE);
#endif
#endif
			}
			if (i * i + i < 2 + TEST_PROC_COUNT)
			{
				unitAssert(outputs[i].second == "before\nafter\n");
			}
			else
			{
				// these ones got killed during the middle sleep
				unitAssert(outputs[i].second == "before\n");
			}
		}
	}


}
