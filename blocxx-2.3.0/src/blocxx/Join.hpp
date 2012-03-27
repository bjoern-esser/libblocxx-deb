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
 * @author Kevin Harris
 * @author Joel Smith
 */

#ifndef BLOCXX_JOIN_HPP_INCLUDE_GUARD_
#define BLOCXX_JOIN_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"

namespace BLOCXX_NAMESPACE
{
	namespace JoinImpl
	{
		/**
		 * Append to the storage using append()
		 * A Binary Function class for use with @ref Join
		 */
		struct AppendAppender
		{
			/**
			 * Function operator which calls storage.append(t)
			 * @param storage The storage to append the t item to
			 * @param t The item to be appended to the storage
			 * @tparam StorageType A storage type with an append() method
			 * @tparam T The type of the item
			 */
			template <typename StorageType, typename T>
			void operator()(StorageType& storage, const T& t) { storage.append(t); }
		};

		/**
		 * Append to the storage using +=
		 * A Binary Function class for use with @ref Join
		 */
		struct PlusEqualsAppender
		{
			/**
			 * Function operator which calls storage += t
			 * @param storage The storage to append the t item to
			 * @param t The item to be appended to the storage
			 * @tparam StorageType A storage type to which t will be += added
			 * @tparam T The type of the item
			 */
			template <typename StorageType, typename T>
			void operator()(StorageType& storage, const T& t) { storage += t; }
		};

		/**
		 * Append to the storage using <<
		 * A Binary Function class for use with @ref Join
		 */
		struct InsertionAppender
		{
			/**
			 * Function operator which calls storage << t
			 * @param storage The storage to append the t item to
			 * @param t The item to be appended to the storage
			 * @tparam StorageType A storage type to which t will be << inserted
			 * @tparam T The type of the item
			 */
			template <typename StorageType, typename T>
			void operator()(StorageType& storage, const T& t) { storage << t; }
		};
	} // end namespace JoinImpl

	/**
	 * A class to join items together, separating them with a delimiter
	 * @tparam StorageType The type of object used to append items and
	 *                     delimiters, and to hold the result
	 * @tparam ConversionType The type of item and delimiter to be joined together
	 * @tparam AppenderType An appender used to join a delimiter or item to a
	 *                      storage object.  The appender must be a Binary Function
	 *                      object that takes a StorageType and a ConversionType.
	 * @see AppendAppender
	 * @see PlusEqualsAppender
	 * @see InsertionAppender
	 */
	template <typename StorageType, typename ConversionType, typename AppenderType = JoinImpl::PlusEqualsAppender>
	class Join
	{
	public:
		/**
		 * Create a Join object by iterating across a range, and joining the
		 * items together with a delimiter, using an appender function to
		 * perform the join.
		 * @param first The iterator for the beginning of the range.
		 * @param last The iterator for the end of the range.
		 * @param delim The new delimiter to put between each item in the range
		 * @param append An appender binary function to append an item or
		 *        delimiter to a StorageType object.
		 * @tparam InputIter An input iterator type which supports pre-fix
		 *                   increment (++a), binary in-equality (a != b)
		 *                   and dereferencing operations (*a).
		 */
		template <typename InputIter>
		Join(InputIter first, InputIter last, const ConversionType& delim, AppenderType append = AppenderType());
		~Join() { }
		/**
		 * Get the StorageType object containing the joined items
		 * @return The joined items
		 */
		const StorageType& storage() const { return m_storage; }

	private:
		StorageType m_storage;
	};

	template <typename StorageType, typename ConversionType, typename AppenderType>
	template <typename InputIter>
	Join<StorageType, ConversionType, AppenderType>::Join(InputIter first, InputIter last, const ConversionType& delim, AppenderType append)
		: m_storage()
	{
		if( first != last )
		{
			append(m_storage, ConversionType(*first));
			for( ++first; first != last; ++first )
			{
				append(m_storage, delim);
				append(m_storage, ConversionType(*first));
			}
		}
	}

} // end namespace BLOCXX_NAMESPACE
#endif
