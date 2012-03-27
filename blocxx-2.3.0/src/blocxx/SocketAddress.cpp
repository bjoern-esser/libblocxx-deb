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
 * @author Bart Whiteley
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/SocketAddress.hpp"
#include "blocxx/ByteSwap.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/Mutex.hpp"
#include "blocxx/MutexLock.hpp"
#include "blocxx/ExceptionIds.hpp"
#include "blocxx/Format.hpp"

extern "C"
{
#if !defined(BLOCXX_WIN32)
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/param.h>
#include <sys/utsname.h>
#include <unistd.h>
#endif

#include <errno.h>
}

namespace BLOCXX_NAMESPACE
{

#ifdef BLOCXX_WIN32

#include <WS2tcpip.h>

	const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt)
	{
		if (af == AF_INET)
		{
			struct sockaddr_in in;
			memset(&in, 0, sizeof(in));
			in.sin_family = AF_INET;
			memcpy(&in.sin_addr, src, sizeof(struct in_addr));
			getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in), dst, cnt, NULL, 0, NI_NUMERICHOST);
			return dst;
		}
		else if (af == AF_INET6)
		{
			struct sockaddr_in6 in;
			memset(&in, 0, sizeof(in));
			in.sin6_family = AF_INET6;
			memcpy(&in.sin6_addr, src, sizeof(struct in_addr6));
			getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in6), dst, cnt, NULL, 0, NI_NUMERICHOST);
			return dst;
		}
		return NULL;
	}
#endif

	BLOCXX_DEFINE_EXCEPTION_WITH_ID(UnknownHost);
	BLOCXX_DEFINE_EXCEPTION_WITH_ID(SocketAddress);

	const char* const SocketAddress::ALL_LOCAL_ADDRESSES = "0.0.0.0";

#if !defined(BLOCXX_WIN32)
	//////////////////////////////////////////////////////////////////////////////
	//static
	SocketAddress
	SocketAddress::getUDS(const String& filename)
	{
		SocketAddress rval;

		const size_t maxsocklen = sizeof(rval.m_UDSNativeAddress.sun_path) - 1;
		if( filename.length() > maxsocklen  )
		{
			BLOCXX_THROW(SocketAddressException, Format("UDS filename (%1) is too long.  It must be <= %2 bytes.", filename, maxsocklen).c_str());
		}

		rval.m_type = UDS;
		rval.m_name = filename;
		rval.m_address = "localhost";
		memset(&rval.m_UDSNativeAddress, 0, sizeof(rval.m_UDSNativeAddress));
		rval.m_UDSNativeAddress.sun_family = AF_UNIX;
		strncpy(rval.m_UDSNativeAddress.sun_path, filename.c_str(),
			sizeof(rval.m_UDSNativeAddress.sun_path) - 1);
#ifdef BLOCXX_SOLARIS
		rval.m_nativeSize = ::strlen(rval.m_UDSNativeAddress.sun_path) +
			offsetof(struct sockaddr_un, sun_path);
#elif defined BLOCXX_OPENUNIX
		rval.m_UDSNativeAddress.sun_len = sizeof(rval.m_UDSNativeAddress);
		rval.m_nativeSize = ::strlen(rval.m_UDSNativeAddress.sun_path) +
			offsetof(struct sockaddr_un, sun_path);
#elif defined BLOCXX_AIX || defined BLOCXX_DARWIN
		// AIX requires the NULL terminator to be included in the sizes.
		rval.m_UDSNativeAddress.sun_len = filename.length() + 1;
		rval.m_nativeSize = ::strlen(rval.m_UDSNativeAddress.sun_path) +
			offsetof(struct sockaddr_un, sun_path) + 1;
#elif defined BLOCXX_FREEBSD
		rval.m_nativeSize = ::strlen(rval.m_UDSNativeAddress.sun_path)
			+ sizeof(rval.m_UDSNativeAddress.sun_len)
			+ sizeof(rval.m_UDSNativeAddress.sun_family);
#else
		rval.m_nativeSize = sizeof(rval.m_UDSNativeAddress.sun_family) +
			::strlen(rval.m_UDSNativeAddress.sun_path);
#endif
		return rval;
	}

