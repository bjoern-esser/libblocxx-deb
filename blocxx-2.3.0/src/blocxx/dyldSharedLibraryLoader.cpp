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

#include "blocxx/BLOCXX_config.h"
#if defined(BLOCXX_USE_DYLD)
#include "blocxx/dyldSharedLibraryLoader.hpp"
#include "blocxx/dyldSharedLibrary.hpp"
#include "blocxx/Format.hpp"

namespace BLOCXX_NAMESPACE
{

namespace
{
	String COMPONENT_NAME("blocxx.dyldSharedLibraryLoader");
}

std::ostream& operator<<(std::ostream& o, NSObjectFileImageReturnCode code)
{
#define SIMPLE_NSOBJ_CASE(X) case X: o << ""#X
	switch (code)
	{
		SIMPLE_NSOBJ_CASE(NSObjectFileImageFailure);
		break;
		SIMPLE_NSOBJ_CASE(NSObjectFileImageSuccess);
		break;
		SIMPLE_NSOBJ_CASE(NSObjectFileImageInappropriateFile);
		break;
		SIMPLE_NSOBJ_CASE(NSObjectFileImageArch);
		break;
		SIMPLE_NSOBJ_CASE(NSObjectFileImageFormat);
		break;
		SIMPLE_NSOBJ_CASE(NSObjectFileImageAccess);
		break;
	}
#undef SIMPLE_NSOBJ_CASE
	return o;
}

///////////////////////////////////////////////////////////////////////////////
SharedLibraryRef
dyldSharedLibraryLoader::loadSharedLibrary(const String& filename) const
{
	Logger logger(COMPONENT_NAME);
	BLOCXX_LOG_DEBUG2(logger, Format("Load request for %1 received.", filename));
	NSObjectFileImage image = 0;
	NSObjectFileImageReturnCode dsoerr = NSCreateObjectFileImageFromFile(filename.c_str(), &image);
	const char* err_msg = NULL;
	NSModule libhandle = NULL;

	if (dsoerr == NSObjectFileImageSuccess)
	{
		libhandle = NSLinkModule(image, filename.c_str(), NSLINKMODULE_OPTION_RETURN_ON_ERROR | NSLINKMODULE_OPTION_PRIVATE);
		if (!libhandle)
		{
			NSLinkEditErrors errors;
			int errorNumber;
			const char *fileName;
			NSLinkEditError(&errors, &errorNumber, &fileName, &err_msg);
		}
		NSDestroyObjectFileImage(image);
	}
	else
	{
		err_msg = "cannot create object file image";
		BLOCXX_LOG_ERROR(logger, Format("NSCreateObject: %1 failed with error %2",
														filename, dsoerr));
	}



	if (libhandle)
	{
		try
		{
			return SharedLibraryRef( new dyldSharedLibrary(libhandle,
																										 filename));
		}
		catch (...)
		{
			NSUnLinkModule(libhandle, FALSE);
			throw;
		}
	}
	else
	{
		BLOCXX_LOG_ERROR(logger, Format("dyldSharedLibraryLoader::loadSharedLibrary:"
														" %1", err_msg));
		return SharedLibraryRef( 0 );
	}
}
///////////////////////////////////////////////////////////////////////////////
SharedLibraryLoaderRef
SharedLibraryLoader::createSharedLibraryLoader()
{
	return SharedLibraryLoaderRef(new dyldSharedLibraryLoader);
}
///////////////////////////////////////////////////////////////////////////////
dyldSharedLibraryLoader::~dyldSharedLibraryLoader()
{
}

} // end namespace BLOCXX_NAMESPACE

#endif // #if defined(BLOCXX_USE_DYLD)
