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
 * @author Kevin S. Van Horn
 * @author Anton Afanasiev - for Win
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/PathSecurity.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/FileSystem.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/String.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/UserUtils.hpp"
#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <vector>

namespace BLOCXX_NAMESPACE
{

typedef Array<std::pair<String, EFileStatusReturn> > path_results_t;

namespace
{
	using namespace FileSystem::Path;

	unsigned const MAX_SYMBOLIC_LINKS = 100;

	// Conceptually, this class consists of two values:
	// - A resolved part, which can have path components added or removed
	//   at the end, and
	// - an unresolved part, which can have path components added or removed
	//   at the beginning.
	//
	class PartiallyResolvedPath
	{
	public:
		// PROMISE: Resolved part is base_dir and unresolved part is empty.
		// REQUIRE: base_dir is in canonical form.
		PartiallyResolvedPath(char const * base_dir);

		// Prepends the components of path to the unresolved part of the path.
		// Note that path may be empty.
		// REQUIRE: path does not start with '/'.
		void multi_push_unresolved(char const * path);

		// Discards the first component of the unresolved part.
		// REQUIRE: unresolved part nonempty.
		void pop_unresolved();

		// RETURNS: true iff the unresolved part is empty
		bool unresolved_empty() const;

		// RETURNS: true iff the first component of the unresolved part is ".".
		bool unresolved_starts_with_curdir() const;

		// RETURNS: true iff the first component of the unresolved part is "..".
		bool unresolved_starts_with_parent() const;

		// RETURNS: true iff push_unresolved(path) has ever been called
		// with an empty unresolved part and path ending in '/'.
		bool dir_specified() const;

		// Transfers the first component of the unresolved part to the end
		// of the resolved part.
		// REQUIRE: unresolved part is nonempty.
		void xfer_component();

		// Discards the last component of the resolved part.
		// If resolved part is "/", does nothing (parent of "/" is "/").
		void pop_resolved();

		// Resets the resolved part ot "/".
		void reset_resolved();

		// RETURNS: the resolved part, as a String.
		String get_resolved() const;

		// Calls lstat on the resolved part.
		void lstat_resolved(struct stat & st) const;

		// REQUIRE: resolved part is not "/", and last component is a symbolic
		// link.
		// PROMISE: Reads the symbolic link and assigns it to path (including
		// a terminating '\0' character).
		void read_symlink(std::vector<char> & path);

	private:

		// INVARIANT: Holds an absolute path with no duplicate '/' chars,
		// no "." or ".." components, no component (except possibly the last)
		// that is a symlink, and no terminating '/' unless the whole path is
		// "/". 
		mutable std::vector<char> m_resolved;

		// INVARIANT: holds a relative path with no repeated or terminating '/'
		// chars, stored in reverse order.
		std::vector<char> m_unresolved;

		bool m_dir_specified;
	};

	PartiallyResolvedPath::PartiallyResolvedPath(char const * base_dir)
	: m_resolved(base_dir, base_dir + std::strlen(base_dir)),
	  m_unresolved(),
	  m_dir_specified(false)
	{
	}

	void PartiallyResolvedPath::multi_push_unresolved(char const * path)
	{
		BLOCXX_ASSERT(path && *path != '/');
		if (*path == '\0')
		{
			return;
		}
		char const * end = path;
		while (*end != '\0')
		{
			++end;
		}
		if (end != path && *(end - 1) == '/')
		{
			m_dir_specified = true;
		}
		m_unresolved.push_back('/');
		bool last_separator = true;
		while (end != path)
		{
			char c = *--end;
			bool separator = (c == '/');
			if (!(separator && last_separator))
			{
				m_unresolved.push_back(c);
			}
			last_separator = separator;
		}
	}

	void PartiallyResolvedPath::pop_unresolved()
	{
		BLOCXX_ASSERT(!m_unresolved.empty());
		while (!m_unresolved.empty() && m_unresolved.back() != '/')
		{
			m_unresolved.pop_back();
		}
		while (!m_unresolved.empty() && m_unresolved.back() == '/')
		{
			m_unresolved.pop_back();
		}
	}

	inline bool PartiallyResolvedPath::unresolved_empty() const
	{
		return m_unresolved.empty();
	}

	bool PartiallyResolvedPath::unresolved_starts_with_curdir() const
	{
		std::size_t n = m_unresolved.size();
		return (
			n > 0 && m_unresolved[n - 1] == '.' &&
			(n == 1 || m_unresolved[n - 2] == '/')
		);
	}

	bool PartiallyResolvedPath::unresolved_starts_with_parent() const
	{
		std::size_t n = m_unresolved.size();
		return (
			n >= 2 && m_unresolved[n - 1] == '.' && m_unresolved[n - 2] == '.'
			&& (n == 2 || m_unresolved[n - 3] == '/')
		);
	}

	inline bool PartiallyResolvedPath::dir_specified() const
	{
		return m_dir_specified;
	}

