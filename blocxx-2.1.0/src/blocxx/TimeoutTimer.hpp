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

#ifndef BLOCXX_TIMEOUT_TIMER_HPP_INCLUDE_GUARD_
#define BLOCXX_TIMEOUT_TIMER_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/DateTime.hpp"

#ifdef BLOCXX_HAVE_SYS_TIME_H
#include <sys/time.h> // for timeval
#endif
#include <time.h> // for timespec

namespace BLOCXX_NAMESPACE
{

/**
 * A TimeoutTimer is used by an algorithm to determine when a timeout has expired.
 * 
 * It is used like this:
 * 
 * TimeoutTimer timer(theTimeout);
 * while (... && !timer.expired())
 * {
 *     // wait for something to happen, maybe calling select() or something else.
 *     struct timeval tv;
 *     select(... , timer.asTimeval(tv));
 * 
 *     ... // processing
 * 
 *     if (something happened that might reset the interval)
 *         timer.resetOnLoop();
 *     else
 *         timer.loop();
 * }
 * 
 */
class BLOCXX_COMMON_API TimeoutTimer
{
public:
	TimeoutTimer(const Timeout& x);
	virtual ~TimeoutTimer();

	/**
	 * Meant to be called by timeout functions which loop. Normally, it should be called once, at the beginning of the function. 
	 * The time this is called marks the beginning of a relative interval. 
	 * For an absolute timeout, it does nothing.
	 * It is not necessary to call this function, only if the time the constructor ran isn't desireable to be used as the start time.
	 */
	void start();

	/**
	 * Meant to be called by timeout functions which loop, and that want to reset the interval.
	 * It should be called each time through the loop.
	 * For a relative with reset timeout it will reset the interval.
	 * Either resetOnLoop() or loop() should be called each time through the loop, but not both.
	 */
	void resetOnLoop();

	/**
	 * Meant to be called by timeout functions which loop, but don't want to reset the interval.
	 * It should be called each time through the loop.
	 * Either resetOnLoop() or loop() should be called each time through the loop, but not both.
	 */
	void loop();

	/**
	 * Indicates whether the last loop time has exceeded the timeout. The current time is not used, but the time when resetOnLoop() or loop() was called.
	 */
	bool expired() const;

	/**
	 * Indicates whether the timeout will never expire
	 */
	bool infinite() const;

#ifdef BLOCXX_HAVE_STRUCT_TIMEVAL
	// return 0 for infinite, otherwise a pointer to tv, and tv will be modified to contain an interval suitable for use with select().
	::timeval* asTimeval(::timeval& tv, double maxSeconds) const;
	// return 0 for infinite, otherwise a pointer to tv, and tv will be modified to contain an interval suitable for use with select().
	::timeval* asTimeval(::timeval& tv) const;
#endif

#ifdef BLOCXX_HAVE_STRUCT_TIMESPEC
	// fills out ts and returns the address of it.
	::timespec* asTimespec(::timespec& ts) const;
#endif
	
	Timeout asTimeout() const;
	Timeout asRelativeTimeout() const;
	Timeout asRelativeTimeout(double maxSeconds) const;
	/**
	 * Converts the timer to an absolute timeout.
	 * If the timer is infinite, the return will be an infinite relative timeout
	 */
	Timeout asAbsoluteTimeout() const;

#ifdef BLOCXX_WIN32
	// returns INFINITE for an infinite timeout.
	::DWORD asDWORDMs() const;
#endif
	
	// returns -1 for infinite
	int asIntMs() const;
	int asIntMs(double maxSeconds) const;

private:

	double calcSeconds() const;
	double calcSeconds(double maxSeconds) const;

	// for test purposes, the current time can be faked by a derived class.
	virtual DateTime getCurrentTime() const;

	Timeout m_timeout;
	DateTime m_start;
	DateTime m_loopTime;
};

} // end namespace BLOCXX_NAMESPACE

#endif



