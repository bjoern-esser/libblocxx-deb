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
 * @author Anton Afanasiev - Win32 porting
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/PosixFileSystem.hpp"
#include "blocxx/FileSystem.hpp"
#include "blocxx/Mutex.hpp"
#include "blocxx/MutexLock.hpp"
#include "blocxx/GlobalMutex.hpp"
#include "blocxx/File.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/ExceptionIds.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/GlobalPtr.hpp"
#include "blocxx/FileSystemMockObject.hpp"
#include "blocxx/AutoPtr.hpp"
#include "blocxx/SafeCString.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/GlobalString.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/StaticAssert.hpp"
#include "blocxx/Types.hpp"


extern "C"
{
#ifdef BLOCXX_WIN32

	#include <direct.h>
	#include <io.h>
	#include <share.h>
	#include <AccCtrl.h.>
	#include <Aclapi.h>
	#include "blocxx/PathSecurity.hpp"
	using namespace BLOCXX_NAMESPACE;

//////////////////////////////////////////////////////////////////////////////
	static unsigned long MapPosixPermissionsMask( PACCESS_ALLOWED_ACE pAce, int PermissionMask )
	{
		pAce->Mask = 0;
		pAce->Mask |= ((PermissionMask & S_IROTH) == S_IROTH) ? BLOCXX_WIN32_ACCESSMASK_GENERIC_READ : 0;
		pAce->Mask |= ((PermissionMask & S_IWOTH) == S_IWOTH) ? BLOCXX_WIN32_ACCESSMASK_GENERIC_WRITE : 0;
		pAce->Mask |= ((PermissionMask & S_IXOTH) == S_IXOTH) ? BLOCXX_WIN32_ACCESSMASK_GENERIC_EXEC : 0;
		return pAce->Mask;
	}

//////////////////////////////////////////////////////////////////////////////
	/*
	 * A wrapper over POSIX chmod functionality for Windows
	 * It tries to set the appropriate permissions via discretionary access control list
	 * onto the path security descriptor associated.
	 */
	static int posix_chmod(const char* path, int mode)
	{
		int result, nLenghtNeeded;
		PSID ppOwnerSid = NULL, ppGroupSid = NULL, pSecurityDescriptor = NULL;
		PACL pAcl = NULL;
		if ( (result = GetNamedSecurityInfo( (LPTSTR)path,
										SE_FILE_OBJECT,
										DACL_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION,
										&ppOwnerSid,
										&ppGroupSid,
										&pAcl,
										NULL,
										&pSecurityDescriptor) ) )
		{
			return result;
		}
		/*
		* We got the NTFS security information and DACL associated to path specified.
		* Now we gonna change it according to UNIX access mode.
		*/
		for (unsigned short aceIdx = 0; aceIdx < pAcl->AceCount; aceIdx++)
		{
			ACE_HEADER* pAce;
			if (!::GetAce(pAcl, aceIdx, (void**)&pAce))
			{
				continue;
			}
			switch( pAce->AceType )
			{
				case ACCESS_ALLOWED_ACE_TYPE:
				{
					PACCESS_ALLOWED_ACE pAllowedAce = (PACCESS_ALLOWED_ACE) pAce;
					unsigned long sNameLen, sDNameLen = sNameLen = MAX_PATH;
					char sName[MAX_PATH] = {0}, sDName[MAX_PATH] = {0};
					SID_NAME_USE eUse;

					if ( !::LookupAccountSid( NULL, &(pAllowedAce->SidStart), sName, &sNameLen, sDName, &sDNameLen, &eUse) )
					{
						continue;
					}

					if ( EqualSid( ppOwnerSid, &pAllowedAce->SidStart ) || (eUse == SidTypeWellKnownGroup && !strcmp(sName, "CREATOR OWNER")) )
					{
						// modifying permissions for the object's owner
						int hundreds = mode / 100;
						MapPosixPermissionsMask( pAllowedAce, (hundreds - (hundreds/10)*10) );
						break;
					}
					if ( EqualSid( ppGroupSid, &pAllowedAce->SidStart ) || eUse == WinCreatorGroupSid )
					{
						// modifying permissions for the object's group
						int decimals = mode / 10;
						MapPosixPermissionsMask( pAllowedAce, (decimals - (decimals/10)*10) );
						break;
					}
					// modifying permissions for others
					MapPosixPermissionsMask( pAllowedAce, (mode - (mode/10)*10) );
				}
				break;

				case ACCESS_DENIED_ACE_TYPE:
				{
					/*
					* There is no need to change it, because the permission modes
					* that limitates security descriptor usage are set via allowed ACE
					*/
					DeleteAce(pAcl, aceIdx);
				}
				break;
			}
		}
		/*
		* All the operations with dACL and its ACEs were made in the shared memory.
		* That's why we don't need to make a copy of the new dACL, just apply the
		* changed dACL onto path security descriptor.
		*/
		result = SetNamedSecurityInfo((LPTSTR)path,
										SE_FILE_OBJECT,
										PROTECTED_DACL_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION,
										ppOwnerSid,
										ppGroupSid,
										pAcl,
										NULL);

		if (pSecurityDescriptor) LocalFree((HLOCAL)pSecurityDescriptor);

		return result;
	}

//////////////////////////////////////////////////////////////////////////////
	static int posix_mkdir(const char* path, int mode)
	{
		int result;
		if ( result = _mkdir(path) ) return result;
		/**
		 * We break the permissions inheritance using posix_chmod
		 * only if we need that (when mode is different than -1 (default)
		 * and set up manually for _MKDIR macros wrapper or other way)
		 */
		return ((mode!=-1) ? result = posix_chmod(path, mode) : result);
	}

//////////////////////////////////////////////////////////////////////////////
	#define _ACCESS ::_access
	#define R_OK 4
	#define F_OK 0
	#define W_OK 2
	#define _CHDIR _chdir
	#define _MKDIR(a,b)	posix_mkdir((a), (b))
	#define _RMDIR _rmdir
	#define _UNLINK _unlink

#else

	#ifdef BLOCXX_HAVE_UNISTD_H
	#include <unistd.h>
	#endif
	#ifdef BLOCXX_HAVE_DIRENT_H
	#include <dirent.h>
	#endif

	#define _ACCESS ::access
	#define _CHDIR chdir
	#define _MKDIR(a,b) mkdir((a),(b))
	#define _RMDIR rmdir
	#define _UNLINK unlink

#ifdef BLOCXX_NETWARE
#define MAXSYMLINKS 20
#endif

#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
}

