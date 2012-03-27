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


#include "TestSuite.hpp"
#include "TestCaller.hpp"
#include "ExceptionTestCases.hpp"
#include "blocxx/Semaphore.hpp"
#include "blocxx/Exception.hpp"
#include "blocxx/Thread.hpp"

using namespace blocxx;

BLOCXX_DECLARE_EXCEPTION(Test);
BLOCXX_DEFINE_EXCEPTION(Test);

void ExceptionTestCases::setUp()
{
}

void ExceptionTestCases::tearDown()
{
}

static Semaphore g_sem;
static bool g_caught = false;

namespace {
class ExTestRunnable: public Thread
{
protected:
	Int32 run()
	{
		try
		{
			BLOCXX_THROW(TestException, "test");
		}
		catch(Exception& e)
		{
			g_caught = true;
			g_sem.signal();
		}
		return 0;
	}
};
} // end anonymous namespace

void ExceptionTestCases::testThreadThrow()
{
	g_caught = false;
	try
	{
		BLOCXX_THROW(TestException, "test");
	}
	catch(TestException& e)
	{
		g_caught = true;
	}
	unitAssert(g_caught);

	g_caught = false;
	ExTestRunnable t1;
	t1.start();
	unitAssert(g_sem.timedWait(Timeout::relative(30)));

	unitAssert(g_caught);
}

namespace 
{
BLOCXX_DECLARE_EXCEPTION(test1);
BLOCXX_DEFINE_EXCEPTION(test1);
BLOCXX_DECLARE_EXCEPTION(test2);
BLOCXX_DEFINE_EXCEPTION(test2);
}

void ExceptionTestCases::testSubException()
{
	try
	{
		try
		{
			BLOCXX_THROW_ERR(test1Exception, "message 1", 1);
		}
		catch (const test1Exception& e)
		{
			BLOCXX_THROW_ERR_SUBEX(test2Exception, "message 2", 2, e);
		}
		unitAssert(false);
	}
	catch (const test2Exception& e2)
	{
		unitAssert(e2.getFile() != 0);
		unitAssert(e2.getLine() != 0);
		unitAssert(e2.getMessage() == String("message 2"));
		unitAssert(e2.getErrorCode() == 2);
		unitAssert(e2.type() == String("test2Exception"));
		unitAssert(e2.getSubClassId() == Exception::UNKNOWN_SUBCLASS_ID);
		unitAssert(e2.what() == String("message 2"));
		unitAssert(e2.getSubException() != 0);

		unitAssert(e2.getSubException()->getFile() != 0);
		unitAssert(e2.getSubException()->getLine() != 0);
		unitAssert(e2.getSubException()->getMessage() == String("message 1"));
		unitAssert(e2.getSubException()->getErrorCode() == 1);
		unitAssert(e2.getSubException()->type() == String("test1Exception"));
		unitAssert(e2.getSubException()->getSubClassId() == Exception::UNKNOWN_SUBCLASS_ID);
		unitAssert(e2.getSubException()->what() == String("message 1"));
		unitAssert(e2.getSubException()->getSubException() == 0);
	}

}

Test* ExceptionTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("Exception");

	ADD_TEST_TO_SUITE(ExceptionTestCases, testThreadThrow);
	ADD_TEST_TO_SUITE(ExceptionTestCases, testSubException);
	

	return testSuite;
}

