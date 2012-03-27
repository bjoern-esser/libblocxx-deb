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
 * @author Kevin Van Horn
 * @author Dan Nuffer
 */

#ifndef BLOCXX_SECURE_RAND_HPP_INCLUDE_GUARD_
#define BLOCXX_SECURE_RAND_HPP_INCLUDE_GUARD_


#include "blocxx/BLOCXX_config.h"

#ifdef BLOCXX_HAVE_OPENSSL
// If you don't have SSL, you don't have cryptographically secure random
// numbers.  Don't try to fall back to a weaker PRNG, as this violates the
// security principle of "fail safe".

#include "blocxx/Exception.hpp"
#include <cstdlib>
#include "blocxx/Types.hpp"

namespace BLOCXX_NAMESPACE
{
BLOCXX_DECLARE_EXCEPTION(SecureRand);

namespace Secure
{
	namespace Impl
	{
		// This template must remain undefined for any type that is not
		// an unsigned integer type.
		template <typename T> struct assert_unsigned_integer_type;

		template <> struct assert_unsigned_integer_type<unsigned char> { };
		template <> struct assert_unsigned_integer_type<unsigned short> { };
		template <> struct assert_unsigned_integer_type<unsigned int> { };
		template <> struct assert_unsigned_integer_type<unsigned long> { };
		template <> struct assert_unsigned_integer_type<unsigned long long> { };

		// This template must remain undefined for any type that is not
		// an integer type.
		template <typename T> struct assert_integer_type;

		template <> struct assert_integer_type<char> { };

		template <> struct assert_integer_type<signed char> { };
		template <> struct assert_integer_type<short> { };
		template <> struct assert_integer_type<int> { };
		template <> struct assert_integer_type<long> { };
		template <> struct assert_integer_type<long long> { };

		template <> struct assert_integer_type<unsigned char> { };
		template <> struct assert_integer_type<unsigned short> { };
		template <> struct assert_integer_type<unsigned int> { };
		template <> struct assert_integer_type<unsigned long> { };
		template <> struct assert_integer_type<unsigned long long> { };

		// This template must remain undefined for any type that is not
		// a floating-point type.
		template <typename T> struct assert_float_type;

		template <> struct assert_float_type<float> { };
		template <> struct assert_float_type<double> { };
		template <> struct assert_float_type<long double> { };

		template <typename UnsignedInt>
		UnsignedInt rand_uint_lt(UnsignedInt n);

		template <typename Integer>
		Integer rand_range(Integer min_value, Integer max_value);

		template <typename Real>
		Real rand_unit_interval();
	}

	/**
	* Initializes and seeds the cryptographic PRNG, if this has not been
	* done already.  Note that this function gets called automatically
	* the first time rand data are generated.
	*/
	BLOCXX_COMMON_API void rand_init();

	/**
	*/
	BLOCXX_COMMON_API void rand_save_state();

	/**
	* Generates @a n cryptographically secure pseudo-random bytes and stores
	* them in @a buf.
	*
	* @pre @c rand_init() has been called.
	*
	* @return @a buf
	*/
	BLOCXX_COMMON_API unsigned char * rand(unsigned char * buf, std::size_t n);

	/**
	* @return @a cryptographically secure random number of type
	* @a UnsignedInt, with all values equally probable.
	*
	* @pre @c rand_init() has been called.
	* @pre @a UnsignedInt is an unsigned integer type.
	*
	*/
	template <typename UnsignedInt>
	inline UnsignedInt rand_uint()
	{
		Impl::assert_unsigned_integer_type<UnsignedInt> dummy;
		UnsignedInt n;
		rand(reinterpret_cast<unsigned char *>(&n), sizeof(n));
		return n;
	}

	/**
	* @return @a cryptographically secure random number @a x of type
	* @a UnsignedInt, with 0 <= @a x < @a n and all allowed values equally
	* probable.
	*
	* @pre @c rand_init() has been called.
	* @pre @a UnsignedInt is an unsigned integer type.
	*
	*/
	template <typename UnsignedInt>
	inline UnsignedInt rand_uint_lt(UnsignedInt n)
	{
		Impl::assert_unsigned_integer_type<UnsignedInt> dummy;
		return Impl::rand_uint_lt(n);
	}

	/**
	* @return @a cryptographically secure random number @a x of type
	* @a Integer, with @a min_val <= @a x <= @a max_val and all allowed values
	* equally probable.
	*
	* @pre @c rand_init() has been called.
	* @pre @a Integer is an integer type (signed or unsigned).
	* @pre @a min_val <= @a max_val.
	*
	*/
	template <typename Integer>
	inline Integer rand_range(Integer min_val, Integer max_val)
	{
		Impl::assert_integer_type<Integer> dummy;
		return Impl::rand_range(min_val, max_val);
	}

	/**
	* @return @a cryptographically secure random number @a x of type
	* @a Real, chosen from a uniform distribution over the
	* interval 0 <= @a x < 1.
	*
	* @pre @c rand_init() has been called.
	* @pre @a Real is one of the floating-point numeric types.
	*
	*/
	template <typename Real>
	inline Real rand_unit_interval()
	{
		Impl::assert_float_type<Real> dummy;
		return Impl::rand_unit_interval<Real>();
	}

	/**
	* Forks and reseeds the PRNG so that a compromise of the child cannot
	* compromise the parent's PRNG state, and a compromise of the parent
	* cannot compromise the child's PRNG state.
	*
	* @return 0 for the child; the child's process ID for the parent; or -1
	* on failure (@c errno set accordingly).
	*
	* @pre The process has only one thread.
	*/
	pid_t fork_reseed();

} // namespace Secure
} // namespace BLOCXX_NAMESPACE

#endif

#endif
