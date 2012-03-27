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

#include "blocxx/BLOCXX_config.h"
#include "blocxx_test/UnmockedFunctions.hpp"
#include "blocxx_test/TextUtils.hpp"
#include "blocxx_test/LogUtils.hpp"

#include "blocxx/String.hpp"
#include "blocxx/StringStream.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/Format.hpp"

#include <istream>
#include <fstream>

namespace
{
	const char* const COMPONENT_NAME("UnmockedFunctions");
} // anonymous

#define LOG_DEBUG(X) STANDARD_LOG_DEBUG( "UnmockedFunctions: ", (X) )

namespace BLOCXX_NAMESPACE
{
	namespace Test
	{
		namespace Unmocked
		{
			BLOCXX_DEFINE_EXCEPTION(BadFile);

			bool getFileContents(const blocxx::String& filename,
				blocxx::String& contents)
			{
				Logger logger(COMPONENT_NAME);

				// This cannot use anything in blocxx::FileSystem because
				// of the mock objects.
				LOG_DEBUG(Format("Opening %1 for binary input.", filename));
				std::ifstream input( filename.c_str(), std::ios::binary );
				if( !input )
				{
					LOG_DEBUG(Format("Failed to open %1.", filename));
					return false;
				}

				blocxx::OStringStream text;
				LOG_DEBUG("Copying file stream.");
				TextUtils::copyStream(text, input);
				LOG_DEBUG("Releasing file contents string.");
				contents = text.releaseString();
				LOG_DEBUG(Format("File Contents: %1", contents));

				return true;
			}

			blocxx::String getFileContents(const blocxx::String& filename)
			{
				String contents;
				if( !getFileContents(filename, contents) )
				{
					BLOCXX_THROW(BadFileException, Format("Failed to open file \"%1\"", filename).c_str());
				}
				return contents;
			}
		} // end namespace Unmocked
	} // end namespace Test
} // end namespace BLOCXX_NAMESPACE
