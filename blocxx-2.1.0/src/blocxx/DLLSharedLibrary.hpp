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
 * @author Jon Carey
 */

#ifndef BLOCXX_DLL_SHAREDLIBRARY_HPP_INCLUDE_GUARD_
#define BLOCXX_DLL_SHAREDLIBRARY_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#if defined(BLOCXX_USE_DLL)
#include "blocxx/SharedLibrary.hpp"

#include "blocxx/Types.hpp"

// The classes and functions defined in this file are not meant for general
// use, they are internal implementation details.  They may change at any time.

namespace BLOCXX_NAMESPACE
{

/**
 * DLLSharedLibrary loads and queries shared libraries. Using LoadLibrary,
 * GetProcAddress & friends. This class is not intended for general
 * consumption. It provides an abstraction layer over the Win32 shared
 * library interface for the SharedLibraryLoader class.
 */
class BLOCXX_COMMON_API DLLSharedLibrary : public SharedLibrary
{
public:
	DLLSharedLibrary(HINSTANCE libhandle, const String& libName);
	virtual ~DLLSharedLibrary();

protected:
	/**
	 * Derived classes have to override this function to implement
	 * the symbol loading.  The symbol to be looked up is contained in
	 * functionName, and the pointer to the function should be written
	 * into *fp.  Return true if the function succeeded, false otherwise.
	 * @param functionName	The name of the function to resolve.
	 * @param fp				Where to store the function pointer.
	 * @return true if function succeeded, false otherwise.
	 */
	virtual bool doGetFunctionPointer( const String& functionName,
		void** fp ) const;

private:
	HINSTANCE m_libhandle;
	String m_libName;
};

} // end namespace BLOCXX_NAMESPACE

#endif // BLOCXX_USE_DLL
#endif
