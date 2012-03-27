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
 * @author Andrey Mukha
 */

#include "blocxx/BLOCXX_config.h"
#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"

#include "blocxx/UnnamedPipe.hpp"
#include "blocxx/String.hpp"
#include "blocxx/Thread.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/System.hpp"
#include "blocxx/Process.hpp"

#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/types.h>

#ifndef BLOCXX_WIN32
#include <sys/wait.h>
#endif

using namespace blocxx;

#if !defined(BLOCXX_DARWIN)
AUTO_UNIT_TEST(UnnamedPipeTestCases_testDescriptorPassing)
{
// Mac OS X needs a workaround which prevents a descriptor being passed to the same process.
	UnnamedPipeRef up1 = UnnamedPipe::createUnnamedPipe();
	UnnamedPipeRef up2, up3;
	UnnamedPipe::createConnectedPipes(up2, up3);
	up1->passDescriptor(up3->getInputDescriptor());
	AutoDescriptor d1 = up1->receiveDescriptor(up1);
	up1->passDescriptor(up3->getOutputDescriptor());
	AutoDescriptor d2 = up1->receiveDescriptor(up1);

	UnnamedPipeRef upPassed = UnnamedPipe::createUnnamedPipeFromDescriptor(d2, d1);

	String text = "abc";
	up2->writeString(text);
	String s;
	upPassed->readString(s);
	unitAssert(s == text);

	text = "xyz";
	upPassed->writeString(text);
	up2->readString(s);
	unitAssert(s == text);
}
#endif

#ifdef BLOCXX_WIN32
AUTO_UNIT_TEST(UnnamedPipeTestCases_testProcessDescriptorPassing)
{
	CHAR buf[256];
	String text = "abcxyz";

	UnnamedPipeRef up1 = UnnamedPipe::createUnnamedPipe();

	ltoa((long)(up1->getOutputDescriptor()), buf, 10);
	SetEnvironmentVariable("PipeHandle", buf);

	DWORD procId = GetCurrentProcessId();

	ltoa(procId, buf, 10);
	SetEnvironmentVariable("ParentProcessId", buf);

	STARTUPINFO si = { sizeof(si) };
	SECURITY_ATTRIBUTES saProcess;
	PROCESS_INFORMATION piProcess;

	saProcess.nLength = sizeof(saProcess);
	saProcess.lpSecurityDescriptor = NULL;
	saProcess.bInheritHandle = TRUE;

	BOOL bFuncRetn = CreateProcess(NULL,
			"ChildProcess",			// command line
			&saProcess,				// process security attributes
			NULL,						// primary thread security attributes
			TRUE,						// handles are inherited
			0,						// creation flags
			0,						// use parent's environment
			NULL,						// use parent's current directory
			&si,						// STARTUPINFO pointer
			&piProcess);				// receives PROCESS_INFORMATION

	unitAssert(bFuncRetn);

	AutoDescriptor d1 = up1->receiveDescriptor();
	unitAssert(d1);
	AutoDescriptor d2 = up1->receiveDescriptor();
	unitAssert(d2);
	UnnamedPipeRef upPassed = UnnamedPipe::createUnnamedPipeFromDescriptor(d2, d1);
	unitAssert(upPassed)

	String s;
	unitAssertEquals(upPassed->writeString("abc"), 4);
	unitAssertEquals(upPassed->readString(s), 8);
	unitAssertEquals(s, text);


	// Wait until child process exits.
	WaitForSingleObject(piProcess.hProcess, INFINITE);

	// Close process and thread handles.
	CloseHandle(piProcess.hProcess);
	CloseHandle(piProcess.hThread);

}

#else

AUTO_UNIT_TEST(UnnamedPipeTestCases_testProcessDescriptorPassing)
{

	UnnamedPipeRef upp;
	UnnamedPipeRef upc;
	UnnamedPipe::createConnectedPipes(upp, upc);
	unitAssert(upp);
	unitAssert(upc);
	String text = "abc";
	::pid_t forkrv = ::fork();
	unitAssertNotEquals(forkrv, -1);
	if (forkrv == 0)
	{
		// child sends descriptors
		UnnamedPipeRef up2, up3;
		UnnamedPipe::createConnectedPipes(up2, up3);
		unitAssert(up2);
		unitAssert(up3);
		upc->passDescriptor(up3->getInputDescriptor(), upc);

		upc->passDescriptor(up3->getOutputDescriptor(), upc);
		unitAssertEquals(up2->writeString(text), 4);
		String s;
		unitAssertEquals(up2->readString(s), 4);
		unitAssertEquals(s, "xyz");
		_exit(0);
	}
	else
	{
		// parent receives descriptors
		AutoDescriptor d1 = upp->receiveDescriptor(upp);
		unitAssert(d1);
		AutoDescriptor d2 = upp->receiveDescriptor(upp);
		unitAssert(d2);
		UnnamedPipeRef upPassed = UnnamedPipe::createUnnamedPipeFromDescriptor(d2, d1);
		unitAssert(upPassed)
		String s;
		unitAssertEquals(upPassed->readString(s), 4);
		unitAssertEquals(s, text);
		unitAssertEquals(upPassed->writeString("xyz"), 4);
		int status = 0;
		unitAssertEquals(::waitpid(forkrv, &status, 0), forkrv);
		unitAssertNotEquals(WIFEXITED(status), 0);
		unitAssertEquals(WEXITSTATUS(status), 0);
	}
}
#endif