#include <cstdio> // for rename
#include <fstream>
#include <cerrno>

namespace BLOCXX_NAMESPACE
{

BLOCXX_DEFINE_EXCEPTION_WITH_ID(FileSystem);

namespace FileSystem
{

#ifdef BLOCXX_INT64_IS_LONG_LONG
	const Int64 CURRENT_OFFSET = Int64(-1ll);
#else
	const Int64 CURRENT_OFFSET = Int64(-1l);
#endif

typedef GlobalPtr<FileSystemMockObject, NullFactory> FileSystemMockObject_t;
FileSystemMockObject_t g_fileSystemMockObject = BLOCXX_GLOBAL_PTR_INIT;

GlobalString COMPONENT_NAME = BLOCXX_GLOBAL_STRING_INIT("blocxx");

//////////////////////////////////////////////////////////////////////////////
// STATIC
int
changeFileOwner(const String& filename,
	const UserId& userId)
{
#ifdef BLOCXX_WIN32
	return 0;	// File ownership on Win32?
#else
	return ::chown(filename.c_str(), userId, gid_t(-1));
#endif
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
File
openFile(const String& path)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->openFile(path);
	}
#ifdef BLOCXX_WIN32
	HANDLE fh = ::CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);

	return (fh  != INVALID_HANDLE_VALUE) ? File(fh) : File();
#else
	return File(::open(path.c_str(), O_RDWR));
#endif
}
namespace
{
#ifdef BLOCXX_WIN32
	DWORD
	windowsOpenMode(EOpenModeFlag mode)
	{
		switch (mode)
		{
			case E_READ:
				return GENERIC_READ;

			case E_WRITE:
				return GENERIC_WRITE;

			case E_READWRITE:
				return GENERIC_READ | GENERIC_WRITE;

			default:
			{
				BLOCXX_ASSERTMSG( false, "Incorrect OpenMode value." );
				return GENERIC_READ;
			}

		}
	}
#else
	mode_t
	posixOpenMode(EOpenModeFlag mode)
	{
		switch (mode)
		{
			case E_READ:
				return O_RDONLY;

			case E_WRITE:
				return O_WRONLY;

			case E_READWRITE:
				return O_RDWR;

			default:
			{
				BLOCXX_ASSERTMSG( false, "Incorrect OpenMode value." );
				return O_RDONLY;
			}

		}
	}
#endif
} // end unnamed namespace
//////////////////////////////////////////////////////////////////////////////
// STATIC
File
openFile(const String& path, EOpenModeFlag mode)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->openFile(path, mode);
	}
