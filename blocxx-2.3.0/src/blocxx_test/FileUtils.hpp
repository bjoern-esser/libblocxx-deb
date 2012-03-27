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
 * @author Mat Bess
 */

#if       !defined(BLOCXX_TEST_FILE_UTILS_HPP_INCLUDE_GUARD_)
#define            BLOCXX_TEST_FILE_UTILS_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/String.hpp"
#include "blocxx/FileInformation.hpp"
#include "blocxx/GlobalString.hpp"

namespace BLOCXX_NAMESPACE
{
	namespace FileUtils
	{
		/*
		 * The default path used for locating executables.
		 */
		extern const blocxx::GlobalString EXECUTABLE_DEFAULT_PATH;

		/*
		 * Locate the given executable name in a predefined list of paths
		 * (standard locations).  If that fails, go to the PATH (or path)
		 * environment variable for the path.
		 *
		 * If the executable is found, then the full path will be returned.
		 * Otherwise, an empty string will be returned.
		 */
		blocxx::String locateExecutable(const blocxx::String& executable_name, const blocxx::String& valid_paths = EXECUTABLE_DEFAULT_PATH);

		/**
		 * If fileName exisits in path, return the absolute path to filename.
		 *     Otherwise, return an empty string.
		 *
		 * @param path
		 *   A sequence of directory names, separatored by separator.
		 *
		 * @param fileName
		 *   The file name to look for.
		 *
		 * @param pwd
		 *   The directory to check before the normal path is searched.
		 *
		 * @param separator
		 *   The tokenization characters used to separate adjacent dirnames in
		 *   the string path.  The default is usually appropriate for values
		 *   obtained from the PATH environment variables.
		 *
		 * @return
		 *   The file's absolute name, or an empty string.
		 */
		blocxx::String searchPathForFile(const blocxx::String& path,
			const blocxx::String& fileName,
			const blocxx::String& pwd = blocxx::String(),
			const blocxx::String& separator = blocxx::String(BLOCXX_PATHNAME_SEPARATOR));


		/*
		 * Send the output of the given file to the given logger, each line prefixed
		 * by the given prefix.
		 *
		 * @param pathname
		 *   The full path to the file which will be dumped to the log.
		 * @param prefix
		 *   The text to prepend to each line of the output.
		 * @param debug
		 *   If true, send output to the debug log, otherwise use the error log.
		 *
		 * @return
		 *   The number of lines sent to the log.
		 */
		blocxx::UInt32 catToLogUnprivileged(
			const blocxx::String& pathname,
			blocxx::Logger& logger,
			const blocxx::String& prefix,
			bool debug = true);

		/*
		 * Read the contents of a file, separating it by lines, returning the output.
		 *
		 * @param pathname
		 *   The full path of the file to read.
		 * @param lines
		 *   An output parameter which will contain all the lines from the input file.
		 *
		 * @return
		 *   True if the file was read, false otherwise.
		 */
		bool readContentsUnprivileged(const blocxx::String& pathname, blocxx::StringArray& lines);

		/*
		 * Read the contents of a file assigning the output to the 2nd argument blocxx::String.
		 *
		 * @param pathname
		 *   The full path of the file to read.
		 * @param contents
		 *   An output parameter which will contain all the data from the input file.
		 *
		 * @return
		 *   True if the file was read, false otherwise.
		 */
		bool readContentsUnprivileged(const blocxx::String& pathname, blocxx::String & contents);

		/*
		 * Read some number of lines from a file.
		 *
		 * @param pathname
		 *   The full path of the file to read.
		 * @param lines
		 *   An output parameter which will contain lines from the input file.
		 * @param max_lines
		 *   The maximum number of lines to read from the file (0 = entire file).
		 *
		 * @return
		 *   True if the file was read, false otherwise.
		 */
		bool readHeadUnprivileged(const blocxx::String& pathname, blocxx::StringArray& lines, blocxx::UInt32 max_lines = 0);

		/*
		 * Return the file the link is linked to.
		 * If the given pathname is NOT a link (but exists), then the original
		 * string is returned.  If it does not exist then an empty string is
		 * returned.
		 */
		blocxx::String followSymlink(const blocxx::String& pathname);

		/*
		 * Remove duplicate slashes and perform other fixups required for the
		 * monitor to accept the given path.
		 *
		 * @param pathanme A path which may contain multiple adjacent slashes
		 * (/), or a trailing slash.
		 *
		 * @return A fixed version of the path which should be acceptable to the
		 * monitor
		 */
		blocxx::String fixPathForMonitor(const blocxx::String& pathname);

		/**
		 * This function returns if the given symlinkPath path is acceptable for
		 * recursion from the given startDirectory.  This returns false if the
		 * link could cause infinite recursion.
		 *
		 * @param startDirectory This should normally be dirname(symlinkPath),
		 *   but can be any directory that is a parent (or suspected parent) of
		 *   the given symlink path.
		 * @param symlinkPath The link to check.
		 */
		bool linkIsAcceptableForRecursion(const blocxx::String& startDirectory, const blocxx::String& symlinkPath);

		enum FileDeletionTypes
			{
				E_DO_NOT_DELETE,
				E_I_REALLY_MEAN_TO_DELETE
			};
		/**
		 * Resursively (unprivileged) delete files.  Don't use this unless you
		 * *REALLY* mean it.  If you use this outside of test code then you are
		 * probably doing something wrong.
		 */
		void recursiveDeleteFiles(const blocxx::String& basePath, FileDeletionTypes reallyRemove = E_DO_NOT_DELETE);

	} // end namespace FileUtils
} // end namespace BLOCXX_NAMESPACE
#endif /* !defined(BLOCXX_TEST_FILE_UTILS_HPP_INCLUDE_GUARD_) */
