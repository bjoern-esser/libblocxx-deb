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
 * @author Bart Whiteley
 * @author Dan Nuffer
 */


#include "blocxx/BLOCXX_config.h"
#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"

#include <iostream>

#include "blocxx/Exec.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/UnnamedPipe.hpp"
#include "blocxx/EnvVars.hpp"

using namespace blocxx;

#if defined(BLOCXX_DEBUG_MEMORY) && !defined(BLOCXX_WIN32)
AUTO_UNIT_TEST(MemTracerTestCases_testSomething)
{
	enum codes
	{
		UNDERRUN=1,
		OVERRUN=2,
		UNKNOWN_ADDR=3,
		DOUBLE_DELETE=4,
		DOUBLE_DELETE_NOFREE=5,
		AGGRESSIVE=7
	};


	String execName = "./MemTracerTest";

	bool wasDisabled = false;
	EnvVars env(EnvVars::E_CURRENT_ENVIRONMENT);
	if (getenv("BLOCXX_MEM_DISABLE") && getenv("BLOCXX_MEM_DISABLE")[0] == '1')
	{
		wasDisabled = true;
		env.setVar("BLOCXX_MEM_DISABLE=0");
	}

	env.setVar("BLOCXX_MEM_AGGRESSIVE=0");
	env.setVar("BLOCXX_MEM_NOFREE=0");

	Array<String> cmd;
	cmd.append(execName);
	cmd.append(String(UNDERRUN));
	ProcessRef rval = Exec::spawn(cmd, env);
	String er = rval->err()->readAll();
	rval->waitCloseTerm();
	size_t idx = er.indexOf("UNDERRUN");
	unitAssert(idx != String::npos);

	cmd.clear();
	cmd.append(execName);
	cmd.append(String(OVERRUN));
	rval = Exec::spawn(cmd, env);
	er = rval->err()->readAll();
	rval->waitCloseTerm();
	idx = er.indexOf("OVERRUN");
	unitAssert(idx != String::npos);

	cmd.clear();
	cmd.append(execName);
	cmd.append(String(UNKNOWN_ADDR));
	rval = Exec::spawn(cmd, env);
	er = rval->err()->readAll();
	rval->waitCloseTerm();
	idx = er.indexOf("UNKNOWN ADDRESS");
	unitAssert(idx != String::npos);

	cmd.clear();
	cmd.append(execName);
	cmd.append(String(DOUBLE_DELETE));
	rval = Exec::spawn(cmd, env);
	er = rval->err()->readAll();
	rval->waitCloseTerm();
	idx = er.indexOf("DOUBLE DELETE");
	unitAssert(idx != String::npos);

	env.setVar("BLOCXX_MEM_NOFREE=1");

	cmd.clear();
	cmd.append(execName);
	cmd.append(String(DOUBLE_DELETE));
	rval = Exec::spawn(cmd, env);
	er = rval->err()->readAll();
	rval->waitCloseTerm();
	idx = er.indexOf("DOUBLE DELETE (NOFREE)");
	unitAssert(idx != String::npos);

	env.setVar("BLOCXX_MEM_NOFREE=0");

	cmd.clear();
	cmd.append(execName);
	cmd.append(String(AGGRESSIVE));
	rval = Exec::spawn(cmd, env);
	er = rval->err()->readAll();
	rval->waitCloseTerm();
	unitAssert(rval->processStatus().terminatedSuccessfully());

	env.setVar("BLOCXX_MEM_AGGRESSIVE=1");

	cmd.clear();
	cmd.append(execName);
	cmd.append(String(AGGRESSIVE));
	rval = Exec::spawn(cmd, env);
	er = rval->err()->readAll();
	rval->waitCloseTerm();
	idx = er.indexOf("OVERRUN");
	unitAssert(idx != String::npos);
}
#endif // #if defined(BLOCXX_DEBUG_MEMORY) && !defined(BLOCXX_WIN32)
