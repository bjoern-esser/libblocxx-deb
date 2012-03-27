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

#ifndef BLOCXX_NETWORK_TYPES_HPP_INCLUDE_GUARD_
#define BLOCXX_NETWORK_TYPES_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Types.hpp"

extern "C"
{
#ifdef BLOCXX_HAVE_UNISTD_H
 #include <unistd.h>
#endif

#include <signal.h>

#ifdef BLOCXX_HAVE_SYS_SOCKET_H
 #include <sys/socket.h>
#endif

#ifdef BLOCXX_HAVE_NETINET_IN_H
 #include <netinet/in.h>
#endif

#ifdef BLOCXX_HAVE_SYS_UN_H
 #include <sys/un.h>
#endif

#if defined(BLOCXX_WIN32)
#include <winsock2.h>
#endif
}

#undef shutdown // On OpenUnix, sys/socket.h defines shutdown to be
				// _shutdown.  (which breaks HTTPServer, etc.)

namespace BLOCXX_NAMESPACE
{

// Platform specific socket address type
typedef sockaddr		SocketAddress_t;
// Platform specific inet socket address type
#ifdef BLOCXX_HAVE_IPV6
typedef sockaddr_storage	InetSocketAddress_t;
#else
typedef sockaddr_in		InetSocketAddress_t;
#endif

#if !defined(BLOCXX_WIN32)
// Platform specific unix socket address type
typedef sockaddr_un		UnixSocketAddress_t;
#endif

// Platform specific socket address type
typedef in_addr		InetAddress_t;

#if defined (BLOCXX_WIN32)
// Platform specific socket fd type
typedef SOCKET			SocketHandle_t;
#else
// Platform specific socket fd type
typedef int 			SocketHandle_t;
#endif

// Select_t is the type of object that can be used in
// synchronous I/O multiplexing (i.e. select).
#if defined(BLOCXX_WIN32)
struct Select_t
{
	Select_t() 
		: event(NULL)
		, descriptor(INVALID_HANDLE_VALUE)
		, sockfd(INVALID_SOCKET)
		, isSocket(false)
		, networkevents(0)
		, doreset(false)
	{
	}

	Select_t(const Select_t& arg)
		: event(arg.event)
		, descriptor(arg.descriptor)
		, sockfd(arg.sockfd)
		, isSocket(arg.isSocket)
		, networkevents(arg.networkevents)
		, doreset(arg.doreset)
	{
	}
	
	HANDLE event;
	HANDLE descriptor;
	SOCKET sockfd;
	bool isSocket;
	bool doreset;
	long networkevents;
		
	bool operator<(const Select_t& s1) const
	{
		//we can not use the fields 'sockfd' or 'descriptor' because it's possible
		//the comparison of objects with different types - SOCKET and HANDLE
		//but the field 'event' is presented for both types
		return (event < s1.event);
	}
	
	bool operator==(const Select_t& s1) const
	{
		return (event == s1.event);
	}
};
#else
typedef int Select_t;
#endif

} // end namespace BLOCXX_NAMESPACE

#if defined(BLOCXX_WIN32) || defined(BLOCXX_NCR)
typedef int socklen_t;
#else
#ifndef BLOCXX_HAVE_SOCKLEN_T
typedef unsigned socklen_t;
#endif
#endif


#endif
