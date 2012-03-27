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
 * @author Jon Carey (Win32)
 */

#include "blocxx/BLOCXX_config.h"

#if defined(BLOCXX_WIN32)

#include "blocxx/SocketBaseImpl.hpp"
#include "blocxx/SocketUtils.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/IOException.hpp"
#include "blocxx/Mutex.hpp"
#include "blocxx/MutexLock.hpp"
#include "blocxx/Socket.hpp"
#include "blocxx/Thread.hpp"
#include "blocxx/System.hpp"
#include "blocxx/TimeoutTimer.hpp"

#include <cstdio>
#include <cerrno>
#include <fstream>
#include <ws2tcpip.h>

namespace
{

class SockInitializer
{
public:
	SockInitializer()
	{
		WSADATA wsaData;
		::WSAStartup(MAKEWORD(2,2), &wsaData);
	}

	~SockInitializer()
	{
		::WSACleanup();
	}
};

// Force Winsock initialization on load
SockInitializer _sockInitializer;

//////////////////////////////////////////////////////////////////////////////
void
_closeSocket(SOCKET& sockfd)
{
	if (sockfd != INVALID_SOCKET)
	{
		::closesocket(sockfd);
		sockfd = INVALID_SOCKET;
	}
}

//////////////////////////////////////////////////////////////////////////////
int
getAddrFromIface(BLOCXX_NAMESPACE::InetSocketAddress_t& addr)
{
	SOCKET sd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sd == SOCKET_ERROR)
	{
		return -1;
	}

	int cc = -1;
	INTERFACE_INFO interfaceList[20];
	unsigned long nBytesReturned;
	if (::WSAIoctl(sd, SIO_GET_INTERFACE_LIST, 0, 0, &interfaceList,
			sizeof(interfaceList), &nBytesReturned, 0, 0) != SOCKET_ERROR)
	{
		int nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);
		for (int i = 0; i < nNumInterfaces; ++i)
		{
			u_long nFlags = interfaceList[i].iiFlags;
			if (nFlags & IFF_UP)
			{
				cc = 0;
				::memcpy(&addr, &(interfaceList[i].iiAddress), sizeof(addr));
				if (!(nFlags & IFF_LOOPBACK))
				{
					break;
				}
			}
		}
	}

	::closesocket(sd);
	return 0;
}

}	// end of unnamed namespace

