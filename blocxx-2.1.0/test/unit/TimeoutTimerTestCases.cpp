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
#include "TimeoutTimerTestCases.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/TimeoutTimer.hpp"
#include "blocxx/Thread.hpp"
#include "blocxx/Infinity.hpp"

#include <iostream>
using namespace std;

#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif

using namespace blocxx;

namespace
{
	class TestTimeoutTimer : public TimeoutTimer
	{
	public:
		TestTimeoutTimer(const Timeout& x)
		: TimeoutTimer(x)
		{
		}

		static DateTime currentTime;

		virtual DateTime getCurrentTime() const
		{
			return currentTime;
		}
	};
	DateTime TestTimeoutTimer::currentTime;
}

void TimeoutTimerTestCases::setUp()
{
}

void TimeoutTimerTestCases::tearDown()
{
}

void TimeoutTimerTestCases::testRelative()
{
	{
		Timeout t = Timeout::relative(0.00001);
		TestTimeoutTimer::currentTime = DateTime::getCurrent();
		TestTimeoutTimer tt(t);
		TestTimeoutTimer::currentTime.addSeconds(1);
		tt.loop();
		unitAssert( tt.expired() );
	}
	{
		Timeout t = Timeout::relative(0.00001);
		TestTimeoutTimer::currentTime = DateTime::getCurrent();
		TestTimeoutTimer tt(t);
		TestTimeoutTimer::currentTime.addSeconds(1);
		tt.resetOnLoop();
		unitAssert( tt.expired() );
	}
	{
		Timeout t = Timeout::relative(2);
		TestTimeoutTimer::currentTime = DateTime::getCurrent();
		TestTimeoutTimer tt(t);
		TestTimeoutTimer::currentTime.addSeconds(1);
		tt.loop();
		unitAssert( !tt.expired() );
		TestTimeoutTimer::currentTime.addSeconds(2);
		tt.loop();
		unitAssert( tt.expired() );
	}
	{
		Timeout t = Timeout::infinite;
		TimeoutTimer tt(t);
		tt.start();
		tt.loop();
		unitAssert(tt.infinite());
		unitAssert(!tt.expired());
	}
	{
		Timeout t = Timeout::relative(0);
		TimeoutTimer tt(t);
		tt.start();
		unitAssert(tt.expired());
	}
}

void TimeoutTimerTestCases::testRelativeReset()
{
	{
		Timeout t = Timeout::relativeWithReset(0.0001);
		TestTimeoutTimer::currentTime = DateTime::getCurrent();
		TestTimeoutTimer tt(t);
		TestTimeoutTimer::currentTime.addSeconds(1);
		tt.loop();
		unitAssert( tt.expired() );
		tt.resetOnLoop();
		unitAssert(!tt.expired());
	}
	{
		Timeout t = Timeout::relativeWithReset(0.00001);
		TestTimeoutTimer::currentTime = DateTime::getCurrent();
		TestTimeoutTimer tt(t);
		TestTimeoutTimer::currentTime.addSeconds(1);
		tt.resetOnLoop();
		unitAssert( !tt.expired() );
		TestTimeoutTimer::currentTime.addSeconds(1);
		tt.loop();
		unitAssert( tt.expired() );
	}
}

void TimeoutTimerTestCases::testAbsolute()
{
	{
		DateTime begin = DateTime::getCurrent();
		Timeout t1 = Timeout::relative(0.01);
		TimeoutTimer tt1(t1);
		Timeout absolute = tt1.asAbsoluteTimeout();
		TimeoutTimer absoluteTimer(absolute);
		while (!absoluteTimer.expired())
		{
			absoluteTimer.loop();
		}
		DateTime end = DateTime::getCurrent();
		DateTime diff = end - begin;
		unitAssert(diff.get() == 0);
		unitAssert(diff.getMicrosecond() >= 10000); // timeout value
	}
	// try again using resetOnLoop()
	{
		DateTime begin = DateTime::getCurrent();
		Timeout t1 = Timeout::relative(0.01);
		TimeoutTimer tt1(t1);
		Timeout absolute = tt1.asAbsoluteTimeout();
		TimeoutTimer absoluteTimer(absolute);
		while (!absoluteTimer.expired())
		{
			absoluteTimer.resetOnLoop();
		}
		DateTime end = DateTime::getCurrent();
		DateTime diff = end - begin;
		unitAssert(diff.get() == 0);
		unitAssert(diff.getMicrosecond() >= 10000); // timeout value
	}
}

