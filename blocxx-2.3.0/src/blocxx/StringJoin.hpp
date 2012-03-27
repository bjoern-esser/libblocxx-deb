/*******************************************************************************
* Copyright (C) 2010, Quest Software, Inc. All rights reserved.
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
 * @author Kevin Harris
 * @author Joel Smith
 */

#ifndef BLOCXX_STRING_JOINER_HPP_INCLUDE_GUARD_
#define BLOCXX_STRING_JOINER_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Join.hpp"
#include "blocxx/Reference.hpp"

namespace BLOCXX_NAMESPACE
{
	namespace JoinImpl
	{
		/**
		 * Append to a StringBuffer using append()
		 * A Binary Function class for use with @ref StringJoin
		 */
		struct StringBufferAppender
		{
			/**
			 * Function operator which calls append() on storage's
			 * StringBuffer, after creating it, if necessary.
			 * @param storage The StringBuffer to append the String to
			 * @param t The String to be appended to the StringBuffer
			 */
			void operator()(Reference<StringBuffer>& storage, const String& s);
		};
	} // end namespace JoinImpl

	
	/**
	 * An efficient implementation of Join<StringBuffer,String>,
	 * which joins items that can be converted to String, together
	 */
	class StringJoin : private Join<Reference<StringBuffer>, String, JoinImpl::StringBufferAppender>
	{
	public:
		typedef Join<Reference<StringBuffer>, String, JoinImpl::StringBufferAppender> ParentType;

		/**
		 * Create a StringBuffer object by iterating across a range, and joining
		 * the items together with a delimiter.
		 * 
		 * @param first The iterator for the beginning of the range.
		 * @param last The iterator for the end of the range.
		 * @param delim The new delimiter to put between each item in the range
		 * @tparam InputIter An input iterator type which supports pre-fix
		 *                   increment (++a), binary in-equality (a != b)
		 *                   and dereferencing (*a) operations.  Dereferenced
		 *                   items will be converted to String.
		 */
		template <typename InputIter>
		StringJoin(InputIter first, InputIter last, const String& delim)
			: ParentType(first, last, delim) { }
		/**
		 * @return the joined string
		 */
		operator String() const;
		/**
		 * @return the joined string
		 */
		String toString() const;

		friend BLOCXX_COMMON_API std::ostream& operator<<(std::ostream& os, const StringJoin& j);
	};
} // end namespace BLOCXX_NAMESPACE

#endif
