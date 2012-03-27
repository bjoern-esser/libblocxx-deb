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
#include "ConditionTestCases.hpp"
#include "blocxx/Condition.hpp"
#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"
#include "blocxx/DateTime.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/Thread.hpp"

#include <limits>

using namespace blocxx;
using namespace std;

void ConditionTestCases::setUp()
{
}

void ConditionTestCases::tearDown()
{
}

void ConditionTestCases::testTimedWait()
{
	NonRecursiveMutex mtx;
	NonRecursiveMutexLock lock(mtx);
	Condition cond;
	DateTime a = DateTime::getCurrent();
	unitAssert(!cond.timedWait(lock, Timeout::relative(0.01)));
	DateTime b = DateTime::getCurrent();
	DateTime diff = b - a;
	unitAssert(diff.get() == 0);
	unitAssert(diff.getMicrosecond() >= 1000);
}

namespace
{
	class Waiter : public Thread
	{
	public:
		Waiter()
		: m_done(false),
		  m_wait_count(0),
		  m_timeout_count(0)
		{
		}

		void my_cancel()
		{
			NonRecursiveMutexLock lock(m_mtx);
			m_done = true;
			m_cond.notifyOne();
		}

		Int32 run()
		{
			DateTime dt = DateTime((std::numeric_limits<time_t>::max)());
			NonRecursiveMutexLock lock(m_mtx);
			while (!m_done)
			{
				bool rv = m_cond.timedWait(lock, Timeout::absolute(dt));
				++m_wait_count;
				m_timeout_count += !rv;
			}
			return 0;
		}

		unsigned wait_count() const
		{
			return m_wait_count;
		}

		unsigned timeout_count() const
		{
			return m_timeout_count;
		}

	private:
		Condition m_cond;
		NonRecursiveMutex m_mtx;
		bool m_done;
		unsigned m_wait_count;
		unsigned m_timeout_count;
	};
}

void ConditionTestCases::testTimedWaitLong()
{
	// We have this test because we found that Solaris won't let you wait
	// more than about three years (10 ** 8 seconds) -- pthread_cond_timedwait
	// immediately returns with errno == EINVAL -- and this led to a busy
	// wait when we did a timedWait with an absolute timeout time far into
	// the future.

	Waiter th;
	th.start();
	Thread::sleep(100);
	th.my_cancel();
	th.join();
	unitAssert(th.timeout_count() == 0);
	// Allow for the possibility of one spurious wakeup, but no more
	unitAssert(th.wait_count() <= 2);
}


Test* ConditionTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("Condition");

	ADD_TEST_TO_SUITE(ConditionTestCases, testTimedWait);
	ADD_TEST_TO_SUITE(ConditionTestCases, testTimedWaitLong);

	return testSuite;
}

