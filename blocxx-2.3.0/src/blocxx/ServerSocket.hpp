/*******************************************************************************
* Copyright (C) 2010, Quest Software, Inc. All rights reserved.
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
 * @author Jon Carey
 * @author Bart Whiteley
 * @author Dan Nuffer
 */

#ifndef BLOCXX_SERVER_SOCKET_HPP_INCLUDE_GUARD_
#define BLOCXX_SERVER_SOCKET_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/SelectableIFC.hpp"
#include "blocxx/ServerSocketImpl.hpp"
#include "blocxx/IntrusiveReference.hpp"
#include "blocxx/Types.hpp"
#include "blocxx/SocketFlags.hpp"

namespace BLOCXX_NAMESPACE
{

class BLOCXX_COMMON_API ServerSocket : public SelectableIFC
{
public:
	/** Allocate a new Inet Server Socket.
	 * @param isSSL is the Server Socket an SSL socket?
	 */
	ServerSocket(SSLServerCtxRef sslCtx);
	/** Allocate a new Inet Server Socket.
	 * @param isSSL is the Server Socket an SSL socket?
	 */
	ServerSocket(SocketFlags::ESSLFlag isSSL = SocketFlags::E_NOT_SSL);
	/**
	 * Copy ctor
	 */
	ServerSocket(const ServerSocket& arg);
	/**
	 * Accept a connection to the server socket
	 * @param timeoutSecs the timeout
	 * @return an Socket for the connection just accepted.
	 */
	Socket accept(const Timeout& timeout);
	/**
	 * Start listening on a port
	 *
	 * @param port The port to listen on
	 * @param queueSize the size of the listen queue
	 * @param allInterfaces do we listen on all interfaces?
	 * @throws SocketException
	 */
	void doListen(UInt16 port, int queueSize=10,
			const String& listenAddr = SocketAddress::ALL_LOCAL_ADDRESSES,
			SocketFlags::EReuseAddrFlag reuseAddr = SocketFlags::E_REUSE_ADDR);
#ifndef BLOCXX_WIN32
	/**
	 * Start listening on a Unix Domain Socket
	 *
	 * @param filename The filename for the unix domain socket
	 * @param queueSize the size of the listen queue
	 * @throws SocketException
	 */
	void doListen(const String& filename, int queueSize=10,
		bool reuseAddr=true);
#endif
	/**
	 * Close the listen socket
	 * @throws SocketException
	 */
	void close();

	/**
	 * Return the address of the local host
	 * @return an SocketAddress representing the local node
	 */
	SocketAddress getLocalAddress();
	/**
	 * Get the file descriptor of the listen socket
	 * @return a handle to the listen socket
	 */
	SocketHandle_t getfd() const;
	Select_t getSelectObj() const;
private:

#ifdef BLOCXX_WIN32
#pragma warning (push)
#pragma warning (disable: 4251)
#endif

	IntrusiveReference<ServerSocketImpl> m_impl;

#ifdef BLOCXX_WIN32
#pragma warning (pop)
#endif

};

} // end namespace BLOCXX_NAMESPACE

#endif
