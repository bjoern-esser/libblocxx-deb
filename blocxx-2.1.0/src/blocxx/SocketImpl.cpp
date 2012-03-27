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
 * @name		BLOCXX_SocketImpl.cpp
 * @author	J. Bart Whiteley
 * @author Dan Nuffer
 *
 * @description
 *		Implementation file for the BLOCXX_SocketImpl class.
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/SocketImpl.hpp"
#include "blocxx/SignalScope.hpp"

namespace BLOCXX_NAMESPACE
{

//////////////////////////////////////////////////////////////////////////////
SocketImpl::SocketImpl() 
	: SocketBaseImpl()
{
}
//////////////////////////////////////////////////////////////////////////////
SocketImpl::SocketImpl(SocketHandle_t fd, SocketAddress::AddressType addrType)
	: SocketBaseImpl(fd, addrType)
{
}
//////////////////////////////////////////////////////////////////////////////
SocketImpl::SocketImpl(const SocketAddress& addr) 
	: SocketBaseImpl(addr)
{
}
//////////////////////////////////////////////////////////////////////////////
SocketImpl::~SocketImpl()
{
}
//////////////////////////////////////////////////////////////////////////////
Select_t
SocketImpl::getSelectObj() const
{
#if defined(BLOCXX_WIN32)
	Select_t st;
	st.event = m_event;
	st.sockfd = m_sockfd;
	st.isSocket = true;
	st.networkevents = FD_READ | FD_WRITE;
	st.doreset = true;
	return st;
#else
	return m_sockfd;
#endif
}
//////////////////////////////////////////////////////////////////////////////
int SocketImpl::readAux(void* dataIn, int dataInLen) 
{
#if defined(BLOCXX_WIN32)
	return ::recv(m_sockfd, static_cast<char*>(dataIn), dataInLen, 0);
#else
	return ::read(m_sockfd, dataIn, dataInLen);
#endif
}
//////////////////////////////////////////////////////////////////////////////
int SocketImpl::writeAux(const void* dataOut, int dataOutLen)
{
#if defined(BLOCXX_WIN32)
	return ::send(m_sockfd, static_cast<const char*>(dataOut), dataOutLen, 0);
#else
	// block SIGPIPE so we don't kill the process if the socket is closed.
	SignalScope ss(SIGPIPE, SIG_IGN);
	return ::write(m_sockfd, dataOut, dataOutLen);
#endif
}

//////////////////////////////////////////////////////////////////////////////

} // end namespace BLOCXX_NAMESPACE

