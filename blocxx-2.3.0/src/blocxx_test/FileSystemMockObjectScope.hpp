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

#if       !defined(BLOCXX_TEST_FILESYSTEM_MOCK_OBJECT_SCOPE_HPP_INCLUDE_GUARD_)
#define            BLOCXX_TEST_FILESYSTEM_MOCK_OBJECT_SCOPE_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Reference.hpp"
#include "blocxx/FileSystemMockObject.hpp"

namespace BLOCXX_NAMESPACE
{
	namespace Test
	{
		typedef blocxx::Reference<blocxx::FileSystemMockObject> FSMockObjectRef;

		/**
		 * A simple filesystem mock object manipulation class.  This will set the
		 * global filesystem mock object at construction time and reset it when
		 * this object is destroyed (gone out of scope).
		 */
		class FileSystemMockObjectScope
		{
		public:
			FileSystemMockObjectScope(blocxx::Reference<blocxx::FileSystemMockObject> mock);
			virtual ~FileSystemMockObjectScope();

		private:
			// The old mock object.  This is a raw pointer because the global it
			// is copied from object is also a raw pointer.
			blocxx::FileSystemMockObject* old_mock;

			// This variable is needed to keep the new mock object alive for
			// as long as this mock object scope is active.  If this
			// variable was not present, it would not be possible to create
			// an instance with a temporary copy of a filesystem mock
			// object, such as is returned by createCannedFSObject.
			blocxx::Reference<blocxx::FileSystemMockObject> m_mock;
		};
	} // end namespace Test
} // end namespace BLOCXX_NAMESPACE

#endif /* !defined(BLOCXX_TEST_FILESYSTEM_MOCK_OBJECT_SCOPE_HPP_INCLUDE_GUARD_) */
