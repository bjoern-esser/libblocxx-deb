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
 * @author Kevin S. Van Horn
 */

#ifndef BLOCXX_FILESYSTEM_HPP_INCLUDE_GUARD_
#define BLOCXX_FILESYSTEM_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Types.hpp"
#include "blocxx/ArrayFwd.hpp"
#include "blocxx/Exception.hpp"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/String.hpp"
#ifdef BLOCXX_ENABLE_TEST_HOOKS
#include "blocxx/GlobalPtr.hpp"
#endif

#include <utility>

#ifdef BLOCXX_HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifndef MAXPATHLEN
#ifdef PATH_MAX
#define MAXPATHLEN PATH_MAX
#else
#define MAXPATHLEN 1024
#endif
#endif

namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_APIEXCEPTION(FileSystem, BLOCXX_COMMON_API)

/**
 * The purpose of the FileSystem class is to provide an abstraction layer
 * over the platform dependant functionality related to a file system.
 */
namespace FileSystem
{
	/**
	 * Open a file for read/write and return an File object that can be used
	 * for reading and writing.
	 */
	BLOCXX_COMMON_API File openFile(const String& path);
	/**
	 * Create the file for the given name.
	 * @param path	The name of the file to create.
	 * @return On success an File object that can be used for reading and
	 * writing. Otherwise a NULL File object.  If the file exists, a NULL
	 * File object will be returned.
	 */
	BLOCXX_COMMON_API File createFile(const String& path);
	/**
	 * Opens or creates the file for the given name.
	 * @param path	The name of the file to create.
	 * @return On success an File object that can be used for reading and
	 * writing. Otherwise a null File object.
	 */
	BLOCXX_COMMON_API File openOrCreateFile(const String& path);
	/**
	 * Opens the file for the given name to append data or create if it does not exist.
	 * @param path	The name of the file to create.
	 * @return On success an File object that can be used for reading and
	 * writing. Otherwise a null File object.
	 */
	BLOCXX_COMMON_API File openForAppendOrCreateFile(const String& path);
	/**
	 * Create a tempororary file that will be removed when the returned
	 * File object is closed.
	 * @param dir The directory to create the temporary file in.
	 * @return On success a File object that can be used for reading and
	 * writing. Otherwise a null File object. The underlying file represented
	 * by this File object will be deleted when the file is closed.
	 */
	BLOCXX_COMMON_API File createAutoDeleteTempFile(const String& dir=String());
	/**
	 * Create a tempororary file in an optional directory.
	 * @param filePath Output parameter that will contain the name of the 
	 * 		temporary file on return. It is the responsibility of the caller
	 * 		to delete this file when it is no longer needed.
	 * @param dir The directory to create the temporary file in.
	 * @return On success a File object that can be used for reading and
	 * writing. Otherwise a null File object. The underlying file represented
	 * by this File object will still remain on the file system after it is
	 * closed. The caller is responsible for removing it.
	 */
	BLOCXX_COMMON_API File createTempFile(String& filePath,
		const String& dir=String());
	/**
	 * Change the given file ownership
	 * @param filename The name of the file to change ownership on.
	 * @param userId The user id to change ownership to
	 * @return 0 on success. Otherwise -1
	 */
	BLOCXX_COMMON_API int changeFileOwner(const String& filename,
		const UserId& userId);
	/**
	 * @return true if the file exists (and false otherwise).
	 */
	BLOCXX_COMMON_API bool exists(const String& path);
#ifndef BLOCXX_WIN32
	/**
	 * Tests if a file is executable
	 *
	 * This method is not available on platforms that do not have
	 * support for executable file attributes
	 * @return true if the file exists and is executable (and false
	 * otherwise).
	 */
	BLOCXX_COMMON_API bool isExecutable(const String& path);
#endif
	/**
	 * @return true if the file exists and can be read
	 */
	BLOCXX_COMMON_API bool canRead(const String& path);
	/**
	 * @return true if the file exists and can be written
	 */
	BLOCXX_COMMON_API bool canWrite(const String& path);
#ifndef BLOCXX_WIN32
	/**
	 * Tests if a file is a symbolic link
	 *
	 * This method is not available on platforms that do not have
	 * support for symbolic links
	 *
	 * @return true if file exists and is a symbolic link
	 */
	BLOCXX_COMMON_API bool isLink(const String& path);
#endif
	/**
	 * @return true if file exists and is a directory
	 */
	BLOCXX_COMMON_API bool isDirectory(const String& path);
	/**
	 * Change to the given directory
	 * @param path	The directory to change to
	 * @return true if the operation succeeds. Otherwise false.
	 */
	BLOCXX_COMMON_API bool changeDirectory(const String& path);
	/**
	 * Create a directory
	 * @param path	The name of the directory to create.
	 * @param mode specifies the permissions to use.
	 * @return true if the operation succeeds. Otherwise false.
	 */
#ifndef BLOCXX_WIN32
	BLOCXX_COMMON_API bool makeDirectory(const String& path, int mode=0777);
#else
	BLOCXX_COMMON_API bool makeDirectory(const String& path, int mode=-1);
#endif
	/**
	 * Get the size of the file in bytes
	 * @param path	The name of the file to get the size for.
	 * @param size	Put the size of the file in this variable.
	 * @return true if the operation succeeds. Otherwise false.
	 */
	BLOCXX_COMMON_API bool getFileSize(const String& path, Int64& size);
	/**
	* Get the size of a file from the file handle.
	* @param fh Handle of the desired file.
	* @return The size of the file.
	*/
	BLOCXX_COMMON_API UInt64 fileSize(FileHandle fh);
	/**
	 * Remove the given directory
	 * @param path	The name of the directory to remove
	 * @return true if the operation succeeds. Otherwise false.
	 */
	BLOCXX_COMMON_API bool removeDirectory(const String& path);
	/**
	 * Remove the given file
	 * @param path	The name of the file to remove.
	 * @return true if the operation succeeds. Otherwise false.
	 */
	BLOCXX_COMMON_API bool removeFile(const String& path);
	/**
	 * Get the names of the files (and directories) in the given directory
	 * @param path			The name of the directory to get the contents of.
	 * @param dirEntries	The directory contents will be placed in this array.
	 * @return true if the operation succeeds. Otherwise false.
	 */
	BLOCXX_COMMON_API bool getDirectoryContents(const String& path,
		StringArray& dirEntries);
	/**
	 * Rename the given file to the new name
	 * @param oldFileName	The name of the file to rename
	 * @param newFileName	The new name for the oldFileName
	 * @return true if the operation succeeds. Otherwise false.
	 */
	BLOCXX_COMMON_API bool renameFile(const String& oldFileName,
		const String& newFileName);
	/**
	 * Read data from file.
	 * @param hdl				The file handle to perform the read operation on.
	 * @param bfr				The location where the read operation will place
	 *								what is read.
	 * @param numberOfBytes	The number of bytes to read.
	 * @param offset			The offset to seek to in the file before the read
	 *								operation is done. -1 will use the current offset.
	 * @return The number of bytes read. If EOF or an error occurs, a short
	 * count or size_t(-1) is returned.
	 */
	BLOCXX_COMMON_API size_t read(const FileHandle& hdl, void* bfr, size_t numberOfBytes,
		Int64 offset=-1L);
	/**
	 * Write data to a file.
	 * @param hdl				The file handle to perform the write operation on.
	 * @param bfr				The locaction to get the contents to write.
	 * @param numberOfBytes	The number of bytes to write.
	 * @param offset			The offset to seek to in the file before the write
	 *								operation is done. -1 will use the current offset.
	 * @return The number of bytes written. If an error occurs, a short count
	 * or size_t(-1) is returned.
	 */
	BLOCXX_COMMON_API size_t write(FileHandle hdl, const void* bfr,
		size_t numberOfBytes, Int64 offset=-1L);
	/**
	 * Seek to a given offset within the file.
	 * @param hdl			The file handle to use in the seek operation.
	 * @param offset		The offset to seek to relative to the whence parm.
	 * @param whence		Can be one of the follwing values:
	 *							SEEK_SET - Seek relative to the beginning of the file.
	 *							SEEK_CUR - Seek relative to the current position.
	 *							SEEK_END - Seek relative to the end of the file (bwd).
	 * @return The the current location in the file relative to the beginning
	 * of the file on success. Other -1.
	 */
	BLOCXX_COMMON_API Int64 seek(const FileHandle& hdl, Int64 offset, int whence);
	/**
	 * @param hdl	The file handle to use in the tell operation.
	 * @return The current position in the file relative to the beginning of
	 * the file on success. Otherwise -1.
	 */
	BLOCXX_COMMON_API Int64 tell(const FileHandle& hdl);
	/**
	 * Position the file pointer associated with the given file handle to the
	 * beginning of the file.
	 * @param hdl	The file handle to use in the rewind operation.
	 */
	BLOCXX_COMMON_API void rewind(const FileHandle& hdl);
	/**
	 * Close file handle.
	 * @param hdl	The file handle to close.
	 * @return 0 on success. Otherwise -1.
	 */
	BLOCXX_COMMON_API int close(const FileHandle& hdl);
	/**
	 * Flush any buffered data to the file if buffering supported.
	 * @param hdl	The file handle to flush the buffer on.
	 */
	BLOCXX_COMMON_API int flush(FileHandle& hdl);
	/**
	 * Read and return the contents of a text file.  If the file contains 
	 * a null character ('\\0') then only previous data will be returned.
	 * @param filename The name of the file to read
	 * @exception FileSystemException if the file doesn't exist or reading 
	 *  fails for any reason.
	 */
	BLOCXX_COMMON_API String getFileContents(const String& filename);
	
