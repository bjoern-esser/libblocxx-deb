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
* @author Andrey Mukha
*/

#ifndef BLOCXX_WIN_EXEC_HPP_INCLUDE_GUARD_
#define BLOCXX_WIN_EXEC_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Exec.hpp"

namespace BLOCXX_NAMESPACE
{

/////////////////////////////////////////////////////////////////////////////
namespace WinExec
{

class WinStandardPreExec : public Exec::PreExec
{
public:
	WinStandardPreExec();

	/// @return true
	virtual bool keepStd(int d) const;

	virtual void call(pipe_pointer_t const pparr[]);

	STARTUPINFO getStartUpInfo() { return m_startInfo; }

	void setStartUpInfo(pipe_pointer_t const pparr[]);

protected:
	STARTUPINFO m_startInfo;
};

class WinSystemPreExec : public Exec::PreExec
{
public:

	WinSystemPreExec() : PreExec(true) { }

	virtual bool keepStd(int d) const { return true; }

	virtual void call(pipe_pointer_t const pparr[]) { }
};

ProcessRef spawnImpl(char const * exec_path, char const * const argv[], char const * const envp[],
					 Exec::PreExec & pre_exec);

} // end WinExec namespace

} // end BLOCXX_NAMESPACE

#endif
