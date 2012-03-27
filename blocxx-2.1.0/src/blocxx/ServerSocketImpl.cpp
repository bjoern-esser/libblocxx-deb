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

#include "blocxx/BLOCXX_config.h"
#include "blocxx/ServerSocketImpl.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/ByteSwap.hpp"
#include "blocxx/FileSystem.hpp"
#include "blocxx/File.hpp"
#include "blocxx/Thread.hpp"
#include "blocxx/SocketUtils.hpp"
#include "blocxx/System.hpp"
#include "blocxx/TimeoutTimer.hpp"

extern "C"
{
#if !defined(BLOCXX_WIN32)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#define INVALID_SOCKET -1
#endif
}

#include <cerrno>

namespace BLOCXX_NAMESPACE
{
//////////////////////////////////////////////////////////////////////////////
ServerSocketImpl::ServerSocketImpl(SSLServerCtxRef sslCtx)
	: m_sockfd(INVALID_SOCKET)
	, m_localAddress(SocketAddress::allocEmptyAddress(SocketAddress::INET))
	, m_isActive(false)
	, m_sslCtx(sslCtx)
#if defined(BLOCXX_WIN32)
	, m_event(NULL)
	, m_shuttingDown(false)
{
	m_event = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	BLOCXX_ASSERT(m_event != NULL);
}
#else
	, m_udsFile()
{
}
#endif

//////////////////////////////////////////////////////////////////////////////
ServerSocketImpl::ServerSocketImpl(SocketFlags::ESSLFlag isSSL)
	: m_sockfd(INVALID_SOCKET)
	, m_localAddress(SocketAddress::allocEmptyAddress(SocketAddress::INET))
	, m_isActive(false)
	, m_isSSL(isSSL)
#if defined(BLOCXX_WIN32)
	, m_event(NULL)
	, m_shuttingDown(false)
{
	m_event = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	BLOCXX_ASSERT(m_event != NULL);
}
#else
	, m_udsFile()
{
}
#endif

//////////////////////////////////////////////////////////////////////////////
ServerSocketImpl::~ServerSocketImpl()
{
	try
	{
		close();
	}
	catch (...)
	{
		// don't let exceptions escape
	}
#if defined(BLOCXX_WIN32)
	::CloseHandle(m_event);
#endif
}

//////////////////////////////////////////////////////////////////////////////
Select_t
ServerSocketImpl::getSelectObj() const
{
#if defined(BLOCXX_WIN32)
	Select_t st;
	st.event = m_event;
	st.sockfd = m_sockfd;
	st.isSocket = true;
	st.networkevents = FD_ACCEPT;
	return st;
#else
	return m_sockfd;
#endif
}

//////////////////////////////////////////////////////////////////////////////
void
ServerSocketImpl::doListen(UInt16 port, SocketFlags::ESSLFlag isSSL,
	int queueSize, const String& listenAddr,
	SocketFlags::EReuseAddrFlag reuseAddr)
{
	m_isSSL = isSSL;
	doListen(port, queueSize,listenAddr, reuseAddr);
}

#if defined(BLOCXX_WIN32)
//////////////////////////////////////////////////////////////////////////////
void
ServerSocketImpl::doListen(UInt16 port,
	int queueSize, const String& listenAddr,
	SocketFlags::EReuseAddrFlag reuseAddr)
{
	SocketHandle_t sock_ipv6 = INVALID_SOCKET;
	m_localAddress = SocketAddress::allocEmptyAddress(SocketAddress::INET);
	close();

#ifdef BLOCXX_HAVE_IPV6
	// This will try to use IPv6 (AF_INET6).  If it is not supported, it will
	// fall back to the normal IPv4 (AF_INET).
	sock_ipv6 = ::socket(AF_INET6, SOCK_STREAM, 0);
	if (sock_ipv6 == INVALID_SOCKET)
	{
		switch(errno)
		{
		case EPROTONOSUPPORT:
		case EAFNOSUPPORT:
		case EPFNOSUPPORT:
		case EINVAL:
			m_sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
			break;
		default:
			break;
		}
	}
	else
	{
	    m_sockfd = sock_ipv6;
	}
#else
	m_sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
#endif
	if (m_sockfd  == INVALID_SOCKET)
	{
		BLOCXX_THROW(SocketException, Format("ServerSocketImpl: %1",
			System::lastErrorMsg(true)).c_str());
	}
	// Set listen socket to nonblocking
	unsigned long cmdArg = 1;
	if (::ioctlsocket(m_sockfd, FIONBIO, &cmdArg) == SOCKET_ERROR)
	{
		BLOCXX_THROW(SocketException, Format("ServerSocketImpl: %1",
			System::lastErrorMsg(true)).c_str());
	}

	if (reuseAddr)
	{
		DWORD reuse = 1;
		::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR,
			(const char*)&reuse, sizeof(reuse));
	}

