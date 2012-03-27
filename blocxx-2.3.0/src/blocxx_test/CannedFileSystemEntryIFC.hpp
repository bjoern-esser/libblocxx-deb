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
 */


#if      !defined (BLOCXX_TEST_CANNED_FILE_SYSTEM_ENTRY_IFC_HPP_INCLUDE_GUARD_)
#define            BLOCXX_TEST_CANNED_FILE_SYSTEM_ENTRY_IFC_HPP_INCLUDE_GUARD_

/// This file is just the interface.  You probably want to use a "normal file"
/// from CannedFileSystemNormalFile.hpp instead.

#include "blocxx/BLOCXX_config.h"

#include "blocxx/String.hpp"
#include "blocxx/IntrusiveCountableBase.hpp"
#include "blocxx/IntrusiveReference.hpp"
#include "blocxx/FileSystem.hpp"

namespace BLOCXX_NAMESPACE
{
	namespace Test
	{
		// Typedefs/using statements to keep existing code functional and simplify stuff.
		typedef blocxx::FileSystem::FileInformation::EFileType FileEntryTypes;
		using blocxx::FileSystem::FileInformation;

		/// A simple interface for a filesystem entry.
		/// Users are ignored for file operations.  This is intended for
		/// simple file-based operations such as faking a directory tree with
		/// readable (or read/write) files.
		///
		/// You only need to derive from this class if you need to have callbacks
		/// or do logging of specific accesses, etc.  For normal use, use the
		/// "normal file".
		class FileSystemEntry : public blocxx::IntrusiveCountableBase
		{
		public:
			FileSystemEntry();
			virtual ~FileSystemEntry();

			// Functions to get the status of a file.
			virtual bool readable() const = 0;
			virtual bool writeable() const = 0;
			virtual bool executable() const = 0;
			virtual FileInformation getFileInfo() const = 0;

			// Get the path of your file.  The underlying implementation should
			// not change this path unless instructed to by a call to setPath().
			// That means that consecutive calls to this function for the same
			// object should return the same value.
			virtual blocxx::String path() const = 0;

			// Get the type of file that this entry represents.
			virtual FileEntryTypes getFileType() const = 0;

			// Get the contents of the file.  The contents should not normally
			// change unless setContents is called.  This function is used in
			// emulating a "read" from the file, so if multiple reads (each used
			// to obtain a small chunk of the file) are done, it should remain
			// consistent througout.
			virtual blocxx::String contents() const = 0;

			// Change the path of the file (rename).  This should only be called
			// by the canned filesystem mock object so it can preserve its
			// internal state.
			virtual bool setPath(const blocxx::String & location) = 0;

			// Change the contents of the file (such as by a write).
			virtual bool setContents(const blocxx::String & contents) = 0;
		};

		typedef blocxx::IntrusiveReference<FileSystemEntry> FSEntryRef;

	} // end namespace Test
} // end namespace BLOCXX_NAMESPACE
#endif /* !defined(BLOCXX_TEST_CANNED_FILE_SYSTEM_ENTRY_IFC_HPP_INCLUDE_GUARD_) */
