/*******************************************************************************
* Copyright (C) 2008, Quest Software, Inc. All rights reserved.
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
#include "TmUtils_normTmTestCases.hpp"

#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"
#include "TestSuite.hpp"
#include "TestCaller.hpp"
AUTO_UNIT_TEST_SUITE_NAMED(TmUtils_normTmTestCases, "TmUtils_normTm");
#include "blocxx/TmUtils.hpp"
#include <time.h>
#include <iostream>
#include "blocxx/String.hpp"

using namespace blocxx;
using namespace std;

// This is here only to prevent nasty compiler warnings about implicit virtual constructors.
TmUtils_normTmTestCases::~TmUtils_normTmTestCases()
{
}

void TmUtils_normTmTestCases::setUp()
{
}

void TmUtils_normTmTestCases::tearDown()
{
}

namespace
{
	void strToTm(char const * tmstr, struct tm & tm)
	{
		// .toInt(); .substring(pos, length);
		String s(tmstr);
		// year-month-day hour:minute:second
		size_t year_beg = 0;
		size_t year_end = s.indexOf('-');
		size_t mon_beg = year_end + 1;
		size_t mon_end = s.indexOf('-', mon_beg);
		size_t day_beg = mon_end + 1;
		size_t day_end = s.indexOf(' ', day_beg);
		size_t hour_beg = day_end + 1;
		size_t hour_end = s.indexOf(':', hour_beg);
		size_t min_beg = hour_end + 1;
		size_t min_end = s.indexOf(':', min_beg);
		size_t sec_beg = min_end + 1;
		tm.tm_year = s.substring(year_beg, year_end - year_beg).toInt() - 1900;
		tm.tm_mon = s.substring(mon_beg, mon_end - mon_beg).toInt() - 1;
		tm.tm_mday = s.substring(day_beg, day_end - day_beg).toInt();
		tm.tm_hour = s.substring(hour_beg, hour_end - hour_beg).toInt();
		tm.tm_min = s.substring(min_beg, min_end - min_beg).toInt();
		tm.tm_sec = s.substring(sec_beg).toInt();
	}
}

void TmUtils_normTmTestCases::TmUtils_normTmTestCase(char const * inp, char const * expected)
{
	struct tm tminp;
	struct tm tmexpected;
	strToTm(inp, tminp);
	strToTm(expected, tmexpected);

	normTm(tminp);

	unitAssertEquals(tmexpected.tm_year, tminp.tm_year);
	unitAssertEquals(tmexpected.tm_mon, tminp.tm_mon);
	unitAssertEquals(tmexpected.tm_mday, tminp.tm_mday);
	unitAssertEquals(tmexpected.tm_hour, tminp.tm_hour);
	unitAssertEquals(tmexpected.tm_min, tminp.tm_min);
	unitAssertEquals(tmexpected.tm_sec, tminp.tm_sec);
}

void TmUtils_normTmTestCases::testAll()
{
	// overflow month
	TmUtils_normTmTestCase("1984-13-15 12:06:37", "1985-01-15 12:06:37");
	TmUtils_normTmTestCase("1984-54-15 12:06:37", "1988-06-15 12:06:37");
	// overflow day
	TmUtils_normTmTestCase("1993-03-32 07:43:58", "1993-04-01 07:43:58");
	TmUtils_normTmTestCase("1993-02-29 07:43:58", "1993-03-01 07:43:58");
	TmUtils_normTmTestCase("1993-04-31 07:43:58", "1993-05-01 07:43:58");
	TmUtils_normTmTestCase("1993-04-107 07:43:58", "1993-07-16 07:43:58");
	// overflow hour
	TmUtils_normTmTestCase("2005-04-05 24:31:11", "2005-04-06 00:31:11");
	TmUtils_normTmTestCase("2005-04-05 85:31:11", "2005-04-08 13:31:11");
	// overflow minute
	TmUtils_normTmTestCase("2011-05-12 22:60:43", "2011-05-12 23:00:43");
	TmUtils_normTmTestCase("2011-05-12 22:527:43", "2011-05-13 06:47:43");
	// overflow second
	TmUtils_normTmTestCase("2015-06-30 13:17:60", "2015-06-30 13:18:00");
	TmUtils_normTmTestCase("2015-06-30 13:17:928", "2015-06-30 13:32:28");
	// overflow everything
	TmUtils_normTmTestCase("2016-13-32 24:60:60", "2017-02-02 01:01:00");
	TmUtils_normTmTestCase("2016-27-92 103:426:777", "2018-06-04 14:18:57");
	// cascading overflow
	TmUtils_normTmTestCase("2018-12-31 23:59:72", "2019-01-01 00:00:12");
	// overflow involving leap day
	TmUtils_normTmTestCase("2004-02-29 25:36:24", "2004-03-01 01:36:24");
	TmUtils_normTmTestCase("2000-02-29 25:36:24", "2000-03-01 01:36:24");
	// non-overflow that would be an overflow if not leap year
	TmUtils_normTmTestCase("2004-02-29 15:36:24", "2004-02-29 15:36:24");
	TmUtils_normTmTestCase("2000-02-29 15:36:24", "2000-02-29 15:36:24");
	// overflow that would not be an overflow in a leap year
	TmUtils_normTmTestCase("2003-02-29 15:36:24", "2003-03-01 15:36:24");

	// no overflow
	TmUtils_normTmTestCase("2008-07-24 18:21:37", "2008-07-24 18:21:37");
	TmUtils_normTmTestCase("2008-12-31 23:59:59", "2008-12-31 23:59:59");
}

void TmUtils_normTmTestCases::negativeTestCase(
	int year, int month, int day, int hour, int minute, int second,
	int eyear, int emonth, int eday, int ehour, int eminute, int esecond)
{
	struct tm tm;
	tm.tm_year = year;
	tm.tm_mon = month;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = minute;
	tm.tm_sec = second;

	normTm(tm);

	unitAssertEquals(eyear, tm.tm_year);
	unitAssertEquals(emonth, tm.tm_mon);
	unitAssertEquals(eday, tm.tm_mday);
	unitAssertEquals(ehour, tm.tm_hour);
	unitAssertEquals(eminute, tm.tm_min);
	unitAssertEquals(esecond, tm.tm_sec);
}

void TmUtils_normTmTestCases::testNegative()
{
	negativeTestCase(
		80, -1, 1, 0, 0, 0,
		79, 11, 1, 0, 0, 0);
	negativeTestCase(
		80,  0,  0, 0, 0, 0,
		79, 11, 31, 0, 0, 0);
	negativeTestCase(
		80,  0, -1, 0, 0, 0,
		79, 11, 30, 0, 0, 0);
	negativeTestCase(
		80,  0,  1, -1, 0, 0,
		79, 11, 31, 23, 0, 0);
	negativeTestCase(
		80,  0,  1,  0, -1, 0,
		79, 11, 31, 23, 59, 0);
	negativeTestCase(
		80,  0,  1,  0,  0, -1,
		79, 11, 31, 23, 59, 59);

	// Feb 29 - leap years
	negativeTestCase(
		84, 2,  0, 0, 0, 0,
		84, 1, 29, 0, 0, 0);
	negativeTestCase(
		84, 2, -1, 0, 0, 0,
		84, 1, 28, 0, 0, 0);
	negativeTestCase(
		84, 2,  1, -1, 0, 0,
		84, 1, 29, 23, 0, 0);
	negativeTestCase(
		84, 2,  1,  0, -1, 0,
		84, 1, 29, 23, 59, 0);
	negativeTestCase(
		84, 2,  1,  0,  0, -1,
		84, 1, 29, 23, 59, 59);

	// Feb 29 - non-leap years
	negativeTestCase(
		83, 2,  0, 0, 0, 0,
		83, 1, 28, 0, 0, 0);
	negativeTestCase(
		83, 2, -1, 0, 0, 0,
		83, 1, 27, 0, 0, 0);
	negativeTestCase(
		83, 2,  1, -1, 0, 0,
		83, 1, 28, 23, 0, 0);
	negativeTestCase(
		83, 2,  1,  0, -1, 0,
		83, 1, 28, 23, 59, 0);
	negativeTestCase(
		83, 2,  1,  0,  0, -1,
		83, 1, 28, 23, 59, 59);

	// Negative year interaction with leap years

	// overflow at end of Feb, not a leap year
	negativeTestCase(
		-1, 1, 29, 0, 0, 0,
		-1, 2,  1, 0, 0, 0);
	// would be overflow at end of Feb except that it is a leap year
	negativeTestCase(
		-4, 1, 29, 0, 0, 0,
		-4, 1, 29, 0, 0, 0);
	// overflow at end of Feb on a leap year
	negativeTestCase(
		-4, 1, 30, 0, 0, 0,
		-4, 2,  1, 0, 0, 0);

	// underflow at beginning of Mar, not a leap year
	negativeTestCase(
		-1, 2,  0, 0, 0, 0,
		-1, 1, 28, 0, 0, 0);
	// underflow at beginning of Mar, leap year
	negativeTestCase(
		-4, 2,  0, 0, 0, 0,
		-4, 1, 29, 0, 0, 0);
}

Test* TmUtils_normTmTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("TmUtils_normTm");

	ADD_TEST_TO_SUITE(TmUtils_normTmTestCases, testAll);
	ADD_TEST_TO_SUITE(TmUtils_normTmTestCases, testNegative);

	return testSuite;
}