	if ( sock_ipv6 == INVALID_SOCKET )
	{
		// IPv4
		ServerSocketImpl::doListenIPv4( port, queueSize, listenAddr);
	}
#ifdef BLOCXX_HAVE_IPV6
	else
	{
		// IPv6
		ServerSocketImpl::doListenIPv6( port, queueSize, listenAddr, reuseAddr);
	}
#endif
}
///////////////////////////////////////////////////////////////////////////////////////
void
ServerSocketImpl::doListenIPv4(UInt16 port, int queueSize, const String& listenAddr )
{
	//	use IPv4 protocol
	InetSocketAddress_t 	inetAddr;
	memset(&inetAddr, 0, sizeof(inetAddr));
	reinterpret_cast<sockaddr_in*>(&inetAddr)->sin_family  =  AF_INET;
	reinterpret_cast<sockaddr_in*>(&inetAddr)->sin_port    =  hton16(port);
	if (listenAddr == SocketAddress::ALL_LOCAL_ADDRESSES)
	{
		reinterpret_cast<sockaddr_in*>(&inetAddr)->sin_addr.s_addr = hton32(INADDR_ANY);
	}
	else
	{
		SocketAddress addr = SocketAddress::getByName(listenAddr);
		reinterpret_cast<sockaddr_in*>(&inetAddr)->sin_addr.s_addr = 
		    reinterpret_cast<const sockaddr_in*>(addr.getInetAddress())->sin_addr.s_addr;
	}
	if (::bind(m_sockfd, reinterpret_cast<sockaddr*>(&inetAddr), sizeof(inetAddr)) == -1)
	{
		close();
		BLOCXX_THROW(SocketException, Format("ServerSocketImpl: %1",
			System::lastErrorMsg(true)).c_str());
	}
	if (::listen(m_sockfd, queueSize) == -1)
	{
		close();
		BLOCXX_THROW(SocketException, Format("ServerSocketImpl: %1",
			System::lastErrorMsg(true)).c_str());
	}
	fillAddrParms();
	m_isActive = true;
}
///////////////////////////////////////////////////////////////////////////////////////
#ifdef BLOCXX_HAVE_IPV6
void
ServerSocketImpl::doListenIPv6(UInt16 port, int queueSize, const String& listenAddr)
{
	//	use IPv6 protocol
	InetSocketAddress_t 	inetAddr;
	memset(&inetAddr, 0, sizeof(inetAddr));
	reinterpret_cast<sockaddr_in6*>(&inetAddr)->sin6_family =  AF_INET6;
	reinterpret_cast<sockaddr_in6*>(&inetAddr)->sin6_port   =  hton16(port);
	if (listenAddr == SocketAddress::ALL_LOCAL_ADDRESSES)
	{
		// copy IPv6 address any
		memcpy(reinterpret_cast<sockaddr_in6*>(&inetAddr)->sin6_addr.s6_addr, &in6addr_any, sizeof(struct in6_addr));
	}
	else
	{
		// create network address structure for IPv6 protocol
		if(!inet_pton(AF_INET6, listenAddr.c_str(), (void*)&reinterpret_cast<sockaddr_in6*>(&inetAddr)->sin6_addr))
		{
			addrinfo *addrinfos;
			addrinfo hints;
			memset(&hints, 0, sizeof(hints));
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_family   = AF_INET6;
			if(getaddrinfo(listenAddr.c_str(), NULL, &hints, &addrinfos))
			{
				close();
				BLOCXX_THROW(SocketException, Format("ServerSocketImpl:: doListen(): getaddrinfo() %1",
					System::lastErrorMsg(true)).c_str());
			}
			memcpy(reinterpret_cast<sockaddr_in6*>(&inetAddr), addrinfos->ai_addr, addrinfos->ai_addrlen);
			freeaddrinfo(addrinfos);
		}
	}
	if (::bind(m_sockfd, reinterpret_cast<sockaddr*>(&inetAddr), sizeof(inetAddr)) == -1)
	{
		close();
		BLOCXX_THROW(SocketException, Format("ServerSocketImpl: %1",
			System::lastErrorMsg(true)).c_str());
	}
	if (::listen(m_sockfd, queueSize) == -1)
	{
		close();
		BLOCXX_THROW(SocketException, Format("ServerSocketImpl: %1",
			System::lastErrorMsg(true)).c_str());
	}
	fillAddrParms();
	m_isActive = true;
}
#endif // BLOCXX_HAVE_IPV6

