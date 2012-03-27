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
#if       !defined(BLOCXX_TEST_FED_PIPE_HPP_INCLUDE_GUARD_)
#define            BLOCXX_TEST_FED_PIPE_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/String.hpp"
#include "blocxx/UnnamedPipe.hpp"
#include "blocxx/IntrusiveReference.hpp"

namespace BLOCXX_NAMESPACE
{
	namespace Test
	{
		class FedPipeImpl;

		/**
		 * This is a class intended to be used where an UnnamedPipe can be used for
		 * read operations.  It is incapable of accepting writes of any kind.  It
		 * will (internally) fork another copy of the process to write to the pipe
		 * to prevent deadlock and other signalling issues.
		 *
		 * NOTE: This is not intended for common use.  It should mainly be used from
		 * test code since it is not currently thread or signal safe.
		 */
		class FedPipe : public blocxx::UnnamedPipe
		{
		public:
			// Create a pipe that is prefed with the given data.  This will always be
			// a blocking pipe.  If the data is not read from this pipe then another
			// forked process will stay blocked for as long as this object exists.
			FedPipe(const blocxx::String& data);
			virtual ~FedPipe();

			virtual int read(void* ptr, int size, IOIFC::ErrorAction errorAsException);
			virtual int close();
			virtual bool isOpen() const;
			virtual blocxx::Select_t getReadSelectObj() const;
			virtual blocxx::Descriptor getInputDescriptor() const;

			// All of these virtual functions are not supported.
			virtual int write(const void*, int, IOIFC::ErrorAction);
			virtual void open();
			virtual blocxx::Select_t getWriteSelectObj() const;
			virtual void setBlocking(UnnamedPipe::EBlockingMode);
			virtual void setWriteBlocking(UnnamedPipe::EBlockingMode);
			virtual void setReadBlocking(UnnamedPipe::EBlockingMode);
			virtual blocxx::Descriptor getOutputDescriptor() const;
			virtual void passDescriptor(blocxx::Descriptor, const blocxx::UnnamedPipeRef&, const blocxx::ProcessRef&);
			virtual blocxx::AutoDescriptor receiveDescriptor(const blocxx::UnnamedPipeRef&);
			virtual blocxx::UnnamedPipe::EBlockingMode getReadBlocking() const;
			virtual blocxx::UnnamedPipe::EBlockingMode getWriteBlocking() const;
			virtual int closeInputHandle();
			virtual int closeOutputHandle();

		private:
			blocxx::IntrusiveReference<FedPipeImpl> m_impl;
		};

	} // end namespace Test
} // end namespace BLOCXX_NAMESPACE

#endif /* !defined(BLOCXX_TEST_FED_PIPE_HPP_INCLUDE_GUARD_) */
