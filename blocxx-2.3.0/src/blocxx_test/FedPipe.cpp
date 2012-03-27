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
#include "blocxx_test/FedPipe.hpp"

#include <sys/types.h>
#include <unistd.h>

namespace BLOCXX_NAMESPACE
{
	namespace Test
	{

		/**
		 * The structure to hide the actual pipe information from anyone using a
		 * FedPipe.
		 *
		 * TODO: This should be altered to use a single pipe in such a way that the
		 * read and write sides are one-way.  For our current use, it shouldn't
		 * matter that the pipe is writeable, as it will just block (nobody's
		 * listening).
		 */
		class FedPipeImpl : public IntrusiveCountableBase
		{
		public:
			UnnamedPipeRef read_side;
			UnnamedPipeRef write_side;

			FedPipeImpl()
			{
				UnnamedPipe::createConnectedPipes(read_side, write_side);
			}
		};

		FedPipe::FedPipe(const String& data)
			: m_impl(new FedPipeImpl)
		{
			m_impl->read_side->setBlocking();
			m_impl->write_side->setBlocking();

			// It truly scares me to fork in a constructor.  We need to be sure that
			// any operations done within this if statement DO NOT allocate memory,
			// open resources, or do anything else.
			if( fork() == 0 )
			{
				// FIXME! The signal handlers should be altered immediately after the fork.

				// We can't read from this pipe, so we close it.
				m_impl->read_side->close();

				int ignored = ::write(m_impl->write_side->getOutputDescriptor(), data.c_str(), data.length());
				(void) ignored;
				// This should send a sigpipe to the other end...  At least it does on Linux.
				m_impl->write_side->close();

				_exit(0); // Exit without cleaning anything.  It will close descriptors.
			}

			// We can't ever write to this pipe.
			m_impl->write_side->close();
		}

		FedPipe::~FedPipe()
		{
		}

		int FedPipe::read(void* ptr, int size, IOIFC::ErrorAction errorAsException)
		{
			return m_impl->read_side->read(ptr, size, errorAsException);
		}

		int FedPipe::write(const void* ptr, int size, IOIFC::ErrorAction errorAsException)
		{
			return -1;
		}

		void FedPipe::open()
		{
			// Open in the constructor.
		}

		int FedPipe::close()
		{
			return m_impl->read_side->close();
		}

		bool FedPipe::isOpen() const
		{
			return m_impl->read_side->isOpen();
		}

		Select_t FedPipe::getReadSelectObj() const
		{
			return m_impl->read_side->getReadSelectObj();
		}

		Select_t FedPipe::getWriteSelectObj() const
		{
			return Select_t(BLOCXX_INVALID_FILEHANDLE);
		}

		void FedPipe::setBlocking(UnnamedPipe::EBlockingMode mode)
		{
			// Ignored.
		}

		void FedPipe::setWriteBlocking(UnnamedPipe::EBlockingMode mode)
		{
			// Ignored.
		}

		void FedPipe::setReadBlocking(UnnamedPipe::EBlockingMode mode)
		{
			// Sorry... We always block.
		}

		Descriptor FedPipe::getInputDescriptor() const
		{
			return m_impl->read_side->getInputDescriptor();
		}

		Descriptor FedPipe::getOutputDescriptor() const
		{
			return Descriptor(BLOCXX_INVALID_FILEHANDLE);
		}

		void FedPipe::passDescriptor(Descriptor, const UnnamedPipeRef&, const ProcessRef&)
		{
			// Ignored.
		}

		AutoDescriptor FedPipe::receiveDescriptor(const UnnamedPipeRef&)
		{
			// Ignored.
			return AutoDescriptor();
		}

		UnnamedPipe::EBlockingMode FedPipe::getReadBlocking() const
		{
			return UnnamedPipe::E_BLOCKING;
		}

		UnnamedPipe::EBlockingMode FedPipe::getWriteBlocking() const
		{
			return UnnamedPipe::E_BLOCKING;
		}

		int FedPipe::closeInputHandle()
		{
			return 0;
		}

		int FedPipe::closeOutputHandle()
		{
			return 0;
		}

	} // end namespace Test
} // end namespace BLOCXX_NAMESPACE