namespace
{

int
waitForAcceptIO(SocketHandle_t fd, HANDLE eventArg, const Timeout& timeOutSecs,
	long networkEvents)
{
	TimeoutTimer timer(timeOutSecs);

	if (networkEvents != -1L)
	{
		if(::WSAEventSelect(fd, eventArg, networkEvents) != 0)
		{
			BLOCXX_THROW(SocketException,
				Format("WSAEventSelect failed in waitForAcceptIO: %1",
				System::lastErrorMsg(true)).c_str());
		}
	}

	int cc;
	switch(::WaitForSingleObject(eventArg, timer.asDWORDMs()))
	{
		case WAIT_OBJECT_0:
			::ResetEvent(eventArg);
			cc = 0;
			break;
		case WAIT_TIMEOUT:
			cc = ETIMEDOUT;
			break;
		default:
			cc = -1;
			break;
	}

	return cc;
}

}

//////////////////////////////////////////////////////////////////////////////
Socket
ServerSocketImpl::accept(const Timeout& timeoutSecs)
{
	BLOCXX_ASSERT(m_localAddress.getType() == SocketAddress::INET);

	if (!m_isActive)
	{
		BLOCXX_THROW(SocketException, "ServerSocketImpl::accept: NONE");
	}

	// Register interest in FD_ACCEPT events
	if(::WSAEventSelect(m_sockfd, m_event, FD_ACCEPT) != 0)
	{
		BLOCXX_THROW(SocketException,
			Format("WSAEventSelect failed in accept: %1",
			System::lastErrorMsg(true)).c_str());
	}

	SOCKET clntfd;
	socklen_t clntlen;
	InetSocketAddress_t clntInetAddr;
	HANDLE events[2];
	int cc;

	while (true)
	{
		clntlen = sizeof(clntInetAddr);
		clntfd = ::accept(m_sockfd,
			reinterpret_cast<struct sockaddr*>(&clntInetAddr), &clntlen);
		if (clntfd != INVALID_SOCKET)
		{
			// Got immediate connection
			break;
		}

		if (::WSAGetLastError() != WSAEWOULDBLOCK)
		{
			BLOCXX_THROW(SocketException, Format("ServerSocketImpl: %1",
				System::lastErrorMsg(true)).c_str());
		}

		//cc = SocketUtils::waitForIO(m_sockfd, m_event, INFINITE, FD_ACCEPT);
		cc = waitForAcceptIO(m_sockfd, m_event, timeoutSecs, FD_ACCEPT);
		if(m_shuttingDown)
		{
			cc = -2;
		}

		switch (cc)
		{
			case -1:
				BLOCXX_THROW(SocketException, "Error while waiting for network events");
			case -2:
				BLOCXX_THROW(SocketException, "Shutdown event was signaled");
			case ETIMEDOUT:
				BLOCXX_THROW(SocketTimeoutException,"Timed out waiting for a connection");
		}
	}

	// Unregister for any events.  necessary to put us back in blocking mode.
	if(::WSAEventSelect(clntfd, NULL, 0) != 0)
	{
		BLOCXX_THROW(SocketException,
			Format("WSAEventSelect failed in accept: %1",
			System::lastErrorMsg(true)).c_str());
	}

	// set socket back to blocking; otherwise it'll inherit non-blocking from the listening socket
	unsigned long cmdArg = 0;
	if (::ioctlsocket(clntfd, FIONBIO, &cmdArg) == SOCKET_ERROR)
	{
		BLOCXX_THROW(SocketException, Format("ServerSocketImpl: %1",
			System::lastErrorMsg(true)).c_str());
	}

	if (!m_sslCtx && m_isSSL == SocketFlags::E_SSL)
	{
		return Socket(clntfd, m_localAddress.getType(), m_isSSL);
	}

	return Socket(clntfd, m_localAddress.getType(), m_sslCtx);
}
#else
//////////////////////////////////////////////////////////////////////////////
void
ServerSocketImpl::doListen(UInt16 port,
	int queueSize, const String& listenAddr,
	SocketFlags::EReuseAddrFlag reuseAddr)
{
	SocketHandle_t sock_ipv6 = INVALID_SOCKET;
	m_localAddress = SocketAddress::allocEmptyAddress(SocketAddress::INET);
	close();

#ifdef BLOCXX_HAVE_IPV6
	// This will try to use IPv6 (AF_INET6).  If it is not supported, it will
	// fall back to the normal IPv4 (AF_INET).
	sock_ipv6 = ::socket(AF_INET6, SOCK_STREAM, 0);
	if (sock_ipv6 == INVALID_SOCKET)
	{
		switch(errno)
		{
		case EPROTONOSUPPORT:
		case EAFNOSUPPORT:
		case EPFNOSUPPORT:
		case EINVAL:
			// open socket for IPv4 protocol
			m_sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
			break;
		default:
			break;
		}
	}
	else
	{
		m_sockfd = sock_ipv6;
#ifdef IPV6_V6ONLY
		int ipv6_proto = IPPROTO_IPV6;
#ifdef SOL_IP
		ipv6_proto = SOL_IP;
#endif
		int ipv6_only=0;
		::setsockopt(m_sockfd, ipv6_proto, IPV6_V6ONLY, &ipv6_only, sizeof(ipv6_only));
#endif
	}
#else
	m_sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
#endif
	if (m_sockfd  == INVALID_SOCKET)
	{
		BLOCXX_THROW_ERRNO_MSG(SocketException, "ServerSocketImpl::doListen(): socket()");
	}
	// set the close on exec flag so child process can't keep the socket.
	if (::fcntl(m_sockfd, F_SETFD, FD_CLOEXEC) == -1)
	{
		close();
		BLOCXX_THROW_ERRNO_MSG(SocketException, "ServerSocketImpl::doListen(): fcntl() failed to set "
			"close-on-exec flag on listen socket");
	}
	// set listen socket to nonblocking; see Unix Network Programming,
	// pages 422-424.
	int fdflags = ::fcntl(m_sockfd, F_GETFL, 0);
	::fcntl(m_sockfd, F_SETFL, fdflags | O_NONBLOCK);
	// is this safe? Should be, but some OS kernels have problems with it.
	// It's OK on current linux versions.  Definitely not on
	// OLD (kernel < 1.3.60) ones.  Who knows about on other OS's like UnixWare or
	// OpenServer?
	// See http://monkey.org/openbsd/archive/misc/9601/msg00031.html
	// or just google for "bind() Security Problems"
	// Let the kernel reuse the port without waiting for it to time out.
	// Without this line, you can't stop and immediately re-start the daemon.
	if (reuseAddr)
	{
		int reuse = 1;
#if defined(BLOCXX_NCR)
		::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
#else
		::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#endif
	}
	if ( sock_ipv6 == INVALID_SOCKET)
	{
		// IPv4
		ServerSocketImpl::doListenIPv4( port, queueSize, listenAddr );
	}
#ifdef BLOCXX_HAVE_IPV6
	else
	{	// IPv6
		ServerSocketImpl::doListenIPv6( port, queueSize, listenAddr );
	}
#endif
}

