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

#ifndef BLOCXX_SOCKET_HPP_INCLUDE_GUARD_
#define BLOCXX_SOCKET_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/SelectableIFC.hpp"
#include "blocxx/IOIFC.hpp"
#include "blocxx/SocketBaseImpl.hpp"
#include "blocxx/Types.hpp"
#include "blocxx/UnnamedPipe.hpp"
#include "blocxx/SocketFlags.hpp"
#include "blocxx/NetworkTypes.hpp"
#include "blocxx/SocketAddress.hpp"
#include "blocxx/IntrusiveReference.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/Exception.hpp"
#include "blocxx/LazyGlobal.hpp"

#ifndef BLOCXX_NO_SSL
#include <openssl/ssl.h>
#endif


namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_APIEXCEPTION(SocketTimeout, BLOCXX_COMMON_API)
class BLOCXX_COMMON_API Socket : public SelectableIFC, public IOIFC
{
public:
	/**
	 * Construct a non-SSL Socket
	 */
	Socket();
	/**
	 * Construct a Socket
	 * @param sslCtx The SSL client context. If null, the socket will not use SSL.
	 */
	Socket(const SSLClientCtxRef& sslCtx);
	/**
	 * Allocate a new  Socket
	 * @param isSSL is it an ssl socket?
	 */
	Socket(SocketFlags::ESSLFlag isSSL) BLOCXX_DEPRECATED;
	/**
	 * TODO: Make a replacement for this function so it can be deprecated.
	 * Allocate a new  Socket based on an existing handle.
	 * This is used by ServerSocket::accept()
	 * @param fd a handle to the existing socket
	 * @param isSSL is it an SSL socket?
	 */
	Socket(SocketHandle_t fd, SocketAddress::AddressType addrType,
		SocketFlags::ESSLFlag isSSL = SocketFlags::E_NOT_SSL);
	/**
	 * TODO: Make a replacement for this function so it can be deprecated.
	 * Allocate a new Socket and connect it to a peer machine
	 * @param addr the address of the peer machine
	 * @param addrType the address type of the socket
	 * @isSSL is it an SSL socket?
	 * @exception SocketException
	 */
	Socket(const SocketAddress& addr, SocketFlags::ESSLFlag isSSL = SocketFlags::E_NOT_SSL);
	/**
	 * Connect to a peer node
	 * @param addr The address of the machine to connect to.
	 * @exception SocketException
	 */
	void connect(const SocketAddress& addr)
		{ m_impl->connect(addr); }
	/**
	 * Disconnect the (presumably) open connection
	 */
	void disconnect() { m_impl->disconnect(); }

	static const int INFINITE_TIMEOUT BLOCXX_DEPRECATED = -1;
	/**
	 * Set the receive timeout on the socket
	 * @param seconds the number of seconds for the receive timeout
	 */
	BLOCXX_DEPRECATED void setReceiveTimeout(int seconds) { m_impl->setReceiveTimeout(Timeout::relative(seconds));}
	/**
	 * Set the receive timeout on the socket
	 * @param seconds the number of seconds for the receive timeout
	 */
	void setReceiveTimeout(const Timeout& timeout) { m_impl->setReceiveTimeout(timeout);}
	/**
	 * Get the receive timeout
	 * @return The number of seconds of the receive timeout
	 */
	Timeout getReceiveTimeout() const { return m_impl->getReceiveTimeout(); }
	/**
	 * Set the send timeout on the socket
	 * @param seconds the number of seconds for the send timeout
	 */
	BLOCXX_DEPRECATED void setSendTimeout(int seconds) { m_impl->setSendTimeout(Timeout::relative(seconds)); }
	/**
	 * Set the send timeout on the socket
	 * @param seconds the number of seconds for the send timeout
	 */
	void setSendTimeout(const Timeout& timeout) { m_impl->setSendTimeout(timeout); }
	/**
	 * Get the send timeout
	 * @return The number of seconds of the send timeout
	 */
	Timeout getSendTimeout() const { return m_impl->getSendTimeout(); }
	/**
	 * Set the connect timeout on the socket
	 * @param seconds the number of seconds for the connect timeout
	 */
	BLOCXX_DEPRECATED void setConnectTimeout(int seconds) { m_impl->setConnectTimeout(Timeout::relative(seconds)); }
	/**
	 * Set the connect timeout on the socket
	 * @param seconds the number of seconds for the connect timeout
	 */
	void setConnectTimeout(const Timeout& timeout) { m_impl->setConnectTimeout(timeout); }
	/**
	 * Get the connect timeout
	 * @return The number of seconds of the connect timeout
	 */
	Timeout getConnectTimeout() const { return m_impl->getConnectTimeout(); }
	/**
	 * Set all timeouts (send, receive, connect)
	 * @param seconds the number of seconds for the timeouts
	 */
	BLOCXX_DEPRECATED void setTimeouts(int seconds) { m_impl->setTimeouts(Timeout::relative(seconds)); }
	/**
	 * Set all timeouts (send, receive, connect)
	 * @param seconds the number of seconds for the timeouts
	 */
	void setTimeouts(const Timeout& timeout) { m_impl->setTimeouts(timeout); }
	/**
	 * Has the receive timeout expired?
	 * @return true if the receive timeout has expired.
	 */
	bool receiveTimeOutExpired() const { return m_impl->receiveTimeOutExpired(); }
	/**
	 * Write some data to the socket.
	 * @param dataOut a pointer to the memory to be written to the socket.
	 * @param dataOutLen the length of the data to be written
	 * @param errorAsException true if errors should throw exceptions.
	 * @return the number of bytes written.
	 * @throws SocketException
	 */
	int write(const void* dataOut, int dataOutLen, ErrorAction errorAsException = E_RETURN_ON_ERROR)
		{ return m_impl->write(dataOut, dataOutLen, errorAsException); }
	/**
	 * Read from the socket
	 * @param dataIn a pointer to a buffer where data should be copied to
	 * @param dataInLen the number of bytes to read.
	 * @param errorAsException true if errors should throw exceptions.
	 * @return the number of bytes read.
	 * @throws SocketException
	 */
	int read(void* dataIn, int dataInLen, ErrorAction errorAsException = E_RETURN_ON_ERROR)
		{ return m_impl->read(dataIn, dataInLen, errorAsException); }
	
