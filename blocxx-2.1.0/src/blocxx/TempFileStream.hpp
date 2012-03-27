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
 * @author Bart Whiteley
 * @author Dan Nuffer
 */

#ifndef BLOCXX_TEMPFILESTREAM_HPP_INCLUDE_GUARD_
#define BLOCXX_TEMPFILESTREAM_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Types.hpp"
#include "blocxx/String.hpp"
#include "blocxx/AutoPtr.hpp"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/File.hpp"

#if defined(BLOCXX_HAVE_STREAMBUF)
#include <streambuf>
#elif defined(BLOCXX_HAVE_STREAMBUF_H)
#include <streambuf.h>
#endif

#if defined(BLOCXX_HAVE_ISTREAM) && defined(BLOCXX_HAVE_OSTREAM)
#include <istream>
#include <ostream>
#else
#include <iostream>
#endif

/**
 * TempFileBuffer is the "IntelliBuffer".
 * This buffer is for an iostream similar to a stringstream.
 * The difference is that there is a buffer of a user defined
 * size. Once the  buffer is full, it switches to write to a temp file.
 * Designed to optimize speed in the case of small buffers, and
 * memory in the case of large buffers.
 *
 */
namespace BLOCXX_NAMESPACE
{

class BLOCXX_COMMON_API TempFileBuffer : public std::streambuf
{
public:
	enum EKeepFileFlag
	{
		E_DONT_KEEP_FILE,
		E_KEEP_FILE
	};

	/**
	 * Create a new TempFileBuffer object
	 * @param bufSize The size of the buffer used by this stream.
	 * @param keepflg If E_KEEP_FILE is specified the temporary file used by
	 * 		this object will not be deleted on destruction. The caller is
	 * 		responsible for calling releaseFileAnReset to get the file name of
	 * 		the	underlying temp file. If releaseFileAndReset is never called
	 * 		this object will attempt to delete the temp file on destruction.
	 */
	TempFileBuffer(size_t bufSize, EKeepFileFlag keepflg=E_DONT_KEEP_FILE);
	/**
	 * Create a new TempFileBuffer object
	 * @param dir The directory that will contain the temp file used by this
	 * 		object.
	 * @param bufSize The size of the buffer used by this stream.
	 * @param keepflg If E_KEEP_FILE is specified the temporary file used by
	 * 		this object will not be deleted on destruction. The caller is
	 * 		responsible for calling releaseFileAndReset to get the file name
	 * 		of the underlying temp file. If releaseFileAndReset is never 
	 * 		called this object will attempt to delete the temp file on 
	 * 		destruction.
	 */
	TempFileBuffer(const String& dir, size_t bufSize, EKeepFileFlag keepflg=E_DONT_KEEP_FILE);
	/**
	 * DTOR
	 */
	~TempFileBuffer();
	/**
	 * @return The size of all the data within this object.
	 */
	std::streamsize getSize();
	/**
	 * Set the read/write position to the beginning of the data.
	 */
	void rewind();
	/**
	 * reset puts this stream object back into its initialized state.
	 * If a tempfile exists, it is deleted. It is not recomended to
	 * use this method if you requested the underlying file not be
	 * deleted. You should should call releaseFileAndReset under those
	 * conditions. This will give you the name of the underlying file
	 * that you can delete if you desire.
	 */
	void reset();
	/**
	 * releaseFileAndReset is like the reset method except it ensures
	 * all data has been flused to the underlying file and returned 
	 * name of the file if the caller requested that it not be 
	 * deleted.
	 * @return The name of the underlying temp file if the caller
	 * 		requested that it not be deleted on close.
	 */
	String releaseFileAndReset();
	/**
	 * @return true if the temp is being used. This could return false
	 * if none of the buffered data has been written to disk yet. If
	 * this method returns false, it doesn't necessarily mean that
	 * a temp file won't be used when the data is flushed.
	 */
	bool usingTempFile() const;
protected:
	// for input
	int underflow();
	// for output
	std::streamsize xsputn(const char* s, std::streamsize n);
	virtual int overflow(int c);
	//virtual int sync();
	void initBuffers();
	void initGetBuffer();
	void initPutBuffer();
	int buffer_to_device(const char* c, int n);
	int buffer_from_device(char* c, int n);
private:
	size_t m_bufSize;
	char* m_buffer;
	File m_tempFile;
	std::streamsize m_readPos;
	std::streamsize m_writePos;
	bool m_isEOF;
	String m_dir;
	EKeepFileFlag m_keepFlag;
	String m_filePath;

