/*******************************************************************************
* Copyright (C) 2010, Quest Software, Inc. All rights reserved.
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
 * @author Kevin Harris
 * @author Alexander Furmanov
 */


#if      !defined (BLOCXX_TEST_REROOT_FILE_SYSTEM_IMPL_HPP_INCLUDE_GUARD_)
#define            BLOCXX_TEST_REROOT_FILE_SYSTEM_IMPL_HPP_INCLUDE_GUARD_

// This file is a non-public implementation detaul.
#include "blocxx_test/CannedFileSystem.hpp"

namespace BLOCXX_NAMESPACE
{
	namespace Test
	{
		namespace RerootFSImpl
		{
			/**
			 * Create mock object which prefixes all pathes in FileSystem interface
			 * by newRoot. It allows to place all interesting files in test specific
			 * directory and work with them as with normal files. Since code dealing
			 * with files uses AutoDescriptor, File and these are value types they
			 * are hard to replace by mockable objects.
			 */
			FSMockObjectRef createRerootedFSObject(blocxx::String const& newRoot);

			/**
			 * Add a remapping for a file.  Any file accessed as <path> will be
			 * translated to the real path <realpath> instead of following the
			 * normal reroooted behavior.  Yes, this is kind of a stretch for the
			 * original intention of this mock object, but it adds great
			 * flexibility.  It still retains the ability to do all real file
			 * actions on the files.
			 *
			 * Note: Remapping a file does NOT mean that the parent directories of
			 * the virtual path will be virtualized.  Only the path itself and
			 * subdirectories thereof will be valid.
			 *
			 * Be VERY careful about what you use for a real path here.
			 * File/directory deletion and modification is possible.
			 */
			bool remapFileName(const FSMockObjectRef& obj, const blocxx::String& path, const blocxx::String& realpath);
		} // end namespace Impl
	} // end namespace Test
} // end namespace BLOCXX_NAMESPACE
#endif /* !defined(BLOCXX_TEST_REROOT_FILE_SYSTEM_IMPL_HPP_INCLUDE_GUARD_) */
