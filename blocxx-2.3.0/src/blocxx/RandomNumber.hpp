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

#ifndef BLOCXX_RANDOM_NUMBER_HPP_INCLUDE_GUARD_
#define BLOCXX_RANDOM_NUMBER_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Types.hpp"
#include <stdlib.h> // for RAND_MAX

namespace BLOCXX_NAMESPACE
{

class BLOCXX_COMMON_API RandomNumber
{
public:
	// Precondition: lowVal < highVal
	RandomNumber(Int32 lowVal = 0, Int32 highVal = RAND_MAX);
	Int32 getNextNumber();

public:
	// This function can be called to control when the prng will be initialized.
	// If it hasn't been previously called, it will be called the first time a RandomNumber instance is instantiated.
	static void initRandomness();
	static void saveRandomState();

private:
	Int32 m_lowVal;
	Int32 m_highVal;
};


/** A mersenne twister.  This is not cryptographically secure, but is nicely
 * distributed.  It does not follow the same interface as RandomNumber since
 * it maintains its own state instead of using the system-provided random
 * number facilities.
 */
class BLOCXX_COMMON_API MersenneTwisterRandomNumber
{
public:
	MersenneTwisterRandomNumber();
	MersenneTwisterRandomNumber(UInt32 seed);
	MersenneTwisterRandomNumber(const MersenneTwisterRandomNumber& old);
	MersenneTwisterRandomNumber& operator=(const MersenneTwisterRandomNumber& old);

	UInt32 getNextNumber(UInt32 maxValue = MersenneTwisterRandomNumber::getMaxNumber());

	void setSeed(UInt32 seed);

	static UInt32 getMaxNumber();

private:
	static const size_t STATE_SIZE = 624;
	UInt32 m_state[STATE_SIZE];

	// How many random numbers are left before recalculation needs to take
	// place.
	size_t m_stateUsed;
};

} // end namespace BLOCXX_NAMESPACE

#endif
