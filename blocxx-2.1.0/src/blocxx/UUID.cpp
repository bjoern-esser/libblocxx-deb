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

#include "blocxx/BLOCXX_config.h"
#include "blocxx/UUID.hpp"
#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"
#include "blocxx/Types.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/SecureRand.hpp"
#include "blocxx/ExceptionIds.hpp"

#if !defined(BLOCXX_WIN32)
#include <sys/time.h> // for gettimeofday
#endif

#ifdef BLOCXX_WIN32
#include <winsock2.h> // for timeval
#endif

#include <string.h> // for memcmp
#include <stdlib.h> // for rand
#include <ctype.h> // for isxdigit

namespace BLOCXX_NAMESPACE
{

BLOCXX_DEFINE_EXCEPTION_WITH_ID(UUID);

namespace {
// typedefs
typedef UInt64 uuid_time_t;
struct uuid_node_t 
{
	unsigned char nodeId[6];
};
struct uuid_state
{
	uuid_time_t timestamp;
	uuid_node_t nodeId;
	UInt16 clockSequence;
};
// static generator state
uuid_state g_state;
NonRecursiveMutex g_guard;

#ifdef BLOCXX_WIN32

// FILETIME of Jan 1 1970 00:00:00.
static const unsigned __int64 epoch = 116444736000000000L;

int gettimeofday(struct timeval * tp, int bogusParm)
{
	FILETIME	file_time;
	SYSTEMTIME	system_time;
	ULARGE_INTEGER ularge;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	ularge.LowPart = file_time.dwLowDateTime;
	ularge.HighPart = file_time.dwHighDateTime;

	tp->tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
	tp->tv_usec = (long) (system_time.wMilliseconds * 1000);

	return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////
void getSystemTime(uuid_time_t *uuid_time)
{
	struct timeval tp;
	gettimeofday(&tp, 0);
	// Offset between UUID formatted times and Unix formatted times.
	// UUID UTC base time is October 15, 1582.
	// Unix base time is January 1, 1970.
	*uuid_time = 
		(static_cast<unsigned long long>(tp.tv_sec) * 10000000) + 
		(tp.tv_usec * 10) +
		((static_cast<unsigned long long>(0x01B21DD2)) << 32) + 
		0x13814000;
}
/////////////////////////////////////////////////////////////////////////////
// these globals are protected by the mutex locked in UUID::UUID()
uuid_time_t timeLast;
UInt16 uuidsThisTick;
bool currentTimeInited = false;
void getCurrentTime(uuid_time_t * timestamp) 
{
	uuid_time_t timeNow;
	if (!currentTimeInited) 
	{
		getSystemTime(&timeLast);
		uuidsThisTick = 0;
		currentTimeInited = true;
	}
	getSystemTime(&timeNow);
	if (timeLast != timeNow) 
	{
		uuidsThisTick = 0;
		timeLast = timeNow;
	}
	else
	{
		uuidsThisTick++;
	}
	// add the count of uuids to low order bits of the clock reading
	*timestamp = timeNow + uuidsThisTick;
}
/////////////////////////////////////////////////////////////////////////////
// these globals are protected by the mutex locked in UUID::UUID()
unsigned char nodeId[6];
bool nodeIdInitDone = false;
void getNodeIdentifier(uuid_node_t *node) 
{
	// If we ever get a portable (or ported) method of acquiring the MAC
	// address, we should use that.  Until then, we'll just use random
	// numbers.
	if (!nodeIdInitDone)
	{
		Secure::rand(nodeId, sizeof(nodeId));
		// Set multicast bit, to prevent conflicts with MAC addressses.
		nodeId[0] |= 0x80;
		nodeIdInitDone = true;
	}
	memcpy(node->nodeId, nodeId, sizeof(node->nodeId));
}
/////////////////////////////////////////////////////////////////////////////
inline unsigned char decodeHex(char c)
{
	if (isdigit(c))
	{
		return c - '0';
	}
	else
	{
		c = toupper(c);
		return c - 'A' + 0xA;
	}
}
/////////////////////////////////////////////////////////////////////////////
inline unsigned char fromHexStr(char c1, char c2, const String& uuidStr)
{
	if (!isxdigit(c1) || !isxdigit(c2))
	{
		BLOCXX_THROW(UUIDException, Format("Invalid UUID: %1", uuidStr).c_str());
	}
	return (decodeHex(c1) << 4) | decodeHex(c2);
}
/////////////////////////////////////////////////////////////////////////////
inline char toHexHi(unsigned char c)
{
	unsigned char t = c >> 4;
	return t >= 10 ? t - 10 + 'a' : t + '0';
}
/////////////////////////////////////////////////////////////////////////////
inline char toHexLow(unsigned char c)
{
	unsigned char t = c & 0xF;
	return t >= 10 ? t - 10 + 'a' : t + '0';
}
} // end anonymous namespace 
/////////////////////////////////////////////////////////////////////////////
UUID::UUID()
{
	NonRecursiveMutexLock l(g_guard);
	uuid_time_t timestamp;
	getCurrentTime(&timestamp);
	uuid_node_t node;
	getNodeIdentifier(&node);
	uuid_time_t last_time = g_state.timestamp;
	UInt16 clockseq = g_state.clockSequence;
	uuid_node_t last_node = g_state.nodeId;
	// If clock went backwards (can happen if system clock resolution is low), change clockseq
	if (timestamp < last_time)
	{
		++clockseq;
	}
	// save the state for next time
	g_state.timestamp = last_time;
	g_state.clockSequence = clockseq;
	g_state.nodeId = last_node;
	l.release();
	// stuff fields into the UUID
	// do time_low 
	UInt32 tmp = static_cast<UInt32>(timestamp & 0xFFFFFFFF);
	m_uuid[3] = static_cast<UInt8>(tmp);
	tmp >>= 8;
	m_uuid[2] = static_cast<UInt8>(tmp);
	tmp >>= 8;
	m_uuid[1] = static_cast<UInt8>(tmp);
	tmp >>= 8;
	m_uuid[0] = static_cast<UInt8>(tmp);
	// do time_mid
	tmp = static_cast<UInt16>((timestamp >> 32) & 0xFFFF);
	m_uuid[5] = static_cast<UInt8>(tmp);
	tmp >>= 8;
	m_uuid[4] = static_cast<UInt8>(tmp);
	// do time_hi_and_version
	tmp = static_cast<UInt16>(((timestamp >> 48) & 0x0FFF) | (1 << 12));
	m_uuid[7] = static_cast<UInt8>(tmp);
	tmp >>= 8;
	m_uuid[6] = static_cast<UInt8>(tmp);
	// do clk_seq_low
	tmp = clockseq & 0xFF;
	m_uuid[9] = static_cast<UInt8>(tmp);
	// do clk_seq_hi_res
	tmp = (clockseq & 0x3F00) >> 8 | 0x80;
	m_uuid[8] = static_cast<UInt8>(tmp);
	memcpy(m_uuid+10, &node, 6);
}
/////////////////////////////////////////////////////////////////////////////
UUID::UUID(const String& uuidStr)
{
	const char* s = uuidStr.c_str();
	if (uuidStr.length() != 36 || s[8] != '-' || s[13] != '-' || s[18] != '-' || s[23] != '-')
	{
		BLOCXX_THROW(UUIDException, Format("Invalid UUID: %1", uuidStr).c_str());
	}
	m_uuid[0] = fromHexStr(s[0], s[1], uuidStr);
	m_uuid[1] = fromHexStr(s[2], s[3], uuidStr);
	m_uuid[2] = fromHexStr(s[4], s[5], uuidStr);
	m_uuid[3] = fromHexStr(s[6], s[7], uuidStr);
	m_uuid[4] = fromHexStr(s[9], s[10], uuidStr);
	m_uuid[5] = fromHexStr(s[11], s[12], uuidStr);
	m_uuid[6] = fromHexStr(s[14], s[15], uuidStr);
	m_uuid[7] = fromHexStr(s[16], s[17], uuidStr);
	m_uuid[8] = fromHexStr(s[19], s[20], uuidStr);
	m_uuid[9] = fromHexStr(s[21], s[22], uuidStr);
	m_uuid[10] = fromHexStr(s[24], s[25], uuidStr);
	m_uuid[11] = fromHexStr(s[26], s[27], uuidStr);
	m_uuid[12] = fromHexStr(s[28], s[29], uuidStr);
	m_uuid[13] = fromHexStr(s[30], s[31], uuidStr);
	m_uuid[14] = fromHexStr(s[32], s[33], uuidStr);
	m_uuid[15] = fromHexStr(s[34], s[35], uuidStr);
}
/////////////////////////////////////////////////////////////////////////////
String 
UUID::toString() const
{
	// This will return a string like this: 
	// 6ba7b810-9dad-11d1-80b4-00c04fd430c8
	const size_t uuidlen = 37;
	char* buf = new char[uuidlen];
	buf[0] = toHexHi(m_uuid[0]); buf[1] = toHexLow(m_uuid[0]);
	buf[2] = toHexHi(m_uuid[1]); buf[3] = toHexLow(m_uuid[1]);
	buf[4] = toHexHi(m_uuid[2]); buf[5] = toHexLow(m_uuid[2]);
	buf[6] = toHexHi(m_uuid[3]); buf[7] = toHexLow(m_uuid[3]);
	buf[8] = '-';
	buf[9] = toHexHi(m_uuid[4]); buf[10] = toHexLow(m_uuid[4]);
	buf[11] = toHexHi(m_uuid[5]); buf[12] = toHexLow(m_uuid[5]);
	buf[13] = '-';
	buf[14] = toHexHi(m_uuid[6]); buf[15] = toHexLow(m_uuid[6]);
	buf[16] = toHexHi(m_uuid[7]); buf[17] = toHexLow(m_uuid[7]);
	buf[18] = '-';
	buf[19] = toHexHi(m_uuid[8]); buf[20] = toHexLow(m_uuid[8]);
	buf[21] = toHexHi(m_uuid[9]); buf[22] = toHexLow(m_uuid[9]);
	buf[23] = '-';
	buf[24] = toHexHi(m_uuid[10]); buf[25] = toHexLow(m_uuid[10]);
	buf[26] = toHexHi(m_uuid[11]); buf[27] = toHexLow(m_uuid[11]);
	buf[28] = toHexHi(m_uuid[12]); buf[29] = toHexLow(m_uuid[12]);
	buf[30] = toHexHi(m_uuid[13]); buf[31] = toHexLow(m_uuid[13]);
	buf[32] = toHexHi(m_uuid[14]); buf[33] = toHexLow(m_uuid[14]);
	buf[34] = toHexHi(m_uuid[15]); buf[35] = toHexLow(m_uuid[15]);
	buf[36] = '\0';

	return String(String::E_TAKE_OWNERSHIP, buf, uuidlen-1);
}
/////////////////////////////////////////////////////////////////////////////
bool operator==(const UUID& x, const UUID& y)
{
	return memcmp(x.m_uuid, y.m_uuid, sizeof(x.m_uuid)) == 0;
}
/////////////////////////////////////////////////////////////////////////////
bool operator<(const UUID& x, const UUID& y)
{
	return memcmp(x.m_uuid, y.m_uuid, sizeof(x.m_uuid)) < 0;
}
/////////////////////////////////////////////////////////////////////////////
bool operator!=(const UUID& x, const UUID& y)
{
	return !(x == y);
}

} // end namespace BLOCXX_NAMESPACE

