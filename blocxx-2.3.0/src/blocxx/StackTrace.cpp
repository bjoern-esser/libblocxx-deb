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
 * @author Bart Whiteley
 * @author Dan Nuffer
 */
#include "blocxx/BLOCXX_config.h"
#include "blocxx/StackTrace.hpp"

#ifdef BLOCXX_WIN32
#include <iostream>	// for cerr
namespace BLOCXX_NAMESPACE
{
using std::cerr;
using std::endl;
void StackTrace::printStackTrace(EDoStackTraceFlag)
{
	cerr << "StackTrace::printStackTrace not implemented yet" << endl;
}
}
#else

#include "blocxx/Exec.hpp"
#include "blocxx/UnnamedPipe.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/StringBuffer.hpp"

#include <fstream>
#include <iostream>	// for cerr

#if defined(BLOCXX_HAVE_BACKTRACE)
#include <execinfo.h>
#endif

#if defined(BLOCXX_HAVE_CXXABI_H)
#include <cxxabi.h>
#endif

#ifdef BLOCXX_HAVE_UNISTD_H
extern "C"
{
#include <unistd.h> // for getpid()
}
#endif

namespace BLOCXX_NAMESPACE
{

using std::ifstream;
using std::ofstream;
using std::flush;

#ifndef BLOCXX_DEFAULT_GDB_PATH
#define BLOCXX_DEFAULT_GDB_PATH "/usr/bin/gdb"
#endif

// static
void StackTrace::printStackTrace(EDoStackTraceFlag doStackTrace)
{
	std::cerr << getStackTrace(doStackTrace);
}

String StackTrace::getStackTrace(EDoStackTraceFlag doStackTrace)
{

	if (doStackTrace == E_NO_CHECK_ENV_VAR || (doStackTrace == E_CHECK_ENV_VAR && getenv("BLOCXX_STACKTRACE")))
	{
		// if we have the GNU backtrace functions we use them.  They don't give
		// as good information as gdb does, but they are orders of magnitude
		// faster!
#ifdef BLOCXX_HAVE_BACKTRACE
		void *array[200];

		size_t size = backtrace (array, 200);
		char **strings = backtrace_symbols (array, size);

		StringBuffer bt;

		size_t i;
		for (i = 0; i < size; i++)
		{
#if defined(BLOCXX_HAVE_CXXABI_H)
			bt += strings[i];
			int status;
			// extract the identifier from strings[i].  It's inside of parens.
			char* firstparen = ::strchr(strings[i], '(');
			char* lastparen = ::strchr(strings[i], '+');
			if (firstparen != 0 && lastparen != 0 && firstparen < lastparen)
			{
				bt += ": ";
				*lastparen = '\0';
				char* realname = abi::__cxa_demangle(firstparen+1, 0, 0, &status);
				bt += realname;
				free(realname);
			}
#else
			bt += strings[i];
#endif
			bt += "\n";
		}

		free (strings);

		return bt.releaseString();
#else
		ifstream file(BLOCXX_DEFAULT_GDB_PATH);
		if (file)
		{
			file.close();
			String scriptName("/tmp/owgdb-");
			String outputName("/tmp/owgdbout-");
			/// @todo don't use getppid, get it from somewhere else!
			outputName += String(UInt32(::getpid()));
			scriptName += String(UInt32(::getpid())) + ".sh";
			String exeName("/proc/");
			exeName += String(UInt32(::getpid())) + "/exe";

			ofstream scriptFile(scriptName.c_str(), std::ios::out);
			scriptFile << "#!/bin/sh\n"
				<< "gdb " << exeName << " " << ::getpid() << " << EOS > " << outputName << " 2>&1\n"
// doesn't work with gdb 5.1				<< "thread apply all bt\n"
				<< "bt\n"
				<< "detach\n"
				<< "q\n"
				<< "EOS\n" << flush;
			scriptFile.close();
			Array<String> command;
			command.push_back( "/bin/sh" );
			command.push_back( scriptName );
			Exec::system(command);
			ifstream outputFile(outputName.c_str(), std::ios::in);
			StringBuffer output;
			while (outputFile)
			{
				output += String::getLine(outputFile);
				output += "\n";
			}
			outputFile.close();
			unlink(outputName.c_str());
			unlink(scriptName.c_str());
			return output.releaseString();
		}
#endif
	}
	return String();
}

} // end namespace BLOCXX_NAMESPACE

#endif	// ifdef BLOCXX_WIN32