#ifdef BLOCXX_WIN32
	HANDLE fh = ::CreateFile(path.c_str(), windowsOpenMode(mode),
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);

	return (fh  != INVALID_HANDLE_VALUE) ? File(fh) : File();
#else
	return File(::open(path.c_str(), posixOpenMode(mode)));
#endif
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
File
createFile(const String& path)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->createFile(path);
	}
#ifdef BLOCXX_WIN32
	HANDLE fh = ::CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL, NULL);
	return (fh  != INVALID_HANDLE_VALUE) ? File(fh) : File();
#else
	int fd = ::open(path.c_str(), O_CREAT | O_EXCL | O_TRUNC | O_RDWR, 0660);
	if (fd != -1)
	{
		return File(fd);
	}
	return File();
#endif

}
//////////////////////////////////////////////////////////////////////////////
// STATIC
File
openOrCreateFile(const String& path)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->openOrCreateFile(path);
	}
#ifdef BLOCXX_WIN32
	HANDLE fh = ::CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
	return (fh  != INVALID_HANDLE_VALUE) ? File(fh) : File();
#else
	return File(::open(path.c_str(), O_RDWR | O_CREAT, 0660));
#endif
}

//////////////////////////////////////////////////////////////////////////////
// STATIC
File
openForAppendOrCreateFile(const String& path)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->openForAppendOrCreateFile(path);
	}
#ifdef BLOCXX_WIN32
	//Exclude FILE_WRITE_DATA flag, because it truncates file if it exists
	//Add FILE_SHARE_DELETE flag, because file can be deleted (renamed) by another process
	HANDLE fh = ::CreateFile(path.c_str(), FILE_APPEND_DATA | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | STANDARD_RIGHTS_WRITE | SYNCHRONIZE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
	return (fh  != INVALID_HANDLE_VALUE) ? File(fh) : File();
#else
	return File(::open(path.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0660));
#endif
}

//////////////////////////////////////////////////////////////////////////////
namespace
{
GlobalMutex tmpfileMutex = BLOCXX_GLOBAL_MUTEX_INIT();
}

#ifndef BLOCXX_WIN32
//////////////////////////////////////////////////////////////////////////////
File
createTempFile(String& filePath, const String& dir)
{
	filePath.erase();
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->createTempFile(filePath, dir);
	}

	String sfname;
	if (dir.empty())
	{
		const char* envtmp = ::getenv("TMPDIR");
		if (!envtmp)
		{
			sfname = "/tmp/";
		}
		else
		{
			sfname = envtmp;
			if (!sfname.endsWith('/'))
				sfname += '/';
		}
	}
	else
	{
		sfname = (dir.endsWith('/')) ? dir : dir+"/";
	}

	sfname += "blocxxtmpfileXXXXXX";
	size_t len = sfname.length();

	AutoPtrVec<char> filename(new char[len + 1]);
	SafeCString::strcpy_check(filename.get(), len + 1, sfname.c_str());
	MutexLock tmpfileML(tmpfileMutex);
	int hdl = mkstemp(filename.get());
	if (hdl == -1)
	{
		return File();
	}
	filePath = filename.get();
	return File(hdl);
}

