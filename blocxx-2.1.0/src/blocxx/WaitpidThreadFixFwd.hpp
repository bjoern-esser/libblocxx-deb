/*******************************************************************************
* Copyright (C) 2005, Quest Software, Inc. All rights reserved.
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
*       Quest Software, Inc., 
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
 * @author Mat Bess
 */

#ifndef BLOCXX_EXECIMPL_HPP_INCLUDE_GUARD_2006AUG25
#define BLOCXX_EXECIMPL_HPP_INCLUDE_GUARD_2006AUG25
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Process.hpp"
#include "blocxx/Exec.hpp"
#include "blocxx/Cstr.hpp"


namespace BLOCXX_NAMESPACE
{
	Process::Status pollStatusImpl(ProcId pid);

	namespace Exec
	{
		ProcessRef spawnImpl(
			char const * exec_path,
			char const * const argv[], char const * const envp[],
			PreExec & pre_exec
		);

		/**
		 * Variant of @c spawnImpl that allows @a exec_path to have an arbitrary
		 * string-like type, and @a argv and @a envp to have arbitrary
		 * string-array-like types. If argv or envp are of type StringArray
		 * a terminating null is not necessary.
		 *
		 * @pre @a S is a type for which <tt>Cstr::to_char_ptr</tt> is defined.
		 *
		 * @pre Specializations of the <tt>Cstr::CstrArr</tt> class template are
		 * defined for both types @a SA1 and @a SA2.
		 */
		template <typename S, typename SA1, typename SA2>
		ProcessRef spawnImpl(
			S const & exec_path, SA1 const & argv, SA2 const & envp,
			PreExec & pre_exec
		)
		{
			Cstr::CstrArr<SA1> sa_argv(argv);
			Cstr::CstrArr<SA2> sa_envp(envp);
			char const * s_exec_path = Cstr::to_char_ptr(exec_path);
			return spawnImpl(s_exec_path, sa_argv.sarr, sa_envp.sarr, pre_exec);
		}

		/// Variant of spawnImpl that uses @c StandardPreExec.
		//
		ProcessRef spawnImpl(
			char const * const argv[], char const * const envp[]
		);

		/**
		* Variant of @c spawnImpl that uses @c StandardPreExec and
		* @a argv and @a envp to have arbitrary string-array-like types.
		*
		* @pre Specializations of the <tt>Cstr::CstrArr</tt> class template are
		* defined for both types @a SA1 and @a SA2.
		*/
		template <typename SA1, typename SA2>
		ProcessRef spawnImpl(
			SA1 const & argv, SA2 const & envp
		)
		{
			Cstr::CstrArr<SA1> sa_argv(argv);
			Cstr::CstrArr<SA2> sa_envp(envp);
			return spawnImpl(sa_argv.sarr, sa_envp.sarr);
		}

		template <typename SA1>
		ProcessRef spawnImpl(
			SA1 const & argv
		)
		{
			return spawnImpl(argv, Exec::currentEnvironment);
		}

	} // end namespace Exec

} // end namespace BLOCXX_NAMESPACE


#endif // BLOCXX_EXECIMPL_HPP_INCLUDE_GUARD_2006AUG25
