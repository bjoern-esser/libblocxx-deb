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
 * @author Jon Carey
 * @author Dan Nuffer
 */

#ifndef BLOCXX_FILE_HPP_INCLUDE_GUARD_
#define BLOCXX_FILE_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Types.hpp"
#include "blocxx/FileSystem.hpp"
#include "blocxx/SafeBool.hpp"
#include <algorithm> // for std::swap

namespace BLOCXX_NAMESPACE
{

/**
 * The purpose of the File class is to provide an abstraction layer
 * over the platform dependant functionality related to a file.
 */
class BLOCXX_COMMON_API File
{
public:
	enum ELockType
	{
		/**
		 * Flag to place a read / shared lock.
		 * More than one process may hold a shared lock for
		 * a given file at a given time.
		 */
		E_READ_LOCK,
		/**
		 * Flag to place an write / exclusive lock.
		 * Only one process may hold an exclusive lock for
		 * a given file at a given time.
		 */
		E_WRITE_LOCK
	};

	/**
	 * Create a NULL File object.
	 */
	File() : m_hdl(BLOCXX_INVALID_FILEHANDLE)
	{
	}
	/**
	 * Copy constructor
	 * @param x	The File object to copy.
	 */
	File(const File& x);
	/**
	 * Destructor.
	 */
	~File();
	/**
	 * Assignment operator
	 * @param x	The File object to copy.
	 * @return A reference to this File object.
	 */
	File& operator= (const File& x)
	{
		File(x).swap(*this);
		return *this;
	}
	void swap(File& x)
	{
		std::swap(m_hdl, x.m_hdl);
	}
	/**
	 * Read from the underlying file.
	 * @param bfr           The location where the read operation will
	 *                      place what is read.
	 * @param numberOfBytes The number of bytes to read.
	 * @param offset        The offset to seek to in the file before the
	 *                      read operation is done or -1 to use the
	 *                      current offset.
	 * @return The number of bytes read. If an error occurs, size_t(-1)
	 *         is returned and 0 on EOF.
	 */
	size_t read(void* bfr, size_t numberOfBytes, Int64 offset=-1L) const
	{
		return FileSystem::read(m_hdl, bfr, numberOfBytes, offset);
	}
	/**
	 * Write to the underlying file
	 * @param bfr           The locaction to get the contents to write.
	 * @param numberOfBytes The number of bytes to write.
	 * @param offset        The offset to seek to in the file before the
	 *                      write operation is done or -1 to use the
	 *                      current offset.
	 * @return The number of bytes written. If an error occurs,
	 *         size_t(-1) is returned.
	 */
	size_t write(const void* bfr, size_t numberOfBytes, Int64 offset=-1L)
	{
		return FileSystem::write(m_hdl, bfr, numberOfBytes, offset);
	}
	/**
	 * Seek to a given offset within the file.
	 * @param offset  The offset to seek to relative to the whence parm.
	 * @param whence  Can be one of the follwing values:
	 *                SEEK_SET - Seek relative to the beginning of the file.
	 *                SEEK_CUR - Seek relative to the current position.
	 *                SEEK_END - Seek relative to the end of the file (bwd).
	 * @return The current location in the file relative to the beginning
	 * of the file on success. Other -1.
	 */
	Int64 seek(Int64 offset, int whence) const
	{
		return FileSystem::seek(m_hdl, offset, whence);
	}
	/**
	 * @return The current position in the file relative to the beginning
	 *         of the file on success. Otherwise -1.
	 */
	Int64 tell() const
	{
		return FileSystem::tell(m_hdl);
	}
	/**
	 * Position the file pointer to the beginning of the file.
	 */
	void rewind() const
	{
		FileSystem::rewind(m_hdl);
	}
	/**
	* Current size of file
	*/
	UInt64 size() const
	{
		return FileSystem::fileSize(m_hdl);
	}
	/**
	 * Close the underlying file object.
	 * @return 0 on success. Otherwise -1.
	 */
	int close()
	{
		if (m_hdl != BLOCXX_INVALID_FILEHANDLE)
		{
			int rv = FileSystem::close(m_hdl);
			m_hdl = BLOCXX_INVALID_FILEHANDLE;
			return rv;
		}
		return 0;
	}
	/**
	 * Flush any buffered data to the file.
	 * @return 0 on success. Otherwise -1.
	 */
	int flush()
	{
		return FileSystem::flush(m_hdl);
	}
	/**
	 * Acquire a kernel lock on the file. This call may block.
	 *
	 * The lock may be released by calling unlock().
	 * The lock will also be released on when the file is closed or when
	 * locking application exits.
	 * This is only an advisory lock and will not prevent non-coordinated
	 * access to the file.
	 *
	 * @param type Whether to acquire a read (shared) or a write
	 *             (exclusive) lock. Default is a write lock.
	 * @return 0 on sucess. On error, -1 is returned, and errno is set
	 *        appropriately. errno may be one of: EBADF, EDEADLK, EINVAL,
	 *        ENOLCK.
	 */
	int getLock(ELockType type = E_WRITE_LOCK);
	/**
	 * Acquire a kernel lock on the file. This call won't block.
	 *
	 * The lock may be released by calling unlock().
	 * The lock will also be released when the locking application exits.
	 * This is only an advisory lock and will not prevent non-coordinated
	 * access to the file.
	 *
	 * @param type Whether to try to acquire a read (shared) or a write
	 *             (exclusive) lock. Default is a write lock.
	 * @return 0 on sucess.  On error, -1 is returned, and errno is set
	 *         appropriately. errno may be one of: EACCES, EAGAIN, EBADF,
	 *         EDEADLK, EINVAL, ENOLCK. EACCES or EAGAIN signifies the
	 *         file is already locked.
	 */
	int tryLock(ELockType type = E_WRITE_LOCK);
	/**
	 * Release a lock on the file. This call will not block.
	 * @return 0 on sucess. On error, -1 is returned, and errno is set
	 *         appropriately. errno may be one of: EAGAIN, EBADF, EDEADLK,
	 *         EINVAL, ENOLCK.
	 */
	int unlock();

	/**
	 * @return true if this is a valid File object.
	 */
	BLOCXX_SAFE_BOOL_IMPL(File, FileHandle, File::m_hdl, (m_hdl != BLOCXX_INVALID_FILEHANDLE))

	/**
	 * Equality operator.
	 * @param rhs The File object to compare this object to.
	 * @return true if this File represents the same file as rhs.
	 */
	bool operator==(const File& rhs)
	{
		return m_hdl == rhs.m_hdl;
	}

	File(FileHandle hdl) : m_hdl(hdl)
	{
	}

private:

	FileHandle	m_hdl;
};

} // end namespace BLOCXX_NAMESPACE

#endif

