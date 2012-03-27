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
 * @author Bart Whiteley
 * @author Dan Nuffer
 */

// gptr() && !pptr() == input mode
// !gptr() && pptr() == output mode
// pptr() && gptr() should never happen
// !gptr() && !pptr() should never happen
// pptr() should never be 0 unless we're gone to file.
#include "blocxx/BLOCXX_config.h"
#include "blocxx/TempFileStream.hpp"
#include "blocxx/FileSystem.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/IOException.hpp"
#include <cstring>
#include <cstdio>

namespace BLOCXX_NAMESPACE
{

using std::iostream;
//////////////////////////////////////////////////////////////////////////////
TempFileBuffer::TempFileBuffer(size_t bufSize, EKeepFileFlag keepflg)
	: m_bufSize(bufSize)
	, m_buffer(new char[m_bufSize])
	, m_tempFile()
	, m_readPos(0)
	, m_writePos(0)
	, m_isEOF(false)
	, m_dir()
	, m_keepFlag(keepflg)
	, m_filePath()
{
	setg(0,0,0); // start out in output mode.
	initPutBuffer();
}

//////////////////////////////////////////////////////////////////////////////
TempFileBuffer::TempFileBuffer(const String& dir, size_t bufSize, EKeepFileFlag keepflg)
	: m_bufSize(bufSize)
	, m_buffer(new char[m_bufSize])
	, m_tempFile()
	, m_readPos(0)
	, m_writePos(0)
	, m_isEOF(false)
	, m_dir(dir)
	, m_keepFlag(keepflg)
	, m_filePath()
{
	setg(0,0,0); // start out in output mode.
	initPutBuffer();
}

//////////////////////////////////////////////////////////////////////////////
void
TempFileBuffer::initBuffers()
{
	initPutBuffer();
	initGetBuffer();
}
//////////////////////////////////////////////////////////////////////////////
void
TempFileBuffer::initPutBuffer()
{
	setp(m_buffer, m_buffer + m_bufSize);
}
//////////////////////////////////////////////////////////////////////////////
void
TempFileBuffer::initGetBuffer()
{
	setg(m_buffer, m_buffer, m_buffer);
}
//////////////////////////////////////////////////////////////////////////////
TempFileBuffer::~TempFileBuffer()
{
	try
	{
		if (m_filePath.length())
		{
			String fname = releaseFileAndReset();
			if (fname.length())
			{
				// At this point we know we have been ask to keep the underlying
				// file but the user never called releaseFileAndReset, so we'll 
				// attempt to removed that here.
				FileSystem::removeFile(fname);
			}
		}
	}
	catch(...)
	{
		// Ignore?
	}

	delete [] m_buffer;
}
//////////////////////////////////////////////////////////////////////////////
int
TempFileBuffer::buffer_out()
{
	int cnt = pptr() - pbase();
	int retval = buffer_to_device(m_buffer, cnt);
	initPutBuffer();
	return retval;
}
//////////////////////////////////////////////////////////////////////////////
int
TempFileBuffer::overflow(int c)
{
	if (pptr()) // buffer is full
	{
		if (buffer_out() < 0)
		{
			return EOF;
		}
	}
	else  // switching from input to output
	{
		if (!m_tempFile)
		{
			initPutBuffer();
			pbump(m_writePos);
			m_readPos = gptr() - eback();
		}
		else
		{
			m_readPos = m_tempFile.tell() - (egptr() - gptr());
			m_tempFile.seek(m_writePos, SEEK_SET);
			initPutBuffer();
		}
		setg(0,0,0);
	}
	if (c != EOF)
	{
		return sputc(c);
	}
	else
	{
		return c;
	}
}
//////////////////////////////////////////////////////////////////////////////
std::streamsize
TempFileBuffer::xsputn(const char* s, std::streamsize n)
{
	if (n < epptr() - pptr())
	{
		memcpy(pptr(), s, n * sizeof(char));
		pbump(n);
		return n;
	}
	else
	{
		for (std::streamsize i = 0; i < n; i++)
		{
			if (sputc(s[i]) == EOF)
			{
				return i;
			}
		}
		return n;
	}
}
//////////////////////////////////////////////////////////////////////////////
int
TempFileBuffer::underflow()
{
	if (m_isEOF)
	{
		return EOF;
	}
	if (gptr()) // need to fill buffer
	{
		if (buffer_in() < 0)
		{
			return EOF;
		}
	}
	else // we're in output mode; switch to input mode
	{
		if (m_tempFile)
		{
			buffer_out();
			m_writePos = m_tempFile.tell();
			m_tempFile.seek(m_readPos, SEEK_SET);
			if (buffer_in() < 0)
			{
				return EOF;
			}
		}
		else
		{
			m_writePos = pptr() - pbase();
			setg(m_buffer, m_buffer + m_readPos, pptr());
		}
		setp(0,0);
	}
	return static_cast<unsigned char>(*gptr());
}
//////////////////////////////////////////////////////////////////////////////
int
TempFileBuffer::buffer_in()
{
	int retval = buffer_from_device(m_buffer, m_bufSize);
	if (retval <= 0)
	{
		setg(0,0,0);
		m_isEOF = true;
		return -1;
	}
	else
	{
		setg(m_buffer, m_buffer, m_buffer + retval);
		return retval;
	}
}
//////////////////////////////////////////////////////////////////////////////
int
TempFileBuffer::buffer_to_device(const char* c, int n)
{
	if (!m_tempFile)
	{
		if (m_keepFlag == E_KEEP_FILE)
		{
			m_tempFile = FileSystem::createTempFile(m_filePath, m_dir);
		}
		else
		{
			m_tempFile = FileSystem::createAutoDeleteTempFile(m_dir);
		}

		if (!m_tempFile)
		{
			BLOCXX_THROW(IOException, "Failed to create temp file");
		}
	}
	return static_cast<int>(m_tempFile.write(c, n));
}
//////////////////////////////////////////////////////////////////////////////
int
TempFileBuffer::buffer_from_device(char* c, int n)
{
	if (!m_tempFile)
	{
		return -1;
	}
	else
	{
		return static_cast<int>(m_tempFile.read(c, n));
	}
}
//////////////////////////////////////////////////////////////////////////////
std::streamsize
TempFileBuffer::getSize()
{
	if (gptr() && !m_tempFile)
	{
		return egptr() - eback();
	}
	std::streamsize rval = m_writePos;
	if (m_tempFile)
	{
		rval = m_tempFile.size();
	}
	if (pptr())
	{
		rval += pptr() - pbase();
	}
	return rval;
}
//////////////////////////////////////////////////////////////////////////////
void
TempFileBuffer::rewind()
{
	m_readPos = 0;
	if (m_tempFile)
	{
		if (pptr())
		{
			m_writePos += pptr() - pbase();
			buffer_out();
		}
		m_tempFile.seek(0, SEEK_SET);
		initGetBuffer();
	}
	else
	{
		if (pptr())
		{
			m_writePos = pptr() - pbase();
		}
		else if (gptr())
		{
			m_writePos = egptr() - eback();
		}
		setg(m_buffer, m_buffer, m_buffer + m_writePos);
	}
	setp(0,0);
	m_isEOF = false;
}
//////////////////////////////////////////////////////////////////////////////
void
TempFileBuffer::reset()
{
	if (m_tempFile)
	{
		m_tempFile.close();
	}

	m_writePos = m_readPos = 0;
	setg(0,0,0);
	initPutBuffer();
	m_isEOF = false;
}

//////////////////////////////////////////////////////////////////////////////
String
TempFileBuffer::releaseFileAndReset()
{
	buffer_out();	// Flush the buffer and cause the temp file to be written
	reset();		// Close file and reset
	String rv = m_filePath;		// Save to return
	m_filePath.erase();			// Clear filePath to indicate release called
	return rv;
}

//////////////////////////////////////////////////////////////////////////////
bool
TempFileBuffer::usingTempFile() const
{
	return bool(m_tempFile);
}
//////////////////////////////////////////////////////////////////////////////
TempFileStream::TempFileStream(size_t bufSize, TempFileBuffer::EKeepFileFlag keepflg)
	: std::basic_iostream<char, std::char_traits<char> >(new TempFileBuffer(bufSize, keepflg))
	, m_buffer(dynamic_cast<TempFileBuffer*>(rdbuf()))
{
}

//////////////////////////////////////////////////////////////////////////////
TempFileStream::TempFileStream(const String& dir, size_t bufSize, TempFileBuffer::EKeepFileFlag keepflg)
	: std::basic_iostream<char, std::char_traits<char> >(new TempFileBuffer(dir, bufSize, keepflg))
	, m_buffer(dynamic_cast<TempFileBuffer*>(rdbuf()))
{
}

//////////////////////////////////////////////////////////////////////////////
TempFileStream::~TempFileStream()
{
}

//////////////////////////////////////////////////////////////////////////////
void
TempFileStream::rewind()
{
	m_buffer->rewind();
	// clear eof bit
	clear(rdstate() & ~std::ios::eofbit);
}
//////////////////////////////////////////////////////////////////////////////
void
TempFileStream::reset()
{
	m_buffer->reset();
	clear();
}
//////////////////////////////////////////////////////////////////////////////
String
TempFileStream::releaseFileAndReset()
{
	String rval = m_buffer->releaseFileAndReset();
	clear();
	return rval;
}
//////////////////////////////////////////////////////////////////////////////
bool
TempFileStream::usingTempFile() const
{
	return m_buffer->usingTempFile();
}

} // end namespace BLOCXX_NAMESPACE