///////////////////////////////////////////////////////
void
ServerSocketImpl::doListenIPv4(UInt16 port, int queueSize, const String& listenAddr )
{
	//	use IPv4 protocol
	InetSocketAddress_t 	inetAddr;
	memset(&inetAddr, 0, sizeof(inetAddr));
	reinterpret_cast<sockaddr_in*>(&inetAddr)->sin_family  =  AF_INET;
	reinterpret_cast<sockaddr_in*>(&inetAddr)->sin_port    =  hton16(port);
	if (listenAddr == SocketAddress::ALL_LOCAL_ADDRESSES)
	{
		reinterpret_cast<sockaddr_in*>(&inetAddr)->sin_addr.s_addr = hton32(INADDR_ANY);
	}
	else
	{
		SocketAddress addr = SocketAddress::getByName(listenAddr);
		reinterpret_cast<sockaddr_in*>(&inetAddr)->sin_addr.s_addr = 
		    reinterpret_cast<const sockaddr_in*>(addr.getInetAddress())->sin_addr.s_addr;
	}
	if (::bind(m_sockfd, reinterpret_cast<sockaddr*>(&inetAddr), sizeof(inetAddr)) == -1)
	{
		close();
		BLOCXX_THROW_ERRNO_MSG(SocketException, "ServerSocketImpl::doListen(): bind");
	}
	if (::listen(m_sockfd, queueSize) == -1)
	{
		close();
		BLOCXX_THROW_ERRNO_MSG(SocketException, "ServerSocketImpl::doListen(): listen");
	}
	fillAddrParms();
	m_isActive = true;
}

