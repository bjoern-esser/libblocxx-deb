/*******************************************************************************
* Copyright (C) 2001-2004 Novell, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
*  - Neither the name of Novell, Inc. nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Novell, Inc. OR THE CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/**
 * @author Jon Carey
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/System.hpp"
#include <cstring>
#include <cerrno>

#ifdef BLOCXX_WIN32
#include <winsock2.h>
#endif

namespace BLOCXX_NAMESPACE
{
namespace System
{

#ifdef BLOCXX_WIN32
//////////////////////////////////////////////////////////////////////////////
String errorMsg(int errorCode)
{
	LPVOID lpMsgBuf;
	if (!::FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER
					| FORMAT_MESSAGE_FROM_SYSTEM
					| FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				errorCode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL))
	{
		return String();
	}

	String rmsg((const char*)lpMsgBuf);

	// Free the buffer.
	::LocalFree(lpMsgBuf);
	return rmsg;
}

///////////////////////////////////////////////////////////////////////////////////
UInt32 lastErrorMsg(const String &a_action, String &a_errorMessage) 
{ 
	DWORD errcode = GetLastError(); 
	a_errorMessage.format("%s%s%ld%s%s", a_action.c_str(), " failed with error ", errcode, " : ", errorMsg(errcode).c_str());

	return errcode;
}

//////////////////////////////////////////////////////////////////////////////
String lastErrorMsg(bool socketError)
{
	DWORD errcode = (socketError) ? WSAGetLastError() : GetLastError();
	return errorMsg(errcode);
}
#else
String errorMsg(int errorCode)
{
	return String(::strerror(errorCode));
}
//////////////////////////////////////////////////////////////////////////////
String lastErrorMsg(bool socketError)
{
	return errorMsg(errno);
}
#endif

}	// End of System namespace
}	// End of BLOCXX_NAMESPACE