	void PartiallyResolvedPath::xfer_component()
	{
		BLOCXX_ASSERT(!m_unresolved.empty());
		std::size_t n = m_resolved.size();
		BLOCXX_ASSERT(n > 0 && (n == 1 || m_resolved[n - 1] != '/'));
		if (n > 1)
		{
			m_resolved.push_back('/');
		}
		char c;
		while (!m_unresolved.empty() && (c = m_unresolved.back()) != '/')
		{
			m_unresolved.pop_back();
			m_resolved.push_back(c);
		}
		while (!m_unresolved.empty() && m_unresolved.back() == '/')
		{
			m_unresolved.pop_back();
		}
	}

	void PartiallyResolvedPath::pop_resolved()
	{
		std::size_t n = m_resolved.size();
		BLOCXX_ASSERT(n > 0 && m_resolved[0] == '/');
		if (n == 1)
		{
			return; // parent of "/" is "/"
		}
		BLOCXX_ASSERT(m_resolved.back() != '/');
		while (m_resolved.back() != '/')
		{
			m_resolved.pop_back();
		}
		// pop off path separator too, unless we are back to the root dir
		if (m_resolved.size() > 1)
		{
			m_resolved.pop_back();
		}
	}

	inline void PartiallyResolvedPath::reset_resolved()
	{
		std::vector<char>(1, '/').swap(m_resolved);
	}

	class NullTerminate
	{
		std::vector<char> & m_buf;
	public:
		NullTerminate(std::vector<char> & buf)
		: m_buf(buf)
		{
			m_buf.push_back('\0');
		}

		~NullTerminate()
		{
			m_buf.pop_back();
		}
	};

	inline String PartiallyResolvedPath::get_resolved() const
	{
		NullTerminate x(m_resolved);
		return String(&m_resolved[0]);
	}

	void wrapped_lstat(char const * path, struct stat & st)
	{
#ifdef BLOCXX_WIN32
		String tmp_path(path);
		if(path[1] == ':' && path[2] == 0)
		{
			tmp_path += "\\";
		}
		if (LSTAT(tmp_path.c_str(), &st) < 0)
#else
		if (LSTAT(path, &st) < 0)
#endif
		{
			BLOCXX_THROW_ERRNO_MSG(FileSystemException, path);
		}
	}

	void PartiallyResolvedPath::lstat_resolved(struct stat & st) const
	{
		NullTerminate x(m_resolved);
		wrapped_lstat(&m_resolved[0], st);
	}

	void PartiallyResolvedPath::read_symlink(std::vector<char> & path)
	{
		BLOCXX_ASSERT(READLINK_ALLOWED);
		NullTerminate x(m_resolved);
		std::vector<char> buf(MAXPATHLEN + 1);
		while (true)
		{
			char const * symlink_path = &m_resolved[0];
			int rv = READLINK(symlink_path, &buf[0], buf.size());
			// Note that if the link value is too big to fit into buf, but
			// there is no other error, then rv == buf.size(); in particular,
			// we do NOT get rv < 0 with errno == ENAMETOOLONG (this refers
			// to the input path, not the link value returned).
			if (rv < 0)
			{
				BLOCXX_THROW_ERRNO_MSG(FileSystemException, symlink_path);
			}
			else if (static_cast<unsigned>(rv) == buf.size())
			{
				buf.resize(2 * buf.size());
			}
			else
			{
				path.swap(buf);
				return;
			}
		}
	}

	char const * strip_leading_slashes(char const * path)
	{
		while (*path == '/')
		{
			++path;
		}
		return path;
	}

	void logFileStatus(path_results_t const & results, const uid_t uid)
	{
		Logger logger("blocxx.PathSecurity");
		for (path_results_t::const_iterator li = results.begin(); li != results.end(); ++li)
		{
			switch (li->second)
			{
				case E_FILE_BAD_OWNER:
					{
						String hpux;

						bool successful(false);
						String userName = UserUtils::getUserName(uid, successful);
						if (!successful)
						{
							userName = "the proper user";
						}
#if defined(BLOCXX_HPUX) || defined(BLOCXX_AIX)
						else
						{
							if (uid == 0)
							{
								hpux = " (or bin)";
							}
						}
#endif

						BLOCXX_LOG_ERROR(logger, Format("%1 was insecure.  It was not owned by %2%3.", li->first, userName, hpux));
						break;
					}
				case E_FILE_BAD_OTHER:
					{

						BLOCXX_LOG_ERROR(logger, Format("%1 was owned by the proper user, but was not a symlink and was either"
							" world (or non-root group) writable or did not have the sticky bit set on the directory.", li->first));
						break;
					}
				default:
					break;
			}
		}
	}

