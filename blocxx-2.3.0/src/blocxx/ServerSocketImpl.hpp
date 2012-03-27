/*******************************************************************************
* Copyright (C) 2005, 2010, Quest Software, Inc. All rights reserved.
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
 * @name		BLOCXX_ServerSocketImpl.hpp
 * @author	Jon M. Carey
 * @author Dan Nuffer
 *
 * @description
 *		Interface file for the BLOCXX_ServerSocketImpl class
 */
#ifndef BLOCXX_SERVER_SOCKET_IMPL_HPP_INCLUDE_GUARD_
#define BLOCXX_SERVER_SOCKET_IMPL_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/SelectableIFC.hpp"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/SocketAddress.hpp"
#include "blocxx/Types.hpp"
#include "blocxx/File.hpp"
#include "blocxx/SocketFlags.hpp"
#include "blocxx/SSLCtxMgr.hpp"

// The classes and functions defined in this file are not meant for general
// use, they are internal implementation details.  They may change at any time.

namespace BLOCXX_NAMESPACE
{

class BLOCXX_COMMON_API ServerSocketImpl : public SelectableIFC
{
public:
	ServerSocketImpl(SSLServerCtxRef sslCtx);
	ServerSocketImpl(SocketFlags::ESSLFlag isSSL);
	~ServerSocketImpl();
	String addrString();
	Socket accept(const Timeout& timeout);
	void close();
//	unsigned long getLocalAddressRaw() { return m_localAddress; }
//	unsigned short getLocalPortRaw() { return m_localPort; }
	SocketAddress getLocalAddress() { return m_localAddress; }
	SocketHandle_t getfd() const { return m_sockfd; }

	// listen for IPv4 protocol
	void doListenIPv4(UInt16 port, int queueSize, const String& listenAddr);
#ifdef BLOCXX_HAVE_IPV6
	// listen for IPv6 protocol
	void doListenIPv6(UInt16 port, int queueSize, const String& listenAddr);
#endif
	void doListen(UInt16 port, int queueSize=10,
		const String& listenAddr = SocketAddress::ALL_LOCAL_ADDRESSES,
		SocketFlags::EReuseAddrFlag reuseAddr = SocketFlags::E_REUSE_ADDR);

#ifndef BLOCXX_WIN32
	void doListenUDS(const String& filename, int queueSize=10,
		bool reuseAddr = true);
#endif

	Select_t getSelectObj() const;
private:
	void fillAddrParms();
	SocketHandle_t m_sockfd;
//	unsigned long m_localAddress;
//	unsigned short m_localPort;
	SocketAddress m_localAddress;
	bool m_isActive;
	ServerSocketImpl(const ServerSocketImpl& arg);
	ServerSocketImpl& operator=(const ServerSocketImpl& arg);
	SocketFlags::ESSLFlag m_isSSL;

#ifdef BLOCXX_WIN32
#pragma warning (push)
#pragma warning (disable: 4251)
#endif

	SSLServerCtxRef m_sslCtx;
#if defined(BLOCXX_WIN32)
#pragma warning (pop)
	HANDLE m_event;
	bool m_shuttingDown;
#else
	File m_udsFile;
#endif
};

} // end namespace BLOCXX_NAMESPACE

#endif
