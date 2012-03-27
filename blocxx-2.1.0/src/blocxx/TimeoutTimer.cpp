/*******************************************************************************
* Copyright (C) 2005, Vintela, Inc. All rights reserved.
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
*       Vintela, Inc., 
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
#include "blocxx/TimeoutTimer.hpp"
#include "blocxx/DateTime.hpp"
#include "blocxx/Infinity.hpp"

#include <limits>

namespace BLOCXX_NAMESPACE
{

TimeoutTimer::TimeoutTimer(const Timeout& x)
	: m_timeout(x)
	, m_start(getCurrentTime())
	, m_loopTime(m_start)
{
}

TimeoutTimer::~TimeoutTimer()
{
}

void
TimeoutTimer::start()
{
	m_start = m_loopTime = getCurrentTime();
}

void
TimeoutTimer::resetOnLoop()
{
	loop();
	if (m_timeout.getType() == Timeout::E_RELATIVE_WITH_RESET)
	{
		m_start = m_loopTime;
	}
}

void
TimeoutTimer::loop()
{
	m_loopTime = getCurrentTime();
}

namespace
{

template <typename T, typename U>
void safeAssign(T& x, U y)
{
	if (y < (std::numeric_limits<T>::min)())
	{
		x = (std::numeric_limits<T>::min)();
	}
	else if (y > (std::numeric_limits<T>::max)())
	{
		x = (std::numeric_limits<T>::max)();
	}
	else
	{
		x = static_cast<T>(y + 0.5); // we add 0.5 so the behavior is like rounding, instead of truncation.
	}
}

bool compareInterval(const DateTime& first, const DateTime& timeToTest, double seconds)
{
	// Return first + seconds < timeToTest <==> timeToTest - first >= seconds.
	// The latter form avoids overflow problems.
	DateTime diff = timeToTest - first;
	double diff1 = diff.get() + diff.getMicrosecond() * 1e-6;
	return diff1 >= seconds;
}

} // end unnamed namespace

bool 
TimeoutTimer::expired() const
{
	if (infinite())
	{
		return false;
	}

	switch (m_timeout.getType())
	{
		case Timeout::E_ABSOLUTE:
			return m_loopTime >= m_timeout.getAbsolute();

		case Timeout::E_RELATIVE:
		case Timeout::E_RELATIVE_WITH_RESET:
			return compareInterval(m_start, m_loopTime, m_timeout.getRelative());
	}
	return false;
}

bool
TimeoutTimer::infinite() const
{
	return m_timeout == Timeout::infinite;
}

#ifdef BLOCXX_HAVE_STRUCT_TIMEVAL
// return 0 for infinite, otherwise a pointer to tv
::timeval* 
TimeoutTimer::asTimeval(::timeval& tv, double maxSeconds) const
{
	if (infinite() && maxSeconds == INFINITY)
	{
		return 0;
	}

	asTimeval(tv);

	// check & enforce maxSeconds parameter
	::timeval temp;
	safeAssign(temp.tv_sec, maxSeconds);
	double dummy;
	safeAssign(temp.tv_usec, modf(maxSeconds, &dummy) * 1e6);
	if (infinite() || temp.tv_sec < tv.tv_sec)
	{
		tv = temp;
	}
	else if (temp.tv_sec == tv.tv_sec && temp.tv_usec < tv.tv_usec)
	{
		tv.tv_usec = temp.tv_usec;
	}

	return &tv;
}

// return 0 for infinite, otherwise a pointer to tv
::timeval* 
TimeoutTimer::asTimeval(::timeval& tv) const
{
	if (infinite())
	{
		return 0;
	}

	if (m_timeout.getType() == Timeout::E_ABSOLUTE)
	{
		// convert the difference between the last loop and the absolute timeout into a timeval
		DateTime timeoutTime = m_timeout.getAbsolute();
		if (timeoutTime > m_loopTime)
		{
			DateTime diff = m_timeout.getAbsolute() - m_loopTime;
			tv.tv_sec = diff.get();
			tv.tv_usec = diff.getMicrosecond();
		}
		else
		{
			// somehow we got past the timeout, so just return a 0 length timeval.
			tv.tv_sec = 0;
			tv.tv_usec = 0;
		}
	}
	else // it's relative
	{
		double timeTillExpiration = calcSeconds();
		safeAssign(tv.tv_sec, timeTillExpiration);
		double dummy;
		safeAssign(tv.tv_usec, modf(timeTillExpiration, &dummy) * 1e6);
	}


	return &tv;
}
#endif

Timeout
TimeoutTimer::asTimeout() const
{
	return m_timeout;
}

double
TimeoutTimer::calcSeconds() const
{
	// return the number of seconds until the timeout expires.
	double seconds;
	if (m_timeout.getType() == Timeout::E_ABSOLUTE)
	{
		DateTime diff = m_timeout.getAbsolute() - m_loopTime;
		seconds = static_cast<double>(diff.get()) + (static_cast<double>(diff.getMicrosecond()) / 1e6);
	}
	else
	{
		// seconds = (m_start + m_timeout.getRelative()) - m_loopTime
		double start = static_cast<double>(m_start.get()) + (static_cast<double>(m_start.getMicrosecond()) / 1e6);
		double loop = static_cast<double>(m_loopTime.get()) + (static_cast<double>(m_loopTime.getMicrosecond()) / 1e6);
		seconds = start + m_timeout.getRelative() - loop;
	}
	return seconds;
}

double
TimeoutTimer::calcSeconds(double maxSeconds) const
{
	double seconds = calcSeconds();
	seconds = seconds > maxSeconds ? maxSeconds : seconds;
	return seconds;
}

Timeout 
TimeoutTimer::asRelativeTimeout() const
{
	return Timeout::relative(calcSeconds());
}

Timeout 
TimeoutTimer::asRelativeTimeout(double maxSeconds) const
{
	return Timeout::relative(calcSeconds(maxSeconds));
}

Timeout 
TimeoutTimer::asAbsoluteTimeout() const
{
	// an infinite timeout cannot be converted to absolute, it stays infinite
	if (infinite())
	{
		return m_timeout;
	}
	if (m_timeout.getType() == Timeout::E_ABSOLUTE)
	{
		return m_timeout;
	}
	else
	{
		const long MICROSECONDS_PER_SECOND = 1000000;
		DateTime endTime(m_start);
		time_t secs;
		safeAssign(secs, m_start.get() + static_cast<double>(m_timeout.getRelative()));
		UInt32 microSecs;
		double dummy;
		safeAssign(microSecs, 
			m_start.getMicrosecond() +
			modf(m_timeout.getRelative(), &dummy) * MICROSECONDS_PER_SECOND);

		// handle any overflow
		secs += microSecs / MICROSECONDS_PER_SECOND;
		microSecs = microSecs % MICROSECONDS_PER_SECOND;

		return Timeout::absolute(DateTime(secs, microSecs));
	}
}

#ifdef BLOCXX_WIN32
DWORD 
TimeoutTimer::asDWORDMs() const
{
	if (infinite())
	{
		return INFINITE;
	}

	DWORD rval;
	safeAssign(rval, calcSeconds() * 1000.0);
	return rval;
}
#endif

int 
TimeoutTimer::asIntMs() const
{
	if (infinite())
	{
		return -1;
	}

	int rval;
	safeAssign(rval, calcSeconds() * 1000.0);
	return rval;
}

int
TimeoutTimer::asIntMs(double maxSeconds) const
{
	if (infinite() && maxSeconds == INFINITY)
	{
		return -1;
	}

	int rval;
	safeAssign(rval, calcSeconds(maxSeconds) * 1000.0);
	return rval;
}

#ifdef BLOCXX_HAVE_STRUCT_TIMESPEC
::timespec* 
TimeoutTimer::asTimespec(::timespec& ts) const
{
	const long NANOSECONDS_PER_MICROSECOND = 1000;
	const long NANOSECONDS_PER_MILLISECOND = 1000000;
	const long NANOSECONDS_PER_SECOND      = 1000000000;

	if (m_timeout.getType() == Timeout::E_ABSOLUTE)
	{
		// convert the timeout to a timespec
		DateTime timeoutTime = m_timeout.getAbsolute();
		if (timeoutTime > m_loopTime)
		{
			ts.tv_sec = timeoutTime.get();
			ts.tv_nsec = timeoutTime.getMicrosecond() * NANOSECONDS_PER_MICROSECOND;
		}
		else
		{
			// somehow we got past the timeout (a time warp?), so just return a 0 timespec.
			ts.tv_sec = 0;
			ts.tv_nsec = 0;
		}
	}
	else // relative
	{
		safeAssign(ts.tv_sec, m_start.get() + static_cast<double>(m_timeout.getRelative()));
		double dummy;
		safeAssign(ts.tv_nsec, 
			static_cast<double>(m_start.getMicrosecond()) * NANOSECONDS_PER_MICROSECOND +
			modf(m_timeout.getRelative(), &dummy) * NANOSECONDS_PER_SECOND);

		// handle any overflow because ts.tv_nsec has to be in the correct range.
		ts.tv_sec += ts.tv_nsec / NANOSECONDS_PER_SECOND;
		ts.tv_nsec = ts.tv_nsec % NANOSECONDS_PER_SECOND;
	}

	return &ts;
}
#endif

DateTime
TimeoutTimer::getCurrentTime() const
{
	return DateTime::getCurrent();
}

} // end namespace BLOCXX_NAMESPACE