#endif	// #if !defined(BLOCXX_WIN32)

	//////////////////////////////////////////////////////////////////////////////
	SocketAddress::SocketAddress()
		: m_nativeSize(0) , m_type(UNSET)
	{
	}

#ifndef BLOCXX_HAVE_GETHOSTBYNAME_R
	Mutex gethostbynameMutex;
#endif

	//////////////////////////////////////////////////////////////////////////////
	//static
	SocketAddress
	SocketAddress::getByName(const String& hostName, UInt16 port)
	{
#ifdef BLOCXX_HAVE_IPV6
		// create SocketAddress structure for IPV6 protocol
		InetSocketAddress_t sa;
		memset(&sa, 0, sizeof(sa));
		if( inet_pton(AF_INET6, hostName.c_str(), (void*)&(reinterpret_cast<sockaddr_in6*>(&sa)->sin6_addr)))
		{
			reinterpret_cast<sockaddr_in6*>(&sa)->sin6_family = AF_INET6;
			reinterpret_cast<sockaddr_in6*>(&sa)->sin6_port   = htons(port);
			SocketAddress p = SocketAddress(sa);
			p.m_type = INET;
			p.m_name = hostName;
			return p;
		}
#endif
#if defined(BLOCXX_HAVE_GETHOSTBYNAME_R) && defined(BLOCXX_GETHOSTBYNAME_R_ARGUMENTS)
		hostent hostbuf;
		hostent* host = &hostbuf;
#if (BLOCXX_GETHOSTBYNAME_R_ARGUMENTS == 6)
		char buf[2048];
		int h_err = 0;
		if (gethostbyname_r(hostName.c_str(), &hostbuf, buf, sizeof(buf),
				&host, &h_err) == -1)
		{
			host = NULL;
		}
#elif (BLOCXX_GETHOSTBYNAME_R_ARGUMENTS == 5)

		char buf[2048];
		int h_err(0);
		// returns NULL if not successful
		host = gethostbyname_r(hostName.c_str(), &hostbuf, buf, sizeof(buf), &h_err);

#elif (BLOCXX_GETHOSTBYNAME_R_ARGUMENTS == 3)
		hostent_data hostdata;
		if (gethostbyname_r(hostName.c_str(), &hostbuf, &hostdata) != 0)
		{
			host = NULL;
		}

#else
#error Not yet supported: gethostbyname_r() with other argument counts.
#endif /* BLOCXX_GETHOSTBYNAME_R_ARGUMENTS */
#else
		MutexLock mlock(gethostbynameMutex);
#if defined(BLOCXX_NCR)
		hostent* host = gethostbyname(const_cast<char *>(hostName.c_str()));
#else
		hostent* host = gethostbyname(hostName.c_str());
#endif
#endif /* defined(BLOCXX_HAVE_GETHOSTBYNAME_R) && defined(BLOCXX_GETHOSTBYNAME_R_ARGUMENTS) */

		if (!host)
		{
			BLOCXX_THROW(UnknownHostException, String("Unknown host: ").concat(hostName).c_str());
		}
		in_addr addr;
		memcpy(&addr, host->h_addr_list[0], sizeof(addr));
		return getFromNativeForm(addr, port, host->h_name);
	}

	//////////////////////////////////////////////////////////////////////////////
	//static
	SocketAddress
	SocketAddress::getFromNativeForm( const InetSocketAddress_t& nativeForm)
	{
		return SocketAddress(nativeForm);
	}

#if !defined(BLOCXX_WIN32)
	//////////////////////////////////////////////////////////////////////////////
	//static
	SocketAddress
	SocketAddress::getFromNativeForm( const UnixSocketAddress_t& nativeForm)
	{
		return SocketAddress(nativeForm);
	}