namespace
{

#ifdef BLOCXX_HAVE_STRUCT_TIMESPEC
Real64 calcDiff(const timeval& x, const timespec& y)
{
	return y.tv_sec - x.tv_sec + static_cast<Real64>(y.tv_nsec - x.tv_usec * 1000) / 1000000000.0;
}

Real64 calcDiff(const timespec& x, const timeval& y)
{
	return y.tv_sec - x.tv_sec + static_cast<Real64>(y.tv_usec * 1000 - x.tv_nsec) / 1000000000.0;
}
#endif

Real64 calcDiff(const DateTime& x, const DateTime& y)
{
	DateTime diff = y - x;
	return diff.get() + static_cast<Real64>(diff.getMicrosecond()) / 1000000.0;
}

} // end unnamed namespace

void TimeoutTimerTestCases::testasTimespec()
{
#ifdef BLOCXX_HAVE_STRUCT_TIMESPEC
	Real64 vals[] = {1, 0.00001, 0.05, 9, 1000, 123456, 12345.6, 1234.56, 123.456, 12.3456, 1.23456, .123456, .0123456, 0};
	for (unsigned i = 0; i < sizeof(vals)/sizeof(vals[0]); ++i)
	{
		Timeout t = Timeout::relative(vals[i]);
		struct timeval now1;
		struct timeval now2;
		struct timespec timeout;
		gettimeofday(&now1, 0);
		TimeoutTimer tt(t);
		gettimeofday(&now2, 0);
		tt.asTimespec(timeout);
		unitAssert(calcDiff(now1, timeout) >= vals[i] - vals[i] * 0.01);
		unitAssert(calcDiff(now2, timeout) <= vals[i] + vals[i] * 0.01);
	}
	for (unsigned i = 0; i < sizeof(vals)/sizeof(vals[0]); ++i)
	{
		Timeout t = Timeout::relativeWithReset(vals[i]);
		struct timeval now1;
		struct timeval now2;
		struct timespec timeout;
		gettimeofday(&now1, 0);
		TimeoutTimer tt(t);
		gettimeofday(&now2, 0);
		tt.asTimespec(timeout);
		unitAssert(calcDiff(now1, timeout) >= vals[i] - vals[i] * 0.01);
		unitAssert(calcDiff(now2, timeout) <= vals[i] + vals[i] * 0.01);
	}
	{
		Timeout t = Timeout::relative(2);
		TestTimeoutTimer::currentTime = DateTime::getCurrent();
		TestTimeoutTimer tt(t);
		TestTimeoutTimer::currentTime.addSeconds(1);
		tt.loop();
		struct timespec timeout;
		tt.asTimespec(timeout);
		unitAssert(timeout.tv_sec == TestTimeoutTimer::currentTime.get() + 1);
	}
	{
		Timeout t = Timeout::relativeWithReset(2);
		TestTimeoutTimer::currentTime = DateTime::getCurrent();
		TestTimeoutTimer tt(t);
		TestTimeoutTimer::currentTime.addSeconds(1);
		tt.loop();
		struct timespec timeout;
		tt.asTimespec(timeout);
		unitAssert(timeout.tv_sec == TestTimeoutTimer::currentTime.get() + 1);
	}
	{
		Timeout t = Timeout::relativeWithReset(2);
		TestTimeoutTimer::currentTime = DateTime::getCurrent();
		TestTimeoutTimer tt(t);
		TestTimeoutTimer::currentTime.addSeconds(1);
		tt.resetOnLoop();
		struct timespec timeout;
		tt.asTimespec(timeout);
		unitAssert(timeout.tv_sec == TestTimeoutTimer::currentTime.get() + 2);
	}

#endif
}

