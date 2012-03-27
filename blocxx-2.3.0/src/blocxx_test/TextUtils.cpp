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

#include "blocxx/BLOCXX_config.h"
#include "blocxx_test/TextUtils.hpp"
#include "blocxx_test/LogUtils.hpp"
#include "blocxx/Array.hpp"

namespace BLOCXX_NAMESPACE
{
	namespace TextUtils
	{
		namespace // anonymous
		{
			const char* const COMPONENT_NAME = "blocxx.test.textutils";
		}

		String removeRedundantSeparators(const String& pathName, char separator)
		{
			String result;
			bool haveSeparator = false;

			for (size_t currentChar = 0; currentChar < pathName.length(); ++currentChar)
			{
				if (haveSeparator && pathName[currentChar] == separator)
				{
					/* skip extra separator */
				}
				else
				{
					result += pathName[currentChar];
				}
				haveSeparator = (pathName[currentChar] == separator);
			}
			return result;
		}

		String untokenize(const StringArray& array, const String& seperator)
		{
			String ret;

			for( StringArray::const_iterator current = array.begin();
				  current != array.end();
				  ++current )
			{
				ret += *current;
				// Only put the separator in the string if there is another element.
				if ((current + 1) != array.end())
				{
					ret += seperator;
				}
			}
			return ret;
		}

		blocxx::String substringBefore(const blocxx::String& source, const blocxx::String& pattern)
		{
			return substringBefore(source, pattern.c_str());
		}

		blocxx::String substringBefore(const blocxx::String& source, const char* pattern)
		{
			size_t index = source.indexOf(pattern);
			if( index != blocxx::String::npos )
			{
				return source.substring(0, index);
			}
			return String();
		}

		blocxx::String substringBefore(const blocxx::String& source, char c)
		{
			size_t index = source.indexOf(c);
			if( index != blocxx::String::npos )
			{
				return source.substring(0, index);
			}
			return String();
		}

		blocxx::String substringAfter(const blocxx::String& source, const blocxx::String& pattern)
		{
			size_t index = source.indexOf(pattern);
			if( index != blocxx::String::npos )
			{
				return source.substring(index + pattern.length(), String::npos);
			}
			return String();
		}

		blocxx::String substringAfter(const blocxx::String& source, const char* pattern)
		{
			size_t index = source.indexOf(pattern);
			if( index != blocxx::String::npos )
			{
				return source.substring(index + ::strlen(pattern), String::npos);
			}
			return String();
		}

		blocxx::String substringAfter(const blocxx::String& source, char c)
		{
			size_t index = source.indexOf(c);
			if( index != blocxx::String::npos )
			{
				return source.substring(index + 1, String::npos);
			}
			return String();
		}

		size_t copyStream(std::ostream& dest, std::istream& source)
		{
			Logger logger(COMPONENT_NAME);
			const size_t buffer_size = 2048;
			char buffer[buffer_size];
			size_t total_written = 0;

			BLOCXX_LOG_DEBUG3(logger, "Copy from source to dest.");
			while( source.good() && dest.good() )
			{
				BLOCXX_LOG_DEBUG3(logger, Format("Attempting to read %1 bytes from source stream.", buffer_size));
				source.read(&buffer[0], buffer_size);
				size_t count = source.gcount();
				BLOCXX_LOG_DEBUG3(logger, Format("Actually read %1 bytes from source stream.", count));
				dest.write(&buffer[0], count);
				BLOCXX_LOG_DEBUG3(logger, Format("Attempted to write %1 bytes to dest stream.", count));
				total_written += count;
			}
			BLOCXX_LOG_DEBUG3(logger, Format("Copied a total of %1 bytes from source to dest.", total_written));
			return total_written;
		}

		String searchAndReplace(const String& source, const String& search, const String& replace)
		{
			Logger logger(COMPONENT_NAME);
			BLOCXX_LOG_DEBUG(logger, Format("Replacing '%1' with '%2' in '%3' .", search, replace, source));
			String dest;
			size_t lastEndOfSearch= 0;
			for(size_t foundSearch= 0
					 ; lastEndOfSearch < source.length() && (foundSearch = source.indexOf(search, lastEndOfSearch)) != String::npos
					 ; /*empty*/)
			{
				if (search.empty())
				{
					BLOCXX_LOG_DEBUG(logger, "search string empty, exiting.");
					return source;
				}
				dest += source.substring(lastEndOfSearch, foundSearch - lastEndOfSearch);
				dest += replace;
				lastEndOfSearch= foundSearch + search.length();
			}
			if( lastEndOfSearch < source.length() )
			{
				dest += source.substring(lastEndOfSearch);
			}
			BLOCXX_LOG_DEBUG(logger, Format("Transformed '%1' to '%2'", source, dest));
			return dest;
		}
	} // end namespace TextUtils
} // end namespace BLOCXX_NAMESPACE