//////////////////////////////////////////////////////////////////////////////
File
createAutoDeleteTempFile(const String& dir)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->createTempFile(dir);
	}

	String sfname;
	if (dir.empty())
	{
		const char* envtmp = ::getenv("TMPDIR");
		if (!envtmp)
		{
			sfname = "/tmp/";
		}
		else
		{
			sfname = envtmp;
			if (!sfname.endsWith('/'))
				sfname += '/';
		}
	}
	else
	{
		sfname = (dir.endsWith('/')) ? dir : dir+"/";
	}

	sfname += "blocxxtmpfileXXXXXX";
	size_t len = sfname.length();
	AutoPtrVec<char> filename(new char[len + 1]);
	SafeCString::strcpy_check(filename.get(), len + 1, sfname.c_str());
	MutexLock tmpfileML(tmpfileMutex);
	int hdl = mkstemp(filename.get());
	if (hdl == -1)
	{
		return File();
	}
	else
	{
		if (::unlink(filename.get()) != 0)
		{
			Logger lgr(COMPONENT_NAME);
			BLOCXX_LOG_ERROR(lgr, Format("PosixFileSystem::createTempFile: unlink failed: %1", errno));
		}
	}
	return File(hdl);
}
#else
//////////////////////////////////////////////////////////////////////////////
File
createTempFile(String& filePath, const String& dir)
{
	filePath.erase();
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->createTempFile(filePath, dir);
	}

	int rc = 0 ;
	String sfname;
	if (dir.empty())
	{
		char envtmp[MAX_PATH];
		rc = ::GetTempPath(MAX_PATH, envtmp);
		if (rc == 0)
		{
			sfname = "c:/tmp/";
		}
		else
		{
			sfname = envtmp;
			if (!sfname.endsWith('/'))
				sfname += '/';
		}
	}
	else
	{
		sfname = (dir.endsWith('/')) ? dir : dir+"/";
	}

	char szTempName[MAX_PATH];
	// Create a temporary file name.
	rc = ::GetTempFileName(sfname.c_str(),	// directory for tmp files
				  							 "blocxxtmpfile", // temp file name prefix - The function uses the first three characters of this string as the prefix of the file name.
												 0,								// create unique name
												 szTempName);			// buffer for name
	if (rc == 0)
	{
		return File();
	}

	sfname = szTempName;
	size_t len = sfname.length();
	AutoPtrVec<char> filename(new char[len + 1]);
	SafeCString::strcpy_check(filename.get(), len + 1, sfname.c_str());
	MutexLock tmpfileML(tmpfileMutex);

	// Create the new file to write the upper-case version to.
	FileHandle hdl = ::CreateFile((LPTSTR)sfname.c_str(), // file name
																GENERIC_READ | GENERIC_WRITE, // open r-w
																0,                    // do not share
																NULL,                 // default security
																CREATE_ALWAYS,        // overwrite existing
																FILE_ATTRIBUTE_NORMAL,// normal file
																NULL);                // no template
	if (hdl == INVALID_HANDLE_VALUE)
	{
	  return File();
	}

	filePath = filename.get();
	return File(hdl);
}

File
createAutoDeleteTempFile(const String& dir)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->createTempFile(dir);
	}

	int rc = 0 ;
	String sfname;
	if (dir.empty())
	{
		char envtmp[MAX_PATH];
		rc = ::GetTempPath(MAX_PATH, envtmp);
		if (rc == 0)
		{
			sfname = "c:/tmp/";
		}
		else
		{
			sfname = envtmp;
			if (!sfname.endsWith('/'))
				sfname += '/';
		}
	}
	else
	{
		sfname = (dir.endsWith('/')) ? dir : dir+"/";
	}

	char szTempName[MAX_PATH];
	// Create a temporary file name.
	rc = ::GetTempFileName(sfname.c_str(),	// directory for tmp files
												 "blocxxtmpfile", // temp file name prefix - The function uses the first three characters of this string as the prefix of the file name.
												 0,								// create unique name
												 szTempName);			// buffer for name
	if (rc == 0)
	{
		return File();
	}

	sfname = szTempName;
	size_t len = sfname.length();
	AutoPtrVec<char> filename(new char[len + 1]);
	SafeCString::strcpy_check(filename.get(), len + 1, sfname.c_str());
	MutexLock tmpfileML(tmpfileMutex);

	// Create the new file to write the upper-case version to.
	FileHandle hdl = ::CreateFile((LPTSTR)sfname.c_str(), // file name
																GENERIC_READ | GENERIC_WRITE, // open r-w
																0,                    // do not share
																NULL,                 // default security
																CREATE_ALWAYS,        // overwrite existing
																FILE_ATTRIBUTE_NORMAL,// normal file
																NULL);                // no template
	if (hdl == INVALID_HANDLE_VALUE)
	{
		return File();
	}
	else
	{
		if (::unlink(filename.get()) != 0)
		{
			Logger lgr(COMPONENT_NAME);
			BLOCXX_LOG_ERROR(lgr, Format("PosixFileSystem::createTempFile: unlink failed: %1", errno));
		}
	}
	return File(hdl);
}
#endif

//////////////////////////////////////////////////////////////////////////////
bool
exists(const String& path)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->exists(path);
	}
	return _ACCESS(path.c_str(), F_OK) == 0;
}

//////////////////////////////////////////////////////////////////////////////
#ifndef BLOCXX_WIN32
bool
isExecutable(const String& path)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->isExecutable(path);
	}
	return _ACCESS(path.c_str(), X_OK) == 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////
