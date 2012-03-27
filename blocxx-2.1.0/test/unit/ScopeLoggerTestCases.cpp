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
 * @author Kevin Harris
 */


#include "TestSuite.hpp"
#include "TestCaller.hpp"
#include "blocxx/BLOCXX_config.h"
#include "ScopeLoggerTestCases.hpp"
#include "blocxx/ScopeLogger.hpp"
#include "blocxx/LogAppenderScope.hpp"


#include "blocxx/CerrLogger.hpp"
#include "blocxx/Format.hpp"

using namespace blocxx;

void ScopeLoggerTestCases::setUp()
{
}

void ScopeLoggerTestCases::tearDown()
{
}


namespace
{
	class TestAppender: public LogAppender
	{
	public:
		TestAppender(const char* format = "%m")
			: LogAppender(ALL_COMPONENTS, ALL_CATEGORIES, format)
		{
		}
		virtual ~TestAppender()
		{
		}

		StringArray getData() const
		{
			return m_logLines;
		}

	protected:
		virtual void doProcessLogMessage(const String& formattedMessage, const LogMessage& message) const
		{
			m_logLines.push_back(formattedMessage);
		}

		LogAppenderRef m_realLogAppender;

		// This is mutable because the doLogMessage function must be const.
		mutable StringArray m_logLines;
	};
}

void ScopeLoggerTestCases::testScope()
{
	// Normal scope usage.
	TestAppender* tst = new TestAppender();
	LogAppenderRef appender(tst);
	{
		LogAppenderScope las(appender);
		ScopeLogger logger("Enter", "Exit");
	}
	const StringArray data = tst->getData();

	unitAssert(data.size() == 2);
	unitAssert(data[0].endsWith("Enter"));
	unitAssert(data[1].endsWith("Exit"));
}

void ScopeLoggerTestCases::testScopeException()
{
	// Exception thrown
	TestAppender* tst = new TestAppender();
	LogAppenderRef appender(tst);
	try
	{
		LogAppenderScope las(appender);
		ScopeLogger logger("Enter", "Exit");
		throw int(1);
	}
	catch(int)
	{
		// Expected.
	}
	const StringArray data = tst->getData();

	unitAssert(data.size() == 2);
	unitAssert(data[0].endsWith("Enter"));
	unitAssert(data[1].endsWith("Exit"));
}

void ScopeLoggerTestCases::testScopeFormat()
{
	// DelayedFormat logging...
	TestAppender* tst = new TestAppender();
	LogAppenderRef appender(tst);
	{
		LogAppenderScope las(appender);
		// These initial values will be logged with the "Enter: " prefix.
		int i = 0;
		float j = 3.14;

		// We need some macros to do this kind of thing.
		Reference<DelayedFormat> logger_format(new DelayedFormat("i=%1 j=%2", i, j));
		ScopeLogger logger("Enter: ", "Exit: ", logger_format);

		// Magically change the log message to show these values.
		i = 2;
		j = 2.787;
	}

	const StringArray data = tst->getData();
	unitAssert(data.size() == 2);

	unitAssert(data[0] == "Enter: i=0 j=3.14");
	unitAssert(data[1] == "Exit: i=2 j=2.787");
}

Test* ScopeLoggerTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("ScopeLogger");
	ADD_TEST_TO_SUITE(ScopeLoggerTestCases, testScope);
	ADD_TEST_TO_SUITE(ScopeLoggerTestCases, testScopeException);
	ADD_TEST_TO_SUITE(ScopeLoggerTestCases, testScopeFormat);
	return testSuite;
}

