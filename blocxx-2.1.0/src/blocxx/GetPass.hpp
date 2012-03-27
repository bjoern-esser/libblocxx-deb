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

#ifndef BLOCXX_GETPASS_HPP_INCLUDE_GUARD_
#define BLOCXX_GETPASS_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/String.hpp"

// TODO: Move this into src/common.  It doesn't belong under src/common/socket.

namespace BLOCXX_NAMESPACE
{

namespace GetPass
{
	/**
	 * The  getpass  function  displays  a prompt to the standard
	 * error output, and reads in a password from  /dev/tty.   If
	 * this  file is not accessible, getpass reads from the stan­
	 * dard input.
	 *
	 * Getpass turns off character echoing and disables the  gen­
	 * eration of signals by tty special characters (interrupt by
	 * control-C, suspend by control-Z, etc.) while  reading  the
	 * password.                     
	 *
	 * This is implemented under the OS abstraction layer.
	 *
	 * @param prompt The prompt to be displayed
	 * @return the password
	 */
	BLOCXX_COMMON_API String getPass(const String& prompt);

} // end namespace GetPass

} // end namespace BLOCXX_NAMESPACE

namespace BLOCXX_GetPass = BLOCXX_NAMESPACE::GetPass;

#endif
	