bool
canRead(const String& path)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->canRead(path);
	}
	return _ACCESS(path.c_str(), R_OK) == 0;
}
//////////////////////////////////////////////////////////////////////////////
bool
canWrite(const String& path)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->canWrite(path);
	}
	return _ACCESS(path.c_str(), W_OK) == 0;
}
//////////////////////////////////////////////////////////////////////////////
#ifndef BLOCXX_WIN32
bool
isLink(const String& path)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->isLink(path);
	}
	struct stat st;
	if (lstat(path.c_str(), &st) != 0)
	{
		return false;
	}
	return S_ISLNK(st.st_mode);
}
#endif
//////////////////////////////////////////////////////////////////////////////
bool
isDirectory(const String& path)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->isDirectory(path);
	}
#ifdef BLOCXX_WIN32
	struct _stat st;
	if (_stat(path.c_str(), &st) != 0)
	{
		return false;
	}
	return ((st.st_mode & _S_IFDIR) != 0);
#else
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
	{
		return false;
	}
	return S_ISDIR(st.st_mode);
#endif
}
//////////////////////////////////////////////////////////////////////////////
bool
changeDirectory(const String& path)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->changeDirectory(path);
	}
	return _CHDIR(path.c_str()) == 0;
}
//////////////////////////////////////////////////////////////////////////////
bool
makeDirectory(const String& path, int mode)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->makeDirectory(path, mode);
	}
	return _MKDIR(path.c_str(), mode) == 0;
}
//////////////////////////////////////////////////////////////////////////////
bool
getFileSize(const String& path, Int64& size)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->getFileSize(path, size);
	}
#ifdef BLOCXX_WIN32
	struct _stat st;
	if (_stat(path.c_str(), &st) != 0)
	{
		return false;
	}
#else
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
	{
		return false;
	}
#endif
	size = st.st_size;
	return true;
}
//////////////////////////////////////////////////////////////////////////////
bool
removeDirectory(const String& path)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->removeDirectory(path);
	}
	return _RMDIR(path.c_str()) == 0;
}
//////////////////////////////////////////////////////////////////////////////
bool
removeFile(const String& path)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->removeFile(path);
	}
	return _UNLINK(path.c_str()) == 0;
}
//////////////////////////////////////////////////////////////////////////////
bool
getDirectoryContents(const String& path,
	StringArray& dirEntries)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->getDirectoryContents(path, dirEntries);
	}
	static Mutex readdirGuard;
	MutexLock lock(readdirGuard);

#ifdef BLOCXX_WIN32
	struct _finddata_t dentry;
	long hFile;
	String _path = path;

	// Find first directory entry
	if (!_path.endsWith(BLOCXX_FILENAME_SEPARATOR))
	{
		_path += BLOCXX_FILENAME_SEPARATOR;
	}
	_path += "*";
	if ((hFile = _findfirst( _path.c_str(), &dentry)) == -1L)
	{
		return false;
	}
	dirEntries.clear();
	while (_findnext(hFile, &dentry) == 0)
	{
		dirEntries.append(String(dentry.name));
	}
	_findclose(hFile);
#else
	DIR* dp(0);
	struct dirent* dentry(0);
	if ((dp = opendir(path.c_str())) == NULL)
	{
		return false;
	}
	dirEntries.clear();
	while ((dentry = readdir(dp)) != NULL)
	{
		dirEntries.append(String(dentry->d_name));
	}
	closedir(dp);
#endif
	return true;
}
//////////////////////////////////////////////////////////////////////////////
bool
renameFile(const String& oldFileName,
	const String& newFileName)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->renameFile(oldFileName, newFileName);
	}
	return ::rename(oldFileName.c_str(), newFileName.c_str()) == 0;
}
//////////////////////////////////////////////////////////////////////////////
size_t
read(const FileHandle& hdl, void* bfr, size_t numberOfBytes,
	Int64 offset)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->read(hdl, bfr, numberOfBytes, offset);
	}
#ifdef BLOCXX_WIN32
	OVERLAPPED ov = { 0, 0, 0, 0, NULL };
	OVERLAPPED *pov = NULL;
	if(offset != -1L)
	{
		ov.Offset = (DWORD) offset;
		// check for truncation
		if (ov.Offset != offset)
		{
			BLOCXX_THROW(FileSystemException, "offset out of range");
		}
		pov = &ov;
	}

	DWORD bytesRead;
	size_t cc = (size_t)-1;
	if(::ReadFile(hdl, bfr, (DWORD)numberOfBytes, &bytesRead, pov))
	{
		cc = (size_t)bytesRead;
	}

	return cc;
