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
 * @author Jon Carey
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"

#if !defined(BLOCXX_WIN32) && !defined(BLOCXX_NETWARE)

extern "C"
{
#ifdef BLOCXX_HAVE_UNISTD_H
  #include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef BLOCXX_HAVE_SYS_SOCKET_H
  #include <sys/socket.h>
#endif

#include <arpa/inet.h>
#include <errno.h>

#ifdef BLOCXX_GNU_LINUX
  #include <sys/ioctl.h>
  #include <linux/if.h>
  #include <string.h>

#elif defined (BLOCXX_OPENSERVER)
  #include <string.h>
  #include <stropts.h>
  #include <net/if.h>
  #include <netinet/in.h>
  #include <strings.h>
  #include <arpa/inet.h>
  #include <fcntl.h>
  #include <paths.h>
  #include <sys/mdi.h>

#elif defined (BLOCXX_DARWIN)
  #include <net/if.h>
  #include <sys/ioctl.h>
#else

  #ifdef BLOCXX_HAVE_STROPTS_H
    #include <stropts.h>
  #endif

  #include <net/if.h>
  #include <netinet/in.h>

  #if defined (BLOCXX_HAVE_SYS_SOCKIO_H)
    #include <sys/sockio.h>
  #endif

  #include <strings.h>
  #include <fcntl.h>
#endif

#include <string.h>
} // extern "C"
// These need to be after the system includes because of some weird openserver
// include order problem
#include "blocxx/NwIface.hpp"
#include "blocxx/String.hpp"
#include "blocxx/Exception.hpp"
#include "blocxx/SocketUtils.hpp"

namespace BLOCXX_NAMESPACE
{

//////////////////////////////////////////////////////////////////////////////
NwIface::NwIface()
{
	int s, lerrno;
	struct ifreq ifr;
	struct sockaddr_in *sin(0);
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		BLOCXX_THROW(SocketException, "socket");
	}
	getInterfaceName(s);
	bzero(&ifr, sizeof(ifr));
	strncpy(ifr.ifr_name, m_name.c_str(), sizeof(ifr.ifr_name));
	////////////////////
	// Get IP address
	if (ioctl(s, SIOCGIFADDR, &ifr) < 0)
	{
		lerrno = errno;
		close(s);
		BLOCXX_THROW(SocketException, "ioctl:SIOCGIFADDR");
	}
	sin = reinterpret_cast<struct sockaddr_in *>(&ifr.ifr_addr);
	m_addr = sin->sin_addr.s_addr;
	////////////////////
	// Get the broadcast address
	// Testing
	if (ioctl(s, SIOCGIFBRDADDR, &ifr) < 0)
	{
		lerrno = errno;
		close(s);
		BLOCXX_THROW(SocketException, "ioctl:SIOCGIFBRDADDR");
	}
	sin = reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_broadaddr);
	m_bcastAddr = sin->sin_addr.s_addr;
	////////////////////
	// Get net mask
	if (ioctl(s, SIOCGIFNETMASK, &ifr) < 0)
	{
		lerrno = errno;
		close(s);
		BLOCXX_THROW(SocketException, "ioctl:SIOCGIFNETMASK");
	}
#ifdef BLOCXX_GNU_LINUX
	sin = reinterpret_cast<struct sockaddr_in *>(&ifr.ifr_netmask);
#else
	sin = reinterpret_cast<struct sockaddr_in *>(&ifr.ifr_broadaddr);
#endif
	m_netmask = sin->sin_addr.s_addr;
	close(s);
}
//////////////////////////////////////////////////////////////////////////////
String
NwIface::getName()
{
	return m_name;
}
//////////////////////////////////////////////////////////////////////////////
unsigned long
NwIface::getIPAddress()
{
	return m_addr;
}
//////////////////////////////////////////////////////////////////////////////
String
NwIface::getIPAddressString()
{
	return SocketUtils::inetAddrToString(m_addr);
}
//////////////////////////////////////////////////////////////////////////////
unsigned long
NwIface::getBroadcastAddress()
{
	return m_bcastAddr;
}
//////////////////////////////////////////////////////////////////////////////
String
NwIface::getBroadcastAddressString()
{
	return SocketUtils::inetAddrToString(m_bcastAddr);
}
//////////////////////////////////////////////////////////////////////////////
/*
String
NwIface::getMACAddressString()
{
	return m_macAddress;
}
*/
//////////////////////////////////////////////////////////////////////////////
unsigned long
NwIface::getNetmask()
{
	return m_netmask;
}
//////////////////////////////////////////////////////////////////////////////
String
NwIface::getNetmaskString()
{
	return SocketUtils::inetAddrToString(m_netmask);
}
//////////////////////////////////////////////////////////////////////////////
bool
NwIface::sameNetwork(unsigned long addr)
{
	return ((addr & m_netmask) == (m_addr & m_netmask));
}
//////////////////////////////////////////////////////////////////////////////
bool
NwIface::sameNetwork(const String& straddr)
{
	return sameNetwork(stringToAddress(straddr));
}
//////////////////////////////////////////////////////////////////////////////
unsigned long
NwIface::stringToAddress(const String& straddr)
{
	return inet_addr(straddr.c_str());
}
//////////////////////////////////////////////////////////////////////////////
void
NwIface::getInterfaceName(SocketHandle_t sockfd)
{
	char *p(0);
	int numreqs = 30;
	struct ifconf ifc;
	struct ifreq *ifr(0);
	struct ifreq ifrcopy;
	int n;
	int oldlen = -1;
	int lerrno = 0;
	const char* appliesTo(0);
	ifc.ifc_buf = NULL;
	for (;;)
	{
		ifc.ifc_len = sizeof(struct ifreq) * numreqs;
		if (ifc.ifc_buf == NULL)
		{
			ifc.ifc_buf = new char[ifc.ifc_len];
		}
		else
		{
			p = new char[ifc.ifc_len];
			memmove(p, ifc.ifc_buf, oldlen);
			delete [] ifc.ifc_buf;
			ifc.ifc_buf = p;
		}
		oldlen = ifc.ifc_len;
		if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0)
		{
			lerrno = errno;
			appliesTo = "ioctl:SIOCGIFCONF";
			break;
		}
		if (ifc.ifc_len == static_cast<int>(sizeof(struct ifreq) * numreqs))
		{
			/* assume it overflowed and try again */
			numreqs += 10;
			continue;
		}
		break;
	}
	if (lerrno == 0)
	{
		lerrno = ENODEV;
		appliesTo = "No interfaces found";
		ifr = ifc.ifc_req;
		for (n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq))
		{
			ifrcopy = *ifr;
			if (ioctl(sockfd, SIOCGIFFLAGS, &ifrcopy) < 0)
			{
				lerrno = errno;
				appliesTo = "ioctl:SIOCGIFFLAGS";
				break;
			}
#ifdef BLOCXX_GNU_LINUX
			if ((ifrcopy.ifr_flags & IFF_UP) && !(ifrcopy.ifr_flags & (IFF_LOOPBACK | IFF_DYNAMIC)))
#else
			if ((ifrcopy.ifr_flags & IFF_UP))
#endif
			{
				m_name = ifr->ifr_name;
				lerrno = 0;
				break;
			}
			ifr++;
		}
	}
	if (ifc.ifc_buf != NULL)
	{
		delete [] ifc.ifc_buf;
	}
	if (lerrno != 0)
	{
		BLOCXX_THROW(SocketException, "NwIface::getInterfaceName");
	}
}

} // end namespace BLOCXX_NAMESPACE

#endif	// #if !defined(BLOCXX_WIN32) && !defined(BLOCXX_NETWARE)