namespace BLOCXX_NAMESPACE
{

using std::istream;
using std::ostream;
using std::iostream;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::ios;
String SocketBaseImpl::m_traceFileOut;
String SocketBaseImpl::m_traceFileIn;

//////////////////////////////////////////////////////////////////////////////
// static
int
SocketBaseImpl::waitForEvent(HANDLE eventArg, int secsToTimeout)
{
	DWORD timeout = (secsToTimeout != -1)
		? static_cast<DWORD>(secsToTimeout * 1000)
		: INFINITE;
	
	int cc;
	if(Socket::getShutDownMechanism() != NULL)
	{
		HANDLE events[2];
		events[0] = Socket::getShutDownMechanism();
		events[1] = eventArg;

		DWORD index = ::WaitForMultipleObjects(
			2,
			events,
			FALSE,
			timeout);

		switch (index)
		{
			case WAIT_FAILED:
				cc = -2;
				break;
			case WAIT_TIMEOUT:
				cc = -1;
				break;
			default:
				index -= WAIT_OBJECT_0;
				// If not shutdown event, then reset
				if (index != 0)
				{
					::ResetEvent(eventArg);
				}
				cc = static_cast<int>(index);
				break;
		}
	}
	else
	{
		switch(::WaitForSingleObject(eventArg, timeout))
		{
			case WAIT_OBJECT_0:
				::ResetEvent(eventArg);
				cc = 1;
				break;
			case WAIT_TIMEOUT:
				cc = -1;
				break;
			default:
				cc = -2;
				break;
		}
	}
		
	return cc;
}

#pragma warning (push)
#pragma warning (disable: 4355)

//////////////////////////////////////////////////////////////////////////////
SocketBaseImpl::SocketBaseImpl()
	: SelectableIFC()
	, IOIFC()
	, m_isConnected(false)
	, m_sockfd(INVALID_SOCKET)
	, m_localAddress()
	, m_peerAddress()
	, m_event(NULL)
	, m_recvTimeoutExprd(false)
	, m_streamBuf(this)
	, m_in(&m_streamBuf)
	, m_out(&m_streamBuf)
	, m_inout(&m_streamBuf)
	, m_recvTimeout(Timeout::infinite)
	, m_sendTimeout(Timeout::infinite)
	, m_connectTimeout(Timeout::relative(0))
{
	m_out.exceptions(std::ios::badbit);
	m_inout.exceptions(std::ios::badbit);
	m_event = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	BLOCXX_ASSERT(m_event != NULL);
}
//////////////////////////////////////////////////////////////////////////////
SocketBaseImpl::SocketBaseImpl(SocketHandle_t fd,
		SocketAddress::AddressType addrType)
	: SelectableIFC()
	, IOIFC()
	, m_isConnected(true)
	, m_sockfd(fd)
	, m_localAddress(SocketAddress::getAnyLocalHost())
	, m_peerAddress(SocketAddress::allocEmptyAddress(addrType))
	, m_event(NULL)
	, m_recvTimeoutExprd(false)
	, m_streamBuf(this)
	, m_in(&m_streamBuf)
	, m_out(&m_streamBuf)
	, m_inout(&m_streamBuf)
	, m_recvTimeout(Timeout::infinite)
	, m_sendTimeout(Timeout::infinite)
	, m_connectTimeout(Timeout::relative(0))
{
	BLOCXX_ASSERT(addrType == SocketAddress::INET);

	m_out.exceptions(std::ios::badbit);
	m_inout.exceptions(std::ios::badbit);
	m_event = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	BLOCXX_ASSERT(m_event != NULL);
	fillInetAddrParms();
}
//////////////////////////////////////////////////////////////////////////////
SocketBaseImpl::SocketBaseImpl(const SocketAddress& addr)
	: SelectableIFC()
	, IOIFC()
	, m_isConnected(false)
	, m_sockfd(INVALID_SOCKET)
	, m_localAddress(SocketAddress::getAnyLocalHost())
	, m_peerAddress(addr)
	, m_event(NULL)
	, m_recvTimeoutExprd(false)
	, m_streamBuf(this)
	, m_in(&m_streamBuf)
	, m_out(&m_streamBuf)
	, m_inout(&m_streamBuf)
	, m_recvTimeout(Timeout::infinite)
	, m_sendTimeout(Timeout::infinite)
	, m_connectTimeout(Timeout::relative(0))
{
	m_out.exceptions(std::ios::badbit);
	m_inout.exceptions(std::ios::badbit);
	m_event = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	BLOCXX_ASSERT(m_event != NULL);
	connect(m_peerAddress);
}

#pragma warning (pop)

//////////////////////////////////////////////////////////////////////////////
SocketBaseImpl::~SocketBaseImpl()
{
	try
	{
		disconnect();
	}
	catch (...)
	{
		// don't let exceptions escape
	}
	::CloseHandle(m_event);
}
//////////////////////////////////////////////////////////////////////////////
Select_t
SocketBaseImpl::getSelectObj() const
{
	Select_t st;
	st.event = m_event;
	st.sockfd = m_sockfd;
	st.isSocket = true;
	st.networkevents = FD_READ | FD_WRITE;
	st.doreset = true;
	return st;
}
//////////////////////////////////////////////////////////////////////////////
void
SocketBaseImpl::connect(const SocketAddress& addr)
{
	if (m_isConnected)
	{
		disconnect();
	}
	m_streamBuf.reset();
	m_in.clear();
	m_out.clear();
	m_inout.clear();
	BLOCXX_ASSERT(addr.getType() == SocketAddress::INET);

	m_sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sockfd == INVALID_SOCKET)
	{
		BLOCXX_THROW(SocketException, 
			Format("Failed to create a socket: %1",
			System::lastErrorMsg(true)).c_str());
	}

	int cc;
	WSANETWORKEVENTS networkEvents;

	// Connect non-blocking
	if(::WSAEventSelect(m_sockfd, m_event, FD_CONNECT) != 0)
	{
		BLOCXX_THROW(SocketException, 
			Format("WSAEventSelect Failed: %1",
			System::lastErrorMsg(true)).c_str());
	}

