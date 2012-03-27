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
#include "TestSuite.hpp"
#include "TestCaller.hpp"
#include "LogAppenderTestCases.hpp"
#include "blocxx/LogAppender.hpp"
#include "blocxx/NullAppender.hpp"

using namespace blocxx;

void LogAppenderTestCases::setUp()
{
}

void LogAppenderTestCases::tearDown()
{
}

class TestLogAppender : public LogAppender
{
public:
	TestLogAppender()
	: LogAppender()
	{}

	virtual void doProcessLogMessage(const String& formattedMessage, const LogMessage& message) const
	{
	}
};

void LogAppenderTestCases::testDefault()
{
	unitAssert( dynamic_pointer_cast<NullAppender>(LogAppender::getDefaultLogAppender()) );
	LogAppender::setDefaultLogAppender(new TestLogAppender);
	unitAssert( dynamic_pointer_cast<TestLogAppender>(LogAppender::getDefaultLogAppender()) );
}

void LogAppenderTestCases::testThread()
{
	unitAssert( !LogAppender::getThreadLogAppender() );
	LogAppender::setThreadLogAppender(new TestLogAppender);
	unitAssert( dynamic_pointer_cast<TestLogAppender>(LogAppender::getThreadLogAppender()) );
	LogAppender::setThreadLogAppender(LogAppenderRef());
	unitAssert( !LogAppender::getThreadLogAppender() );
}

Test* LogAppenderTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("LogAppender");

	ADD_TEST_TO_SUITE(LogAppenderTestCases, testDefault);
	ADD_TEST_TO_SUITE(LogAppenderTestCases, testThread);

	return testSuite;
}

