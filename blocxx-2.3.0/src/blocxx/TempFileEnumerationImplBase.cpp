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
#include "blocxx/TempFileEnumerationImplBase.hpp"
#include "blocxx/EnumerationException.hpp"
#include "blocxx/File.hpp"
#include "blocxx/FileSystem.hpp"

namespace BLOCXX_NAMESPACE
{

namespace
{
const UInt32 TEMPFILE_ENUMERATION_SIG = 0x4f57454e; // "OWEN"
}

TempFileEnumerationImplBase::TempFileEnumerationImplBase()
	: m_size(0), m_Data()
{
	UInt32 enumSig = TEMPFILE_ENUMERATION_SIG;
	m_Data.write(reinterpret_cast<const char*>(&enumSig), sizeof(enumSig));
	if (!m_Data.good())
	{
		BLOCXX_THROW(EnumerationException, "Failed to write signature to "
			"enumeration tempfile.");
	}
	// now we have to read the sig so that the temp file stream is
	// positioned correctly
	UInt32 tmpSig;
	m_Data.read(reinterpret_cast<char*>(&tmpSig), sizeof(tmpSig));
	if (!m_Data.good())
	{
		BLOCXX_THROW(EnumerationException, "Failed to read signature from "
			"enumeration tempfile.");
	}
}

TempFileEnumerationImplBase::~TempFileEnumerationImplBase()
{
}
bool
TempFileEnumerationImplBase::hasMoreElements() const
{

	return m_size > 0;
}
size_t
TempFileEnumerationImplBase::numberOfElements() const
{
	return m_size;
}
void
TempFileEnumerationImplBase::clear()
{
	m_size = 0;
	m_Data.reset();
}
bool
TempFileEnumerationImplBase::usingTempFile() const
{
	return m_Data.usingTempFile();
}

size_t
TempFileEnumerationImplBase::readSize(String const& filename)
{
	size_t size;
	// open the file and read the size that is written to the end of it.
	File f = FileSystem::openFile(filename);
	if (!f)
	{
		BLOCXX_THROW(EnumerationException, "Failed to open file");
	}

	// Check that the correct signature is on the file
	UInt32 fileSig;
	if (f.read(reinterpret_cast<char*>(&fileSig), sizeof(fileSig)) != sizeof(fileSig))
	{
		BLOCXX_THROW(EnumerationException, "Failure to read enumeration "
			"signature");
	}
	if (fileSig != TEMPFILE_ENUMERATION_SIG)
	{
		BLOCXX_THROW(EnumerationException, "Attempted to construct an "
			"enumeration from a file that does not have the correct "
			"signature");
	}

	off_t whence = f.seek(-static_cast<off_t>(sizeof(size)), SEEK_END);
	if (whence == -1)
	{
		BLOCXX_THROW(EnumerationException, "Failure to seek");
	}
	if (f.read(reinterpret_cast<char*>(&size), sizeof(size), whence) != sizeof(size))
	{
		BLOCXX_THROW(EnumerationException, "Failure to read enumeration "
			"size");
	}
	if (f.close() == -1)
	{
		BLOCXX_THROW(EnumerationException, "Failure to close enumeration "
			"file");
	}
	return size;
}

void
TempFileEnumerationImplBase::throwIfEmpty() const
{
	if (!hasMoreElements())
	{
		BLOCXX_THROW (EnumerationException, "Attempt to Extract from empty Enum");
	}
}


} // end namespace BLOCXX_NAMESPACE



