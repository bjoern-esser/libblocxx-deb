/*******************************************************************************
* Copyright (C) 2005, Vintela, Inc. All rights reserved.
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
*       Vintela, Inc., 
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
#include "blocxx/SafeCStringIO.hpp"
#include "blocxx/SafeCString.hpp"
  /* to get OverflowException and RESULT_TRUNCATED */
#include "blocxx/String.hpp"
#include "blocxx/StringBuffer.hpp"
#include "blocxx/IOException.hpp"
#include <cstdio>

namespace BLOCXX_NAMESPACE
{
namespace SafeCString
{

char * fgets_trunc(char * dst, std::size_t dstsize, FILE * fp)
{
	char * res = std::fgets(dst, dstsize, fp);
	if (!res)
	{
		if (feof(fp))
		{
			return 0;
		}
		BLOCXX_THROW(blocxx::IOException, "read error");
	}
	return res;
}

char * fgets_check(char * dst, std::size_t dstsize, FILE * fp)
{
	std::size_t const end = dstsize - 1;
	char savechar = dst[end];
	dst[end] = ' '; // anything but '\0'
	char * res = std::fgets(dst, dstsize, fp);
	char endchar = dst[end];
	dst[end] = savechar;
	if (res)
	{
		if (endchar == '\0' && (end == 0 || dst[end - 1] != '\n'))
		{
			// No newline at end.  Either the input line was truncated, or
			// we read the last line of the file and it had no newline.
			// We test for EOF to distinguish the two cases.  Since the EOF
			// marker doesn't get set until we actually try to read past EOF,
			// we first peek one character ahead.
			std::ungetc(std::fgetc(fp), fp);
			if (!feof(fp))
			{
				BLOCXX_THROW_ERR(
					OverflowException, "fgets overflow", RESULT_TRUNCATED);
			}
		}
		return res;
	}
	else
	{
		if (feof(fp))
		{
			return 0;
		}
		BLOCXX_THROW(blocxx::IOException, "read error");
	}
}

String fget_string(FILE * fp, std::size_t const max_chars)
{
	// This could perhaps be made more efficient by reading directly into a
	// custom resizable character array.
	std::size_t const BUFSIZE = 8 * 1024;
	StringBuffer sb;
	char buf[BUFSIZE];
	while (!sb.endsWith('\n') && sb.length() <= max_chars &&
		fgets_trunc(buf, fp))
	{
		sb.append(buf);
	}
	if (sb.length() > max_chars)
	{
		BLOCXX_THROW(StringConversionException, "input line too long");
	}
	return sb.releaseString();
}

} // namespace SafeCString
} // namespace BLOCXX_NAMESPACE