	int buffer_in();
	int buffer_out();
	// prohibit copying and assigning
	TempFileBuffer(const TempFileBuffer& arg);
	TempFileBuffer& operator=(const TempFileBuffer& arg);
};


/**
 * TempFileStream is an iostream that uses an underlying temp file
 * to hold its content to reduce memory requirements.
 * The amount of data kept in memory is specified by a user supplied
 * buffer size.
 */
class BLOCXX_COMMON_API TempFileStream : public std::iostream
{
public:
	/**
	 * Create a new TempFileStream object.
	 * @param bufSize The desired size of the in memory buffer. This buffer
	 * 		becomes full, the data is written to a temp file. The default
	 * 		bufSize is 4K if not specified.
	 * @param keepflg If E_KEEP_FILE is specified the temporary file used by
	 * 		this object will not be deleted on destruction. The caller is
	 * 		responsible for calling releaseFileAndReset to get the file name
	 * 		of the underlying temp file. If releaseFileAndReset is never 
	 * 		called this object will attempt to delete the temp file on 
	 * 		destruction. The default behaviour is to delete the temp file
	 * 		when it is closed.
	 */
	TempFileStream(size_t bufSize = 4096, TempFileBuffer::EKeepFileFlag keepflg=TempFileBuffer::E_DONT_KEEP_FILE);
	/**
	 * Create a new TempFileStream object.
	 * @param dir This specifies where the temp file will be located.
	 * @param bufSize The desired size of the in memory buffer. This buffer
	 * 		becomes full, the data is written to a temp file. The default
	 * 		bufSize is 4K if not specified.
	 * @param keepflg If E_KEEP_FILE is specified the temporary file used by
	 * 		this object will not be deleted on destruction. The caller is
	 * 		responsible for calling releaseFileAndReset to get the file name
	 * 		of the underlying temp file. If releaseFileAndReset is never 
	 * 		called this object will attempt to delete the temp file on 
	 * 		destruction. The default behaviour is to delete the temp file
	 * 		when it is closed.
	 */
	TempFileStream(const String& dir, size_t bufSize = 4096,
		TempFileBuffer::EKeepFileFlag keepflg=TempFileBuffer::E_DONT_KEEP_FILE);
	/**
	 * @return the size of the data currently in the underlying buffer (this
	 * 		includes the temp file data if it exists).
	 */
	std::streamsize getSize() { return m_buffer->getSize(); }
	/**
	 * Set the read/write position to the beginning of the data.
	 */
	void rewind();
	/**
	 * reset puts the underlying stream object back into its initialized state.
	 * see TempFileBuffer reset.
	 */
	void reset();
	/**
	 * releaseFileAndReset is like the reset method except it ensures
	 * all data has been flused to the underlying file and returned 
	 * name of the file if the caller requested that it not be 
	 * deleted.
	 * @return The name of the underlying temp file if the caller
	 * 		requested that it not be deleted on close.
	 */
	String releaseFileAndReset();
	/**
	 * @return true if the temp is being used. This could return false
	 * if none of the buffered data has been written to disk yet. If
	 * this method returns false, it doesn't necessarily mean that
	 * a temp file won't be used when the data is flushed.
	 */
	bool usingTempFile() const;
private:

#ifdef BLOCXX_WIN32
#pragma warning (push)
#pragma warning (disable: 4251)
#endif

	AutoPtr<TempFileBuffer> m_buffer;

#ifdef BLOCXX_WIN32
#pragma warning (pop)
#endif

	// disallow copying and assigning
	TempFileStream(const TempFileStream&);
	TempFileStream& operator=(const TempFileStream&);
};

} // end namespace BLOCXX_NAMESPACE

#endif
