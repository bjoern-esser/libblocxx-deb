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
 * @author Jon Carey
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/IOIFCStreamBuffer.hpp"
#include <cstring> // for strcmp

namespace BLOCXX_NAMESPACE
{

namespace
{
	IOIFCStreamBuffer::EDirectionFlag directionToEnum(const char* direction)
	{
		if (strcmp(direction, "in") == 0)
		{
			return IOIFCStreamBuffer::E_IN;
		}
		if (strcmp(direction, "out") == 0)
		{
			return IOIFCStreamBuffer::E_OUT;
		}
		return IOIFCStreamBuffer::E_IN_OUT;
	}
}

//////////////////////////////////////////////////////////////////////////////
IOIFCStreamBuffer::IOIFCStreamBuffer(IOIFC* dev, int bufSize,
	const char* direction)
	: BaseStreamBuffer(directionToEnum(direction), bufSize)
	, m_dev(dev)
	, m_tied_buf(0)
	, m_error_action(IOIFC::E_RETURN_ON_ERROR)
{
}
//////////////////////////////////////////////////////////////////////////////
IOIFCStreamBuffer::IOIFCStreamBuffer(IOIFC* dev, EDirectionFlag direction, int bufSize)
	: BaseStreamBuffer(direction, bufSize)
	, m_dev(dev)
	, m_tied_buf(0)
	, m_error_action(IOIFC::E_RETURN_ON_ERROR)
{
}
//////////////////////////////////////////////////////////////////////////////
IOIFCStreamBuffer::~IOIFCStreamBuffer()
{
	try
	{
		sync();
	}
	catch (...)
	{
	}
}
//////////////////////////////////////////////////////////////////////////////
std::streambuf * IOIFCStreamBuffer::tie(std::streambuf * tied_buf)
{
	std::streambuf * retval = m_tied_buf;
	m_tied_buf = tied_buf;
	return retval;
}
//////////////////////////////////////////////////////////////////////////////
void IOIFCStreamBuffer::setErrorAction(IOIFC::ErrorAction error_action)
{
	m_error_action = error_action;
}

//////////////////////////////////////////////////////////////////////////////
int
IOIFCStreamBuffer::buffer_from_device(char* c, int n)
{
	if (m_tied_buf)
	{
		m_tied_buf->pubsync();
	}
	return m_dev->read(c, n, m_error_action);
}
//////////////////////////////////////////////////////////////////////////////
int
IOIFCStreamBuffer::buffer_to_device(const char* c, int n)
{
	while (n > 0)
	{
		int cnt = m_dev->write(c, n, m_error_action);
		if (cnt == -1) // failure
		{
			return -1;
		}
		c += cnt;
		n -= cnt;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
void
IOIFCStreamBuffer::reset()
{ 
	initBuffers(); 
}

} // end namespace BLOCXX_NAMESPACE

