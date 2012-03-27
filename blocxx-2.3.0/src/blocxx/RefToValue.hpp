/*******************************************************************************
* Copyright (C) 2005, 2009, Quest Software, Inc. All rights reserved.
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

////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2006 Richard Sposato
// Copyright (c) 2006 Peter Kümmel
// Permission to use, copy, modify, distribute and sell this software for any
//     purpose is hereby granted without fee, provided that the above copyright
//     notice appear in all copies and that both that copyright notice and this
//     permission notice appear in supporting documentation.
// The authors make no representations about the
//     suitability of this software for any purpose. It is provided "as is"
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////
#ifndef BLOCXX_REF_TO_VALUE_HPP_INCLUDE_GUARD_
#define BLOCXX_REF_TO_VALUE_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
namespace BLOCXX_NAMESPACE
{

	////////////////////////////////////////////////////////////////////////////////
	///  \class RefToValue
	///
	///  Transports a reference as a value
	///  Serves to implement the Colvin/Gibbons trick for SmartPtr/ScopeGuard
	////////////////////////////////////////////////////////////////////////////////

	template <class T>
	class RefToValue
	{
	public:

		RefToValue(T& ref) : ref_(ref)
		{}

		RefToValue(const RefToValue& rhs) : ref_(rhs.ref_)
		{}

		operator T& () const
		{
			return ref_;
		}

	private:
		RefToValue() BLOCXX_FUNCTION_NOT_IMPLEMENTED;
		RefToValue& operator=(const RefToValue&) BLOCXX_FUNCTION_NOT_IMPLEMENTED;

		T& ref_;
	};


	////////////////////////////////////////////////////////////////////////////////
	///  RefToValue creator.
	////////////////////////////////////////////////////////////////////////////////

	template <class T>
	inline RefToValue<T> ByRef(T& t)
	{
		return RefToValue<T>(t);
	}

}



#endif

