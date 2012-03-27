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
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/FileBuf.hpp"
#include "blocxx/IOException.hpp"

#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <fcntl.h>
#include <ios>

#ifdef BLOCXX_WIN32
#include <io.h>
#endif

namespace BLOCXX_NAMESPACE
{

FileBuf::FileBuf()
: BaseStreamBuffer(E_IN_OUT)
, m_file(0)
{
}

FileBuf::~FileBuf()
{
	close();
}

bool
FileBuf::isOpen() const
{
	return m_file != 0;
}

FileBuf*
FileBuf::open(FILE* fp)
{
	if (isOpen())
	{
		return 0;
	}

	m_file = fp;
	return this;
}

FileBuf*
FileBuf::open(AutoDescriptor fd)
{
	if (isOpen())
	{
		return 0;
	}

int d = -1;

#ifdef BLOCXX_WIN32
#pragma message(Reminder "TODO: implement it for Win!")
	//maybe it's enough just to set flags = O_RDWR
	//but it's unclear what to do with O_APPEND
	int flags = O_RDWR;

	d = _open_osfhandle(reinterpret_cast<long>(fd.get()), flags);
#else
	d = fd.get();
	int flags = ::fcntl(d, F_GETFL);
	if (flags == -1)
	{
		return 0;
	}
#endif
	
	m_file = ::fdopen(d, cppModeToCMode(posixModeToCppMode(flags)));
	if (m_file == 0)
	{
		return 0;
	}
	::setbuf(m_file, 0);
	fd.release();
	return this;
}

#if 0
FileBuf*
FileBuf::open(const File& f)
{
	if (isOpen())
	{
		return 0;
	}
	int flags = ::fcntl(f.???, F_GETFL);
	if (flags == -1)
	{
		return 0;
	}

	m_file = ::fdopen(???, cppModeToCMode(posixModeToCppMode(flags)));
	if (m_file == 0)
	{
		return 0;
	}
	::setbuf(m_file, 0);
	return this;
}
#endif

FileBuf*
FileBuf::open(const char* path, std::ios_base::openmode mode, mode_t permissions)
{
	if (isOpen())
	{
		return 0;
	}

	int d = ::open(path, cppModeToPOSIXMode(mode), permissions);

#ifdef BLOCXX_WIN32
	Descriptor h = reinterpret_cast<Descriptor>(_get_osfhandle(d));
	AutoDescriptor fd(h);
#else
	AutoDescriptor fd(d);
#endif

	if (fd.get() == BLOCXX_INVALID_HANDLE)
	{
		return 0;
	}

	m_file = ::fdopen(d, cppModeToCMode(mode));

	if (m_file == 0)
	{
		return 0;
	}

	::setbuf(m_file, 0);
	fd.release();

	return this;
}

FileBuf*
FileBuf::close()
{
	if (isOpen())
	{
		::fclose(m_file);
	}
	m_file = 0;
	return this;
}

int
FileBuf::buffer_to_device(const char* c, int n)
{
	size_t written = ::fwrite(c, 1, n, m_file);
	if (written == static_cast<size_t>(n))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int
FileBuf::buffer_from_device(char* c, int n)
{
	if (!isOpen())
	{
		return -1;
	}
	size_t numRead = ::fread(c, 1, n, m_file);
	if (numRead != static_cast<size_t>(n))
	{
		if (ferror(m_file))
		{
			BLOCXX_THROW_ERRNO_MSG(IOException, "FileBuf::buffer_from_device: ::fread()");
		}
		else if (feof(m_file) && numRead == 0)
		{
			return -1;
		}
		else
		{
			return numRead;
		}
	}
	else
	{
		return n;
	}
}

std::ios_base::openmode
FileBuf::posixModeToCppMode(int posixMode)
{
	std::ios_base::openmode mode = posixMode & O_APPEND ? std::ios::app : std::ios_base::openmode(0);

#ifdef BLOCXX_WIN32
	#define O_ACCMODE   3
#endif
	switch (posixMode & O_ACCMODE)
	{
		case O_RDONLY: mode |= (std::ios::in | std::ios::binary); break;
		case O_WRONLY: mode |= (std::ios::out | std::ios::binary); break;
		case O_RDWR:   mode |= (std::ios::in | std::ios::out | std::ios::binary); break;
	}
	return mode;
}

int
FileBuf::cppModeToPOSIXMode(std::ios_base::openmode cppMode)
{
	int mode = 0;
	if (cppMode & std::ios::app)
	{
		mode |= O_APPEND;
	}

	if (cppMode & std::ios::trunc)
	{
		mode |= O_TRUNC;
	}

	if (cppMode & (std::ios::in | std::ios::out))
	{
		mode |= O_RDWR;
	}
	else if (cppMode & std::ios::in)
	{
		mode |= O_RDONLY;
	}
	else if (cppMode & std::ios::out)
	{
		mode |= O_WRONLY;
	}
	return mode;
}

const char* FileBuf::cppModeToCMode(std::ios_base::openmode cppMode)
{
	// mask out irrelevant bits
	cppMode = cppMode & ~std::ios::ate;

	using std::ios;
	// can't use a switch here because the ios flags aren't native types.
	if( cppMode == ios::app ||
		cppMode == (ios::in|ios::app) ||
		cppMode == (ios::out|ios::app) )
	{
		return "a";
	}

	else if( cppMode == (ios::binary|ios::app) ||
		cppMode == (ios::in|ios::binary|ios::app) ||
		cppMode == (ios::out|ios::binary|ios::app) )
	{
		return "ab";
	}

	else if (cppMode == ios::in)
	{
		return "r";
	}

	else if (cppMode == (ios::in|ios::binary))
	{
		return "rb";
	}
			
	else if (cppMode == (ios::out|ios::in))
	{
		return "r+";
	}

	else if (cppMode == (ios::out|ios::in|ios::app))
	{
		return "a+";
	}

	else if (cppMode == (ios::out|ios::in|ios::binary))
	{
		return "r+b";
	}

	else if (cppMode == (ios::out|ios::in|ios::binary|ios::app))
	{
		return "a+b";
	}

	else if( (cppMode == ios::out) ||
		cppMode == (ios::trunc) ||
		cppMode == (ios::trunc|ios::app) ||
		cppMode == (ios::trunc|ios::out) ||
		cppMode == (ios::trunc|ios::out|ios::app) )
	{
		return "w";
	}

	else if( (cppMode == (ios::trunc|ios::binary)) ||
		(cppMode == (ios::trunc|ios::binary|ios::app)) ||
		(cppMode == (ios::out|ios::binary)) ||
		(cppMode == (ios::trunc|ios::out|ios::binary)) ||
		(cppMode == (ios::trunc|ios::out|ios::binary|ios::app) ))
	{
		return "wb";
	}

	else if( (cppMode == (ios::trunc|ios::out|ios::in)) ||
		(cppMode == (ios::trunc|ios::out|ios::in|ios::app)) )
	{
		return "w+";
	}

	else if( (cppMode == (ios::trunc|ios::out|ios::in|ios::binary)) ||
		(cppMode == (ios::trunc|ios::out|ios::in|ios::binary|ios::app)) )
	{
		return "w+b";
	}
	else
	{
		return "";	 // bad mode
	}

}


} // end namespace BLOCXX_NAMESPACE