	/**
	 * Read and return the lines of a test file. If the file contains a null
	 * character ('\\0') then only previous data will be returned.
	 * @param filename The name of the file to read
	 * @exception FileSystemException if the file doesn't exist or reading 
	 *  fails for any reason.
	 */
	BLOCXX_COMMON_API StringArray getFileLines(const String& filename);

	/**
	 * Read the value of a symbolic link
	 * @param path Path to the symbolic link
	 * @return The target of the symbolic link
	 * @exception FileSystemException: ENOTDIR, ENOENT, EACCES, ELOOP, EINVAL, 
	 *  EIO, EFAULT, ENOMEM
	 */
	BLOCXX_COMMON_API String readSymbolicLink(const String& path);

	namespace Path
	{
		/**
		* @return The canonical path specifying the same directory or file as
		* @a path.  A path is in canonical form iff
		* - it is an absolute path,
		* - no component is ".", "..", nor a symbolic link,
		* - it does not contain repeated '/' characters, and
		* - the last character is not '/' unless the entire path is "/".
		*
		* If @a path is relative, it will be interpreted relative to the
		* current working directory. This function is similar to the SuSv3
		* function, however it's easier to use and thread safe.
		*
		* @param path The path to canonicalize.
		*
		* @pre No path component examined in the course of resolving @a path
		* to its canonical form is renamed, deleted, or (for symbolic links)
		* reassigned by some other thread or process while the function
		* executes.
		*
		* @throws FileSystemException EACCESS, EIO, ELOOP, ENOENT, ENOTDIR
		*/
		BLOCXX_COMMON_API String realPath(const String& path);