#else
	if (offset != -1L)
	{
		::off_t offset2 = static_cast< ::off_t>(offset);
		// check for truncation
		if (offset2 != offset)
		{
			BLOCXX_THROW(FileSystemException, Format("read: converted offset does not match original: %1 != %2", offset2, offset).c_str());
		}

		::lseek(hdl, offset2, SEEK_SET);
	}
	return ::read(hdl, bfr, numberOfBytes);
#endif
}
//////////////////////////////////////////////////////////////////////////////
size_t
write(FileHandle hdl, const void* bfr, size_t numberOfBytes,
	Int64 offset)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->write(hdl, bfr, numberOfBytes, offset);
	}
#ifdef BLOCXX_WIN32
	OVERLAPPED ov = { 0, 0, 0, 0, NULL };
	OVERLAPPED *pov = NULL;
	if(offset != -1L)
	{
		ov.Offset = (DWORD) offset;
		// check for truncation
		if (ov.Offset != offset)
		{
			BLOCXX_THROW(FileSystemException, "offset out of range");
		}
		pov = &ov;
	}

	DWORD bytesWritten;
	size_t cc = (size_t)-1;
	if(::WriteFile(hdl, bfr, (DWORD)numberOfBytes, &bytesWritten, pov))
	{
		cc = (size_t)bytesWritten;
	}
	return cc;
#else

	if (offset != -1L)
	{
		::off_t offset2 = static_cast< ::off_t>(offset);
		// check for truncation
		if (offset2 != offset)
		{
			BLOCXX_THROW(FileSystemException, Format("write: converted offset does not match original: %1 != %2", offset2, offset).c_str());
		}
		::lseek(hdl, offset2, SEEK_SET);
	}
	return ::write(hdl, bfr, numberOfBytes);
#endif
}

//////////////////////////////////////////////////////////////////////////////
Int64
seek(const FileHandle& hdl, Int64 offset, int whence)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->seek(hdl, offset, whence);
	}
#ifdef BLOCXX_WIN32
	DWORD moveMethod;
	switch(whence)
	{
		case SEEK_END: moveMethod = FILE_END; break;
		case SEEK_CUR: moveMethod = FILE_CURRENT; break;
		default: moveMethod = FILE_BEGIN; break;
	}

	LARGE_INTEGER li;
	li.QuadPart = offset;
	li.LowPart = SetFilePointer(hdl, li.LowPart, &li.HighPart, moveMethod);

	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
	   li.QuadPart = -1;
	}

	return li.QuadPart;
#else
	::off_t offset2 = static_cast< ::off_t>(offset);
	// check for truncation
	if (offset2 != offset)
	{
		BLOCXX_THROW(FileSystemException, Format("seek: converted offset does not match original: %1 != %2", offset2, offset).c_str());
	}
	return ::lseek(hdl, offset2, whence);
#endif
}
//////////////////////////////////////////////////////////////////////////////
Int64
tell(const FileHandle& hdl)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->tell(hdl);
	}
#ifdef BLOCXX_WIN32
	LARGE_INTEGER li;
	li.QuadPart = 0;
	li.LowPart = SetFilePointer(hdl, li.LowPart, &li.HighPart, FILE_CURRENT);

	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
	   li.QuadPart = -1;
	}

	return li.QuadPart;
#else
	return ::lseek(hdl, 0, SEEK_CUR);
#endif
}
//////////////////////////////////////////////////////////////////////////////
UInt64 fileSize(FileHandle fh)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->fileSize(fh);
	}

#ifndef BLOCXX_WIN32

	struct stat st;
	int rc = ::fstat(fh, &st);
	if (rc != 0)
	{
		BLOCXX_THROW_ERRNO_MSG(FileSystemException, "Could not stat file handle: ");
	}
	return st.st_size;

#else
	LARGE_INTEGER FileSize;
	BOOL rc = GetFileSizeEx(fh, &FileSize);
	if(!rc)
	{
		BLOCXX_THROW_ERRNO_MSG(FileSystemException, "Could not GetFileSizeEx() for file handle: ");
	}

	UInt64 tmp = FileSize.QuadPart;
	return tmp;

#endif
}
//////////////////////////////////////////////////////////////////////////////
void
rewind(const FileHandle& hdl)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->rewind(hdl);
	}
#ifdef BLOCXX_WIN32
	::SetFilePointer(hdl, 0L, NULL, FILE_BEGIN);
#else
	::lseek(hdl, 0, SEEK_SET);
#endif
}
//////////////////////////////////////////////////////////////////////////////
int
close(const FileHandle& hdl)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->close(hdl);
	}