void TimeoutTimerTestCases::testasTimeval()
{
#ifdef BLOCXX_HAVE_STRUCT_TIMEVAL
	Real64 vals[] = {1, 0.00001, 0.05, 9, 1000, 123456, 12345.6, 1234.56, 123.456, 12.3456, 1.23456, .123456, .0123456, 0};
	for (unsigned i = 0; i < sizeof(vals)/sizeof(vals[0]); ++i)
	{
		Timeout t = Timeout::relative(vals[i]);
		struct timeval timeout;
		TimeoutTimer tt(t);
		tt.asTimeval(timeout);
		unitAssert(timeout.tv_sec + timeout.tv_usec / 1000000.0 >= vals[i] - vals[i] * 0.01);

		// check the max is honored
		Real64 maxVal = 0.1;
		tt.asTimeval(timeout, maxVal);
		if (vals[i] > maxVal)
		{
			unitAssert(timeout.tv_sec == 0);
			unitAssert(timeout.tv_usec == 100000);
		}
		// check timeout again
	}
	for (unsigned i = 0; i < sizeof(vals)/sizeof(vals[0]); ++i)
	{
		Timeout t = Timeout::relativeWithReset(vals[i]);
		struct timeval timeout;
		TimeoutTimer tt(t);
		tt.asTimeval(timeout);
		unitAssert(timeout.tv_sec + timeout.tv_usec / 1000000.0 >= vals[i] - vals[i] * 0.01);
	}
	{
		Timeout t = Timeout::infinite;
		TimeoutTimer tt(t);
		unitAssert(tt.infinite());
		struct timeval timeout;
		unitAssert(tt.asTimeval(timeout) == 0);
		unitAssert(tt.asTimeval(timeout, INFINITY) == 0);
		unitAssert(tt.asTimeval(timeout, 1.0) == &timeout);

		Timeout t2 = Timeout::relative(1.0);
		TimeoutTimer tt2(t2);
		unitAssert(tt2.asTimeval(timeout, INFINITY) == &timeout);
		unitAssert(tt2.asTimeval(timeout, 1.0) == &timeout);
	}
	{
		Timeout t = Timeout::relative(2);
		TestTimeoutTimer::currentTime = DateTime::getCurrent();
		TestTimeoutTimer tt(t);
		TestTimeoutTimer::currentTime.addSeconds(1);
		tt.loop();
		struct timeval timeout;
		tt.asTimeval(timeout);
		unitAssertLongsEqual(1, timeout.tv_sec);
	}
#endif
}

