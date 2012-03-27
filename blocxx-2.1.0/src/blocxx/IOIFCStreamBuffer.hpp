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

#ifndef BLOCXX_IOIFCSTREAMBUFFER_HPP_INCLUDE_GUARD_
#define BLOCXX_IOIFCSTREAMBUFFER_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/IOIFC.hpp"
#include "blocxx/BaseStreamBuffer.hpp"

namespace BLOCXX_NAMESPACE
{

/**
* DO NOT DERIVE FROM THIS CLASS, as the dtor has to call a virtual fct.
*/
class BLOCXX_COMMON_API IOIFCStreamBuffer : public BaseStreamBuffer
{
public:
	using BaseStreamBuffer::EDirectionFlag;

	IOIFCStreamBuffer(IOIFC* dev, int bufSize, const char* direction) BLOCXX_DEPRECATED;
	IOIFCStreamBuffer(IOIFC* dev, EDirectionFlag direction = E_IN_OUT, int bufSize = 64*1024/*BASE_BUF_SIZE*/);
	void setErrorAction(IOIFC::ErrorAction error_action);
	virtual ~IOIFCStreamBuffer();
	virtual void reset();
	std::streambuf * tie(std::streambuf * tied_buf);
private:
	// unimplemented
	IOIFCStreamBuffer(const IOIFCStreamBuffer& arg);
	IOIFCStreamBuffer& operator= (const IOIFCStreamBuffer& arg);
	virtual int buffer_to_device(const char* c, int n);
	virtual int buffer_from_device(char* c, int n);
	IOIFC* m_dev;
	std::streambuf * m_tied_buf;
	IOIFC::ErrorAction m_error_action;
};

} // end namespace BLOCXX_NAMESPACE

#endif
