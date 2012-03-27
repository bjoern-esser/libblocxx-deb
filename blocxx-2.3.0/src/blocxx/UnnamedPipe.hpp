/*******************************************************************************
* Copyright (C) 2005, 2010, Quest Software, Inc. All rights reserved.
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

#ifndef BLOCXX_UNNAMED_PIPE_HPP_INCLUDE_GUARD_
#define BLOCXX_UNNAMED_PIPE_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/IntrusiveCountableBase.hpp"
#include "blocxx/Exception.hpp"
#include "blocxx/IntrusiveReference.hpp"
#include "blocxx/IOIFC.hpp"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/AutoDescriptor.hpp"
#include "blocxx/NetworkTypes.hpp"

namespace BLOCXX_NAMESPACE
{


BLOCXX_DECLARE_APIEXCEPTION(UnnamedPipe, BLOCXX_COMMON_API);

/**
 * Abstract interface for an UnnamedPipe.
 * Blocking is enabled by default.
 */
class BLOCXX_COMMON_API UnnamedPipe : public IOIFC, public IntrusiveCountableBase
{
public:
	virtual ~UnnamedPipe();

	/**
	 * Write an int (native binary representation) to the pipe.
	 * If blocking is enabled, writeInt() will block for up to
	 * getWriteTimeout() seconds or forever if getWriteTimeout() == -1.
	 * Exception safety: No-throw
	 * @param value The value to write.
	 * @return The number of bytes written, -1 on error, ETIMEDOUT on timeout.
	 */
	int writeInt(int value);

	/**
	 * Writes a String to the pipe.
	 * This should be read from the other side by readString. If blocking
	 * is enabled, writeString() will block for up to getWriteTimeout()
	 * seconds or forever if getWriteTimeout() == -1.
	 * Exception safety: No-throw
	 * @param strData The String to write.
	 * @return The number of bytes written, -1 on error, ETIMEDOUT on timeout.
	 */
	int writeString(const String& strData);

	/**
	 * Reads an int (native binary representation) from the pipe.
	 * If blocking is enabled, readInt() will block for up to
	 * getReadTimeout() seconds or forever if getReadTimeout() == -1.
	 * Exception safety: No-throw
	 * @param value Out parameter where the read int will be stored.
	 * @return The number of bytes read, -1 on error, ETIMEDOUT on timeout.
	 */
	int readInt(int* value);

	/**
	 * Reads a String from the pipe.
	 * If blocking is enabled, readInt() will block for up to
	 * getReadTimeout() seconds or forever if getReadTimeout() == -1.
	 * Exception safety: Strong for the C++ instance. Fubared for the pipe.
	 * If an exception is thrown, the pipe won't be reset to it's pre-call
	 * state.
	 * @param strData Out parameter where the read int will be stored.
	 * @return The number of bytes read, -1 on error, ETIMEDOUT on timeout.
	 * @throws std::bad_alloc
	 */
	int readString(String& strData);

	static const int INFINITE_TIMEOUT = -1;
	/**
	 * Sets the read timeout value.
	 * If blocking is enabled this is the number of seconds a read
	 * operation will block.
	 * Exception safety: No-throw
	 * @param timeout The new read timeout.
	 */
	void setReadTimeout(const Timeout& timeout) { m_readTimeout = timeout; }
	/**
	 * Gets the read timeout value.
	 * If blocking is enabled this is the number of seconds a read
	 * operation will block.
	 * Exception safety: No-throw
	 * @return The read timeout.
	 */
	Timeout getReadTimeout() { return m_readTimeout; }
	/**
	 * Sets the write timeout value.
	 * If blocking is enabled this is the number of seconds a write
	 * operation will block.
	 * Exception safety: No-throw
	 * @param timeout The new write timeout.
	 */
	void setWriteTimeout(const Timeout& timeout) { m_writeTimeout = timeout; }
	/**
	 * Gets the write timeout value.
	 * If blocking is enabled this is the number of seconds a write
	 * operation will block.
	 * Exception safety: No-throw
	 * @return The write timeout.
	 */
	Timeout getWriteTimeout() { return m_writeTimeout; }
	/**
	 * Sets the read & write timeout values.
	 * If blocking is enabled this is the number of seconds a read or write
	 * operation will block.
	 * Exception safety: No-throw
	 * @param timeout The new timeout.
	 */
	void setTimeouts(const Timeout& timeout) { m_readTimeout = m_writeTimeout = timeout; }

	/**
	 * Read from the pipe and collect into a string, until the other end of
	 * the pipe is closed.
	 * Exception safety: Strong for the C++ instance. Fubared for the pipe.
	 * If an exception is thrown, the pipe won't be reset to it's pre-call
	 * state.
	 * @return The string with collected data.
	 * @throws IOException on error
	 */
	String readAll();

	/**
	 * Open the pipe.
	 */
	virtual void open() = 0;

	/**
	 * Close the pipe.
	 * @return -1 on error, 0 on success.
	 */
	virtual int close() = 0;

	/**
	 * Is the pipe open or closed?
	 */
	virtual bool isOpen() const = 0;

	/**
	 * Get the read select object
	 */
	virtual Select_t getReadSelectObj() const = 0;

	/**
	 * Get the write select object
	 */
	virtual Select_t getWriteSelectObj() const = 0;


