/*******************************************************************************
* Copyright (C) 2008, Quest Software, Inc. All rights reserved.
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

#include "blocxx/BLOCXX_config.h"
#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"

#include "blocxx/TimeDuration.hpp"
#include "blocxx/DateTime.hpp"
#include "blocxx/String.hpp"
#include "blocxx/TimeUtils.hpp"

#include <cmath>

using namespace blocxx;

namespace BLOCXX_NAMESPACE
{
	namespace Time
	{
		static std::ostream& operator<<(std::ostream& o, const Time::TimeDuration& td)
		{
			return o << td.toString();
		}
	}
}

AUTO_UNIT_TEST(TimeDurationTestCases_testUnitCreation)
{
	{
		Time::TimeDuration td = Time::microseconds(123);
		unitAssertEquals(123, td.microseconds());
		unitAssertEquals(123, td.microsecondInSecond());

		unitAssertEquals(0, td.completeSeconds());
		unitAssertEquals(0, td.completeMinutes());
		unitAssertEquals(0, td.completeHours());
		unitAssertEquals(0, td.completeDays());

		unitAssertEquals(0, td.secondInMinute());
		unitAssertEquals(0, td.minuteInHour());
		unitAssertEquals(0, td.hourInDay());
		unitAssertEquals(0, td.relativeDays());

		unitAssertEquals("0:00:00.000123", td.toString());

		unitAssertEquals(td.microseconds(),
			((((td.relativeDays()*24 + td.hourInDay())*60 + td.minuteInHour())*60 + td.secondInMinute())*1000000) + td.microsecondInSecond());
	}

	{
		Time::TimeDuration td = Time::seconds(123);
		unitAssertEquals(Int64(123) * 1000000, td.microseconds());
		unitAssertEquals(0, td.microsecondInSecond());

		unitAssertEquals(123, td.completeSeconds());
		unitAssertEquals(2, td.completeMinutes());
		unitAssertEquals(0, td.completeHours());
		unitAssertEquals(0, td.completeDays());

		unitAssertEquals(3, td.secondInMinute());
		unitAssertEquals(2, td.minuteInHour());
		unitAssertEquals(0, td.hourInDay());
		unitAssertEquals(0, td.relativeDays());

		unitAssertEquals("0:02:03.000000", td.toString());
	}

	{
		Time::TimeDuration td = Time::minutes(123);
		unitAssertEquals(Int64(123) * 60 * 1000000, td.microseconds());
		unitAssertEquals(0, td.microsecondInSecond());

		unitAssertEquals(123 * 60, td.completeSeconds());
		unitAssertEquals(123, td.completeMinutes());
		unitAssertEquals(2, td.completeHours());
		unitAssertEquals(0, td.completeDays());

		unitAssertEquals(0, td.secondInMinute());
		unitAssertEquals(3, td.minuteInHour());
		unitAssertEquals(2, td.hourInDay());
		unitAssertEquals(0, td.relativeDays());

		unitAssertEquals("2:03:00.000000", td.toString());
	}

	{
		Time::TimeDuration td = Time::hours(123);
		unitAssertEquals(Int64(123) * 60 * 60 * 1000000, td.microseconds());
		unitAssertEquals(0, td.microsecondInSecond());

		unitAssertEquals(123 * 60 * 60, td.completeSeconds());
		unitAssertEquals(123 * 60, td.completeMinutes());
		unitAssertEquals(123, td.completeHours());
		unitAssertEquals(5, td.completeDays());

		unitAssertEquals(0, td.secondInMinute());
		unitAssertEquals(0, td.minuteInHour());
		unitAssertEquals(3, td.hourInDay());
		unitAssertEquals(5, td.relativeDays());

		unitAssertEquals("123:00:00.000000", td.toString());
	}

	{
		Time::TimeDuration td = Time::days(123);
		unitAssertEquals(Int64(123) * 60 * 60 * 24 * 1000000, td.microseconds());
		unitAssertEquals(0, td.microsecondInSecond());

		unitAssertEquals(123 * 60 * 60 * 24, td.completeSeconds());
		unitAssertEquals(123 * 60 * 24, td.completeMinutes());
		unitAssertEquals(123 * 24, td.completeHours());
		unitAssertEquals(123, td.completeDays());

		unitAssertEquals(0, td.secondInMinute());
		unitAssertEquals(0, td.minuteInHour());
		unitAssertEquals(0, td.hourInDay());
		unitAssertEquals(123, td.relativeDays());

		unitAssertEquals("2952:00:00.000000", td.toString());
	}
}

AUTO_UNIT_TEST(TimeDurationTestCases_testDurationMath)
{
	{
		Time::TimeDuration one = Time::microseconds(1);
		Time::TimeDuration neg_one = -one;

		unitAssertEquals(-1, neg_one.relativeDays());
		unitAssertEquals(23, neg_one.hourInDay());
		unitAssertEquals(59, neg_one.minuteInHour());
		unitAssertEquals(59, neg_one.secondInMinute());
		unitAssertEquals(999999, neg_one.microsecondInSecond());
	}

	unitAssert(Time::isInfinite(Time::TimeDuration(10 * Time::TimeDuration(Time::E_TIME_POS_INFINITY))));

	unitAssert(Time::minutes(1) + Time::minutes(1) == Time::minutes(2));
	unitAssert(Time::minutes(2) - Time::minutes(1) == Time::minutes(1));
	unitAssert(Time::minutes(1) * 5 == Time::minutes(5));
	unitAssert(5 * Time::minutes(1) == Time::minutes(5));
	unitAssert(60 * Time::minutes(1) == Time::hours(1));
	unitAssert(123 * Time::minutes(1) == Time::minutes(3) + Time::hours(2));
	unitAssert(4000020 * Time::microseconds(1) == Time::seconds(4) + Time::microseconds(20));
	unitAssert(Time::days(2) - Time::hours(12) == Time::hours(36));
	unitAssert(Time::days(1) - Time::seconds(1) == Time::seconds(86399));

	unitAssert(Time::timeBetween(DateTime(), DateTime() + Time::days(1)) == Time::hours(24));
	unitAssert(Time::timeBetween(DateTime() + Time::days(1), DateTime()) == -Time::hours(24));
	unitAssert(Time::isInfinite(Time::timeBetween(DateTime(), DateTime() + Time::TimeDuration(Time::E_TIME_POS_INFINITY))));
	unitAssert(Time::isInfinite(Time::timeBetween(DateTime() + Time::TimeDuration(Time::E_TIME_POS_INFINITY), DateTime())));
	unitAssert(Time::timeBetween(DateTime() + Time::TimeDuration(Time::E_TIME_POS_INFINITY),
			DateTime() + Time::TimeDuration(Time::E_TIME_POS_INFINITY)) == Time::TimeDuration(Time::E_TIME_NADT));
	unitAssert(Time::timeBetween(DateTime(), DateTime()) == Time::microseconds(0));


	{
		DateTime foo1(2008, 1, 1, 1, 1, 1, 999999);
		DateTime foo2(2008, 1, 1, 1, 1, 2, 3);
		DateTime foo3(2008, 1, 1, 1, 1, 4, 999999);
		DateTime foo4(2008, 1, 1, 1, 1, 5, 20);

		unitAssert(Time::timeBetween(foo1, foo2) == Time::microseconds(4));
		unitAssert(Time::timeBetween(foo2, foo1) == -Time::microseconds(4));
		unitAssert(Time::timeBetween(foo1, foo3) == Time::seconds(3));
		unitAssert(Time::timeBetween(foo3, foo1) == -Time::seconds(3));


		Time::TimeDuration td1 = Time::timeBetween(foo2, foo1);
		unitAssertEquals(-4, td1.microseconds());
		unitAssertEquals(999996, td1.microsecondInSecond());

		unitAssertEquals(-3, Time::timeBetween(foo4, foo1).completeSeconds());
		unitAssertEquals(56, Time::timeBetween(foo4, foo1).secondInMinute());
		// 21 microseconds difference...
		unitAssertEquals(999979, Time::timeBetween(foo4, foo1).microsecondInSecond());
	}


	{
		DateTime feb01(2008, 2, 1); // Feb 1, 2008
		unitAssert(feb01 + Time::days(28) == DateTime(2008,2,29)); // leap year
		unitAssert(feb01 + Time::days(29) == DateTime(2008,3,1));
		
		unitAssert(feb01 - Time::days(365) == DateTime(2007,2,1)); // not through a leap year.
		unitAssert(feb01 + Time::days(365) == DateTime(2009,1,31)); // through a leap year.
		unitAssert(feb01 + 4 * Time::days(365) == DateTime(2012,1,31)); // through a leap year.
	}

	{
		DateTime feb29(2008, 2, 29); // Feb 29, 2008

		unitAssert(feb29 - Time::days(365) == DateTime(2007,3,1));
		unitAssert(feb29 - Time::days(366) == DateTime(2007,2,28));
		unitAssert(feb29 + Time::days(365) == DateTime(2009,2,28));
		unitAssert(feb29 + Time::days(366) == DateTime(2009,3,1));
		unitAssert(feb29 + 5 * Time::days(365) == DateTime(2013,2,27)); // through a leap year.
	}

	{
		using Time::TimeDuration;

		// Infinite numbers remain infinite, unless one is subtracted.
		unitAssert(Time::isInfinite(TimeDuration(Time::E_TIME_POS_INFINITY)));
		unitAssert(Time::isSpecial(TimeDuration(Time::E_TIME_POS_INFINITY)));
		unitAssert(Time::isSpecial(TimeDuration(TimeDuration(Time::E_TIME_POS_INFINITY))));
		unitAssert(Time::isInfinite(TimeDuration(Time::E_TIME_POS_INFINITY) + Time::hours(1)));
		unitAssert(Time::isInfinite(Time::hours(1) + TimeDuration(Time::E_TIME_POS_INFINITY)));
		unitAssert(Time::isInfinite(TimeDuration(Time::E_TIME_POS_INFINITY) * 10));
		unitAssert(Time::isInfinite(10 * TimeDuration(Time::E_TIME_POS_INFINITY)));
		unitAssert(Time::isInfinite(TimeDuration(Time::E_TIME_POS_INFINITY) - Time::hours(1)));

		unitAssert(TimeDuration(Time::hours(1) - TimeDuration(Time::E_TIME_POS_INFINITY)).getSpecialFlag() == Time::E_TIME_NEG_INFINITY);
		unitAssert(TimeDuration(Time::E_TIME_POS_INFINITY) - TimeDuration(Time::E_TIME_POS_INFINITY) == TimeDuration(Int64(0)));
	}

	unitAssertEquals("1:02:03", Time::timeBetween(DateTime(2008,3,4,12,1,0),DateTime(2008,3,4,13,3,3)).toString("%6:%3:%4"));


	// Test floating-point conversions.
	double neg_infinity = std::log(0.0);
	unitAssertEquals(Time::TimeDuration(Time::E_TIME_NEG_INFINITY), Time::TimeDuration(neg_infinity));
	unitAssertEquals(Time::TimeDuration(Time::E_TIME_POS_INFINITY), Time::TimeDuration(-neg_infinity));
	unitAssertEquals(Time::TimeDuration(Time::E_TIME_NADT), Time::TimeDuration(neg_infinity / neg_infinity));
}

AUTO_UNIT_TEST(TimeDurationTestCases_testDurationComparison)
{
	using Time::TimeDuration;

	unitAssert(Time::hours(1) > Time::minutes(1));
	unitAssert(Time::hours(1) >= Time::minutes(1));
	unitAssert(Time::minutes(1) < Time::hours(1));
	unitAssert(Time::minutes(1) <= Time::hours(1));
	unitAssert(Time::days(1) > Time::microseconds(1));

	unitAssert(Time::days(1) < TimeDuration(Time::E_TIME_POS_INFINITY));
	unitAssert(!(Time::days(1) > TimeDuration(Time::E_TIME_POS_INFINITY)));
	unitAssert(TimeDuration(Time::E_TIME_POS_INFINITY) > Time::days(1));
	unitAssert(!(TimeDuration(Time::E_TIME_POS_INFINITY) < Time::days(1)));

	unitAssert(!(TimeDuration(Time::E_TIME_POS_INFINITY) > TimeDuration(Time::E_TIME_POS_INFINITY)));
	unitAssert(!(TimeDuration(Time::E_TIME_POS_INFINITY) < TimeDuration(Time::E_TIME_POS_INFINITY)));
	unitAssert((TimeDuration(Time::E_TIME_POS_INFINITY) == TimeDuration(Time::E_TIME_POS_INFINITY)));
	unitAssert(!(TimeDuration(Time::E_TIME_POS_INFINITY) != TimeDuration(Time::E_TIME_POS_INFINITY)));

	unitAssert(!(TimeDuration(Time::E_TIME_NEG_INFINITY) > TimeDuration(Time::E_TIME_NEG_INFINITY)));
	unitAssert(!(TimeDuration(Time::E_TIME_NEG_INFINITY) < TimeDuration(Time::E_TIME_NEG_INFINITY)));
	unitAssert((TimeDuration(Time::E_TIME_NEG_INFINITY) == TimeDuration(Time::E_TIME_NEG_INFINITY)));
	unitAssert(!(TimeDuration(Time::E_TIME_NEG_INFINITY) != TimeDuration(Time::E_TIME_NEG_INFINITY)));

	unitAssert((TimeDuration(Time::E_TIME_POS_INFINITY) > TimeDuration(Time::E_TIME_NEG_INFINITY)));
	unitAssert(!(TimeDuration(Time::E_TIME_POS_INFINITY) < TimeDuration(Time::E_TIME_NEG_INFINITY)));
	unitAssert(!(TimeDuration(Time::E_TIME_POS_INFINITY) == TimeDuration(Time::E_TIME_NEG_INFINITY)));
	unitAssert((TimeDuration(Time::E_TIME_POS_INFINITY) != TimeDuration(Time::E_TIME_NEG_INFINITY)));

	unitAssert(!(TimeDuration(Time::E_TIME_NEG_INFINITY) > TimeDuration(Time::E_TIME_POS_INFINITY)));
	unitAssert((TimeDuration(Time::E_TIME_NEG_INFINITY) < TimeDuration(Time::E_TIME_POS_INFINITY)));
	unitAssert(!(TimeDuration(Time::E_TIME_NEG_INFINITY) == TimeDuration(Time::E_TIME_POS_INFINITY)));
	unitAssert((TimeDuration(Time::E_TIME_NEG_INFINITY) != TimeDuration(Time::E_TIME_POS_INFINITY)));
}

AUTO_UNIT_TEST(TimeDurationTestCases_testInvalidDurations)
{
	using Time::TimeDuration;

	// Only nadt is greater or less comparable to nadt.
	unitAssert(!(Time::hours(1) < TimeDuration(Time::E_TIME_NADT)));
	unitAssert(!(Time::hours(1) > TimeDuration(Time::E_TIME_NADT)));
	unitAssert(!(Time::hours(1) >= TimeDuration(Time::E_TIME_NADT)));
	unitAssert(!(Time::hours(1) <= TimeDuration(Time::E_TIME_NADT)));
	unitAssert(!(Time::hours(1) == TimeDuration(Time::E_TIME_NADT)));
	unitAssert((Time::hours(1) != TimeDuration(Time::E_TIME_NADT)));
	unitAssert(!(TimeDuration(Time::E_TIME_NADT) > TimeDuration(Time::E_TIME_NADT)));
	unitAssert(!(TimeDuration(Time::E_TIME_NADT) < TimeDuration(Time::E_TIME_NADT)));
	unitAssert((TimeDuration(Time::E_TIME_NADT) >= TimeDuration(Time::E_TIME_NADT)));
	unitAssert((TimeDuration(Time::E_TIME_NADT) <= TimeDuration(Time::E_TIME_NADT)));
	unitAssert((TimeDuration(Time::E_TIME_NADT) == TimeDuration(Time::E_TIME_NADT)));
	unitAssert(!(TimeDuration(Time::E_TIME_NADT) != TimeDuration(Time::E_TIME_NADT)));

	// Any math on invalid durations should return an invalid duration.
	unitAssert(Time::isInvalid(Time::hours(1) + TimeDuration(Time::E_TIME_NADT)));
	unitAssert(Time::isInvalid(Time::hours(1) - TimeDuration(Time::E_TIME_NADT)));
	unitAssert(Time::isInvalid(DateTime::getPosInfinity() - TimeDuration(Time::E_TIME_NADT)));
	unitAssert(Time::isInvalid(DateTime::getPosInfinity() + TimeDuration(Time::E_TIME_NADT)));
	unitAssert(Time::isInvalid(DateTime::getNegInfinity() - TimeDuration(Time::E_TIME_NADT)));
	unitAssert(Time::isInvalid(DateTime::getNegInfinity() + TimeDuration(Time::E_TIME_NADT)));

	unitAssert(Time::isInvalid(TimeDuration(Time::E_TIME_NADT) - TimeDuration(Time::E_TIME_NADT)));
	unitAssert(Time::isInvalid(TimeDuration(Time::E_TIME_NADT) + TimeDuration(Time::E_TIME_NADT)));
	unitAssert(Time::isInvalid(10 * TimeDuration(Time::E_TIME_NADT)));
	unitAssert(Time::isInvalid(TimeDuration(Time::E_TIME_NADT) * 10));

	{
		TimeDuration td(Time::hours(10));
		td = TimeDuration(Time::E_TIME_NADT);
		unitAssert(td.isInvalid());
	}
}
