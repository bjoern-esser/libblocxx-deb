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


#include "TestSuite.hpp"
#include "TestCaller.hpp"
#include "MutexTestCases.hpp"
#include "blocxx/Mutex.hpp"
#include "blocxx/Thread.hpp"
#include <iostream>

using std::cout;
using std::endl;
using std::cerr;

using namespace blocxx;

void MutexTestCases::setUp()
{
}

void MutexTestCases::tearDown()
{
}

static Mutex g_mutex;
static int g_int;

namespace {

class testClass
{
public:
	static Mutex m;
};

class testThread1: public Thread
{
protected:
	Int32 run()
	{
		try
		{
			cout << "\nIn run" << endl;
			g_mutex.acquire();
			g_int = 2;
			g_mutex.release();
		}
		catch(Exception& e)
		{
			cout << e << endl;
			throw;
		}
		catch(...)
		{
			cout << "Unknown exception in testThread1::run" << endl;
			throw;
		}
		return 0;
	}
};

Mutex testClass::m;

} // end anonymous namespace

void MutexTestCases::testAcquireRelease()
{
	Mutex m;

	m.acquire();
	m.release();

	// defaults to recursive mutex, so test that
	m.acquire();
	m.acquire();
	m.release();
	m.release();

	// try a static class mutex
	testClass::m.acquire();
	testClass::m.release();

	// test with multiple threads
	try
	{
		g_int = 1;
		g_mutex.acquire();
		testThread1 t1;
		t1.start();
		Thread::sleep(100);
		unitAssert(g_int == 1);
		g_mutex.release();
		Thread::sleep(100);
		unitAssert(g_int == 2);
	}
	catch(Exception& e)
	{
		cout << e << endl;
		unitAssert(0);
	}
	catch(...)
	{
		cout << "Unknown exception in MutexTestCases::testAcquireRelease"
			<< endl;
		unitAssert(0);
	}
}

Test* MutexTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("Mutex");

	testSuite->addTest (new TestCaller <MutexTestCases>
			("testAcquireRelease",
			&MutexTestCases::testAcquireRelease));

	return testSuite;
}

