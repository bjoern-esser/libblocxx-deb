/*******************************************************************************
* Copyright (C) 2001-2004, 2010, Quest Software, Inc. All rights reserved.
* Copyright (C) 2004 Novell, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
*  - Neither the name of Quest Software, Inc. nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Quest Software, Inc. OR THE CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/**
 * @author Jon Carey
 * @author Bart Whiteley
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/ServerSocket.hpp"
#include "blocxx/Socket.hpp"
#include "blocxx/Timeout.hpp"

namespace BLOCXX_NAMESPACE
{

//////////////////////////////////////////////////////////////////////////////
ServerSocket::ServerSocket(SSLServerCtxRef sslCtx)
	: SelectableIFC()
	, m_impl(new ServerSocketImpl(sslCtx))
{
}
//////////////////////////////////////////////////////////////////////////////
ServerSocket::ServerSocket(SocketFlags::ESSLFlag isSSL)
	: SelectableIFC()
	, m_impl(new ServerSocketImpl(isSSL))
{
}
//////////////////////////////////////////////////////////////////////////////
ServerSocket::ServerSocket(const ServerSocket& arg)
	: IntrusiveCountableBase()
	, SelectableIFC()
	, m_impl(arg.m_impl)
{
}
//////////////////////////////////////////////////////////////////////////////
Socket
ServerSocket::accept(const Timeout& timeout)
{
	return m_impl->accept(timeout);
}
//////////////////////////////////////////////////////////////////////////////
void
ServerSocket::doListen(UInt16 port, int queueSize,
		const String& listenAddr, SocketFlags::EReuseAddrFlag reuseAddr)
{
	m_impl->doListen(port, queueSize, listenAddr, reuseAddr);
}
//////////////////////////////////////////////////////////////////////////////
#ifndef BLOCXX_WIN32
void
ServerSocket::doListen(const String& filename, int queueSize, bool reuseAddr)
{
	m_impl->doListenUDS(filename, queueSize, reuseAddr);
}
#endif
//////////////////////////////////////////////////////////////////////////////
void
ServerSocket::close()
{
	m_impl->close();
}
//////////////////////////////////////////////////////////////////////////////
SocketAddress
ServerSocket::getLocalAddress()
{
	return m_impl->getLocalAddress();
}
//////////////////////////////////////////////////////////////////////////////
SocketHandle_t
ServerSocket::getfd() const
{
	return m_impl->getfd();
}
//////////////////////////////////////////////////////////////////////////////
Select_t
ServerSocket::getSelectObj() const
{
	return m_impl->getSelectObj();
}

} // end namespace BLOCXX_NAMESPACE

