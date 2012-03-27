/*******************************************************************************
* Copyright (C) 2005, Quest Software, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Quest Software, Inc., nor the
*       names of its contributors or employees may be used to endorse or promote
*       products derived from this software without specific prior written
*       permission.
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
* @author Mat Bess
*/

#ifndef BLOCXX_WAITPID_THREAD_FIX_HPP_INCLUDE_GUARD_
#define BLOCXX_WAITPID_THREAD_FIX_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Types.hpp"
#include "blocxx/Process.hpp"

namespace BLOCXX_NAMESPACE
{

/*******************************************************************************
* Older linux threading libraries do not allow one thread to wait on the
* children of another thread. This class is used by Exec and Process to work
* around that limitation by having all calls to fork() and waitpid() done
* by the same thread when BLOCXX_WAITPID_THREADING_PROBLEM is defined.
*******************************************************************************/
namespace WaitpidThreadFix
{

	/**
	 * If a program is single threaded (like the monitor code is), then
	 * this function can be called to ensure that the fork/waitpid threading
	 * fix is or is not used (on the few platforms that its actually needed for).
	 * @return The previous setting.
	 */
	BLOCXX_COMMON_API bool setWaitpidThreadFixEnabled(bool enabled);

	/*******************************************************************************
	* Exec::spawn & Process::pollStatus will call this function
	* to determine if WaitpidThreadFix should be used.
	*******************************************************************************/
	BLOCXX_COMMON_API bool shouldUseWaitpidThreadFix();

	BLOCXX_COMMON_API ProcessRef spawnProcess(
		char const * exec_path,
		char const * const argv[],
		char const * const envp[],
		Exec::PreExec & pre_exec
	);

	BLOCXX_COMMON_API Process::Status waitPid(const ProcId& pid);

} // end namespace WaitpidThreadFix


} // namespace BLOCXX_NAMESPACE

#endif // BLOCXX_WAITPID_THREAD_FIX_HPP_INCLUDE_GUARD_