#ifdef BLOCXX_HAVE_IPV6
//////////////////////////////////////////////////////////////////////////////
void
ServerSocketImpl::doListenIPv6(UInt16 port, int queueSize, const String& listenAddr)
{
	//	use IPv6 protocol
	InetSocketAddress_t 	inetAddr;
	memset(&inetAddr, 0, sizeof(inetAddr));
	reinterpret_cast<sockaddr_in6*>(&inetAddr)->sin6_family =  AF_INET6;
	reinterpret_cast<sockaddr_in6*>(&inetAddr)->sin6_port   =  hton16(port);
	if (listenAddr == SocketAddress::ALL_LOCAL_ADDRESSES)
	{
		// copy IPv6 address any
		memcpy(reinterpret_cast<sockaddr_in6*>(&inetAddr)->sin6_addr.s6_addr, &in6addr_any, sizeof(struct in6_addr));
	}
	else
	{
		// create network address structure for IPv6 protocol
		if(!inet_pton(AF_INET6, listenAddr.c_str(), (void*)&reinterpret_cast<sockaddr_in6*>(&inetAddr)->sin6_addr))
		{
			addrinfo *addrinfos;
			addrinfo hints;
			memset(&hints, 0, sizeof(hints));
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_family   = AF_INET6;
			if(getaddrinfo(listenAddr.c_str(), NULL, &hints, &addrinfos))
			{
				close();
				BLOCXX_THROW_ERRNO_MSG(SocketException,"ServerSocketImpl::doListen(): getaddrinfo()");
			}
			memcpy(reinterpret_cast<sockaddr_in6*>(&inetAddr), addrinfos->ai_addr, addrinfos->ai_addrlen);
			freeaddrinfo(addrinfos);
		}
	}
	if (::bind(m_sockfd, reinterpret_cast<sockaddr*>(&inetAddr), sizeof(inetAddr)) == -1)
	{
		close();
		BLOCXX_THROW_ERRNO_MSG(SocketException, "ServerSocketImpl::doListen(): bind");
	}
	if (::listen(m_sockfd, queueSize) == -1)
	{
		close();
		BLOCXX_THROW_ERRNO_MSG(SocketException, "ServerSocketImpl::doListen(): listen");
	}
	fillAddrParms();
	m_isActive = true;
}
#endif
//////////////////////////////////////////////////////////////////////////////
void
ServerSocketImpl::doListenUDS(const String& filename, int queueSize, bool reuseAddr)
{
	m_localAddress = SocketAddress::getUDS(filename);
	close();
	m_sockfd = ::socket(PF_UNIX,SOCK_STREAM, 0);
	if (m_sockfd == INVALID_SOCKET)
	{
		BLOCXX_THROW_ERRNO_MSG(SocketException, "ServerSocketImpl::doListen(): socket()");
	}
	// set the close on exec flag so child process can't keep the socket.
	if (::fcntl(m_sockfd, F_SETFD, FD_CLOEXEC) == -1)
	{
		close();
		BLOCXX_THROW_ERRNO_MSG(SocketException, "ServerSocketImpl::doListen(): fcntl() failed to set "
			"close-on-exec flag on listen socket");
	}

	// set listen socket to nonblocking; see Unix Network Programming,
	// pages 422-424.
	int fdflags = ::fcntl(m_sockfd, F_GETFL, 0);
	::fcntl(m_sockfd, F_SETFL, fdflags | O_NONBLOCK);

	if (reuseAddr)
	{
		// Let the kernel reuse the port without waiting for it to time out.
		// Without this line, you can't stop and immediately re-start the daemon.
		int reuse = 1;
#if defined(BLOCXX_NCR)
		::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
#else
		::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#endif
	}
	String lockfilename = filename + ".lock";
	m_udsFile = FileSystem::openOrCreateFile(lockfilename);
	if (!m_udsFile)
	{
		BLOCXX_THROW_ERRNO_MSG(SocketException,
			Format("ServerSocketImpl::doListen(): Unable to open or create Unix Domain Socket lock: %1",
				lockfilename).c_str());
	}
	// if we can't get a lock, someone else has got it open.
	if (m_udsFile.tryLock() == -1)
	{
		BLOCXX_THROW_ERRNO_MSG(SocketException,
			Format("ServerSocketImpl::doListen(): Unable to lock Unix Domain Socket: %1",
				filename).c_str());
	}
	// We got the lock, so clobber the UDS if it's there so bind will succeed.
	// If it's not gone, bind will fail.
	if (FileSystem::exists(filename))
	{
		if (!FileSystem::removeFile(filename.c_str()))
		{
			BLOCXX_THROW_ERRNO_MSG(SocketException,
				Format("ServerSocketImpl::doListen(): Unable to unlink Unix Domain Socket: %1",
					filename).c_str());
		}
	}

#if defined(BLOCXX_NCR)
	if (::bind(m_sockfd, const_cast<SocketAddress_t *>(m_localAddress.getNativeForm()), m_localAddress.getNativeFormSize()) == -1)
#else
	if (::bind(m_sockfd, m_localAddress.getNativeForm(), m_localAddress.getNativeFormSize()) == -1)
#endif
	{
		close();
		BLOCXX_THROW_ERRNO_MSG(SocketException, "ServerSocketImpl::doListen(): bind()");
	}
	// give anybody access to the socket
	// unfortunately, fchmod() doesn't work on a UDS
	if (::chmod(filename.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1)
	{
		close();
		BLOCXX_THROW_ERRNO_MSG(SocketException, "ServerSocketImpl::doListen(): chmod()");
	}
	if (::listen(m_sockfd, queueSize) == -1)
	{
		close();
		BLOCXX_THROW_ERRNO_MSG(SocketException, "ServerSocketImpl::doListen(): listen()");
	}
	fillAddrParms();
	m_isActive = true;
}
//////////////////////////////////////////////////////////////////////////////
/*
String
ServerSocketImpl::addrString()
{
	return inetAddrToString(m_localAddress, m_localPort);
}
*/
//////////////////////////////////////////////////////////////////////////////
Socket
ServerSocketImpl::accept(const Timeout& timeout)
{
	if (!m_isActive)
	{
		BLOCXX_THROW(SocketException, "ServerSocketImpl::accept(): m_isActive == false");
	}
	int rc = 0;
	if ((rc = SocketUtils::waitForIO(m_sockfd, timeout, SocketFlags::E_WAIT_FOR_INPUT)) == 0)
	{
		int clntfd;
		socklen_t clntlen;
		InetSocketAddress_t clntInetAddr;
		struct sockaddr_un clntUnixAddr;
		struct sockaddr* pSA(0);
		if (m_localAddress.getType() == SocketAddress::INET)
		{
			pSA = reinterpret_cast<struct sockaddr*>(&clntInetAddr);
			clntlen = sizeof(clntInetAddr);
		}
		else if (m_localAddress.getType() == SocketAddress::UDS)
		{
			pSA = reinterpret_cast<struct sockaddr*>(&clntUnixAddr);
			clntlen = sizeof(clntUnixAddr);
		}
		else
		{
			BLOCXX_ASSERT(0);
		}
		
		clntfd = ::accept(m_sockfd, pSA, &clntlen);
		if (clntfd < 0)
		{
			// check to see if client aborts connection between select and accept.
			// see Unix Network Programming pages 422-424.
			if (errno == EWOULDBLOCK
				 || errno == ECONNABORTED
#ifdef EPROTO
				 || errno == EPROTO
#endif
				)
			{
				BLOCXX_THROW(SocketException, "Client aborted TCP connection "
					"between select() and accept()");
			}
		
			if (errno == EINTR)
			{
				Thread::testCancel();
			}
			BLOCXX_THROW_ERRNO_MSG(SocketException, "ServerSocketImpl::accept(): accept()");
		}
		// set socket back to blocking; see Unix Network Programming,
		// pages 422-424.
		int fdflags = ::fcntl(clntfd, F_GETFL, 0);
		// On most OSs non-blocking is inherited from the listen socket,
		// but it's not on Openserver.
		if ((fdflags & O_NONBLOCK) == O_NONBLOCK)
		{
			::fcntl(clntfd, F_SETFL, fdflags ^ O_NONBLOCK);
		}
		// TODO, how to make this bw compatible?
		//return Socket(clntfd, m_localAddress.getType(), m_isSSL);
		if (!m_sslCtx && m_isSSL == SocketFlags::E_SSL)
		{
			return Socket(clntfd, m_localAddress.getType(), m_isSSL); // for bw compat.
		}
		return Socket(clntfd, m_localAddress.getType(), m_sslCtx);
	}
	else if (rc == ETIMEDOUT)
	{
		// The timeout expired.
		BLOCXX_THROW(SocketTimeoutException,"Timed out waiting for a connection");
	}
	else
	{
		BLOCXX_THROW_ERRNO_MSG(SocketException,"Timed out waiting for a connection");
	}
}
#endif

//////////////////////////////////////////////////////////////////////////////
void
ServerSocketImpl::close()
{
	if (m_isActive)
	{
#if defined(BLOCXX_WIN32)
		::closesocket(m_sockfd);
		m_sockfd = INVALID_SOCKET;
#else
		::close(m_sockfd);
		if (m_localAddress.getType() == SocketAddress::UDS)
		{
			String filename = m_localAddress.toString();
			if (!FileSystem::removeFile(filename.c_str()))
			{
				BLOCXX_THROW_ERRNO_MSG(SocketException,
					Format("ServerSocketImpl::close(): Unable to unlink Unix Domain Socket: %1",
						filename).c_str());
			}
			if (m_udsFile)
			{
				String lockfilename = filename + ".lock";
				if (m_udsFile.unlock() == -1)
				{
					BLOCXX_THROW_ERRNO_MSG(SocketException,
						Format("ServerSocketImpl::close(): Failed to unlock Unix Domain Socket: %1",
							lockfilename).c_str());
				}
				m_udsFile.close();
				if (!FileSystem::removeFile(lockfilename.c_str()))
				{
					BLOCXX_THROW_ERRNO_MSG(SocketException,
						Format("ServerSocketImpl::close(): Unable to unlink Unix Domain Socket lock: %1",
							lockfilename).c_str());
				}
			}
		}
#endif
		m_isActive = false;
	}
}
//////////////////////////////////////////////////////////////////////////////
void
ServerSocketImpl::fillAddrParms()
{
	socklen_t len;
	if (m_localAddress.getType() == SocketAddress::INET)
	{
		// get information of local address for IPv6 protocol
		struct sockaddr *p_addr;
		InetSocketAddress_t ss;
		memset(&ss, 0, sizeof(ss));
		len = sizeof(ss);
		p_addr = reinterpret_cast<struct sockaddr*>(&ss);
		if (getsockname(m_sockfd, p_addr, &len) == -1)
		{
	    	BLOCXX_THROW_ERRNO_MSG(SocketException, "SocketImpl::fillAddrParms(): getsockname");
		}
		m_localAddress.assignFromNativeForm(&ss, len);
	}
#if !defined(BLOCXX_WIN32)
	else if (m_localAddress.getType() == SocketAddress::UDS)
	{
		struct sockaddr_un addr;
		memset(&addr, 0, sizeof(addr));
		len = sizeof(addr);
		if (getsockname(m_sockfd, reinterpret_cast<struct sockaddr*>(&addr), &len) == -1)
		{
			BLOCXX_THROW_ERRNO_MSG(SocketException, "SocketImpl::fillAddrParms(): getsockname");
		}
		m_localAddress.assignFromNativeForm(&addr, len);
	}
#endif
	else
	{
		BLOCXX_ASSERT(0);
	}
}

} // end namespace BLOCXX_NAMESPACE

