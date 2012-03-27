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

#ifndef BLOCXX_SocketADDRESS_HPP_INCLUDE_GUARD_
#define BLOCXX_SocketADDRESS_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Array.hpp"
#include "blocxx/Types.hpp"
#include "blocxx/NetworkTypes.hpp"
#include "blocxx/String.hpp"
#include "blocxx/Exception.hpp"

namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_APIEXCEPTION(UnknownHost, BLOCXX_COMMON_API);
BLOCXX_DECLARE_APIEXCEPTION(SocketAddress, BLOCXX_COMMON_API);
// TODO: This class needs to be rewritten to take into account the fact that a given hostname may have multiple 
// ip addresses, and a given ip address may have multiple hostnames & aliases..
class BLOCXX_COMMON_API SocketAddress
{
public:
	static const char* const ALL_LOCAL_ADDRESSES;

	enum AddressType
	{
		UNSET,
		INET,
		UDS
	};
	AddressType getType() const { return m_type; }
	/**
	 * Do a DNS lookup on a hostname and return an SocketAddress for that host
	 *
	 * @param host The hostname
	 * @param port The port
	 *
	 * @return An SocketAddress for the host and port
	 * @throws UnknownHostException
	 */
	static SocketAddress getByName(const String& host, unsigned short port = 0);
	/**
	 * Do a DNS lookup on a hostname and return a list of all addresses
	 * that map to that hostname.
	 *
	 * @param host The hostname
	 * @param port The port
	 *
	 * @return An Array of SocketAddresses for the host and port
	 */

	/**
	 * Get an SocketAddress appropriate for referring to the local host
	 * @param port The port
	 * @return An SocketAddress representing the local machine
	 */
	static SocketAddress getAnyLocalHost(UInt16 port = 0);
	/**
	 * Allocate an empty SocketAddress.
	 * @return an empty address
	 */
	static SocketAddress allocEmptyAddress(AddressType type);
	/**
	 * Get the port associated with the address
	 * @return The port
	 */
	UInt16 getPort() const;
	~SocketAddress() {}
	/**
	 * Returns the hostname (FQDN) of the address.
	 *
	 * @return The hostname of the address.
	 */
	const String getName() const;
	/**
	 * Returns the IP address of the host
	 *
	 * @return The IP address of the host
	 */
	const String getAddress() const;
	const SocketAddress_t* getNativeForm() const;
	size_t getNativeFormSize() const;

	/**
	 * Get a pointer to the InetSocketAddress_t
	 * precondition: getType() == INET
	 */
	const InetSocketAddress_t* getInetAddress() const;

#if !defined(BLOCXX_WIN32)
	//static Array<SocketAddress> getAllByName(const String& hostName,
	//		unsigned short port = 0);
	static SocketAddress getUDS(const String& filename);

	/**
	 * Get a pointer to the UnixSocketAddress_t
	 * precondition: getType() == UDS
	 */
	const UnixSocketAddress_t* getUnixAddress() const;

	void assignFromNativeForm(const UnixSocketAddress_t* address, size_t len);
#endif

	/**
	 * Returns the IP address and the port with a colon in between.
	 *
	 * @return The IP and port seperated by a colon.
	 */
	const String toString() const;
	void assignFromNativeForm(const InetSocketAddress_t* address, size_t len);

	SocketAddress();
private:
	SocketAddress(const InetSocketAddress_t& nativeForm);

#if !defined(BLOCXX_WIN32)
	SocketAddress(const UnixSocketAddress_t& nativeForm);
#endif

	String m_name;
	String m_address;
	size_t m_nativeSize;
	
	InetSocketAddress_t m_inetNativeAddress;

#if !defined(BLOCXX_WIN32)
	UnixSocketAddress_t m_UDSNativeAddress;
#endif

	AddressType m_type;
	static SocketAddress getFromNativeForm(const InetAddress_t& nativeForm,
			UInt16 nativePort, const String& hostname );
	static SocketAddress getFromNativeForm(
		const InetSocketAddress_t& nativeForm);

#if !defined(BLOCXX_WIN32)
	static SocketAddress getFromNativeForm(
		const UnixSocketAddress_t& nativeForm);
#endif

#ifdef BLOCXX_WIN32
const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt); 
#endif

};

} // end namespace BLOCXX_NAMESPACE

#endif
