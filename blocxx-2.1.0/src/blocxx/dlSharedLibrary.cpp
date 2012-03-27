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
#if defined(BLOCXX_USE_DL)
#include "blocxx/dlSharedLibrary.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/Mutex.hpp"
#include "blocxx/MutexLock.hpp"
#include "blocxx/GlobalMutex.hpp"
#include <dlfcn.h>
#ifdef BLOCXX_HAVE_FCNTL_H
// For O_RDONLY
#include <fcntl.h>
#endif

#include "blocxx/Types.hpp"

#if defined(BLOCXX_USE_FAKE_LIBS)
/////////////////////////////////////////////////////////////////////////////
#include "blocxx/FileSystem.hpp"
#include "blocxx/File.hpp"
#include "blocxx/Array.hpp"

#define BLOCXX_FAKELIB_HEADING "FAKE"
#define BLOCXX_FAKELIB_HEADING_LENGTH 4

#endif

#include <iostream>

namespace BLOCXX_NAMESPACE
{

// static
bool dlSharedLibrary::s_call_dlclose = true;

GlobalMutex dlSharedLibrary_guard = BLOCXX_GLOBAL_MUTEX_INIT();

dlSharedLibrary::dlSharedLibrary(void * libhandle, const String& libName)
	: SharedLibrary(), m_libhandle( libhandle ), m_libName(libName)
{
#if defined(BLOCXX_USE_FAKE_LIBS)
	// Find out if it is a fake library.
	m_fakeLibrary = dlSharedLibrary::isFakeLibrary(libName);

	if ( m_fakeLibrary )
	{
		initializeSymbolMap();
	}
#endif /* defined(BLOCXX_USE_FAKE_LIBS) */
}
  
dlSharedLibrary::~dlSharedLibrary()
{
#if !defined(BLOCXX_VALGRIND_SUPPORT) // dlclose()ing shared libs make it impossible to see where memory leaks occurred with valgrind.
	if (s_call_dlclose)
	{
		dlclose( m_libhandle );
	}
#endif
}
bool dlSharedLibrary::doGetFunctionPointer(const String& functionName,
		void** fp) const
{
	MutexLock l(dlSharedLibrary_guard);
#if defined(BLOCXX_USE_FAKE_LIBS)
	String realFunctionName = functionName;
	// If this is a fake library, extract convert the requested function
	// name into the proper function name for the main executable.
	if ( m_fakeLibrary )
	{
		Map<String,String>::const_iterator symIter = m_symbolMap.find(functionName);
		if ( symIter == m_symbolMap.end() )
		{
			return false;
		}
		realFunctionName = symIter->second;
	}
	*fp = dlsym( m_libhandle, realFunctionName.c_str() );
#else
	*fp = dlsym( m_libhandle, functionName.c_str() );
#endif /* defined(BLOCXX_USE_FAKE_LIBS) */
	  
	if (!*fp)
	{
		return false;
	}
	return true;
}

bool dlSharedLibrary::isFakeLibrary(const String& library_path)
{
	bool fake = false;
#if defined(BLOCXX_USE_FAKE_LIBS)
	if ( FileSystem::canRead(library_path) )
	{
		// Read the beginning of the file and see if it
		// contains the fake library heading.
		int libfd = open(library_path.c_str(), O_RDONLY);

		if ( libfd )
		{
			char buffer[(BLOCXX_FAKELIB_HEADING_LENGTH) + 1];
			size_t num_read = read(libfd, buffer,(BLOCXX_FAKELIB_HEADING_LENGTH));
			if ( num_read == (BLOCXX_FAKELIB_HEADING_LENGTH) )
			{
				// Null terminate it.
				buffer[BLOCXX_FAKELIB_HEADING_LENGTH] = '\0';
				if ( String(BLOCXX_FAKELIB_HEADING) == buffer )
				{
					// Yes, it's a fake library.
					fake = true;
				}
			}
			close(libfd);
		}
	}
#endif
	return fake;
}

#if defined(BLOCXX_USE_FAKE_LIBS)
void dlSharedLibrary::initializeSymbolMap()
{
	if ( ! m_fakeLibrary )
	{
		return;
	}
	// Read the contents of the file to find out what the function names
	// (normally available from dlsym) are mapped to functions in the main
	// program. 
	StringArray lines = FileSystem::getFileLines(m_libName);

	for ( StringArray::const_iterator iter = lines.begin();
		iter != lines.end();
		++iter )
	{
		// Skip commented lines.  
		if ( iter->startsWith('#') )
		{
			continue;
		}
		StringArray current_line = iter->tokenize("=");
		// Skip invalid lines.
		if ( current_line.size() != 2 )
		{
			continue;
		}
		// Add the data into the map.
		String option = String(current_line[0]).trim();
		String value = String(current_line[1]).trim();
		m_symbolMap[option] = value;
	}      
}
#endif /* defined(BLOCXX_USE_FAKE_LIBS) */

} // end namespace BLOCXX_NAMESPACE

#endif

