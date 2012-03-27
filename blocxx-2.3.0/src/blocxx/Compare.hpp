/*******************************************************************************
* Copyright (C) 2011, Kevin Harris
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
 * @author Kevin Harris
 */


#ifndef BLOCXX_COMPARE_HPP_INCLUDE_GUARD
#define BLOCXX_COMPARE_HPP_INCLUDE_GUARD
#include "blocxx/BLOCXX_config.h"

namespace BLOCXX_NAMESPACE
{
	/*
	 * Return the maximum of two or more arguments of the same type.
	 */
	template <typename T>
	inline const T& MultiMax(const T& t1, const T& t2)
	{
		if( t1 < t2 )
		{
			return t2;
		}
		return t1;
	}

	/*
	 * Return the minimum of two or more arguments of the same type.
	 */
	template <typename T>
	inline const T& MultiMin(const T& t1, const T& t2)
	{
		if( t2 < t1 )
		{
			return t2;
		}
		return t1;
	}

#if defined(BLOCXX_USE_CXX_0X)
	template <typename T, typename ...Args>
	inline const T& MultiMax(const T& t1, const T& t2, const Args& ...args)
	{
		if( t1 < t2 )
		{
			return MultiMax(t2, args...);
		}
		return MultiMax(t1, args...);
	}

	template <typename T, typename ...Args>
	inline const T& MultiMin(const T& t1, const T& t2, const Args& ...args)
	{
		if( t1 < t2 )
		{
			return MultiMin(t1, args...);
		}
		return MultiMin(t2, args...);
	}
#else
	template <typename T>
	inline const T& MultiMax(const T& t1, const T& t2, const T& t3)
	{
		return MultiMax(MultiMax(t1, t2), t3);
	}

	template <typename T>
	inline const T& MultiMax(const T& t1, const T& t2, const T& t3, const T& t4)
	{
		return MultiMax(MultiMax(t1, t2), MultiMax(t3, t4));
	}

	template <typename T>
	inline const T& MultiMin(const T& t1, const T& t2, const T& t3)
	{
		return MultiMin(MultiMin(t1, t2), t3);
	}

	template <typename T>
	inline const T& MultiMin(const T& t1, const T& t2, const T& t3, const T& t4)
	{
		return MultiMin(MultiMin(t1, t2), MultiMin(t3, t4));
	}
#endif
} // end namespace BLOCXX_NAMESPACE
#endif
