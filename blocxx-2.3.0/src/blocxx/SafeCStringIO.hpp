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
 * @author Kevin S. Van Horn
 */

#ifndef BLOCXX_SAFE_CSTRING_IO_HPP_INCLUDE_GUARD_
#define BLOCXX_SAFE_CSTRING_IO_HPP_INCLUDE_GUARD_


#include "blocxx/BLOCXX_config.h"
#include <cstdio>

namespace BLOCXX_NAMESPACE
{
	class String;

namespace SafeCString
{
	/**
	 * PROMISE: Equivalent to std::fgets, except that I/O errors are reported by
	 * throwing an exception.
	 *
	 * THROWS: blocxx::IOException if there is a read error.
	 * 
	 * REQUIRES: dst nonnull, fp nonnull, dstsize > 0.
	 *
	 * RETURNS: NULL if no characters read because of reaching EOF; dst otherwise.
	 */
	char * fgets_trunc(char * dst, std::size_t dstsize, std::FILE * fp);

	/**
	 * A variant of fgets_trunc that infers the destination buffer size.
	 */
	template <std::size_t N> inline
	char * fgets_trunc(char (&dst)[N], std::FILE * fp)
	{
		return fgets_trunc(dst, N, fp);
	}

	/**
	 * PROMISE: Equivalent to std::fgets, except that I/O errors and truncated
	 * lines are reported by throwing an exception.
	 * 
	 * THROWS: blocxx::IOException if there is a read error.
	 *
	 * THROWS: OverflowException if the input line (including terminating '\0')
	 * must be truncated to fit into dstsize chars.
	 *
	 * REQUIRES: dst nonnull, fp nonnull, dstsize > 0.
	 *
	 * RETURNS: NULL if no characters read because of reaching EOF; dst otherwise.
	 */
	char * fgets_check(char * dst, std::size_t dstsize, std::FILE * fp);

	/**
	 * A variant of fgets_check that infers the destination buffer size.
	 */
	template <std::size_t N> inline
	char * fgets_check(char (&dst)[N], std::FILE * fp)
	{
		return fgets_check(dst, N, fp);
	}

	/**
	* RETURNS: A String created by reading from fp until either EOF or a
	* newline is encountered.  If a newline is found it is included in the
	* return String.
	*
	* THROWS: blocxx::IOException if there is a read error.
	*
	* THROWS: blocxx::StringConversionException if neither EOF nor newline
	* is found within max_chars characters.  (This is a protection against
	* unreasonably long inputs.)
	*/
	String fget_string(std::FILE * fp, std::size_t max_chars);

} // namespace SafeCString
} // namespace BLOCXX_NAMESPACE

#endif