#ifdef BLOCXX_WIN32
	return (::CloseHandle(hdl)) ? 0 : -1;
#else
	return ::close(hdl);
#endif
}
//////////////////////////////////////////////////////////////////////////////
int
flush(FileHandle& hdl)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->flush(hdl);
	}
#ifdef BLOCXX_WIN32
	return (::FlushFileBuffers(hdl)) ? 0 : -1;
#else
	return ::fsync(hdl);
#endif
}
//////////////////////////////////////////////////////////////////////////////
String getFileContents(const String& filename)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->getFileContents(filename);
	}
	std::ifstream in(filename.c_str());
	if (!in)
	{
		BLOCXX_THROW(FileSystemException, Format("Failed to open file %1", filename).c_str());
	}
	OStringStream ss;
	ss << in.rdbuf();
	return ss.toString();
}

//////////////////////////////////////////////////////////////////////////////
StringArray getFileLines(const String& filename)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->getFileLines(filename);
	}
	return getFileContents(filename).tokenize("\r\n");
}

//////////////////////////////////////////////////////////////////////////////
String readSymbolicLink(const String& path)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->readSymbolicLink(path);
	}
#ifdef BLOCXX_WIN32
	return Path::realPath(path);
#else
	std::vector<char> buf(MAXPATHLEN + 1);
	int rc;
	while (true)
	{
		rc = ::readlink(path.c_str(), &buf[0], buf.size());
		// If the link value is too big to fit into buf, but
		// there is no other error, then rc == buf.size(); in particular,
		// we do NOT get rc < 0 with errno == ENAMETOOLONG (this indicates
		// a problem with the input path, not the link value returned).
		if (rc < 0)
		{
			BLOCXX_THROW_ERRNO_MSG(FileSystemException, path);
		}
		else if (static_cast<unsigned>(rc) == buf.size())
		{
			buf.resize(buf.size() * 2);
		}
		else
		{
			buf.resize(rc);
			buf.push_back('\0');
			return String(&buf[0]);
		}
	}
#endif
	// Not reachable.
	return String();
}

//////////////////////////////////////////////////////////////////////////////
namespace Path
{

//////////////////////////////////////////////////////////////////////////////
String realPath(const String& path)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->realPath(path);
	}
#ifdef BLOCXX_WIN32
	char c, *bfr, *pname;
	const char *pathcstr;
	DWORD cc;

	pathcstr = path.c_str();
	while (*pathcstr == '/' || *pathcstr == '\\')
	{
		++pathcstr;
	}

	// if we ate some '\' or '/' chars, the back up to
	// allow for 1
	if(pathcstr != path.c_str())
	{
		--pathcstr;
	}

	cc = GetFullPathName(path.c_str(), 1, &c, &pname);
	if(!cc)
	{
		BLOCXX_THROW(FileSystemException, Format("Can't get full path name for path %s", path).c_str());
	}
	bfr = new char[cc];
	cc = GetFullPathName(path.c_str(), cc, bfr, &pname);
	if(!cc)
	{
		delete [] bfr;
		BLOCXX_THROW(FileSystemException, Format("Can't get full path name for path %s", path).c_str());
	}
	String rstr(bfr);
	delete [] bfr;
	return rstr;
#else
	if (path.startsWith("/"))
	{
		return security(path, 0, E_SECURITY_DO_NOTHING).second;
	}
	else
	{
		return security(getCurrentWorkingDirectory(), path, 0, E_SECURITY_DO_NOTHING).second;
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////
String dirname(const String& filename)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->dirname(filename);
	}
	// skip over trailing slashes
	if (filename.length() == 0)
	{
		return ".";
	}
	size_t lastSlash = filename.length() - 1;
	while (lastSlash > 0
		&& filename[lastSlash] == BLOCXX_FILENAME_SEPARATOR_C)
	{
		--lastSlash;
	}

	lastSlash = filename.lastIndexOf(BLOCXX_FILENAME_SEPARATOR_C, lastSlash);

	if (lastSlash == String::npos)
	{
		return ".";
	}

	while (lastSlash > 0 && filename[lastSlash - 1] == BLOCXX_FILENAME_SEPARATOR_C)
	{
		--lastSlash;
	}

	if (lastSlash == 0)
	{
		return BLOCXX_FILENAME_SEPARATOR;
	}

	return filename.substring(0, lastSlash);
}

