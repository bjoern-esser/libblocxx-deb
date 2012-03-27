/*******************************************************************************
* Copyright (C) 2005 Novell, Inc. All rights reserved.
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
*  - Neither the name of Quest Software, Inc., Novell, Inc., nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Quest Software, Inc., Novell, Inc., OR THE
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
/**
 * @author Marius Tomaschewski
 */

#ifndef   BLOCXX_REGEX_EXCEPTION_HPP_INCLUDE_GUARD_HPP_
#define   BLOCXX_REGEX_EXCEPTION_HPP_INCLUDE_GUARD_HPP_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Exception.hpp"

namespace BLOCXX_NAMESPACE
{

/**
 * Compilation exception.
 *
 * The RegExCompileException is thrown on regex compilation
 * failure, the try to execute a not compiled regex or if the
 * compiled regex is not compatible to the operation, e.g. if
 * a non-capturing regex (REG_NOSUB flag set) if used in a
 * method that requires capturing.
 *
 */
BLOCXX_DECLARE_APIEXCEPTION(RegExCompile, BLOCXX_COMMON_API);


/**
 * Execution exception.
 *
 * The RegExExecuteException is thrown on execution failures
 * in methods executing a regex internally.
 */
BLOCXX_DECLARE_APIEXCEPTION(RegExExecute, BLOCXX_COMMON_API);


} // End of BLOCXX_NAMESPACE

#endif // BLOCXX_REGEX_EXCEPTION_HPP_INCLUDE_GUARD_HPP_