	if (::connect(m_sockfd, addr.getNativeForm(), addr.getNativeFormSize())
		== SOCKET_ERROR)
	{
		int lastError = ::WSAGetLastError();
		if (lastError != WSAEWOULDBLOCK && lastError != WSAEINPROGRESS)
		{
			_closeSocket(m_sockfd);
			BLOCXX_THROW(SocketException,
				Format("Failed to connect to: %1: %2(%3)", addr.toString(),
					lastError, System::lastErrorMsg(true)).c_str());
		}

		TimeoutTimer timer(m_connectTimeout);
		int tmoutval = timer.asDWORDMs();

		// Wait for connection event to come through
		while (true)
		{
			// Wait for the socket's event to get signaled
			if ((cc = waitForEvent(m_event, tmoutval)) < 1)
			{
				_closeSocket(m_sockfd);
				switch (cc)
				{
					case 0:		// Shutdown event
						BLOCXX_THROW(SocketException,
							"Sockets have been shutdown");
					case -1:	// Timed out
						BLOCXX_THROW(SocketException,
							Format("Win32SocketBaseImpl connection"
								" timed out. Timeout val = %1",
								tmoutval).c_str());
					default:	// Error on wait
						BLOCXX_THROW(SocketException, Format("SocketBaseImpl::"
							"connect() wait failed: %1(%2)",
							::WSAGetLastError(),
							System::lastErrorMsg(true)).c_str());
				}
			}

			// Find out what network event took place
			if (::WSAEnumNetworkEvents(m_sockfd, m_event, &networkEvents)
				== SOCKET_ERROR)
			{
				_closeSocket(m_sockfd);
				BLOCXX_THROW(SocketException,
					Format("SocketBaseImpl::connect()"
						" failed getting network events: %1(%2)",
						::WSAGetLastError(),
						System::lastErrorMsg(true)).c_str());
			}

			// Was it a connect event?
			if (networkEvents.lNetworkEvents & FD_CONNECT)
			{
				// Did connect fail?
				if (networkEvents.iErrorCode[FD_CONNECT_BIT])
				{
					::WSASetLastError(networkEvents.iErrorCode[FD_CONNECT_BIT]);
					_closeSocket(m_sockfd);
					BLOCXX_THROW(SocketException,
						Format("SocketBaseImpl::connect() failed: %1(%2)",
						::WSAGetLastError(),
						System::lastErrorMsg(true)).c_str());
				}
				break;
			}
		}	// while (true) - waiting for connection event
	}	// if SOCKET_ERROR on connect

	// Set socket back to blocking
	if(::WSAEventSelect(m_sockfd, m_event, 0) != 0)
	{
		_closeSocket(m_sockfd);
		BLOCXX_THROW(SocketException, 
			Format("Resetting socket with WSAEventSelect Failed: %1",
			System::lastErrorMsg(true)).c_str());
	}
	u_long ioctlarg = 0;
	::ioctlsocket(m_sockfd, FIONBIO, &ioctlarg);

	m_isConnected = true;

	m_peerAddress = addr; // To get the hostname from addr

	BLOCXX_ASSERT(addr.getType() == SocketAddress::INET);

	fillInetAddrParms();
}

//////////////////////////////////////////////////////////////////////////////
void
SocketBaseImpl::disconnect()
{
	if(m_in)
	{
		m_in.clear(ios::eofbit);
	}
	if(m_out)
	{
		m_out.clear(ios::eofbit);
	}
	if(!m_inout.fail())
	{
		m_inout.clear(ios::eofbit);
	}

	::SetEvent(m_event);
	_closeSocket(m_sockfd);
	m_isConnected = false;
}

//////////////////////////////////////////////////////////////////////////////
void
SocketBaseImpl::fillInetAddrParms()
{
	socklen_t len;
	InetSocketAddress_t addr;
	::memset(&addr, 0, sizeof(addr));
	len = sizeof(addr);
	bool gotAddr = false;

	if (m_sockfd != INVALID_SOCKET)
	{
		len = sizeof(addr);
		if (::getsockname(m_sockfd,
			reinterpret_cast<struct sockaddr*>(&addr), &len) != SOCKET_ERROR)
		{
			m_localAddress.assignFromNativeForm(&addr, len);
		}
		else if (getAddrFromIface(addr) == 0)
		{
			len = sizeof(addr);
			m_localAddress.assignFromNativeForm(&addr, len);
		}

		len = sizeof(addr);
		if (::getpeername(m_sockfd, reinterpret_cast<struct sockaddr*>(&addr),
			&len) != SOCKET_ERROR)
		{
			m_peerAddress.assignFromNativeForm(&addr, len);
		}
	}
	else if (getAddrFromIface(addr) == 0)
	{
		m_localAddress.assignFromNativeForm(&addr, len);
	}
}