	enum EBlockingMode
	{
		E_NONBLOCKING,	//<! Non-Blocking mode flag.
		E_BLOCKING	//<! Blocking mode flag.
	};
	/**
	 * Set the pipe's blocking mode.
	 * Precondition: The pipe is open.
	 * @param isBlocking new blocking mode.
	 */
	virtual void setBlocking(EBlockingMode isBlocking=E_BLOCKING) = 0;

	/**
	 * Set blocking mode for writing to pipe.
	 * Precondition: The pipe output is open.
	 * @param isBlocking new write blocking mode.
	 */
	virtual void setWriteBlocking(EBlockingMode isBlocking=E_BLOCKING) = 0;

	/**
	 * Set blocking mode for reading from pipe.
	 * Precondition: The pipe input is open.
	 * @param isBlocking new read blocking mode.
	 */
	virtual void setReadBlocking(EBlockingMode isBlocking=E_BLOCKING) = 0;

	/**
	* Get the current blocking mode for reading from pipe.
	*/
	virtual EBlockingMode getReadBlocking() const = 0;

	/**
	* Get the current blocking mode for writing from pipe.
	*/
	virtual EBlockingMode getWriteBlocking() const = 0;

	/**
	 * Get the underlying input descriptor. The UnnamedPipe instance
	 * retains ownership of the descriptor.
	 */
	virtual Descriptor getInputDescriptor() const = 0;

	/**
	 * Get the underlying output descriptor. The UnnamedPipe instance
	 * retains ownership of the descriptor.
	 */
	virtual Descriptor getOutputDescriptor() const = 0;

	/**
	 * Sends a @c Descriptor to the peer.
	 *
	 * @param h The @c Descriptor to send.
	 *
	 * @throw IOException on I/O error.
	 */
	virtual void passDescriptor(Descriptor h, const UnnamedPipeRef& ackPipe = 0, const ProcessRef& targetProcess = 0) = 0;

	/**
	 * Gets a @c Descriptor from the peer.
	 *
	 * @return The @c Descriptor sent by the peer.
	 *
	 * @throw IOException on I/O error or if the peer does not send a
	 * @c Descriptor
	 */
	virtual AutoDescriptor receiveDescriptor(const UnnamedPipeRef& ackPipe = 0) = 0;

	virtual int closeInputHandle() = 0;
	virtual int closeOutputHandle() = 0;


	enum EOpen
	{
		E_DONT_OPEN,
		E_OPEN
	};
	/**
	 * Create an instance of the concrete class that implements the
	 * UnnamedPipe interface.
	 * The input and output of the pipe will be connected.
	 * The pipe defaults to have 10 minute timeouts.
	 * @param doOpen Open the pipe or not.
	 */
	static UnnamedPipeRef createUnnamedPipe(EOpen doOpen=E_OPEN);
	/**
	 * Create an instance of the concrete class that implements the
	 * UnnamedPipe interface connected bidirectionally to the
	 * specified descriptor.
	 * @param inputAndOutput The input and output descriptor to connect.
	 * @return Reference to the UnnamedPipe object.
	 */
	static UnnamedPipeRef createUnnamedPipeFromDescriptor(AutoDescriptor inputAndOutput);
	/**
	 * Create an instance of the concrete class that implements the
	 * UnnamedPipe interface connected to the specified descriptors.
	 * @param input The input descriptor to connect.
	 * @param output The output descriptor to connect.
	 * @return Reference to the UnnamedPipe object.
	 */
	static UnnamedPipeRef createUnnamedPipeFromDescriptor(AutoDescriptor input, AutoDescriptor output);

	/**
	 * Create a connected pair of pipes. The input from first will be
	 * connected to the output of second, and the input from second will
	 * be connected to the output of first.
	 * The pipes default to have 10 minute timeouts.
	 * @param first The first pipe.
	 * @param second The second pipe.
	 * @throws UnnamedPipeException with error code EMFILE if too many descriptors are in use by the process.
	 */
	static void createConnectedPipes(UnnamedPipeRef& first,
	                                 UnnamedPipeRef& second);

	/**
	 * Create an instance of the concrete class that implements the
	 * UnnamedPipe interface connected to stdin.
	 * @return Reference to the UnnamedPipe object.
	 */
	static UnnamedPipeRef createStdin();
	/**
	 * Create an instance of the concrete class that implements the
	 * UnnamedPipe interface connected to stdout.
	 * @return Reference to the UnnamedPipe object.
	 */
	static UnnamedPipeRef createStdout();
	/**
	 * Create an instance of the concrete class that implements the
	 * UnnamedPipe interface connected to stdin and stdout.
	 * @return Reference to the UnnamedPipe object.
	 */
	static UnnamedPipeRef createStdinStdout();
	/**
	 * Create an instance of the concrete class that implements the
	 * UnnamedPipe interface connected to stderr.
	 * @return Reference to the UnnamedPipe object.
	 */
	static UnnamedPipeRef createStderr();

protected:
	UnnamedPipe()
		: m_readTimeout(Timeout::infinite)
		, m_writeTimeout(Timeout::infinite)
	{}
private:
	Timeout m_readTimeout;
	Timeout m_writeTimeout;
};
BLOCXX_EXPORT_TEMPLATE(BLOCXX_COMMON_API, IntrusiveReference, UnnamedPipe);

} // end namespace BLOCXX_NAMESPACE

#endif	// BLOCXX_UNNAMED_PIPE_HPP_INCLUDE_GUARD_
