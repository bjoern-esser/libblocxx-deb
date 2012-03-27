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
 * @author Dan Nuffer
 */

#ifndef BLOCXX_UUID_HPP_INCLUDE_GUARD
#define BLOCXX_UUID_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/String.hpp"
#include "blocxx/Exception.hpp"

namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_APIEXCEPTION(UUID, BLOCXX_COMMON_API);
/**
 * UUIDs (Universally Unique IDentifiers), also known as GUIDs (Globally Unique 
 * IDentifiers), are 128 bits long. It is either guaranteed to be different
 * from all other UUIDs/GUIDs generated until 3400 A.D. or extremely
 * likely to be different.
 * sizeof(UUID) == 16.  The internal data is stored in network byte order, 
 * independent of host byte order.
 *
 * This implementation based off of IETF internet draft:
 *   draft-leach-uuids-guids-01.txt
 * That has expired, so if you want a real spec, see:
 *   DCE: Remote Procedure Call, Open Group CAE Specification C309
 *    ISBN 1-85912-041-5 28cm. 674p. pbk. 1,655g. 8/94
 */
class BLOCXX_COMMON_API UUID
{
public:
	/**
	 * Create a new UUID.  Version 1, based on MAC address (or random if not 
	 * available) and time+counter.
	 */
	UUID();
	/**
	 * Construct a UUID from string representation.
	 * The formal definition of the UUID string representation is provided
	 * by the following extended BNF:
	 *
	 @verbatim
	  UUID                   = <time_low> "-" <time_mid> "-"
	                           <time_high_and_version> "-"
	                           <clock_seq_and_reserved>
	                           <clock_seq_low> "-" <node>
	  time_low               = 4*<hexOctet>
	  time_mid               = 2*<hexOctet>
	  time_high_and_version  = 2*<hexOctet>
	  clock_seq_and_reserved = <hexOctet>
	  clock_seq_low          = <hexOctet>
	  node                   = 6*<hexOctet>
	  hexOctet               = <hexDigit> <hexDigit>
	  hexDigit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
	           | "a" | "b" | "c" | "d" | "e" | "f"
	           | "A" | "B" | "C" | "D" | "E" | "F"
	 @endverbatim
	 *
	 * The following is an example of the string representation of a UUID:
	 *       f81d4fae-7dec-11d0-a765-00a0c91e6bf6
	 * @throw UUIDException if uuidStr isn't valid.
	 */
	UUID(const String& uuidStr);
	/**
	 * Get the string representation of this UUID.
	 *  The following is an example of the string representation of a UUID:
	 *       f81d4fae-7dec-11d0-a765-00a0c91e6bf6
	 */
	String toString() const;
private:
	unsigned char m_uuid[16];
	friend BLOCXX_COMMON_API bool operator==(const UUID& x, const UUID& y);
	friend BLOCXX_COMMON_API bool operator<(const UUID& x, const UUID& y);
};
BLOCXX_COMMON_API bool operator!=(const UUID& x, const UUID& y);

} // end namespace BLOCXX_NAMESPACE

#endif
