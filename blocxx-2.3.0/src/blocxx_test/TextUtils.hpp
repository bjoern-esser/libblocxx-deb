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
 * @author LLeweLLyn Reese
 */

#if       !defined(BLOCXX_TEST_TEXT_UTILS_HPP_INCLUDE_GUARD_)
#define            BLOCXX_TEST_TEXT_UTILS_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/String.hpp"

namespace BLOCXX_NAMESPACE
{
	namespace TextUtils
	{
		/**
		 * Replaces multiple consecutive occurances of the separator with a
		 * single occurance of the separator; eg, foo//bar => foo/bar.
		 *
		 * @param pathName
		 *   A pathName which man contain redundant separators.
		 *
		 * @param separator
		 *
		 * @return
		 *   A copy of pathName with redundant separators removed.
		 *
		 */
		blocxx::String removeRedundantSeparators(blocxx::String const& pathName, char separator= '/');

		/**
		 * Conatenates all of the blocxx::Strings in array into a single blocxx::String,
		 *     seperating the elements of array with separator.
		 *
		 * @param array
		 *   An array of blocxx::Strings to be concatenated.
		 *
		 * @param separator
		 *   A string used to separate the elements of array as they are
		 *   copied into the return value.
		 *
		 * @return
		 *   A single blocxx::String which contains all of the elements of array, separated by separator.
		 *
		 */
		blocxx::String untokenize(blocxx::StringArray const& array, blocxx::String const& separator = "\n");

		/**
		 * Search for the first occurance of the given pattern in the source string,
		 * and return all text before it.
		 *
		 * If the source string is empty, the pattern was not found (or found at the
		 * beginning), an empty string will be returned.
		 */
		blocxx::String substringBefore(const blocxx::String& source, const blocxx::String& pattern);
		blocxx::String substringBefore(const blocxx::String& source, const char* pattern);
		blocxx::String substringBefore(const blocxx::String& source, char c);

		/**
		 * Search for first occurance of the the given pattern in the source string,
		 * and return all text after it.
		 *
		 * If the source string is empty, the pattern was not found (or found at the
		 * end), an empty string will be returned.
		 */
		blocxx::String substringAfter(const blocxx::String& source, const blocxx::String& pattern);
		blocxx::String substringAfter(const blocxx::String& source, const char* pattern);
		blocxx::String substringAfter(const blocxx::String& source, char c);

		/**
		 * Copy the contents of one stream to another.
		 */
		size_t copyStream(std::ostream& dest, std::istream& source);

		/**
		 * Searches 'source', replacing all occurances of 'search' with 'replace' .
		 */
		blocxx::String searchAndReplace(const blocxx::String& source, const blocxx::String& search, const blocxx::String& replace);

	} // end namespace TextUtils
} // end namespace BLOCXX_NAMESPACE
#endif /* !defined(BLOCXX_TEST_TEXT_UTILS_HPP_INCLUDE_GUARD_) */
