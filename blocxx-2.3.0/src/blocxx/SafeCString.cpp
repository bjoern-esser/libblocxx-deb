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

#include "blocxx/BLOCXX_config.h"
#include "blocxx/SafeCString.hpp"
#include <cstring>
#include <new>

using namespace blocxx;
using namespace blocxx::SafeCString;

namespace
{
	inline char const * strend(char const * s, std::size_t n)
	{
		return static_cast<char const *>(std::memchr(s, '\0', n));
	}

	inline char * strend(char * s, std::size_t n)
	{
		return const_cast<char *>(strend(static_cast<char const *>(s), n));
	}

	char * strend_checked(char * s, std::size_t n)
	{
		char * retval = strend(s, n);
		if (retval)
		{
			return retval;
		}
		else
		{
			BLOCXX_THROW_ERR(
				OverflowException,
				"cstring catenation first operand unterminated",
				DEST_UNTERMINATED
			);
		}
	}

	// PROMISE: Copies first min(n, strlen(src) + 1) chars of src to dst.
	// RETURNS: dst + strlen(src) if strlen(src) < n, NULL otherwise.
	//
	inline char * safe_strcpy(char * dst, char const * src, std::size_t n)
	{
#ifdef BLOCXX_HAS_MEMCCPY
		char * rv = static_cast<char *>(std::memccpy(dst, src, '\0', n));
		return rv ? rv - 1 : 0;
#else
		char const * end = strend(src, n);
		if (end) // '\0' found
		{
			size_t len = end - src;
			std::memcpy(dst, src, len + 1);  // include terminating '\0'
			return dst + len;
		}
		else
		{
			std::memcpy(dst, src, n);
			return 0;
		}
#endif
	}
}

namespace BLOCXX_NAMESPACE
{
namespace SafeCString
{

BLOCXX_DEFINE_EXCEPTION(Overflow);

char * str_dup(char const * s)
{
	char * retval = new char[std::strlen(s) + 1];
	return std::strcpy(retval, s);
}

char * str_dup_nothrow(char const * s)
{
	char * retval = new (std::nothrow) char[std::strlen(s)];
	if (retval)
	{
		std::strcpy(retval, s);
	}
	return retval;
}

char * strcpy_trunc(char * dst, std::size_t dstsize, char const * src)
{
	std::size_t n = dstsize - 1;
	char * retval = safe_strcpy(dst, src, n);
	if (retval)
	{
		return retval;
	}
	else
	{
		dst[n] = '\0';
		return dst + n;
	}
}

char * strcpy_trunc(
	char * dst, std::size_t dstsize, char const * src, std::size_t srclen
)
{
	std::size_t n = (srclen < dstsize ? srclen : dstsize - 1);
	char * retval = safe_strcpy(dst, src, n);
	if (retval)
	{
		return retval;
	}
	else
	{
		dst[n] = '\0';
		return dst + n;
	}
}

char * strcpy_check(char * dst, std::size_t dstsize, char const * src)
{
	char * retval = safe_strcpy(dst, src, dstsize);
	if (retval)
	{
		return retval;
	}
	else
	{
		dst[dstsize - 1] = '\0';
		BLOCXX_THROW_ERR(
			OverflowException, "cstring copy overflow", RESULT_TRUNCATED);
	}
}

char * strcpy_check(
	char * dst, std::size_t dstsize, char const * src, std::size_t srclen
)
{
	if (srclen >= dstsize)
	{
		return strcpy_check(dst, dstsize, src);
	}
	else // srclen < dstsize
	{
		return strcpy_trunc(dst, srclen + 1, src);
	}
}

char * strcat_trunc(char * dst, std::size_t dstsize, char const * src)
{
	char * dstend = strend_checked(dst, dstsize);
	return strcpy_trunc(dstend, (dst + dstsize) - dstend, src);
}

char * strcat_trunc(
	char * dst, std::size_t dstsize, char const * src, std::size_t srclen
)
{
	char * dstend = strend_checked(dst, dstsize);
	return strcpy_trunc(dstend, (dst + dstsize) - dstend, src, srclen);
}

char * strcat_check(char * dst, std::size_t dstsize, char const * src)
{
	char * dstend = strend_checked(dst, dstsize);
	return strcpy_check(dstend, (dst + dstsize) - dstend, src);
}

char * strcat_check(
	char * dst, std::size_t dstsize, char const * src, std::size_t srclen
)
{
	char * dstend = strend_checked(dst, dstsize);
	return strcpy_check(dstend, (dst + dstsize) - dstend, src, srclen);
}

namespace Impl
{
	std::size_t nchars_check(int retval, std::size_t dstsize)
	{
		if (retval < 0 || retval >= static_cast<int>(dstsize))
		{
			BLOCXX_THROW_ERR(
				OverflowException, "sprintf overflow", RESULT_TRUNCATED);
		}
		return static_cast<std::size_t>(retval);
	}
}

} // namespace SafeCString
} // namespace BLOCXX_NAMESPACE
