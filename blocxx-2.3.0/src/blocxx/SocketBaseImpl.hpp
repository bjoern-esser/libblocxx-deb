/*******************************************************************************
* Copyright (C) 2009, Quest Software, Inc. All rights reserved.
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
 * @name		BLOCXX_SocketBaseImpl.hpp
 * @author	Jon M. Carey
 * @author Dan Nuffer
 *
 * @description
 *		Interface file for the BLOCXX_SocketBaseImpl class
 */
#ifndef BLOCXX_SOCKET_BASE_IMPL_HPP_INCLUDE_GUARD_
#define BLOCXX_SOCKET_BASE_IMPL_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/SelectableIFC.hpp"
#include "blocxx/IntrusiveReference.hpp"
#include "blocxx/String.hpp"
#include "blocxx/SocketAddress.hpp"
#include "blocxx/Types.hpp"
#include "blocxx/SocketStreamBuffer.hpp"
#include "blocxx/IOIFC.hpp"
#include "blocxx/Timeout.hpp"

#if defined(BLOCXX_HAVE_ISTREAM) && defined(BLOCXX_HAVE_OSTREAM)
#include <istream>
#include <ostream>
#else
#include <iostream>
#endif

// The classes and functions defined in this file are not meant for general
// use, they are internal implementation details.  They may change at any time.

namespace BLOCXX_NAMESPACE
{

class BLOCXX_COMMON_API SocketBaseImpl : public SelectableIFC, public IOIFC
{
public:
	SocketBaseImpl();
	SocketBaseImpl(SocketHandle_t fd, SocketAddress::AddressType addrType);
	SocketBaseImpl(const SocketAddress& addr);
	virtual ~SocketBaseImpl();
	virtual void connect(const SocketAddress& addr);
	virtual void disconnect();
	void setReceiveTimeout(const Timeout& timeout) { m_recvTimeout = timeout; }
	Timeout getReceiveTimeout() const { return m_recvTimeout; }
	void setSendTimeout(const Timeout& timeout) { m_sendTimeout = timeout; }
	Timeout getSendTimeout() const { return m_sendTimeout; }
	void setConnectTimeout(const Timeout& timeout) { m_connectTimeout = timeout; }
	Timeout getConnectTimeout() const { return m_connectTimeout; }
	void setTimeouts(const Timeout& timeout) { m_recvTimeout = m_sendTimeout = m_connectTimeout = timeout; }
	bool receiveTimeOutExpired() const { return m_recvTimeoutExprd; }
	int write(const void* dataOut, int dataOutLen,
			ErrorAction errorAsException = E_RETURN_ON_ERROR);
	int read(void* dataIn, int dataInLen,
			ErrorAction errorAsException = E_RETURN_ON_ERROR);
	virtual bool waitForInput(const Timeout& timeout);
	bool waitForOutput(const Timeout& timeout);
	std::istream& getInputStream();
	std::ostream& getOutputStream();
	std::iostream& getIOStream();
	SocketAddress getLocalAddress() const { return m_localAddress; }
	SocketAddress getPeerAddress() const { return m_peerAddress; }
	SocketHandle_t getfd() const { return m_sockfd; }
	Select_t getSelectObj() const;
	virtual bool isConnected() const;
	static void setDumpFiles(const String& in, const String& out);
protected:
	virtual int readAux(void* dataIn, int dataInLen) = 0;
	virtual int writeAux(const void* dataOut, int dataOutLen) = 0;

private:

	void fillInetAddrParms();
#if !defined(BLOCXX_WIN32)
	void fillUnixAddrParms();
#endif
	SocketBaseImpl(const SocketBaseImpl& arg);
	SocketBaseImpl& operator= (const SocketBaseImpl& arg);
#if defined(BLOCXX_WIN32)
	static int waitForEvent(HANDLE event, int secsToTimeout=-1);
#endif

	SocketHandle_t m_sockfd;
	SocketAddress m_localAddress;
	SocketAddress m_peerAddress;
#if defined(BLOCXX_WIN32)
	HANDLE m_event;
#endif

	bool m_recvTimeoutExprd;
	SocketStreamBuffer m_streamBuf;
	std::istream m_in;
	std::ostream m_out;
	std::iostream m_inout;
	Timeout m_recvTimeout;
	Timeout m_sendTimeout;
	Timeout m_connectTimeout;

	static String m_traceFileOut;
	static String m_traceFileIn;
};
BLOCXX_EXPORT_TEMPLATE(BLOCXX_COMMON_API, IntrusiveReference, SocketBaseImpl);

} // end namespace BLOCXX_NAMESPACE

#endif
