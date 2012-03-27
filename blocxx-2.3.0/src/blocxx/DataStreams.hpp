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

#ifndef BLOCXX_DATA_STREAMS_HPP_INCLUDE_GUARD_
#define BLOCXX_DATA_STREAMS_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"

#if defined(BLOCXX_HAVE_ISTREAM) && defined(BLOCXX_HAVE_OSTREAM)
#include <istream>
#include <ostream>
#else
#include <iostream>
#endif

#if defined(BLOCXX_HAVE_STREAMBUF)
#include <streambuf>
#elif defined(BLOCXX_HAVE_STREAMBUF_H)
#include <streambuf.h>
#endif

#include <vector>

// The classes and functions defined in this file are not meant for general
// use, they are internal implementation details.  They may change at any time.

namespace BLOCXX_NAMESPACE
{

//////////////////////////////////////////////////////////////////////////////
class BLOCXX_COMMON_API DataIStreamBuf : public std::streambuf
{
public:
	DataIStreamBuf(int dataLen, const unsigned char* data) :
		std::streambuf()
	{
		setg(const_cast<char*>(reinterpret_cast<const char*>(data)),
			const_cast<char*>(reinterpret_cast<const char*>(data)),
			const_cast<char*>(reinterpret_cast<const char*>(data+dataLen)));
	}
protected:
	virtual int underflow();

	virtual pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which);
	virtual pos_type seekpos(pos_type sp, std::ios_base::openmode which);
};
//////////////////////////////////////////////////////////////////////////////
class BLOCXX_COMMON_API DataIStreamBase
{
protected:
	DataIStreamBase(int dataLen, const unsigned char* data) : m_strbuf(dataLen, data) {}
	DataIStreamBuf m_strbuf;
};
//////////////////////////////////////////////////////////////////////////////
class BLOCXX_COMMON_API DataIStream : private DataIStreamBase, public std::istream
{
public:
	DataIStream(int dataLen, const unsigned char* data)
	: DataIStreamBase(dataLen, data)
	, std::basic_istream<char, std::char_traits<char> >(&m_strbuf)	{}
};
//////////////////////////////////////////////////////////////////////////////
class BLOCXX_COMMON_API DataOStreamBuf : public std::streambuf
{
public:
	DataOStreamBuf(size_t initialSize = 256);
	const unsigned char* getData() const { return &m_bfr[0]; }
	int length() const { return m_bfr.size(); }
	void clear() { m_bfr.clear(); }
protected:
	virtual int overflow(int c);
	virtual std::streamsize xsputn(const char* s, std::streamsize n);
private:

#ifdef BLOCXX_WIN32
#pragma warning (push)
#pragma warning (disable: 4251)
#endif

	std::vector<unsigned char> m_bfr;

#ifdef BLOCXX_WIN32
#pragma warning (pop)
#endif

};
//////////////////////////////////////////////////////////////////////////////
class BLOCXX_COMMON_API DataOStreamBase
{
protected:
	DataOStreamBase(size_t initialSize = 256)
	: m_buf(initialSize) {}

	DataOStreamBuf m_buf;
};
//////////////////////////////////////////////////////////////////////////////
class BLOCXX_COMMON_API DataOStream : private DataOStreamBase, public std::ostream
{
public:
	DataOStream(size_t initialSize = 256)
		: DataOStreamBase(initialSize)
		, std::basic_ostream<char, std::char_traits<char> >(&m_buf)
	{}
	const unsigned char* getData() const { return m_buf.getData(); }
	int length() const { return m_buf.length();  }
	void clearData() { m_buf.clear(); }
};

} // end namespace BLOCXX_NAMESPACE

#endif