		enum ESecurity
		{
			E_INSECURE, E_SECURE_DIR, E_SECURE_FILE
		};

		/**
		* @return A pair (@a sec, @a rpath), as follows:
		* - @a rpath = @c realpath(@a path).
		* - If @a path names a directory, and no user other than @c root
		*   or user @a uid can change the contents of this directory or
		*   make @a path refer to some other file or directory, then
		*   @a sec = @c E_SECURE_DIR.
		* - If @a path names a regular file, and no user other than @c root
		*   or user @a uid can change the contents of this file or make
		*   @a path refer to some other file or directory, then
		*   @a sec = @c E_SECURE_FILE.
		* - If any user other than @c root or user @a uid can change what
		*   @a path refers to, or change the contents of the file or directory
		*   it refers to, then @a sec = @c E_INSECURE.
		*
		* @pre No path component examined in the course of resolving @a path to
		* its canonical form is renamed, deleted, or (for symbolic links)
		* reassigned by @c root or user @a uid while the function executes.
		*
		* @throws FileSystemException EACCESS, EIO, ELOOP, ENOENT, ENOTDIR
		*/
		BLOCXX_COMMON_API std::pair<ESecurity, String>
			security(String const & path, UserId uid);

		/**
		* Equivalent to @c security(@a path, @a uid), where @a uid is the
		* effective user ID of the process.
		*/
		BLOCXX_COMMON_API std::pair<ESecurity, String> security(String const & path);

