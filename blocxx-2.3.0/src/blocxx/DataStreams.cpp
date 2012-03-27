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
 * @author Jon Carey
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/DataStreams.hpp"

#include <cstring>
#include <cstdio>

namespace BLOCXX_NAMESPACE
{

//////////////////////////////////////////////////////////////////////////////
int
DataIStreamBuf::underflow()
{
	return (gptr() < egptr()) ? static_cast<unsigned char>(*gptr()) : EOF;	// need a static_cast so a -1 doesn't turn into an EOF
}

//////////////////////////////////////////////////////////////////////////////
std::streambuf::pos_type
DataIStreamBuf::seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which)
{
	pos_type ret = pos_type(off_type(-1));

	char* begin = eback();
	char* cur = gptr();
	char* end = egptr();

	off_type newOff = 0;

	if (way == std::ios_base::cur)
	{
		newOff = cur - begin;
	}
	else if (way == std::ios_base::end)
	{
		newOff = end - begin;
	}

	if (newOff + off >= 0 && end - begin >= newOff + off)
	{
		setg(begin, begin + newOff + off, end);
		ret = pos_type(newOff);
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////////
std::streambuf::pos_type
DataIStreamBuf::seekpos(pos_type sp, std::ios_base::openmode which)
{
	pos_type ret = pos_type(off_type(-1));

	char* begin = eback();
	char* end = egptr();

	if (sp <= end - begin)
	{
		setg(begin, begin + sp, end);
		ret = sp;
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////////
DataOStreamBuf::DataOStreamBuf(size_t initialSize)
	: std::streambuf()
{
	m_bfr.reserve(initialSize);
}
//////////////////////////////////////////////////////////////////////////////
int
DataOStreamBuf::overflow(int c)
{
	m_bfr.push_back(c);
	return 0;
}
//////////////////////////////////////////////////////////////////////////////
std::streamsize
DataOStreamBuf::xsputn(const char* s, std::streamsize n)
{
	m_bfr.insert(m_bfr.end(), s, s+n);
	return n;
}

} // end namespace BLOCXX_NAMESPACE