static Mutex guard;
//////////////////////////////////////////////////////////////////////////////
int
SocketBaseImpl::write(const void* dataOut, int dataOutLen, ErrorAction errorAsException)
{
	int rc = 0;
	bool isError = false;
	if (m_isConnected)
	{
		isError = waitForOutput(m_sendTimeout);
		if (isError)
		{
			rc = -1;
		}
		else
		{
			rc = writeAux(dataOut, dataOutLen);
			if (!m_traceFileOut.empty() && rc > 0)
			{
				MutexLock ml(guard);
				ofstream traceFile(m_traceFileOut.c_str(), std::ios::app);
				if (!traceFile)
				{
					BLOCXX_THROW(IOException, "Failed opening socket dump file");
				}
				if (!traceFile.write(static_cast<const char*>(dataOut), rc))
				{
					BLOCXX_THROW(IOException, "Failed writing to socket dump");
				}

				ofstream comboTraceFile(String(m_traceFileOut + "Combo").c_str(), std::ios::app);
				if (!comboTraceFile)
				{
					BLOCXX_THROW(IOException, "Failed opening socket dump file");
				}
				comboTraceFile << "\n--->Out " << rc << " bytes<---\n";
				if (!comboTraceFile.write(static_cast<const char*>(dataOut), rc))
				{
					BLOCXX_THROW(IOException, "Failed writing to socket dump");
				}
			}
		}
	}
	else
	{
		rc = -1;
	}
	if (rc < 0 && errorAsException)
	{
		BLOCXX_THROW(SocketException, "SocketBaseImpl::write");
	}
	return rc;
}
//////////////////////////////////////////////////////////////////////////////
int
SocketBaseImpl::read(void* dataIn, int dataInLen, ErrorAction errorAsException) 	
{
	int rc = 0;
	bool isError = false;
	if (m_isConnected)
	{
		isError = waitForInput(m_recvTimeout);
		if (isError)
		{
			rc = -1;
		}
		else
		{
			rc = readAux(dataIn, dataInLen);
			if (!m_traceFileIn.empty() && rc > 0)
			{
				MutexLock ml(guard);
				ofstream traceFile(m_traceFileIn.c_str(), std::ios::app);
				if (!traceFile)
				{
					BLOCXX_THROW(IOException, "Failed opening tracefile");
				}
				if (!traceFile.write(reinterpret_cast<const char*>(dataIn), rc))
				{
					BLOCXX_THROW(IOException, "Failed writing to socket dump");
				}

				ofstream comboTraceFile(String(m_traceFileOut + "Combo").c_str(), std::ios::app);
				if (!comboTraceFile)
				{
					BLOCXX_THROW(IOException, "Failed opening socket dump file");
				}
				comboTraceFile << "\n--->In " << rc << " bytes<---\n";
				if (!comboTraceFile.write(reinterpret_cast<const char*>(dataIn), rc))
				{
					BLOCXX_THROW(IOException, "Failed writing to socket dump");
				}
			}
		}
	}
	else
	{
		rc = -1;
	}
	if (rc < 0)
	{
		if (errorAsException)
			BLOCXX_THROW(SocketException, "SocketBaseImpl::read");
	}
	return rc;
}
//////////////////////////////////////////////////////////////////////////////
bool
SocketBaseImpl::waitForInput(const Timeout& timeOutSecs)
{
	int rval = SocketUtils::waitForIO(m_sockfd, m_event, timeOutSecs, FD_READ);
	if (rval == ETIMEDOUT)
	{
		m_recvTimeoutExprd = true;
	}
	else
	{
		m_recvTimeoutExprd = false;
	}
	return (rval != 0);
}
//////////////////////////////////////////////////////////////////////////////
bool
SocketBaseImpl::waitForOutput(const Timeout& timeOutSecs)
{
	return SocketUtils::waitForIO(m_sockfd, m_event, timeOutSecs,
		FD_WRITE) != 0;
}
//////////////////////////////////////////////////////////////////////////////
istream&
SocketBaseImpl::getInputStream()
{
	return m_in;
}
//////////////////////////////////////////////////////////////////////////////
ostream&
SocketBaseImpl::getOutputStream()
{
	return m_out;
}
//////////////////////////////////////////////////////////////////////////////
iostream&
SocketBaseImpl::getIOStream()
{
	return m_inout;
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
void
SocketBaseImpl::setDumpFiles(const String& in, const String& out)
{
	m_traceFileOut = out;
	m_traceFileIn = in;
}

} // end namespace BLOCXX_NAMESPACE

#endif	// #if defined(BLOCXX_WIN32)