	std::pair<ESecurity, String>
	path_security(char const * base_dir, char const * rel_path, uid_t uid, bool bdsecure)
	{
		BLOCXX_ASSERT(base_dir[0] == '/');
		BLOCXX_ASSERT(
			base_dir[1] == '\0' || base_dir[std::strlen(base_dir) - 1] != '/');
		BLOCXX_ASSERT(rel_path[0] != '/');

		struct stat st;

#ifndef BLOCXX_WIN32
		PartiallyResolvedPath prp(base_dir);
#else
		PartiallyResolvedPath prp("");
		if (rel_path[1] != ':' && base_dir[1] == ':')
		{
			prp = base_dir;
		}
#endif
		ESecurity status_if_secure = E_SECURE_DIR;
		unsigned num_symbolic_links = 0;
		EFileStatusReturn file_status(E_FILE_BAD_OTHER);
		path_results_t results;

		prp.multi_push_unresolved(rel_path);
		// This handles the case where there are no unresolved items in the path (only possible for '/')
		if( prp.unresolved_empty() )
		{
			prp.lstat_resolved(st);
			file_status = getFileStatus(st, uid, true, rel_path);
		}
		while (!prp.unresolved_empty())
		{
			if (prp.unresolved_starts_with_curdir())
			{
				prp.pop_unresolved();
			}
			else if (prp.unresolved_starts_with_parent())
			{
				prp.pop_unresolved();
				prp.pop_resolved();
			}
			else
			{
				prp.xfer_component();
				prp.lstat_resolved(st);
				file_status = getFileStatus(st, uid, prp.unresolved_empty(), prp.get_resolved());

				if (file_status != E_FILE_OK)
				{
					results.push_back(std::make_pair(prp.get_resolved(), file_status));
				}

				if (S_ISREG(st.st_mode))
				{
					status_if_secure = E_SECURE_FILE;
					if (!prp.unresolved_empty() || prp.dir_specified())
					{
						BLOCXX_THROW_ERRNO_MSG1(
							FileSystemException, prp.get_resolved(), ENOTDIR);
					}
				}
				else if (S_ISLNK(st.st_mode))
				{
					if (++num_symbolic_links > MAX_SYMBOLIC_LINKS)
					{
						BLOCXX_THROW_ERRNO_MSG1(
							FileSystemException, prp.get_resolved(), ELOOP);
					}
					std::vector<char> slpath_vec;
					prp.read_symlink(slpath_vec);
					char const * slpath = &slpath_vec[0];
					if (slpath[0] == '/')
					{
						prp.reset_resolved();
						slpath = strip_leading_slashes(slpath);
					}
					else
					{
						prp.pop_resolved();
					}
					prp.multi_push_unresolved(slpath);
				}
				else if (!S_ISDIR(st.st_mode))
				{
					String msg = prp.get_resolved() + 
						" is not a directory, symbolic link, nor regular file";
					BLOCXX_THROW(FileSystemException, msg.c_str());
				}
			}
		}

		ESecurity sec = (bdsecure && file_status == E_FILE_OK) ? status_if_secure : E_INSECURE;
		logFileStatus(results, uid);
		return std::make_pair(sec, prp.get_resolved());
	}

	std::pair<ESecurity, String> path_security(char const * path, UserId uid)
	{
		if(!isPathAbsolute(path))
		{
			BLOCXX_THROW(FileSystemException,
				Format("%1 is not an absolute path", path).c_str());
		}
		char const * relpath = strip_leading_slashes(path);
		struct stat st;
#ifndef BLOCXX_WIN32
		char sRootPath[] = "/";
		wrapped_lstat("/", st);
#else
		char sRootPath[MAX_PATH];
		if (::GetWindowsDirectory(sRootPath, MAX_PATH) < 3 )  // we need at least 3 symbols 
		{
			wrapped_lstat("/", st);
		}
		else
		{
			sRootPath[3] = 0; // we're interesting only for len('X:/')=3 letters
			wrapped_lstat(sRootPath, st);
		}
#endif
		EFileStatusReturn file_status = getFileStatus(st, uid, *relpath == '\0', path);

		path_results_t results;
		if (file_status != E_FILE_OK)
		{
			results.push_back(std::make_pair(String(path), file_status));
			logFileStatus(results, uid);
		}

		return path_security(sRootPath, relpath, uid, (file_status == E_FILE_OK));
	}

} // anonymous namespace

std::pair<ESecurity, String> 
FileSystem::Path::security(String const & path, UserId uid)
{
	String abspath = isPathAbsolute(path) ? path : getCurrentWorkingDirectory() + BLOCXX_FILENAME_SEPARATOR + path;
	return path_security(abspath.c_str(), uid);
}

std::pair<ESecurity, String>
FileSystem::Path::security(
	String const & base_dir, String const & rel_path, UserId uid)
{
	return path_security(base_dir.c_str(), rel_path.c_str(), uid, true);
}

std::pair<ESecurity, String> 
FileSystem::Path::security(String const & path)
{
	return security(path, ::geteuid());
}

std::pair<ESecurity, String>
FileSystem::Path::security(
	String const & base_dir, String const & rel_path)
{
	return security(base_dir, rel_path, ::geteuid());
}

} // end namespace BLOCXX_NAMESPACE

