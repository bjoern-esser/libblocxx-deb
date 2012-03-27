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
#include "LazyGlobalTestCases.hpp"
#include "blocxx/LazyGlobal.hpp"

#include "blocxx/SharedLibraryLoader.hpp"
#include "blocxx/String.hpp"
#include "blocxx/LogMessage.hpp"
#include "blocxx/LogAppenderScope.hpp"
#include "blocxx/ScopeLogger.hpp"

using namespace blocxx;

namespace
{
	class TestAppender: public LogAppender
	{
	public:
		TestAppender(const char* format = "%m")
			: LogAppender(StringArray(1, "test"), ALL_CATEGORIES, format)
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


	//typedef const std::pair<const char* const, const char* const> textpair;
	struct textpair
	{
		const char* const first;
		const char* const second;
	};
	struct TestScopeLoggerFactory
	{
		static ScopeLogger* create(const textpair& messages)
		{
			return new ScopeLogger(messages.first, messages.second, "test");
		}
	};
	typedef LazyGlobal<ScopeLogger, textpair, TestScopeLoggerFactory> GlobalScopeLogger;
}


void LazyGlobalTestCases::setUp()
{
}

void LazyGlobalTestCases::tearDown()
{
}

void LazyGlobalTestCases::testLazyInitialization()
{
	// This test just makes sure initialization is truly lazy.  It makes use of
	// the ScopeLogger to show when the variable is initialized and destroyed.
	TestAppender* tst = new TestAppender();
	LogAppenderRef appender(tst);
	LogAppenderScope las(appender);
	Logger lgr("test");
	lgr.logDebug("before scope");
	{
		lgr.logDebug("before global");
		GlobalScopeLogger simulatedGlobal = BLOCXX_LAZY_GLOBAL_INIT({"logger created", "logger destroyed"});
		lgr.logDebug("after global");

		// Force the variable to be initialized.
		simulatedGlobal.get();
		lgr.logDebug("after get");
	}
	lgr.logDebug("after scope");
	const StringArray data = tst->getData();

	unitAssertEquals(size_t(7), data.size());
	unitAssertEquals("before scope", data[0]);
	unitAssertEquals("before global", data[1]);
	unitAssertEquals("after global", data[2]);
	unitAssertEquals("logger created", data[3]);
	unitAssertEquals("after get", data[4]);
	unitAssertEquals("logger destroyed", data[5]);
	unitAssertEquals("after scope", data[6]);
}

void LazyGlobalTestCases::testLibraryInitialization()
{
	// This test loads a shared library, calls a function from it (forces the
	// initialization of a LazyGlobal), and checks that constructors and
	// destructors are called for these globals when loading and unloading the
	// library.
#if !defined(BLOCXX_STATIC_SERVICES)
	TestAppender* tst = new TestAppender();
	LogAppenderRef appender(tst);
	LogAppenderScope las(appender);
	Logger lgr("test");

	SharedLibraryLoaderRef loader = SharedLibraryLoader::createSharedLibraryLoader();
	unitAssert(loader);

	String libname = String("libLazyGlobalTestLibrary") + BLOCXX_SHAREDLIB_EXTENSION;

	{
		lgr.logDebug("before load");
		SharedLibraryRef lib = loader->loadSharedLibrary(libname);

		unitAssert( lib );
		void (*forceInitialize)();

		unitAssert( lib->getFunctionPointer("forceInitialize", forceInitialize) );
		lgr.logDebug("before initialize");
		forceInitialize();
		lgr.logDebug("after initialize");
	}
	lgr.logDebug("after unload");

	const StringArray data = tst->getData();

	unitAssertEquals(size_t(10), data.size());
	unitAssertEquals("before load",        data[0]);
	unitAssertEquals("global constructor", data[1]); // Constructed a global ScopeLogger
	unitAssertEquals("before initialize",  data[2]);
	unitAssertEquals("preinit",            data[3]);
	unitAssertEquals("initialize",         data[4]); // Initialized a LazyGlobal<ScopeLogger>
	unitAssertEquals("postinit",           data[5]);
	unitAssertEquals("after initialize",   data[6]);
	unitAssertEquals("destroy",            data[7]); // Destroyed a LazyGlobal<ScopeLogger>
	unitAssertEquals("global destructor",  data[8]); // Destroyed a global ScopeLogger
	unitAssertEquals("after unload",       data[9]);
#endif
}

Test* LazyGlobalTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("LazyGlobal");

	ADD_TEST_TO_SUITE(LazyGlobalTestCases, testLazyInitialization);
	ADD_TEST_TO_SUITE(LazyGlobalTestCases, testLibraryInitialization);

	return testSuite;
}

