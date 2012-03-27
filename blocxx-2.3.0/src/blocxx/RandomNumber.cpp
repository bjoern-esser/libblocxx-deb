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


/**
 * @author Bart Whiteley
 * @author Dan Nuffer
 * @author Kevin Harris
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/RandomNumber.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/ThreadOnce.hpp"
#include "blocxx/Mutex.hpp"
#include "blocxx/MutexLock.hpp"
#include "blocxx/GlobalMutex.hpp"
#include "blocxx/DateTime.hpp"
#include <fstream>
#include <sys/types.h>
#include <cmath>

#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef BLOCXX_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <stdlib.h>
#include <time.h>

namespace BLOCXX_NAMESPACE
{

/////////////////////////////////////////////////////////////////////////////
namespace
{
OnceFlag guard = BLOCXX_ONCE_INIT;
unsigned int seed = 0;
}

/////////////////////////////////////////////////////////////////////////////
RandomNumber::RandomNumber(Int32 lowVal, Int32 highVal)
: m_lowVal(lowVal), m_highVal(highVal)
{
	if (lowVal > highVal)
	{
		m_lowVal = highVal;
		m_highVal = lowVal;
	}
	callOnce(guard, &initRandomness);
}

/////////////////////////////////////////////////////////////////////////////
void
RandomNumber::initRandomness()
{
#ifdef BLOCXX_WIN32
	time_t timeval = ::time(NULL);
	seed = timeval;
#else
	// use the time as part of the seed
	struct timeval tv;
	gettimeofday(&tv, 0);
	// try to get something from the kernel
	std::ifstream infile("/dev/urandom", std::ios::in);
	if (!infile)
	{
		infile.open("/dev/random", std::ios::in);
	}
	// don't initialize this, we may get random stack
	// junk in case infile isn't usable.
	unsigned int dev_rand_input;
	if (infile)
	{
		infile.read(reinterpret_cast<char*>(&dev_rand_input), sizeof(dev_rand_input));
		infile.close();
	}
	// Build the seed. Take into account our pid and uid.
	seed = dev_rand_input ^ (getpid() << 16) ^ getuid() ^ tv.tv_sec ^ tv.tv_usec;
#endif
#ifdef BLOCXX_HAVE_SRANDOM
	srandom(seed);
#else
	srand(seed);
#endif
}

/////////////////////////////////////////////////////////////////////////////
void
RandomNumber::saveRandomState()
{
	// Do nothing. This function is so that RandomNumber has the same interface as CryptographicRandomNumber
}

namespace
{
GlobalMutex g_guard = BLOCXX_GLOBAL_MUTEX_INIT();
}
/////////////////////////////////////////////////////////////////////////////
Int32
RandomNumber::getNextNumber()
{
	MutexLock lock(g_guard);
#ifdef BLOCXX_HAVE_RANDOM
	return m_lowVal + (random() % (m_highVal - m_lowVal + 1));
#else
	return m_lowVal + (rand() % (m_highVal - m_lowVal + 1));
#endif
}

///////////////////////////////////////////////////////////////////////////

MersenneTwisterRandomNumber::MersenneTwisterRandomNumber()
{
	DateTime temp = DateTime::getCurrent();
	/**
	 * Use a large prime number & the time.  Microseconds and seconds were
	 * included to provide more randomness in the start state of the twister.
	 */
	setSeed(1073741827ul * (UInt32(temp.get()) ^ UInt32(temp.getMicrosecond())));
}


MersenneTwisterRandomNumber::MersenneTwisterRandomNumber(UInt32 seed)
{
	setSeed(seed);
}


MersenneTwisterRandomNumber::MersenneTwisterRandomNumber(const MersenneTwisterRandomNumber& old)
{
	*this = old;
}

MersenneTwisterRandomNumber& MersenneTwisterRandomNumber::operator=(const MersenneTwisterRandomNumber& old)
{
	for( size_t i = 0; i < STATE_SIZE; ++i )
	{
		m_state[i] = old.m_state[i];
	}
	m_stateUsed = old.m_stateUsed;

	return *this;
}


void MersenneTwisterRandomNumber::setSeed(UInt32 seed)
{
	m_state[0] = seed;
	for( m_stateUsed = 1; m_stateUsed < STATE_SIZE; ++m_stateUsed )
	{
		m_state[m_stateUsed] = (
			1812433253ul * (
				(m_state[m_stateUsed-1]) ^	(m_state[m_stateUsed-1] >> 30)
			) + m_stateUsed);
	}
}

namespace
{
	// These are constants used several times in the getNextNumber
	// function, where it is easier to see what it is doing without these
	// numbers there.
	const UInt32 MERS_M = 397;
	const UInt32 MERS_R = 31;
	const UInt32 MERS_U = 11;
	const UInt32 MERS_S = 7;
	const UInt32 MERS_T = 15;
	const UInt32 MERS_L = 18;
	const UInt32 MERS_A = 0x9908B0DF;
	const UInt32 MERS_B = 0x9D2C5680;
	const UInt32 MERS_C = 0xEFC60000;
}


UInt32 MersenneTwisterRandomNumber::getNextNumber(UInt32 maxNumber)
{
	// generate 32 random bits
	UInt32 y;

	if (m_stateUsed >= STATE_SIZE)
	{
		// generate STATE_SIZE words at one time
		const UInt32 LOWER_MASK = (UInt32(1) << MERS_R) - 1; // lower MERS_R bits
		const UInt32 UPPER_MASK = UInt32(-1)  << MERS_R;      // upper (32 - MERS_R) bits

		// This is to prevent the need for two cases (odd and even).
		const UInt32 mag01[2] = {0, MERS_A};

		size_t kk;
		for (kk=0; kk < STATE_SIZE-MERS_M; kk++)
		{
			y = (m_state[kk] & UPPER_MASK) | (m_state[kk+1] & LOWER_MASK);
			m_state[kk] = m_state[kk+MERS_M] ^ (y >> 1) ^ mag01[y & 1];
		}
		for (; kk < STATE_SIZE-1; kk++)
		{
			y = (m_state[kk] & UPPER_MASK) | (m_state[kk+1] & LOWER_MASK);
			m_state[kk] = m_state[kk+(MERS_M-STATE_SIZE)] ^ (y >> 1) ^ mag01[y & 1];
		}

		y = (m_state[STATE_SIZE-1] & UPPER_MASK) | (m_state[0] & LOWER_MASK);
		m_state[STATE_SIZE-1] = m_state[MERS_M-1] ^ (y >> 1) ^ mag01[y & 1];
		m_stateUsed = 0;
	}

	y = m_state[m_stateUsed++];

	// Tempering:
	y ^=  y >> MERS_U;
	y ^= (y << MERS_S) & MERS_B;
	y ^= (y << MERS_T) & MERS_C;
	y ^=  y >> MERS_L;

	if( maxNumber != UInt32(-1) )
	{
		y = static_cast<UInt32>(ldexp(double(y), -32) * (maxNumber + 1));
	}

	return y;
}

UInt32 MersenneTwisterRandomNumber::getMaxNumber()
{
	return UInt32(-1);
}

///////////////////////////////////////////////////////////////////////////

} // end namespace BLOCXX_NAMESPACE

