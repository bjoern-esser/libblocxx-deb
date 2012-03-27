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

#ifndef BLOCXX_SHARED_LIBRARY_LOADER_HPP_INCLUDE_GUARD_
#define BLOCXX_SHARED_LIBRARY_LOADER_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/SharedLibrary.hpp"
#include "blocxx/IntrusiveReference.hpp"
#include "blocxx/IntrusiveCountableBase.hpp"
#include "blocxx/CommonFwd.hpp"

namespace BLOCXX_NAMESPACE
{

/**
 * SharedLibraryLoader is the base class for a platform class for loading
 * shared libraries.
 */
class BLOCXX_COMMON_API SharedLibraryLoader : public IntrusiveCountableBase
{
public:
	virtual ~SharedLibraryLoader();
	/**
	 * Load a shared library specified by filename.  If the operation fails,
	 * the return value will be null ref counted pointer, and
	 * BLOCXX_LOG_ERROR(logger, ) will be called to report the details of the error.
	 * Exception safety: Strong
	 * @param filename The name of the shared library to load.
	 * @return SharedLibraryRef owning representing the shared library
	 *  identified by filename.  NULL on failure.
	 */
	virtual SharedLibraryRef loadSharedLibrary(const String& filename) const = 0;

	/**
	 * @return A reference to an SharedLibraryLoader object.
	 *
	 * Note: The implementation of createSharedLibraryLoader is contained
	 * in the platforms specific source file.  Only one type of
	 * SharedLibraryLoader exists for a given system.  The build system selects
	 * the correct one to build.
	 */
	static SharedLibraryLoaderRef createSharedLibraryLoader();
};
BLOCXX_EXPORT_TEMPLATE(BLOCXX_COMMON_API, IntrusiveReference, SharedLibraryLoader);

} // end namespace BLOCXX_NAMESPACE

#endif
