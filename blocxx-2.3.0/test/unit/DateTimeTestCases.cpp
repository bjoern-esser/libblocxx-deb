/*******************************************************************************
* Copyright (C) 2009, Quest Software, Inc. All rights reserved.
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

#include "blocxx/BLOCXX_config.h"
#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"

#include "blocxx/DateTime.hpp"
#include "blocxx/String.hpp"

#include <time.h>
#include <limits>

using namespace blocxx;

#undef min

namespace {
time_t calcTimeT(int year, int month, int day, int hour, int minute, int second)
{
	struct tm t;
	t.tm_year = year - 1900;
	t.tm_mon = month - 1;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min = minute;
	t.tm_sec = second;
	t.tm_isdst = 0;
	// since our input was in ust, adjust for the timezone, since mktime assumes it's input is in local time.
#ifdef BLOCXX_HAVE_TIMEGM
	time_t rv = timegm(&t);
#else
#ifdef BLOCXX_NETWARE
	time_t rv = mktime(&t) - _timezone;
#else
	time_t rv = mktime(&t) - timezone;
#endif
#endif
	return rv;
}

time_t calcTimeTLocal(int year, int month, int day, int hour, int minute, int second)
{
	struct tm t;
	t.tm_year = year - 1900;
	t.tm_mon = month - 1;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min = minute;
	t.tm_sec = second;
	t.tm_isdst = -1; // let the system adjust for dst
	time_t rv = mktime(&t);
	return rv;
}
} // end unnamed namespace

AUTO_UNIT_TEST(DateTimeTestCases_testCreation)
{
	// first test that DateTimeException is thrown for an obviously bad input
	unitAssertThrows(DateTime d(String("some bad date")));
	// now try some that look almost right
	unitAssertThrows(DateTime d(String("19980525X33015.000000-300")));
	unitAssertThrows(DateTime d(String("19980525133015.000X00-300")));
	unitAssertThrows(DateTime d(String("19980525133015.000000-30X")));
	unitAssertThrows(DateTime d(String("19980525133015X000000-300")));
	unitAssertThrows(DateTime d(String("19980525133015.000000X300")));
	unitAssertThrows(DateTime d(String("00000000000000.000000-000")));
	unitAssertThrows(DateTime d(String("99999999999999.999999-999")));
	unitAssertThrows(DateTime d(String("19980525X33015.000000-300")));
	unitAssertThrows(DateTime d(String("-9980525133015.000000-300")));
	// boundary conditions
	// negative year
	unitAssertThrows(DateTime d(String("-9980525133015.000000-300")));


	if( BLOCXX_DATETIME_MINIMUM_YEAR >= 1969 )
	{
		unitAssertThrows(DateTime d(String("19691231235959.999999-000")));
		unitAssertThrows(DateTime d(1969, 12, 31, 23, 59, 59, 999999, DateTime::E_UTC_TIME));
		unitAssertThrows(DateTime d(String("00690101235959.999999-000")));
		unitAssertThrows(DateTime d(69, 1, 1, 23, 59, 59, 999999, DateTime::E_UTC_TIME));
	}
	else
	{
		unitAssertNoThrow(DateTime d(String("19691231235959.999999-000")));
		unitAssertNoThrow(DateTime d(1969, 12, 31, 23, 59, 59, 999999, DateTime::E_UTC_TIME));
	}

	if( BLOCXX_DATETIME_MINIMUM_YEAR > 69 )
	{
		unitAssertThrows(DateTime d(String("00690101235959.999999-000")));
		unitAssertThrows(DateTime d(69, 1, 1, 23, 59, 59, 999999, DateTime::E_UTC_TIME));
	}
	else
	{
		unitAssertNoThrow(DateTime d(String("00690101235959.999999-000")));
		unitAssertNoThrow(DateTime d(69, 1, 1, 23, 59, 59, 999999, DateTime::E_UTC_TIME));
	}

	// 0 month
	unitAssertThrows(DateTime d(String("19980025133015.000000-300")));
	// 13 month
	unitAssertThrows(DateTime d(String("19981325133015.000000-300")));
	// 0 day
	unitAssertThrows(DateTime d(String("19980500133015.000000-300")));
	// 32 day
	unitAssertThrows(DateTime d(String("19980532133015.000000-300")));
	// -1 hour
	unitAssertThrows(DateTime d(String("19980525-13015.000000-300")));
	// 24 hour
	unitAssertThrows(DateTime d(String("19980525243015.000000-300")));
	// -1 minute
	unitAssertThrows(DateTime d(String("1998052513-115.000000-300")));
	// 60 minute
	unitAssertThrows(DateTime d(String("19980525136015.000000-300")));
	// -1 second
	unitAssertThrows(DateTime d(String("199805251330-1.000000-300")));
	// 61 second
	unitAssertThrows(DateTime d(String("19980525133061.000000-300")));
	// some ctime style dates that are just a little wrong
	unitAssertThrows(DateTime d(String("WXd Jun 30 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("Wed JXn 30 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 00 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 30 99:49:08 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 30 21:49:08X1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 30 21:49:08 -993")));
	unitAssertThrows(DateTime d(String("Wed-Jun-30-21:49:08-1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 99 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun30 21:49:08 1993")));
	//	unitAssertThrows(DateTime d(String("Jun 30 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("SuX Jun 30 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("SaX Jun 30 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("SXt Jun 30 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("MoX Jun 30 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("MXn Jun 30 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("TuX Jun 30 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("ThX Jun 30 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("TXe Jun 30 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("WeX Jun 30 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("WXd Jun 30 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("FrX Jun 30 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("FXi Jun 30 21:49:08 1993")));
	unitAssertThrows(DateTime d(String("Xri Jun 30 21:49:08 1993")));

	// some more incorrect time patterns
	unitAssertThrows(DateTime d(String("WXd Jun 30 21:49:08 UTC 1993")));
	unitAssertThrows(DateTime d(String("Wed JXn 30 21:49:08 UTC 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun X0 21:49:08 UTC 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 30 X1:49:08 UTC 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 30 21:X9:08 UTC 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 30 21:49:X8 UTC 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 30 21:49:08 UTC X993")));
	unitAssertThrows(DateTime d(String("Wed-Jun-30-21:49:08-UTC-1993")));
	unitAssertThrows(DateTime d(String("Wed Jun-30-21:49:08-UTC-1993")));
	unitAssertThrows(DateTime d(String("Wed Jun-30-21:49:08-UTC-1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 30-21:49:08-UTC-1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 30 21:49:08-UTC-1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 30 21:49:08 UTC-1993")));

	unitAssertThrows(DateTime d(String("Wed Jun 30 21-49-08 UTC 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 30 21:49-08 UTC 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 30 21-49:08 UTC 1993")));

	unitAssertThrows(DateTime d(String("Wed Jun 3 211:49:08 UTC 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 300 2:49:08 UTC 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 3 2:49:08 VERYLONGERROR 1993")));

	unitAssertThrows(DateTime d(String("Wed Jun 30 21:99:08 UTC 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 3 211:49:08 UTC 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 3 2:49:99 UTC 1993")));
	unitAssertThrows(DateTime d(String("Wed Jun 30 21:49:08 UTC 14-3")));
	unitAssertThrows(DateTime d(String("Jun 30 2x:49 1993")));
	unitAssertThrows(DateTime d(String("Jun 30 20:49 1x93")));
	// Everything acceptable except the time zone.
	unitAssertThrows(DateTime d(String("Wed Jun 30 21:49:08 FOO 1993")));
	unitAssertThrows(DateTime d(String("Jun 30 21:49:08 FOO 1993")));

	// Multiple fields of one type:
	unitAssertThrows(DateTime d(String("June 30 21:49:08 1993 Sep")));
	unitAssertThrows(DateTime d(String("June 30 21:49:08 Wed 1993 Fri")));
	unitAssertThrows(DateTime d(String("June 30 21:49 1993 10")));

	// Valid times which should be parsable.
	unitAssertNoThrow(DateTime d(String("Feb 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("1:28:30 1 Feb 1999")));
	unitAssertNoThrow(DateTime d(String("1 Feb 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("1:28:30 Feb 1 1999")));
	unitAssertNoThrow(DateTime d(String("1:28:30 1 Feb 1999")));
	unitAssertNoThrow(DateTime d(String("Feb 1 1999 1:28:30 MST")));
	unitAssertNoThrow(DateTime d(String("1 Feb 1999 1:28:30 MST")));
	unitAssertNoThrow(DateTime d(String("1:28:30 MST Feb 1 1999")));
	unitAssertNoThrow(DateTime d(String("1:28:30 Feb 1 1999 MST")));
	unitAssertNoThrow(DateTime d(String("1:28:30 MST 1 Feb 1999")));
	unitAssertNoThrow(DateTime d(String("1:28:30 1 Feb 1999 MST")));

	unitAssertNoThrow(DateTime d(String("Feb 1 1999 1:28")));
	unitAssertNoThrow(DateTime d(String("1 Feb 1999 1:28")));
	unitAssertNoThrow(DateTime d(String("1:28 Feb 1 1999")));
	unitAssertNoThrow(DateTime d(String("1:28 1 Feb 1999")));
	unitAssertNoThrow(DateTime d(String("Feb 1 1999 1:28 MST")));
	unitAssertNoThrow(DateTime d(String("1 Feb 1999 1:28 MST")));
	unitAssertNoThrow(DateTime d(String("1:28 MST Feb 1 1999")));
	unitAssertNoThrow(DateTime d(String("1:28 Feb 1 1999 MST")));
	unitAssertNoThrow(DateTime d(String("1:28 MST 1 Feb 1999")));
	unitAssertNoThrow(DateTime d(String("1:28 1 Feb 1999 MST")));

	// All the month names (short)
	unitAssertNoThrow(DateTime d(String("Jan 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("Feb 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("Mar 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("Apr 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("May 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("Jun 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("Jul 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("Aug 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("Sep 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("Oct 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("Nov 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("Dec 1 1999 1:28:30")));
	// All the month names (long)
	unitAssertNoThrow(DateTime d(String("January 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("February 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("March 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("April 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("May 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("June 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("July 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("August 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("September 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("October 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("November 1 1999 1:28:30")));
	unitAssertNoThrow(DateTime d(String("December 1 1999 1:28:30")));


	// now test a valid CIM DateTime
	{
		DateTime d("19980525133015.012345-300");
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1998);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 5);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 25);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 18);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 30);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 15);
		unitAssert(d.getMicrosecond() == 12345);
		unitAssert(d.get() == calcTimeT(1998, 5, 25, 18, 30, 15));
	}
	{
		DateTime d("19991231235959.012345-300");
		// year, month, day will wrap because of timezone adjustment
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 2000);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 1);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 1);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 4);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 59);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 59);
		unitAssert(d.getMicrosecond() == 12345);
		unitAssert(d.get() == calcTimeT(2000, 1, 1, 4, 59, 59));
	}
	{
		DateTime d("20000101045959.012345+300");
		// year, month, day will wrap backward because of timezone adjustment
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1999);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 31);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 23);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 59);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 59);
		unitAssert(d.getMicrosecond() == 12345);
		unitAssert(d.get() == calcTimeT(1999, 12, 31, 23, 59, 59));
	}
	// try some with asterisks
	{
		DateTime d("20000101045959.******+000");
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 2000);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 1);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 1);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 4);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 59);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 59);
		unitAssert(d.getMicrosecond() == 0);
		unitAssert(d.get() == calcTimeT(2000, 1, 1, 4, 59, 59));
	}
	// try some ctime style dates - one for each month
	{
		DateTime d(String("Wed Jan 16 13:50:09 1994"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1994);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 1);
		// unless we do some nice calculations to account for our local
		// timezone, we can't check the day or hour.
		//unitAssert(d.getDay(DateTime::E_UTC_TIME) == 15);
		//unitAssert(d.getHour(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 50);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 9);
		unitAssert(d.getMicrosecond() == 0);
		unitAssert(d.get() == calcTimeTLocal(1994, 1, 16, 13, 50, 9));
	}
	{
		DateTime d(String("Wed Feb 17 14:51:10 1995"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1995);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 2);
		// unless we do some nice calculations to account for our local
		// timezone, we can't check the day or hour.
		//unitAssert(d.getDay(DateTime::E_UTC_TIME) == 15);
		//unitAssert(d.getHour(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 51);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 10);
		unitAssert(d.getMicrosecond() == 0);
		unitAssert(d.get() == calcTimeTLocal(1995, 2, 17, 14, 51, 10));
	}
	{
		DateTime d(String("Wed Mar 15 12:49:08 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 3);
		// unless we do some nice calculations to account for our local
		// timezone, we can't check the day or hour.
		//unitAssert(d.getDay(DateTime::E_UTC_TIME) == 15);
		//unitAssert(d.getHour(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
		unitAssert(d.get() == calcTimeTLocal(1993, 3, 15, 12, 49, 8));
	}
	{
		DateTime d(String("Wed Apr 15 12:49:08 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 4);
		// unless we do some nice calculations to account for our local
		// timezone, we can't check the day or hour.
		//unitAssert(d.getDay(DateTime::E_UTC_TIME) == 15);
		//unitAssert(d.getHour(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
		unitAssert(d.get() == calcTimeTLocal(1993, 4, 15, 12, 49, 8));
	}
	{
		DateTime d(String("Wed May 15 12:49:08 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 5);
		// unless we do some nice calculations to account for our local
		// timezone, we can't check the day or hour.
		//unitAssert(d.getDay(DateTime::E_UTC_TIME) == 15);
		//unitAssert(d.getHour(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
		unitAssert(d.get() == calcTimeTLocal(1993, 5, 15, 12, 49, 8));
	}
	{
		DateTime d(String("Wed Jun 15 12:49:08 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 6);
		// unless we do some nice calculations to account for our local
		// timezone, we can't check the day or hour.
		//unitAssert(d.getDay(DateTime::E_UTC_TIME) == 15);
		//unitAssert(d.getHour(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
		unitAssert(d.get() == calcTimeTLocal(1993, 6, 15, 12, 49, 8));
	}
	{
		DateTime d(String("Wed Jul 15 12:49:08 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 7);
		// unless we do some nice calculations to account for our local
		// timezone, we can't check the day or hour.
		//unitAssert(d.getDay(DateTime::E_UTC_TIME) == 15);
		//unitAssert(d.getHour(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
		unitAssert(d.get() == calcTimeTLocal(1993, 7, 15, 12, 49, 8));
	}
	{
		DateTime d(String("Wed Aug 15 12:49:08 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 8);
		// unless we do some nice calculations to account for our local
		// timezone, we can't check the day or hour.
		//unitAssert(d.getDay(DateTime::E_UTC_TIME) == 15);
		//unitAssert(d.getHour(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
		unitAssert(d.get() == calcTimeTLocal(1993, 8, 15, 12, 49, 8));
	}
	{
		DateTime d(String("Wed Sep 15 12:49:08 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 9);
		// unless we do some nice calculations to account for our local
		// timezone, we can't check the day or hour.
		//unitAssert(d.getDay(DateTime::E_UTC_TIME) == 15);
		//unitAssert(d.getHour(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
		unitAssert(d.get() == calcTimeTLocal(1993, 9, 15, 12, 49, 8));
	}
	{
		DateTime d(String("Wed Oct 15 12:49:08 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 10);
		// unless we do some nice calculations to account for our local
		// timezone, we can't check the day or hour.
		//unitAssert(d.getDay(DateTime::E_UTC_TIME) == 15);
		//unitAssert(d.getHour(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
		unitAssert(d.get() == calcTimeTLocal(1993, 10, 15, 12, 49, 8));
	}
	{
		DateTime d(String("Wed Nov 15 12:49:08 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 11);
		// unless we do some nice calculations to account for our local
		// timezone, we can't check the day or hour.
		//unitAssert(d.getDay(DateTime::E_UTC_TIME) == 15);
		//unitAssert(d.getHour(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
		unitAssert(d.get() == calcTimeTLocal(1993, 11, 15, 12, 49, 8));
	}
	{
		DateTime d(String("Wed Dec 15 12:49:08 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 12);
		// unless we do some nice calculations to account for our local
		// timezone, we can't check the day or hour.
		//unitAssert(d.getDay(DateTime::E_UTC_TIME) == 15);
		//unitAssert(d.getHour(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
		unitAssert(d.get() == calcTimeTLocal(1993, 12, 15, 12, 49, 8));
	}

	// Dates of a different form.
	{
		DateTime d(String("Wed Dec 2 12:49:08 UTC 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 2);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
	}
	{
		DateTime d(String("Wed Dec 15 7:49:08 UTC 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 15);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 7);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
	}
	{
		DateTime d(String("Wed Dec 2 7:49:08 UTC 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 2);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 7);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
	}
	{
		DateTime d(String("Wed Dec 20 7:49:08 GMT 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 20);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 7);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
	}
	{
		DateTime d(String("Wed Dec 2 7:49:08 GMT 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 2);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 7);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
	}
	// Local timezones.
	{
		DateTime d(String("Wed Dec 15 12:49:08 MST 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 15);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 19);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
	}
	{
		DateTime d(String("Wed Dec 7 12:49:08 PST 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 7);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 20);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
	}
	{
		DateTime d(String("Wed Dec 15 5:49:08 EST 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 15);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 10);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
	}
	{
		DateTime d(String("Wed Dec 3 5:49:08 CST 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 3);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 11);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
	}
	// Wrapping caused by timezones.
	{
		DateTime d(String("Wed Dec 3 20:49:08 MDT 1993"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 1993);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 12);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 4);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 2);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
	}
	// Wrapping and problems from leap year.
	{
		DateTime d(String("Feb 28 20:49:08 MDT 2000"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 2000);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 2);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 29);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 2);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
	}
	{
		DateTime d(String("Feb 28 20:49:08 MDT 2001"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 2001);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 3);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 1);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 2);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
	}
	{
		DateTime d(String("Mar 1 2:49:08 MSK 2000"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 2000);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 2);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 29);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 23);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
	}
	{
		DateTime d(String("Mar 1 2:49:08 MSK 2001"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 2001);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 2);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 28);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 23);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
	}
	{
		DateTime d(String("Mar 1 2:49:08 C 2001"));
		unitAssert(d.getYear(DateTime::E_UTC_TIME) == 2001);
		unitAssert(d.getMonth(DateTime::E_UTC_TIME) == 2);
		unitAssert(d.getDay(DateTime::E_UTC_TIME) == 28);
		unitAssert(d.getHour(DateTime::E_UTC_TIME) == 23);
		unitAssert(d.getMinute(DateTime::E_UTC_TIME) == 49);
		unitAssert(d.getSecond(DateTime::E_UTC_TIME) == 8);
		unitAssert(d.getMicrosecond() == 0);
	}
	// Another test with a VERY short date string
	{
		DateTime d(String("May 1 3:08 1995"));
		unitAssert(d.getYear(DateTime::E_LOCAL_TIME) == 1995);
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 5);
		unitAssert(d.getDay(DateTime::E_LOCAL_TIME) == 1);
		unitAssert(d.getHour(DateTime::E_LOCAL_TIME) == 3);
		unitAssert(d.getMinute(DateTime::E_LOCAL_TIME) == 8);
		unitAssert(d.getSecond() == 0);
		unitAssert(d.getMicrosecond() == 0);
		unitAssert(d.get() == calcTimeTLocal(1995, 5, 1, 3, 8, 0));
	}
	{
		DateTime d(String("1 May 1995 3:08"));
		unitAssert(d.getYear(DateTime::E_LOCAL_TIME) == 1995);
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 5);
		unitAssert(d.getDay(DateTime::E_LOCAL_TIME) == 1);
		unitAssert(d.getHour(DateTime::E_LOCAL_TIME) == 3);
		unitAssert(d.getMinute(DateTime::E_LOCAL_TIME) == 8);
		unitAssert(d.getSecond() == 0);
		unitAssert(d.getMicrosecond() == 0);
		unitAssert(d.get() == calcTimeTLocal(1995, 5, 1, 3, 8, 0));
	}
	// Test all the month variations (the time doesn't matter)
	{
		DateTime d(String("Jan 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 1);
	}
	{
		DateTime d(String("Feb 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 2);
	}
	{
		DateTime d(String("Mar 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 3);
	}
	{
		DateTime d(String("Apr 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 4);
	}
	{
		DateTime d(String("May 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 5);
	}
	{
		DateTime d(String("Jun 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 6);
	}
	{
		DateTime d(String("Jul 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 7);
	}
	{
		DateTime d(String("Aug 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 8);
	}
	{
		DateTime d(String("Sep 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 9);
	}
	{
		DateTime d(String("Oct 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 10);
	}
	{
		DateTime d(String("Nov 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 11);
	}
	{
		DateTime d(String("Dec 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 12);
	}
	{
		DateTime d(String("January 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 1);
	}
	{
		DateTime d(String("February 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 2);
	}
	{
		DateTime d(String("March 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 3);
	}
	{
		DateTime d(String("April 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 4);
	}
	{
		DateTime d(String("May 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 5);
	}
	{
		DateTime d(String("June 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 6);
	}
	{
		DateTime d(String("July 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 7);
	}
	{
		DateTime d(String("August 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 8);
	}
	{
		DateTime d(String("September 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 9);
	}
	{
		DateTime d(String("October 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 10);
	}
	{
		DateTime d(String("November 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 11);
	}
	{
		DateTime d(String("December 1 1995 3:08"));
		unitAssert(d.getMonth(DateTime::E_LOCAL_TIME) == 12);
	}


	{
		DateTime d1 = DateTime(time_t(8), 1000000);
		DateTime d2 = DateTime(time_t(9), 0);
		unitAssertEquals(d2.get(), d1.get());
		unitAssertEquals(d2.getMicrosecond(), d1.getMicrosecond());

		DateTime d3 = DateTime(time_t(10), -1000000);
		unitAssertEquals(0, d3.getMicrosecond());
		unitAssertEquals(time_t(9), d3.get());
		unitAssert(d3 == d1);

		Time::TimeDuration td4 = (d2 + 0.5) - d1;

		unitAssertEquals(0, td4.completeSeconds());
		unitAssertEquals(500000, td4.microseconds());
		unitAssertEquals(500000, td4.microsecondInSecond());
	}
}

AUTO_UNIT_TEST(DateTimeTestCases_testComparison)
{
	DateTime a(2000,1,1);
	DateTime b(2000,1,1,0,0,0,1);
	DateTime c(1999,1,1);
	DateTime d(2001,1,1);
	DateTime e(2001,1,1,0,0,0,999999);

	unitAssert(!(a < a));
	unitAssert((a < b));
	unitAssert(!(a < c));
	unitAssert((a < d));
	unitAssert((a < e));

	unitAssert(!(a > a));
	unitAssert(!(a > b));
	unitAssert((a > c));
	unitAssert(!(a > d));
	unitAssert(!(a > e));

	unitAssert((a <= a));
	unitAssert((a <= b));
	unitAssert(!(a <= c));
	unitAssert((a <= d));
	unitAssert((a <= e));

	unitAssert((a >= a));
	unitAssert(!(a >= b));
	unitAssert((a >= c));
	unitAssert(!(a >= d));
	unitAssert(!(a >= e));

	unitAssert((a == a));
	unitAssert(!(a == b));
	unitAssert(!(a == c));
	unitAssert(!(a == d));
	unitAssert(!(a == e));

	unitAssert(!(a != a));
	unitAssert((a != b));
	unitAssert((a != c));
	unitAssert((a != d));
	unitAssert((a != e));


	unitAssert(!(b < a));
	unitAssert(!(b < b));
	unitAssert(!(b < c));
	unitAssert((b < d));
	unitAssert((b < e));

	unitAssert((b > a));
	unitAssert(!(b > b));
	unitAssert((b > c));
	unitAssert(!(b > d));
	unitAssert(!(b > e));

	unitAssert(!(b <= a));
	unitAssert((b <= b));
	unitAssert(!(b <= c));
	unitAssert((b <= d));
	unitAssert((b <= e));

	unitAssert((b >= a));
	unitAssert((b >= b));
	unitAssert((b >= c));
	unitAssert(!(b >= d));
	unitAssert(!(b >= e));

	unitAssert(!(b == a));
	unitAssert((b == b));
	unitAssert(!(b == c));
	unitAssert(!(b == d));
	unitAssert(!(b == e));

	unitAssert((b != a));
	unitAssert(!(b != b));
	unitAssert((b != c));
	unitAssert((b != d));
	unitAssert((b != e));



	unitAssert((c < a));
	unitAssert((c < b));
	unitAssert(!(c < c));
	unitAssert((c < d));
	unitAssert((c < e));

	unitAssert(!(c > a));
	unitAssert(!(c > b));
	unitAssert(!(c > c));
	unitAssert(!(c > d));
	unitAssert(!(c > e));

	unitAssert((c <= a));
	unitAssert((c <= b));
	unitAssert((c <= c));
	unitAssert((c <= d));
	unitAssert((c <= e));

	unitAssert(!(c >= a));
	unitAssert(!(c >= b));
	unitAssert((c >= c));
	unitAssert(!(c >= d));
	unitAssert(!(c >= e));

	unitAssert(!(c == a));
	unitAssert(!(c == b));
	unitAssert((c == c));
	unitAssert(!(c == d));
	unitAssert(!(c == e));

	unitAssert((c != a));
	unitAssert((c != b));
	unitAssert(!(c != c));
	unitAssert((c != d));
	unitAssert((c != e));



	unitAssert(!(d < a));
	unitAssert(!(d < b));
	unitAssert(!(d < c));
	unitAssert(!(d < d));
	unitAssert((d < e));

	unitAssert((d > a));
	unitAssert((d > b));
	unitAssert((d > c));
	unitAssert(!(d > d));
	unitAssert(!(d > e));

	unitAssert(!(d <= a));
	unitAssert(!(d <= b));
	unitAssert(!(d <= c));
	unitAssert((d <= d));
	unitAssert((d <= e));

	unitAssert((d >= a));
	unitAssert((d >= b));
	unitAssert((d >= c));
	unitAssert((d >= d));
	unitAssert(!(d >= e));

	unitAssert(!(d == a));
	unitAssert(!(d == b));
	unitAssert(!(d == c));
	unitAssert((d == d));
	unitAssert(!(d == e));

	unitAssert((d != a));
	unitAssert((d != b));
	unitAssert((d != c));
	unitAssert(!(d != d));
	unitAssert((d != e));



	unitAssert(!(e < a));
	unitAssert(!(e < b));
	unitAssert(!(e < c));
	unitAssert(!(e < d));
	unitAssert(!(e < e));

	unitAssert((e > a));
	unitAssert((e > b));
	unitAssert((e > c));
	unitAssert((e > d));
	unitAssert(!(e > e));

	unitAssert(!(e <= a));
	unitAssert(!(e <= b));
	unitAssert(!(e <= c));
	unitAssert(!(e <= d));
	unitAssert((e <= e));

	unitAssert((e >= a));
	unitAssert((e >= b));
	unitAssert((e >= c));
	unitAssert((e >= d));
	unitAssert((e >= e));

	unitAssert(!(e == a));
	unitAssert(!(e == b));
	unitAssert(!(e == c));
	unitAssert(!(e == d));
	unitAssert((e == e));

	unitAssert((e != a));
	unitAssert((e != b));
	unitAssert((e != c));
	unitAssert((e != d));
	unitAssert(!(e != e));

}

AUTO_UNIT_TEST(DateTimeTestCases_testSpecialFunctions)
{
	DateTime nadt = DateTime::getNADT();
	DateTime pinf = DateTime::getPosInfinity();
	DateTime ninf = DateTime::getNegInfinity();
	DateTime zero = DateTime();

	unitAssert(DateTime::getMinimumTime() <= zero);
	unitAssert(DateTime::getMinimumTime() < pinf);
	unitAssert(DateTime::getMinimumTime() > ninf);

	unitAssert(DateTime::getMaximumTime() > zero);
	unitAssert(DateTime::getMaximumTime() < pinf);
	unitAssert(DateTime::getMaximumTime() > ninf);

	unitAssert(nadt.getSpecialTimeFlag() == Time::E_TIME_NADT);
	unitAssert(pinf.getSpecialTimeFlag() == Time::E_TIME_POS_INFINITY);
	unitAssert(ninf.getSpecialTimeFlag() == Time::E_TIME_NEG_INFINITY);
	unitAssert(zero.getSpecialTimeFlag() == Time::E_TIME_NOT_SPECIAL);

	unitAssert(!Time::isInvalid(pinf));
	unitAssert(!Time::isInvalid(ninf));
	unitAssert(Time::isInvalid(nadt));
	unitAssert(!Time::isInvalid(zero));

	unitAssert(Time::isInfinite(pinf));
	unitAssert(Time::isInfinite(ninf));
	unitAssert(!Time::isInfinite(nadt));
	unitAssert(!Time::isInfinite(zero));

	unitAssert(Time::isPosInfinity(pinf));
	unitAssert(!Time::isPosInfinity(ninf));
	unitAssert(!Time::isPosInfinity(nadt));
	unitAssert(!Time::isPosInfinity(zero));

	unitAssert(!Time::isNegInfinity(pinf));
	unitAssert(Time::isNegInfinity(ninf));
	unitAssert(!Time::isNegInfinity(nadt));
	unitAssert(!Time::isNegInfinity(zero));

	unitAssert(Time::isSpecial(pinf));
	unitAssert(Time::isSpecial(ninf));
	unitAssert(Time::isSpecial(nadt));
	unitAssert(!Time::isSpecial(zero));


	Time::TimeDuration td_zero = Time::TimeDuration();
	Time::TimeDuration td_nadt = Time::TimeDuration(Time::E_TIME_NADT);
	Time::TimeDuration td_pinf = Time::TimeDuration(Time::E_TIME_POS_INFINITY);
	Time::TimeDuration td_ninf = Time::TimeDuration(Time::E_TIME_NEG_INFINITY);

	// Any difference with nadt produces nadt
	unitAssert(Time::timeBetween(nadt,nadt) == td_nadt);
	unitAssert(Time::timeBetween(pinf,nadt) == td_nadt);
	unitAssert(Time::timeBetween(nadt,pinf) == td_nadt);
	unitAssert(Time::timeBetween(ninf,nadt) == td_nadt);
	unitAssert(Time::timeBetween(nadt,ninf) == td_nadt);
	unitAssert(Time::timeBetween(zero,nadt) == td_nadt);
	unitAssert(Time::timeBetween(nadt,zero) == td_nadt);

	// Any difference with the second infinitely greater than the first is +inf
	unitAssert(Time::timeBetween(ninf,pinf) == td_pinf);
	unitAssert(Time::timeBetween(ninf,zero) == td_pinf);
	unitAssert(Time::timeBetween(zero,pinf) == td_pinf);

	// subtracting an infinity from itself is ambiguous
	unitAssert(Time::timeBetween(pinf,pinf) == td_nadt);
	unitAssert(Time::timeBetween(ninf,ninf) == td_nadt);

	// Any difference with the second infinitely less than the first is -inf
	unitAssert(Time::timeBetween(pinf,ninf) == td_ninf);
	unitAssert(Time::timeBetween(zero,ninf) == td_ninf);
	unitAssert(Time::timeBetween(pinf,zero) == td_ninf);

	unitAssert(Time::timeBetween(zero,zero) == td_zero);
}

AUTO_UNIT_TEST(DateTimeTestCases_testSpecialComparisons)
{
	DateTime nadt = DateTime::getNADT();
	DateTime pinf = DateTime::getPosInfinity();
	DateTime ninf = DateTime::getNegInfinity();
	DateTime zero = DateTime();

	// nadt, nadt
	unitAssert(nadt <= nadt);
	unitAssert(nadt >= nadt);
	unitAssert(nadt == nadt);
	unitAssert(!(nadt != nadt));
	unitAssert(!(nadt < nadt));
	unitAssert(!(nadt > nadt));


	// nadt, +inf
	unitAssert(!(nadt < pinf));
	unitAssert(!(nadt > pinf));
	unitAssert(!(nadt <= pinf));
	unitAssert(!(nadt >= pinf));
	unitAssert(!(nadt == pinf));
	unitAssert(nadt != pinf);

	unitAssert(!(pinf < nadt));
	unitAssert(!(pinf > nadt));
	unitAssert(!(pinf <= nadt));
	unitAssert(!(pinf >= nadt));
	unitAssert(!(pinf == nadt));
	unitAssert(pinf != nadt);


	// nadt, -inf
	unitAssert(!(nadt < ninf));
	unitAssert(!(nadt > ninf));
	unitAssert(!(nadt <= ninf));
	unitAssert(!(nadt >= ninf));
	unitAssert(!(nadt == ninf));
	unitAssert(nadt != ninf);

	unitAssert(!(ninf < nadt));
	unitAssert(!(ninf > nadt));
	unitAssert(!(ninf <= nadt));
	unitAssert(!(ninf >= nadt));
	unitAssert(!(ninf == nadt));
	unitAssert(ninf != nadt);

	// nadt, zero
	unitAssert(!(nadt < zero));
	unitAssert(!(nadt > zero));
	unitAssert(!(nadt <= zero));
	unitAssert(!(nadt >= zero));
	unitAssert(!(nadt == zero));
	unitAssert(nadt != zero);

	unitAssert(!(zero < nadt));
	unitAssert(!(zero > nadt));
	unitAssert(!(zero <= nadt));
	unitAssert(!(zero >= nadt));
	unitAssert(!(zero == nadt));
	unitAssert(zero != nadt);

	// zero, +inf
	unitAssert(zero < pinf);
	unitAssert(!(zero > pinf));
	unitAssert(zero <= pinf);
	unitAssert(!(zero >= pinf));
	unitAssert(!(zero == pinf));
	unitAssert(zero != pinf);

	unitAssert(!(pinf < zero));
	unitAssert(pinf > zero);
	unitAssert(!(pinf <= zero));
	unitAssert(pinf >= zero);
	unitAssert(!(pinf == zero));
	unitAssert(pinf != zero);


	// zero, -inf
	unitAssert(!(zero < ninf));
	unitAssert(zero > ninf);
	unitAssert(!(zero <= ninf));
	unitAssert(zero >= ninf);
	unitAssert(!(zero == ninf));
	unitAssert(zero != ninf);

	unitAssert(ninf < zero);
	unitAssert(!(ninf > zero));
	unitAssert(ninf <= zero);
	unitAssert(!(ninf >= zero));
	unitAssert(!(ninf == zero));
	unitAssert(ninf != zero);


	unitAssert(ninf == std::min(zero, ninf));
	unitAssert(ninf == std::min(pinf, ninf));
	unitAssert(zero == std::max(zero, ninf));
	unitAssert(pinf == std::max(pinf, ninf));

	// Using std::min or std::max with NADT may produce non-portable results
	// depending on which operator was used to implement min.
}

#define DT_MEMBER_HAS_NO_EFFECT(initial_value, operation) do { DateTime dt(initial_value); dt.operation; unitAssert(dt == DateTime(initial_value)); } while(0)

AUTO_UNIT_TEST(DateTimeTestCases_testNADT)
{
	DateTime nadt = DateTime::getNADT();

	// All operations that attempt to get or set a portion of the time (not
	// including second or microsecond) or  should throw.
	unitAssertThrows(nadt.getHour());
	unitAssertThrows(nadt.getMinute());
	unitAssertThrows(nadt.getSecond());
	unitAssertThrows(nadt.getDay());
	unitAssertThrows(nadt.getDow());
	unitAssertThrows(nadt.setHour(1));
	unitAssertThrows(nadt.setMinute(1));
	unitAssertThrows(nadt.setSecond(1));
	unitAssertThrows(nadt.setMicrosecond(1));
	unitAssertThrows(nadt.setTime(1,1,1));
	unitAssertThrows(nadt.setDay(1));
	unitAssertThrows(nadt.setMonth(1));
	unitAssertThrows(nadt.setYear(1970));

	struct tm foo;
	unitAssertThrows(nadt.toLocal(foo));

	DT_MEMBER_HAS_NO_EFFECT(nadt, addDays(1));
	DT_MEMBER_HAS_NO_EFFECT(nadt, addWeeks(1));
	DT_MEMBER_HAS_NO_EFFECT(nadt, addMonths(1));
	DT_MEMBER_HAS_NO_EFFECT(nadt, addYears(1));
	DT_MEMBER_HAS_NO_EFFECT(nadt, addSeconds(1));
	DT_MEMBER_HAS_NO_EFFECT(nadt, addMinutes(1));
	DT_MEMBER_HAS_NO_EFFECT(nadt, addMicroseconds(1));
	DT_MEMBER_HAS_NO_EFFECT(nadt, addMilliseconds(1));
	DT_MEMBER_HAS_NO_EFFECT(nadt, addHours(1));
	DT_MEMBER_HAS_NO_EFFECT(nadt, operator+=(1));
	DT_MEMBER_HAS_NO_EFFECT(nadt, operator-=(1));
}

AUTO_UNIT_TEST(DateTimeTestCases_testPosInfinity)
{
	DateTime pinf = DateTime::getPosInfinity();

	// All operations that attempt to get or set a portion of the time (not
	// including second or microsecond) or  should throw.
	unitAssertThrows(pinf.getHour());
	unitAssertThrows(pinf.getMinute());
	unitAssertThrows(pinf.getSecond());
	unitAssertThrows(pinf.getDay());
	unitAssertThrows(pinf.getDow());
	unitAssertThrows(pinf.setHour(1));
	unitAssertThrows(pinf.setMinute(1));
	unitAssertThrows(pinf.setSecond(1));
	unitAssertThrows(pinf.setMicrosecond(1));
	unitAssertThrows(pinf.setTime(1,1,1));
	unitAssertThrows(pinf.setDay(1));
	unitAssertThrows(pinf.setMonth(1));
	unitAssertThrows(pinf.setYear(1970));

	struct tm foo;
	unitAssertThrows(pinf.toLocal(foo));

	DT_MEMBER_HAS_NO_EFFECT(pinf, addDays(1));
	DT_MEMBER_HAS_NO_EFFECT(pinf, addWeeks(1));
	DT_MEMBER_HAS_NO_EFFECT(pinf, addMonths(1));
	DT_MEMBER_HAS_NO_EFFECT(pinf, addYears(1));
	DT_MEMBER_HAS_NO_EFFECT(pinf, addSeconds(1));
	DT_MEMBER_HAS_NO_EFFECT(pinf, addMinutes(1));
	DT_MEMBER_HAS_NO_EFFECT(pinf, addMicroseconds(1));
	DT_MEMBER_HAS_NO_EFFECT(pinf, addMilliseconds(1));
	DT_MEMBER_HAS_NO_EFFECT(pinf, addHours(1));
	DT_MEMBER_HAS_NO_EFFECT(pinf, operator+=(1));
	DT_MEMBER_HAS_NO_EFFECT(pinf, operator-=(1));
}

AUTO_UNIT_TEST(DateTimeTestCases_testNegInfinity)
{
	DateTime ninf = DateTime::getNegInfinity();

	// All operations that attempt to get or set a portion of the time (not
	// including second or microsecond) or  should throw.
	unitAssertThrows(ninf.getHour());
	unitAssertThrows(ninf.getMinute());
	unitAssertThrows(ninf.getSecond());
	unitAssertThrows(ninf.getDay());
	unitAssertThrows(ninf.getDow());
	unitAssertThrows(ninf.setHour(1));
	unitAssertThrows(ninf.setMinute(1));
	unitAssertThrows(ninf.setSecond(1));
	unitAssertThrows(ninf.setMicrosecond(1));
	unitAssertThrows(ninf.setTime(1,1,1));
	unitAssertThrows(ninf.setDay(1));
	unitAssertThrows(ninf.setMonth(1));
	unitAssertThrows(ninf.setYear(1970));

	struct tm foo;
	unitAssertThrows(ninf.toLocal(foo));

	DT_MEMBER_HAS_NO_EFFECT(ninf, addDays(1));
	DT_MEMBER_HAS_NO_EFFECT(ninf, addWeeks(1));
	DT_MEMBER_HAS_NO_EFFECT(ninf, addMonths(1));
	DT_MEMBER_HAS_NO_EFFECT(ninf, addYears(1));
	DT_MEMBER_HAS_NO_EFFECT(ninf, addSeconds(1));
	DT_MEMBER_HAS_NO_EFFECT(ninf, addMinutes(1));
	DT_MEMBER_HAS_NO_EFFECT(ninf, addMicroseconds(1));
	DT_MEMBER_HAS_NO_EFFECT(ninf, addMilliseconds(1));
	DT_MEMBER_HAS_NO_EFFECT(ninf, addHours(1));
	DT_MEMBER_HAS_NO_EFFECT(ninf, operator+=(1));
	DT_MEMBER_HAS_NO_EFFECT(ninf, operator-=(1));
}

AUTO_UNIT_TEST(DateTimeTestCases_testDateMath)
{
	DateTime d = DateTime::getCurrent();

	DateTime nadt = DateTime::getNADT();
	DateTime pinf = DateTime::getPosInfinity();
	DateTime ninf = DateTime::getNegInfinity();
	DateTime zero = DateTime();
	DateTime one = DateTime(time_t(0), 1);
	DateTime largest = DateTime::getMaximumTime();
	DateTime smallest = DateTime::getMinimumTime();
	Time::TimeDuration td_zero = Time::TimeDuration();
	Time::TimeDuration td_one = Time::microseconds(1);
	Time::TimeDuration td_nadt = Time::TimeDuration(Time::E_TIME_NADT);
	Time::TimeDuration td_pinf = Time::TimeDuration(Time::E_TIME_POS_INFINITY);
	Time::TimeDuration td_ninf = Time::TimeDuration(Time::E_TIME_NEG_INFINITY);

	unitAssert(d + td_one > d);
	unitAssert(td_one + d > d);
	unitAssert(d + -td_one < d);
	unitAssert(d - td_one < d);
	unitAssert(Time::timeBetween(d, d + td_one) == td_one);
	unitAssert(Time::timeBetween(d, d + -td_one) == -td_one);
	unitAssert(Time::timeBetween(d + td_one, d) == -td_one);
	unitAssert(Time::timeBetween(d + -td_one, d) == td_one);

	unitAssert(d + td_pinf > d);
	unitAssert(d + td_ninf < d);
	unitAssert(Time::timeBetween(d, d + td_pinf) == td_pinf);
	unitAssert(Time::timeBetween(d, d + td_ninf) == td_ninf);
	unitAssert(Time::timeBetween(d + td_pinf, d) == td_ninf);
	unitAssert(Time::timeBetween(d + td_ninf, d) == td_pinf);

	// Negation
	unitAssert(-zero == zero);
	unitAssert(-pinf == ninf);
	unitAssert(-ninf == pinf);
	unitAssert(-nadt == nadt);

	// DateTime - DateTime
	unitAssert(pinf - ninf == td_pinf);
	unitAssert(pinf - zero == td_pinf);
	unitAssert(pinf - pinf == td_nadt);
	unitAssert(ninf - ninf == td_nadt);
	unitAssert(ninf - zero == td_ninf);
	unitAssert(ninf - pinf == td_ninf);

	// DateTime + TimeDuration
	unitAssert(nadt + td_nadt == nadt);
	unitAssert(nadt + td_zero == nadt);
	unitAssert(nadt + td_pinf == nadt);
	unitAssert(nadt + td_ninf == nadt);

	unitAssert(zero + td_nadt == nadt);
	unitAssert(zero + td_zero == zero);
	unitAssert(zero + td_pinf == pinf);
	unitAssert(zero + td_ninf == ninf);

	unitAssert(ninf + td_nadt == nadt);
	unitAssert(ninf + td_zero == ninf);
	unitAssert(ninf + td_pinf == nadt);
	unitAssert(ninf + td_ninf == ninf);

	unitAssert(pinf + td_nadt == nadt);
	unitAssert(pinf + td_zero == pinf);
	unitAssert(pinf + td_pinf == pinf);
	unitAssert(pinf + td_ninf == nadt);

	// DateTime - TimeDuration
	unitAssert(nadt - td_nadt == nadt);
	unitAssert(nadt - td_zero == nadt);
	unitAssert(nadt - td_pinf == nadt);
	unitAssert(nadt - td_ninf == nadt);

	unitAssert(zero - td_nadt == nadt);
	unitAssert(zero - td_zero == zero);
	unitAssert(zero - td_pinf == ninf);
	unitAssert(zero - td_ninf == pinf);

	unitAssert(ninf - td_nadt == nadt);
	unitAssert(ninf - td_zero == ninf);
	unitAssert(ninf - td_pinf == ninf);
	unitAssert(ninf - td_ninf == nadt);

	unitAssert(pinf - td_nadt == nadt);
	unitAssert(pinf - td_zero == pinf);
	unitAssert(pinf - td_pinf == nadt);
	unitAssert(pinf - td_ninf == pinf);

	// Date overflow.
	unitAssertThrows(largest + Time::hours(1));
	unitAssertThrows(smallest - Time::hours(1));
	unitAssertThrows(largest + Time::seconds(1));
	unitAssertThrows(smallest - Time::seconds(1));
	unitAssertThrows(largest + Time::microseconds(1));
	unitAssertThrows(smallest - Time::microseconds(1));
	// No effect
	unitAssertNoThrow(largest + td_zero);
	unitAssertNoThrow(smallest - td_zero);
	// No overflow happens with special values.
	unitAssertNoThrow(largest + td_pinf);
	unitAssertNoThrow(smallest - td_pinf);
	unitAssertNoThrow(largest + td_ninf);
	unitAssertNoThrow(smallest - td_ninf);
	unitAssertNoThrow(largest + td_nadt);
	unitAssertNoThrow(smallest - td_nadt);
}

AUTO_UNIT_TEST(DateTimeTestCases_testSetToCurrent)
{
	DateTime d;
	d.setToCurrent();
	unitAssert(!Time::isSpecial(d));
	unitAssert(d.getMicrosecond() <= 999999);
}

AUTO_UNIT_TEST(DateTimeTestCases_testAddReal64Seconds)
{
	DateTime d(1000000000, 100000);
	DateTime d1 = d + 123.456789;
	unitAssertEquals(1000000123, d1.get());
	unitAssertEquals(556789, d1.getMicrosecond());
}

AUTO_UNIT_TEST(DateTimeTestCases_testNormalization)
{
	{ // overflow microseconds
		DateTime d(1000000000, 1000500);
		unitAssertEquals(1000000001, d.get());
		unitAssertEquals(500, d.getMicrosecond());
	}
	{ // microseconds is a positive multiple of 1e6
		DateTime d(999999999, 1000000);
		unitAssertEquals(1000000000, d.get());
		unitAssertEquals(0, d.getMicrosecond());
	}
	{ // underflow microseconds
		DateTime d(1000000000, -1);
		unitAssertEquals(999999999, d.get());
		unitAssertEquals(999999, d.getMicrosecond());
	}
	{ // microseconds is a negative multiple of 1e6
		DateTime d(1000000000, -1000000);
		unitAssertEquals(999999999, d.get());
		unitAssertEquals(0, d.getMicrosecond());
	}

	time_t const dtmin = BLOCXX_DATETIME_MINIMUM_TIME;
	time_t const dtmax = BLOCXX_DATETIME_MAXIMUM_TIME;
	time_t const tmin = std::numeric_limits<time_t>::min();
	time_t const tmax = std::numeric_limits<time_t>::max();
	Int32 const imin = std::numeric_limits<Int32>::min();
	Int32 const imax = std::numeric_limits<Int32>::max();

	DateTime::ETimeOffset const utc = DateTime::E_UTC_TIME;
	{ // create minimum possible time
		DateTime d(dtmin, 0);
		unitAssertEquals(dtmin, d.get());
		unitAssertEquals(0, d.getMicrosecond());
		unitAssert(d <= DateTime(1970, 1, 1, 0, 0, 0, 0, utc));
	}
	if (dtmin > tmin)
	{ // explicitly violate lower bound for a DateTime
		unitAssertThrowsEx(DateTime(dtmin - 1, 999999), DateTimeException);
	}
	// violate lower bound after normalization
	unitAssertThrowsEx(DateTime(dtmin, -1), DateTimeException);

	// deal with integer underflow in normalization
	unitAssertThrowsEx(DateTime(dtmin, imin), DateTimeException);
	unitAssertThrowsEx(DateTime(tmin, imin), DateTimeException);

	{ // create maximum possible time
		DateTime d(dtmax, 999999);
		unitAssertEquals(dtmax, d.get());
		unitAssertEquals(999999, d.getMicrosecond());
		unitAssert(d >= DateTime(2037, 12, 31, 23, 59, 59, 999999, utc));
	}

	if (dtmax < tmax)
	{ // explicitly violate upper bound for DateTime
		unitAssertThrowsEx(DateTime(dtmax + 1, 0), DateTimeException);
	}

	// violate upper bound after normalization
	unitAssertThrowsEx(DateTime(dtmax, 1000000), DateTimeException);

	// deal with integer overflow in normalization
	unitAssertThrowsEx(DateTime(dtmax, imax), DateTimeException);
	unitAssertThrowsEx(DateTime(tmax, imax), DateTimeException);
}