		/**
		* A variant of @c security() that is more efficient if some
		* ancestor directory of the path is already known to be secure and
		* in canonical form.
		*
		* @pre @a base_dir is a path in canonical form, as described for
		* @c realPath(), and @a rel_path is a relative path.
		*
		* @return @c security(@a path, @a uid), where @a path is the
		* catenation of @a base_dir, "/", and @a rel_path, under the assumption
		* that @a base_dir is a secure directory.
		*/
		BLOCXX_COMMON_API std::pair<ESecurity, String>
			security(String const & base_dir, String const & rel_path, UserId uid);

		/**
		* Equivalent to @c security(@a base_dir, @a rel_path, @a uid), 
		* where @a uid is the effective user ID of the process.
		*/
		BLOCXX_COMMON_API std::pair<ESecurity, String>
			security(String const & base_dir, String const & rel_path);

		/**
		 * Take a string that contains a pathname, and return a string that is
		 * a pathname of the parent directory of that file. Trailing '/'
		 * characters in the path are not counted as part of the path.
		 * If path does not contain a '/', then dirname() shall return the
		 * string ".". If path an empty string, dirname() shall return the
		 * string ".".
		 * @param filename The file pathname
		 * @return The pathname of the parent directory of filename.
		 */
		BLOCXX_COMMON_API String dirname(const String& filename);

		/**
		 * Take a string that contains a pathname, and return a string 
		 * that is the filename with the path removed.
		 * @param filename The file pathname
		 * @return The filename with the path removed
		 */
		BLOCXX_COMMON_API String basename(const String& filename);

		/**
		 * Get the process's current working directory.
		 * Calls to chdir() or fchdir() will modify this. Multi-threaded 
		 * applications must exercise caution changing the current working 
		 * directory.
		 * @throws FileSystemException: ENOENT-The current working directory 
		 *  has been unlinked.
		 */
		BLOCXX_COMMON_API String getCurrentWorkingDirectory();


	} // end namespace Path

	struct NullFactory
	{
		static void* create()
		{
			return 0;
		}
	};
#ifdef BLOCXX_ENABLE_TEST_HOOKS
	typedef GlobalPtr<FileSystemMockObject, NullFactory> FileSystemMockObject_t;
	/** 
	 * If this object is non-null, the default functionality of the 
	 * FileSystem class will be replaced by calls to
	 * g_fileSystemMockObject's member functions. This is to be used 
	 * for unit tests. Not all functions may be
	 * implemented, if you need one that isn't, then please implement it! 
	 * Modifying this variable will affect all
	 * threads, it should not be used in a threaded program.
	 */
	extern FileSystemMockObject_t g_fileSystemMockObject;
#endif

} // end namespace FileSystem

} // end namespace BLOCXX_NAMESPACE

#endif