	/**
	 * Wait for input on the socket for a specified length of time.
	 * @param timeOutSecs the number of seconds to wait.
	 * @return true if the timeout expired
	 * @throws SocketException
	 */
	BLOCXX_DEPRECATED bool waitForInput(int timeOutSecs)
		{ return m_impl->waitForInput(Timeout::relative(timeOutSecs)); }
	
	/**
	 * Wait for input on the socket for a specified length of time.
	 * @param timeOutSecs the number of seconds to wait.
	 * @return true if the timeout expired
	 * @throws SocketException
	 */
	bool waitForInput(const Timeout& timeout = Timeout::infinite)
		{ return m_impl->waitForInput(timeout); }
	
	/**
	 * Wait for output on the socket for a specified length of time.
	 * @param timeOutSecs the number of seconds to wait.
	 * @return true if the timeout expired
	 * @throws SocketException
	 */
	BLOCXX_DEPRECATED bool waitForOutput(int timeOutSecs)
		{ return m_impl->waitForOutput(Timeout::relative(timeOutSecs)); }
	
	/**
	 * Wait for output on the socket for a specified length of time.
	 * @param timeOutSecs the number of seconds to wait.
	 * @return true if the timeout expired
	 * @throws SocketException
	 */
	bool waitForOutput(const Timeout& timeout = Timeout::infinite)
		{ return m_impl->waitForOutput(timeout); }

	/**
	 * Get the local address associated with the socket connection
	 * @return an SocketAddress representing the local machine
	 */
	SocketAddress getLocalAddress() const { return m_impl->getLocalAddress(); }
	/**
	 * Get the peer address associated with the socket connection
	 * @return an SocketAddress representing the peer machine
	 */
	SocketAddress getPeerAddress() const { return m_impl->getPeerAddress(); }
	/**
	 * Get an istream to read from the socket
	 * @return a istream& which can be used for socket input
	 * @throws SocketException
	 */
	std::istream& getInputStream() // TODO: BLOCXX_DEPRECATED in 3.2.0
		{ return m_impl->getInputStream(); }
	/**
	 * Get an ostream to write to the socket
	 * @return an ostream& which can be used for socket output
	 * @throws SocketException
	 */
	std::ostream& getOutputStream() // TODO: BLOCXX_DEPRECATED in 3.2.0
		{ return m_impl->getOutputStream(); }
	/**
	 * Get an iostream to read/write from/to the socket
	 * @return an iostream& which can be used for socket input and output.
	 * @throws SocketException
	 */
	BLOCXX_DEPRECATED std::iostream& getIOStream() // in 3.2.0
		{ return m_impl->getIOStream(); }
	/**
	 * @return The Select_t associated with this sockect.
	 */
	Select_t getSelectObj() const { return m_impl->getSelectObj(); }
	/**
	 * Get the socket handle for the socket
	 * @return the socket handle
	 */
	SocketHandle_t getfd() { return m_impl->getfd(); }

	/**
	 * Get connected state
	 */
	bool isConnected() const { return m_impl->isConnected(); }

	static void createShutDownMechanism();
	/**
	 * Call this to shutdown all sockets.  This is usefull when a server
	 * is shutting down.  We want any outstanding connections to close
	 * immediately.
	 */
	static void shutdownAllSockets();
	/**
	 * Have the sockets been shutdown?
	 * @return true if the sockets have been shutdown
	 */
	static bool gotShutDown() BLOCXX_DEPRECATED; // deprecated in 3.1.0
	static void deleteShutDownMechanism();

#if defined(BLOCXX_WIN32)
	typedef HANDLE ShutDownMechanism_t;
#else
	typedef UnnamedPipeRef ShutDownMechanism_t;
#endif

	static ShutDownMechanism_t getShutDownMechanism()
	{
		return s_shutDownMechanism;
	}

#ifndef BLOCXX_NO_SSL
	/**
	 * get the SSL structure associated with the socket (if it
	 * is an SSL socket)
	 * @return a pointer to the SSL structure
	 */
	SSL* getSSL() const;

	/**
	 * did the peer certificate pass verification?
	 * @return true if peer cert verified.
	 */
	bool peerCertVerified() const;
#endif

private:
	/**
	 * Allocate a new  Socket based on an existing handle.
	 * This is used by ServerSocket::accept()
	 * @param fd a handle to the existing socket
	 * @param addrType the address type of the socket
	 * @param sslCtx a SSL server context reference
	 */
	Socket(SocketHandle_t fd, SocketAddress::AddressType addrType,
		const SSLServerCtxRef& sslCtx);

#ifdef BLOCXX_WIN32
#pragma warning (push)
#pragma warning (disable: 4251)
#endif

	SocketBaseImplRef m_impl;

#ifdef BLOCXX_WIN32
#pragma warning (pop)
#endif

	struct ShutDownMechanismFactory
	{
		static Socket::ShutDownMechanism_t* create(int initVal)
		{
			return new Socket::ShutDownMechanism_t();
		}
	};
	static LazyGlobal<Socket::ShutDownMechanism_t, int, ShutDownMechanismFactory> s_shutDownMechanism;

	friend class ServerSocketImpl;

};

} // end namespace BLOCXX_NAMESPACE

#endif
