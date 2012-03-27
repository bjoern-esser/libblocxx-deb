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
*       nor Network Associates, 
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


#ifndef BLOCXX_CSTR_HPP_INCLUDE_GUARD_
#define BLOCXX_CSTR_HPP_INCLUDE_GUARD_

/**
 * @author Kevin S. Van Horn
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/Array.hpp"
#include <cstdlib>

namespace BLOCXX_NAMESPACE
{

namespace Cstr
{

template <typename S>
struct is_char_ptr
{
	enum { value = false };
};

template <>
struct is_char_ptr<char *>
{
	enum { value = true };
};

template <>
struct is_char_ptr<char const *>
{
	enum { value = true };
};

template <std::size_t N>
struct is_char_ptr<char[N]>
{
	enum { value = true };
};

template <std::size_t N>
struct is_char_ptr<char const [N]>
{
	enum { value = true };
};

template <typename S, bool is_char_pointer>
struct CstrStringAux
{
	static char const * c_str(S const & s)
	{
		return s.c_str();
	}
};

template <typename S>
struct CstrStringAux<S, true>
{
	static char const * c_str(S const & s)
	{
		return s;
	}
};

template <typename S>
struct CstrString : public CstrStringAux<S, is_char_ptr<S>::value>
{
};

/// @return <tt>char const *</tt> corresponding to @a s.
/// @pre @S is <tt>char *</tt> or <tt>char const *</tt>, or it is a class
/// that has a @c c_str member function return a <tt>char const *</tt>.
//
template <typename S>
inline char const * to_char_ptr(S const & s)
{
	return CstrString<S>::c_str(s);
}

/**
* Class for converting values of type @c S into <tt>char const * const *</tt>.
* @f BLOCXX_Cstr.hpp defines specializations for @a S being any one of
* - <tt>char const * const *</tt>
* - <tt>char const * *</tt>
* - <tt>char * const *</tt>
* - <tt>char * *</t>
* - <tt>char const * const [N]</tt>
* - <tt>char const * [N]</tt>
* - <tt>char * const [N]</tt>
* - <tt>char * [N]</t>
* - <tt>Array<S></tt>, for any type @a S for which @c to_char_ptr is defined.
*/
template <typename SA>
struct CstrArr
{
	/// Converted value
	char const * const * sarr;

private:
	/// Initializes @c sarr with the null-terminated <tt>char const * const *</tt>
	/// value corresponding to @a sa.
	/// Note that this generic specialization is unimplemented and private to cause 
	/// a compile-time error instead of an undefined symbol at link time.
	CstrArr(SA const & s);
};

template <bool b>
struct ctassert;

template <>
struct ctassert<true>
{
};

template <typename S>
struct CstrArr<S *> : private ctassert<is_char_ptr<S>::value>
{
	char const * const * sarr;
	
	CstrArr(S const * sarr0)
	: sarr(sarr0)
	{
	}
};

template <typename S>
struct CstrArr<S const *> : private ctassert<is_char_ptr<S>::value>
{
	char const * const * sarr;
	
	CstrArr(S const * sarr0)
	: sarr(sarr0)
	{
	}
};

template <std::size_t N, typename S>
struct CstrArr<S[N]> : private ctassert<is_char_ptr<S>::value>
{
	char const * const * sarr;
	
	CstrArr(S const sarr0[N])
	: sarr(sarr0)
	{
	}
};

template <std::size_t N, typename S>
struct CstrArr<S const [N]> : private ctassert<is_char_ptr<S>::value>
{
	char const * const * sarr;
	
	CstrArr(S const sarr0[N])
	: sarr(sarr0)
	{
	}
};

template <typename S>
struct CstrArr<Array<S> >
{
	Array<char const *> a;
	char const * const * sarr;

	CstrArr(Array<S> const & s)
	{
		typename Array<S>::const_iterator it, itend = s.end();
		for (it = s.begin(); it != itend; ++it)
		{
			a.push_back(to_char_ptr(*it));
		}
		a.push_back(0);
		sarr = &a[0];
	}
};

} // namespace Cstr

} // namespace BLOCXX_NAMESPACE

#endif