#endif	// !defined(BLOCXX_WIN32)

	//////////////////////////////////////////////////////////////////////////////
	//static
	SocketAddress
	SocketAddress::getFromNativeForm( const InetAddress_t& nativeForm,
		UInt16 nativePort, const String& hostName)
	{
		InetSocketAddress_t addr;
		memset(&addr, 0, sizeof(addr));
		reinterpret_cast<sockaddr_in*>(&addr)->sin_family = AF_INET;
		reinterpret_cast<sockaddr_in*>(&addr)->sin_port = hton16(nativePort);
		reinterpret_cast<sockaddr_in*>(&addr)->sin_addr = nativeForm;
		SocketAddress p = SocketAddress(addr);
		p.m_type = INET;
		p.m_name = hostName;
		return p;
	}
	//////////////////////////////////////////////////////////////////////////////
	const SocketAddress_t* SocketAddress::getNativeForm() const
	{
		if (m_type == INET)
		{
			return reinterpret_cast<const sockaddr*>(&m_inetNativeAddress);
		}

#if !defined(BLOCXX_WIN32)
		else if (m_type == UDS)
		{
			return reinterpret_cast<const sockaddr*>(&m_UDSNativeAddress);
		}
#endif

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////
	const InetSocketAddress_t* SocketAddress::getInetAddress() const
	{
		return &m_inetNativeAddress;
	}

#if !defined(BLOCXX_WIN32)
	//////////////////////////////////////////////////////////////////////////////
	// Get a pointer to the UnixSocketAddress_t
	// precondition: getType() == UDS
	const UnixSocketAddress_t* SocketAddress::getUnixAddress() const
	{
		return &m_UDSNativeAddress;
	}
#endif

	//////////////////////////////////////////////////////////////////////////////
	SocketAddress
	SocketAddress::getAnyLocalHost(UInt16 port)
	{
		struct in_addr addr;
		addr.s_addr = hton32(INADDR_ANY);
		SocketAddress rval = getFromNativeForm(addr, port, "localhost");
		char buf[256];
		gethostname(buf, sizeof(buf));
		String hname(buf);
		if (hname.indexOf('.') == String::npos)
		{
#if defined(BLOCXX_HAVE_GETHOSTBYNAME_R) && defined(BLOCXX_GETHOSTBYNAME_R_ARGUMENTS)
			hostent hostbuf;
			hostent* hent = &hostbuf;
#if (BLOCXX_GETHOSTBYNAME_R_ARGUMENTS == 6)
			char local_buf[2048];
			int h_err = 0;
			if (gethostbyname_r(buf, &hostbuf, local_buf, sizeof(local_buf),
					&hent, &h_err) == -1)
			{
				hent = NULL;
			}
#elif (BLOCXX_GETHOSTBYNAME_R_ARGUMENTS == 5)

			char local_buf[2048];
			int h_err(0);
			// returns NULL if not successful
			hent = gethostbyname_r(buf, &hostbuf, local_buf, sizeof(local_buf), &h_err);

#elif (BLOCXX_GETHOSTBYNAME_R_ARGUMENTS == 3)
			hostent_data hostdata;
			if (gethostbyname_r(buf, &hostbuf, &hostdata) != 0)
			{
				hent = NULL;
			}

#else
#error Not yet supported: gethostbyname_r() with other argument counts.
#endif /* BLOCXX_GETHOSTBYNAME_R_ARGUMENTS */
#else
			MutexLock mlock(gethostbynameMutex);
			hostent* hent = gethostbyname(buf);
#endif
			if (hent && hent->h_name && (strlen(hent->h_name) > 0))
			{
				hname = String(hent->h_name);
			}
		}
		rval.m_name = hname;
		return rval;
	}

	//////////////////////////////////////////////////////////////////////////////
	void SocketAddress::assignFromNativeForm(
		const InetSocketAddress_t* address, size_t /*size*/)
	{
		m_type = INET;
		memcpy(&m_inetNativeAddress, address, sizeof(m_inetNativeAddress));
#ifdef BLOCXX_HAVE_IPV6
		char buf[INET6_ADDRSTRLEN];
		if ( reinterpret_cast<sockaddr*>(&m_inetNativeAddress)->sa_family==AF_INET6)
		{
			m_address = inet_ntop(AF_INET6, &(reinterpret_cast<sockaddr_in6*>(&m_inetNativeAddress)->sin6_addr), buf, sizeof(buf));
		}
		else
		{
			m_address = inet_ntop(AF_INET, &(reinterpret_cast<sockaddr_in*>(&m_inetNativeAddress)->sin_addr), buf, sizeof(buf));
		}
#else
		m_address = inet_ntoa( reinterpret_cast<sockaddr_in*>(&m_inetNativeAddress)->sin_addr);
#endif
		m_nativeSize = sizeof(m_inetNativeAddress);
	}

#if !defined(BLOCXX_WIN32)
	//////////////////////////////////////////////////////////////////////////////
	void SocketAddress::assignFromNativeForm(
		const UnixSocketAddress_t* address, size_t /*size*/)
	{
		m_type = UDS;
		memcpy(&m_UDSNativeAddress, address, sizeof(m_UDSNativeAddress));
		m_address = "localhost";
		m_name = m_UDSNativeAddress.sun_path;
		m_nativeSize = sizeof(m_UDSNativeAddress);
	}
#endif	// !defined(BLOCXX_WIN32)

	//////////////////////////////////////////////////////////////////////////////
	UInt16 SocketAddress::getPort() const
	{
		BLOCXX_ASSERT(m_type == INET);

#ifdef BLOCXX_HAVE_IPV6
		if ( reinterpret_cast<const sockaddr*>(&m_inetNativeAddress)->sa_family==AF_INET6)
		{
			return ntoh16(reinterpret_cast<const sockaddr_in6*>(&m_inetNativeAddress)->sin6_port);
		}
		else
		{
			return ntoh16(reinterpret_cast<const sockaddr_in*>(&m_inetNativeAddress)->sin_port);
		}
#else
		return ntoh16(reinterpret_cast<const sockaddr_in*>(&m_inetNativeAddress)->sin_port);
#endif
	}

#if !defined(BLOCXX_WIN32)
	//////////////////////////////////////////////////////////////////////////////
	SocketAddress::SocketAddress(const UnixSocketAddress_t& nativeForm)
		: m_nativeSize(0), m_type(UDS)
	{
		assignFromNativeForm(&nativeForm, sizeof(nativeForm));
	}
#endif	// !defined(BLOCXX_WIN32)

	//////////////////////////////////////////////////////////////////////////////
	SocketAddress::SocketAddress(const InetSocketAddress_t& nativeForm)
		: m_nativeSize(0), m_type(INET)
	{
		assignFromNativeForm(&nativeForm, sizeof(nativeForm));
	}

	//////////////////////////////////////////////////////////////////////////////
	const String SocketAddress::getName() const
	{
		return m_name;
	}
	//////////////////////////////////////////////////////////////////////////////
	const String SocketAddress::getAddress() const
	{
		return m_address;
	}
	//////////////////////////////////////////////////////////////////////////////
	size_t SocketAddress::getNativeFormSize() const
	{
		return m_nativeSize;
	}
	//////////////////////////////////////////////////////////////////////////////
	SocketAddress SocketAddress::allocEmptyAddress(AddressType type)
	{
		if (type == INET)
		{
			InetSocketAddress_t addr;
			memset(&addr, 0, sizeof(addr));
			reinterpret_cast<sockaddr_in*>(&addr)->sin_family = AF_INET;
			return SocketAddress(SocketAddress::getFromNativeForm(addr));
		}
#if !defined(BLOCXX_WIN32)
		else if (type == UDS)
		{
			sockaddr_un addr;
			memset(&addr, 0, sizeof(addr));
			addr.sun_family = AF_UNIX;
			return SocketAddress(SocketAddress::getFromNativeForm(addr));
		}
#endif

		BLOCXX_THROW(SocketAddressException, "Bad Address Type");
	}
	//////////////////////////////////////////////////////////////////////////////
	const String
	SocketAddress::toString() const
	{
		BLOCXX_ASSERT(m_type != UNSET);
		String rval;
		if (m_type == INET)
		{
			rval = getAddress() + ":" + String(UInt32(getPort()));
		}
		else
		{
			rval = this->m_name;
		}
		return rval;
	}

} // end namespace BLOCXX_NAMESPACE

