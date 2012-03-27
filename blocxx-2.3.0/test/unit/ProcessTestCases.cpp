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


#include "blocxx/BLOCXX_config.h"
#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"
#include "blocxx/String.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/Process.hpp"
#include "blocxx/FileSystem.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/UnnamedPipe.hpp"
#include "blocxx/Exec.hpp"

#include <iostream>

using namespace blocxx;

#ifdef BLOCXX_WIN32
static const char* APP1_NAME = "FillOutputBuffer.exe";
static const char* APP2_NAME = "ChildProcess.exe cat";
#else
static const char* APP1_NAME = "fillOutputBuffer";
static const char* APP2_NAME = "/bin/cat";
#endif

AUTO_UNIT_TEST(ProcessTestCases_testSubprocessBlockingOutput)
{
	String executablePath = Format("%1%2%3",  FileSystem::Path::getCurrentWorkingDirectory(), BLOCXX_FILENAME_SEPARATOR, APP1_NAME);

	// Non-blocking (because there is no output)
	ProcessRef noc = Exec::spawn(StringArray(1,executablePath) += "0");
	unitAssertNoThrow( noc->waitCloseTerm(Timeout::relative(5.0), Timeout::relative(10.0), Timeout::relative(15.0)) );
	unitAssert( noc->processStatus().terminatedSuccessfully() );

	// Blocking output possible (likely due to large amounts of output)
	const size_t bytesToRead = 32768;
	ProcessRef hoc = Exec::spawn(StringArray(1,executablePath) += String(bytesToRead));
	// This should be done in a separate thread.  If things are horribly broken,
	// this will block on both ends and the tests will never complete.
	String output = hoc->out()->readAll();
	unitAssert(bytesToRead == output.length());
	unitAssertNoThrow(hoc->waitCloseTerm(Timeout::relative(5.0),Timeout::relative(10.0),Timeout::relative(15.0)));
	unitAssert(hoc->processStatus().terminatedSuccessfully());
}

AUTO_UNIT_TEST(ProcessTestCases_testSpawn)
{
	ProcessRef rval = Exec::spawn(StringArray(1,APP2_NAME));
	rval->in()->write("hello world\n", 12);
	rval->in()->close();
	String out = rval->out()->readAll();
	rval->waitCloseTerm();
	unitAssert(rval->processStatus().terminatedSuccessfully());
	unitAssert(out == "hello world\n");
	unitAssertThrows(Exec::spawn(StringArray(1,"/a/non-existent/binary")));
}
