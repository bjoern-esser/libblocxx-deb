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

#ifndef BLOCXX_POSIX_UNNAMED_PIPE_HPP_INCLUDE_GUARD_
#define BLOCXX_POSIX_UNNAMED_PIPE_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/UnnamedPipe.hpp"
#include "blocxx/IntrusiveReference.hpp"

// The classes and functions defined in this file are not meant for general
// use, they are internal implementation details.  They may change at any time.

namespace BLOCXX_NAMESPACE
{

class BLOCXX_COMMON_API PosixUnnamedPipe : public UnnamedPipe
{
public:
	PosixUnnamedPipe(EOpen doOpen=E_OPEN);
	/// If @a fd_in == -1, then you cannot read from this object.
	/// If @a fd_out == -1, then you cannot write to this object
	//
	PosixUnnamedPipe(AutoDescriptor inputfd, AutoDescriptor outputfd);
	virtual ~PosixUnnamedPipe();
	virtual int write(const void* data, int dataLen, ErrorAction errorAsException = E_RETURN_ON_ERROR);
	virtual int read(void* buffer, int bufferLen, ErrorAction errorAsException = E_RETURN_ON_ERROR);
	Descriptor getInputHandle() const { return m_fds[0]; }
	Descriptor getOutputHandle() const { return m_fds[1]; }
	virtual void open();
	virtual int close();
	virtual bool isOpen() const;
	virtual int closeInputHandle();
	virtual int closeOutputHandle();
	virtual void setBlocking(EBlockingMode outputIsBlocking=E_BLOCKING);
	virtual void setWriteBlocking(EBlockingMode isBlocking=E_BLOCKING);
	virtual void setReadBlocking(EBlockingMode isBlocking=E_BLOCKING);
	virtual EBlockingMode getReadBlocking() const;
	virtual EBlockingMode getWriteBlocking() const;
	virtual Select_t getReadSelectObj() const;
	virtual Select_t getWriteSelectObj() const;
	virtual Descriptor getInputDescriptor() const;
	virtual Descriptor getOutputDescriptor() const;
	virtual void passDescriptor(Descriptor h, const UnnamedPipeRef& ackPipe = 0, const ProcessRef& targetProcess = 0);
	virtual AutoDescriptor receiveDescriptor(const UnnamedPipeRef& ackPipe);

private:
	// unimplemented
	PosixUnnamedPipe(const PosixUnnamedPipe& x);
	PosixUnnamedPipe& operator=(const PosixUnnamedPipe& x);


	Descriptor m_fds[2];
	EBlockingMode m_blocking[2];
};
typedef IntrusiveReference<PosixUnnamedPipe> PosixUnnamedPipeRef;

} // end namespace BLOCXX_NAMESPACE

#endif
