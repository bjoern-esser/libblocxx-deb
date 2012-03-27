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

#include "blocxx/CommonFwd.hpp"
#include "blocxx/IFileStream.hpp"

namespace BLOCXX_NAMESPACE
{

IFileStream::IFileStream()
: FileBuf()
, std::istream(this)
{
}

IFileStream::~IFileStream()
{
}

bool
IFileStream::isOpen() const
{
	return FileBuf::isOpen();
}

IFileStream* 
IFileStream::open(FILE* fp)
{
	if (FileBuf::open(fp))
	{
		return this;
	}
	return 0;
}

IFileStream* 
IFileStream::open(AutoDescriptor fd)
{
	if (FileBuf::open(fd))
	{
		return this;
	}
	return 0;
}

IFileStream* 
IFileStream::open(const char* path, std::ios_base::openmode mode, mode_t permissions)
{
	if (FileBuf::open(path, mode, permissions))
	{
		return this;
	}
	return 0;
}

IFileStream* 
IFileStream::close()
{
	if (FileBuf::close())
	{
		return this;
	}
	return 0;
}

} // end namespace BLOCXX_NAMESPACE



