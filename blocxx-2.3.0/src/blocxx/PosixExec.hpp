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
* @author Dan Nuffer
*/

#ifndef BLOCXX_POSIX_EXEC_HPP_INCLUDE_GUARD_
#define BLOCXX_POSIX_EXEC_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Exec.hpp"

namespace BLOCXX_NAMESPACE
{

/////////////////////////////////////////////////////////////////////////////
namespace PosixExec
{

class StandardPreExec : public Exec::PreExec
{
public:
	StandardPreExec(bool daemonize = false);

	/// @return true
	//
	virtual bool keepStd(int d) const;

	/**
	* Resets all signals to their default actions and sets to
	* close-on-exec all descriptors except the standard descriptors.
	*/
	virtual void call(pipe_pointer_t const pparr[]);
};

class SystemPreExec : public Exec::PreExec
{
public:

	SystemPreExec(bool daemonize = false);

	/// @return true
	//
	virtual bool keepStd(int d) const ;

	/**
	* Resets all signals to their default actions and sets to
	* close-on-exec all descriptors except the standard descriptors.
	*/
	virtual void call(pipe_pointer_t const pparr[]);
};

ProcessRef spawnImpl(char const * exec_path, char const * const argv[], char const * const envp[],
					 Exec::PreExec & pre_exec);

} // end PosixExec namespace

} // end BLOCXX_NAMESPACE

#endif
