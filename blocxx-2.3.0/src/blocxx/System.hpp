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

#ifndef BLOCXX_SYSTEM_HPP_GUARD_INCLUDE_GUARD_
#define BLOCXX_SYSTEM_HPP_GUARD_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/String.hpp"
#include "blocxx/Types.hpp"

// The classes and functions defined in this file are not meant for general
// use, they are internal implementation details.  They may change at any time.

namespace BLOCXX_NAMESPACE
{
/**
 * @todo Fix the name of this. System is horrible!
 * @todo Fix the implementation of this. It isn't thread safe on some platforms.
 */
namespace System
{

BLOCXX_COMMON_API String errorMsg(int errorCode);
BLOCXX_COMMON_API UInt32 lastErrorMsg(const String &a_action, String &a_errorMessage);
BLOCXX_COMMON_API String lastErrorMsg(bool socketError=false);

}	// End of System namespace
}	// End of BLOCXX_NAMESPACE

#endif	// BLOCXX_SYSTEM_HPP_GUARD_INCLUDE_GUARD_

