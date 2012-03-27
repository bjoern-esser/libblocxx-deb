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

#ifndef BLOCXX_IFILE_STREAM_HPP_INCLUDE_GUARD_
#define BLOCXX_IFILE_STREAM_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"

#include "blocxx/CommonFwd.hpp"
#include "blocxx/FileBuf.hpp"
#include <istream>
#include <cstdio>

namespace BLOCXX_NAMESPACE
{

/**
 * The purpose of this class is to provide an alternative implementation of std::ifstream that is based on a FILE*,
 * and can be created using a FILE*, a raw file descriptor or an instance of blocxx::File.
 *
 * IFileStream does not support all the operations that a std::ifstream does. IFileStream does not support seeking,
 * character putback, facets, i18n. Basically all it does support is reading.
 */
class IFileStream : private FileBuf, public std::istream
{
public:

    /**
     * Constructor.
     * @postcondition isOpen() == false
	 */
	IFileStream();

	/**
     * Calls close().
	 */
	~IFileStream();

	/**
     * @return bool: true if a previous call to open succeeded (returned a non-null value) and there has been no
     *         intervening call to close.
	 */
	bool isOpen() const;

    /**
     * If isOpen() == true, returns 0, otherwise initializes the IFileStream to use fp.
     * @param fp The FILE* to use. The IFileStream will take ownership of fp, so close() will call std::fclose() on fp.
	 *
     * @return IFileStream*: this if successful, 0 otherwise.
	 */
	IFileStream* open(FILE* fp);

	/**
     * If isOpen() == true, returns 0, otherwise initializes the IFileStream to use fd.
     * @param fd The descriptor to use. The IFileStream will take ownership of fd, so close() will close the descriptor.
	 *
     * @return IFileStream*: this if successful, 0 otherwise.
	 */
	IFileStream* open(AutoDescriptor fd);
#if 0
	/**
     * If isOpen() == true, returns 0, otherwise initializes the IFileStream to use f.
     * @param f The File to use. The IFileStream will not take ownership of f or f's underlying descriptor.
	 *
     * @return IFileStream*: this if successful, 0 otherwise.
	 */
	IFileStream* open(const File& f);
#endif
	/**
     * If isOpen() == true, returns 0, otherwise initializes the IFileStream by opening the file identified by path. If the
     * open operation succeeds and (mode & ios_base::ate) != 0, positions the file to the end. If the repositioning
     * operation fails, calls close() and returns a null pointer to indicate failure.
     *
     * @param path A null-terminated string identifying the file to open.
     * @param mode A bitmask of the file open mode flags defined in ios_base. Invalid combinations cause the operation
     *             to fail.
     * @param permissions Specifies the permissions to use in case a new file is created. It is modified by the
     *                    process's umask in the usual way: the permissions of the created file are (mode
     *                    & ~umask).  Note that this mode only applies to future accesses of the newly
     *                    created file; the open call that creates a read-only file may well return a
     *                    read/write file descriptor.
	 *
     * @return IFileStream*: this if successful, 0 otherwise.
	 */
	IFileStream* open(const char* path, std::ios_base::openmode mode, mode_t permissions = 0666);

    /**
     * If isOpen() == false, returns a null pointer. If a put area exists, calls overflow(EOF) to flush characters.
     * Finally it closes the file. If any of the calls to overflow or std::fclose fails then close fails.
     *
     * @post isOpen() == false.
     *
     * @return IFileStream*: this if successful, 0 otherwise.
	 */
	IFileStream* close();

private:

	// prohibit copying and assigning
	// NO IMPLEMENTATION
	IFileStream(const IFileStream& arg);
	IFileStream& operator=(const IFileStream& arg);

};

} // end namespace BLOCXX_NAMESPACE

#endif