void TimeoutTimerTestCases::testasAbsoluteTimeout()
{
	Real64 vals[] = {1, 0.00001, 0.05, 9, 1000, 123456, 12345.6, 1234.56, 123.456, 12.3456, 1.23456, .123456, .0123456, 0};
	for (unsigned i = 0; i < sizeof(vals)/sizeof(vals[0]); ++i)
	{
		Timeout t = Timeout::relative(vals[i]);
		DateTime now1 = DateTime::getCurrent();
		TimeoutTimer tt(t);
		DateTime now2 = DateTime::getCurrent();
		Timeout abst = tt.asAbsoluteTimeout();
		unitAssert(abst.getType() == Timeout::E_ABSOLUTE);
		unitAssert(calcDiff(now1, abst.getAbsolute()) >= vals[i] - vals[i] * 0.01);
		unitAssert(calcDiff(now2, abst.getAbsolute()) <= vals[i] + vals[i] * 0.01);
	}
	for (unsigned i = 0; i < sizeof(vals)/sizeof(vals[0]); ++i)
	{
		Timeout t = Timeout::relativeWithReset(vals[i]);
		DateTime now1 = DateTime::getCurrent();
		TimeoutTimer tt(t);
		DateTime now2 = DateTime::getCurrent();
		Timeout abst = tt.asAbsoluteTimeout();
		unitAssert(abst.getType() == Timeout::E_ABSOLUTE);
		unitAssert(calcDiff(now1, abst.getAbsolute()) >= vals[i] - vals[i] * 0.01);
		unitAssert(calcDiff(now2, abst.getAbsolute()) <= vals[i] + vals[i] * 0.01);
	}

	{
		Timeout t = Timeout::infinite;
		DateTime now1 = DateTime::getCurrent();
		TimeoutTimer tt(t);
		DateTime now2 = DateTime::getCurrent();
		Timeout abst = tt.asAbsoluteTimeout();
		unitAssert(abst == Timeout::infinite);
	}
}

void TimeoutTimerTestCases::testasIntMs()
{
	Real64 vals[] = {1, 0.05, 9, 1000, 123456, 12345.6, 1234.56, 123.456, 12.3456, 0};
	for (unsigned i = 0; i < sizeof(vals)/sizeof(vals[0]); ++i)
	{
		Timeout t = Timeout::relative(vals[i]);
		TimeoutTimer tt(t);
		int timeout = tt.asIntMs();
		unitAssertDoublesEqual(vals[i] * 1000, timeout, vals[i] * .1);

		// check the max is honored
		Real64 maxVal = 0.1;
		timeout = tt.asIntMs(maxVal);
		if (vals[i] > maxVal)
		{
			unitAssertLongsEqual(100, timeout);
		}
	}
	for (unsigned i = 0; i < sizeof(vals)/sizeof(vals[0]); ++i)
	{
		Timeout t = Timeout::relativeWithReset(vals[i]);
		TimeoutTimer tt(t);
		int timeout = tt.asIntMs();
		unitAssertDoublesEqual(vals[i] * 1000, timeout, vals[i] * .1);
	}
	{
		Timeout t = Timeout::infinite;
		TimeoutTimer tt(t);
		unitAssert(tt.infinite());
		unitAssertLongsEqual(-1, tt.asIntMs());
		unitAssertLongsEqual(-1, tt.asIntMs(INFINITY));
		unitAssertLongsEqual(1000, tt.asIntMs(1.0));

		Timeout t2 = Timeout::relative(1.0);
		TimeoutTimer tt2(t2);
		unitAssertLongsEqual(1000, tt2.asIntMs(INFINITY));
		unitAssertLongsEqual(1000, tt2.asIntMs(1.0));
	}
	{
		Timeout t = Timeout::relative(2);
		TestTimeoutTimer::currentTime = DateTime::getCurrent();
		TestTimeoutTimer tt(t);
		TestTimeoutTimer::currentTime.addSeconds(1);
		tt.loop();
		unitAssertLongsEqual(1000, tt.asIntMs());
	}
}

Test* TimeoutTimerTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("TimeoutTimer");

	ADD_TEST_TO_SUITE(TimeoutTimerTestCases, testRelative);
	ADD_TEST_TO_SUITE(TimeoutTimerTestCases, testRelativeReset);
	ADD_TEST_TO_SUITE(TimeoutTimerTestCases, testAbsolute);
	ADD_TEST_TO_SUITE(TimeoutTimerTestCases, testasTimespec);
	ADD_TEST_TO_SUITE(TimeoutTimerTestCases, testasTimeval);
	ADD_TEST_TO_SUITE(TimeoutTimerTestCases, testasAbsoluteTimeout);
	ADD_TEST_TO_SUITE(TimeoutTimerTestCases, testasIntMs);
	

	return testSuite;
}