//////////////////////////////////////////////////////////////////////////////
String basename(const String& filename)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->basename(filename);
	}
	if (filename.length() == 0)
	{
		return filename;
	}
	size_t end = filename.length() -1;
	while (end > 0
		&& filename[end] == BLOCXX_FILENAME_SEPARATOR_C)
	{
		--end;
	}
	if (end == 0 && filename[0] == BLOCXX_FILENAME_SEPARATOR_C)
	{
		return BLOCXX_FILENAME_SEPARATOR;
	}
	if (end == filename.length() -1)
	{
		end = String::npos;
	}
	size_t beg = filename.lastIndexOf(BLOCXX_FILENAME_SEPARATOR_C, end);
	if (beg == String::npos)
	{
		beg = 0;
	}
	else
	{
		++beg;
	}
	size_t len = end == String::npos? end : ++end - beg;
	return filename.substring(beg, len);
}

//////////////////////////////////////////////////////////////////////////////
String getCurrentWorkingDirectory()
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->getCurrentWorkingDirectory();
	}
	std::vector<char> buf(MAXPATHLEN);
	char* p;
	do
	{
		p = ::getcwd(&buf[0], buf.size());
		if (p != 0)
		{
			return p;
		}
		buf.resize(buf.size() * 2);
	} while (p == 0 && errno == ERANGE);

	BLOCXX_THROW_ERRNO(FileSystemException);
}

} // end namespace Path

namespace // anonymous
{
	FileInformation::EFilePerms convertModeToPerms(mode_t mode)
	{
		FileInformation::EFilePerms value = FileInformation::E_FILE_PERM_UNKNOWN;

		// Some sanity checks to ensure that the constants provided in the
		// FileInformation::EFilePerms enum can be converted from the system's
		// mode_t constants (minus any sticky, setuid or other random bits).
		BLOCXX_STATIC_ASSERT(FileInformation::E_FILE_GROUP_WRITE == S_IWGRP);
		BLOCXX_STATIC_ASSERT(FileInformation::E_FILE_OTHER_EXECUTE == S_IXOTH);
		BLOCXX_STATIC_ASSERT(FileInformation::E_FILE_USER_READ == S_IRUSR);

		// Clear out any bits used by setuid, sticky, or other random bits.
		value = FileInformation::EFilePerms(mode & (S_IRWXO | S_IRWXG | S_IRWXU));

		if( mode & S_ISVTX )
		{
			value = FileInformation::EFilePerms(value | FileInformation::E_FILE_STICKY);
		}

		if( mode & S_ISUID )
		{
			value = FileInformation::EFilePerms(value | FileInformation::E_FILE_SETUID);
		}

		if( mode & S_ISGID )
		{
			value = FileInformation::EFilePerms(value | FileInformation::E_FILE_SETGID);
		}

		return value;
	}

	FileInformation::EFileType convertModeToType(mode_t mode)
	{
		if( S_ISREG(mode) )
		{
			return FileInformation::E_FILE_REGULAR;
		}
		else if( S_ISDIR(mode) )
		{
			return FileInformation::E_FILE_DIRECTORY;
		}
		else if( S_ISLNK(mode) )
		{
			return FileInformation::E_FILE_SYMLINK;
		}
		else if( S_ISCHR(mode)
			|| S_ISBLK(mode)
			|| S_ISFIFO(mode)
			|| S_ISSOCK(mode) )
		{
			return FileInformation::E_FILE_SPECIAL;
		}
		return FileInformation::E_FILE_TYPE_UNKNOWN;
	}
} // end anonymous namespace


FileInformation statToFileInfo(const struct stat& statbuf)
{
	FileInformation info;

	info.mtime = DateTime(statbuf.st_mtime);
	info.atime = DateTime(statbuf.st_atime);
	info.ctime = DateTime(statbuf.st_ctime);
	info.owner = UserId(statbuf.st_uid);
	info.group = gid_t(statbuf.st_gid);
	info.size = UInt64(statbuf.st_size);
	info.permissions = convertModeToPerms(statbuf.st_mode);
	info.type = convertModeToType(statbuf.st_mode);

	return info;
}

FileInformation getFileInformation(const String& filename)
{
	if (g_fileSystemMockObject)
	{
		return g_fileSystemMockObject->getFileInformation(filename);
	}

	struct stat statbuf;
	::memset(&statbuf, 0, sizeof(statbuf));
	if( ::lstat(filename.c_str(), &statbuf) != 0 )
	{
		BLOCXX_THROW_ERRNO(FileSystemException);
	}

	return statToFileInfo(statbuf);
}

} // end namespace FileSystem
} // end namespace BLOCXX_NAMESPACE

