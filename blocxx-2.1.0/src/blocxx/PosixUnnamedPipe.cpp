/*******************************************************************************
* Copyright (C) 2005, Vintela, Inc. All rights reserved.
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
*       Vintela, Inc., 
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
 * @author Jon Carey
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"

#if !defined(BLOCXX_WIN32) 

#include "blocxx/PosixUnnamedPipe.hpp"
#include "blocxx/AutoPtr.hpp"
#include "blocxx/IOException.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/SocketUtils.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/DescriptorUtils.hpp"
#include "blocxx/SignalScope.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/GlobalString.hpp"


#include "blocxx/Thread.hpp"
#ifdef BLOCXX_HAVE_UNISTD_H
	#include <unistd.h>
#endif
#include <sys/socket.h>
#include <sys/types.h>

#include <fcntl.h>
#include <errno.h>
#include <cstring>

#if defined(BLOCXX_DARWIN)
// Necessary for detecting the kernel version in order to activate the descriptor passing workaround.
#include "blocxx/ThreadOnce.hpp"
#include "blocxx/PosixRegEx.hpp"
#include <sys/utsname.h>
#endif


namespace BLOCXX_NAMESPACE
{

namespace
{
	int upclose(int fd)
	{
		int rc;
		do
		{
			rc = ::close(fd);
		} while (rc < 0 && errno == EINTR);
		if (rc == -1)
		{
			int lerrno = errno;
			Logger lgr("blocxx");
			BLOCXX_LOG_ERROR(lgr, Format("Closing pipe handle %1 failed: %2", fd, lerrno));
		}
		return rc;
	}

	::ssize_t upread(int fd, void * buf, std::size_t count)
	{
		::ssize_t rv;
		do
		{
			Thread::testCancel();
			rv = ::read(fd, buf, count);
		} while (rv < 0 && errno == EINTR);
		return rv;
	}

	::ssize_t upwrite(int fd, void const * buf, std::size_t count)
	{
		::ssize_t rv;
		// block SIGPIPE so we don't kill the process if the pipe is closed.
		SignalScope ss(SIGPIPE, SIG_IGN);
		do
		{
			Thread::testCancel();
			rv = ::write(fd, buf, count);
		} while (rv < 0 && errno == EINTR);
		return rv;
	}

	int upaccept(int s, struct sockaddr * addr, socklen_t * addrlen)
	{
		int rv;
		do
		{
			rv = ::accept(s, addr, addrlen);
		} while (rv < 0 && errno == EINTR);
		return rv;
	}
	enum EDirection
	{
		E_WRITE_PIPE, E_READ_PIPE
	};

	// bufsz MUST be an int, and not some other integral type (address taken)
	//
	void setKernelBufferSize(Descriptor sockfd, int bufsz, EDirection edir)
	{
		if (sockfd == BLOCXX_INVALID_HANDLE)
		{
			return;
		}

		int optname = (edir == E_WRITE_PIPE ? SO_SNDBUF : SO_RCVBUF);

		int getbufsz;
		socklen_t getbufsz_len = sizeof(getbufsz);

#ifdef BLOCXX_NCR
		int errc = ::getsockopt(sockfd, SOL_SOCKET, optname, (char*)&getbufsz, &getbufsz_len);
#else
		int errc = ::getsockopt(sockfd, SOL_SOCKET, optname, &getbufsz, &getbufsz_len);
#endif
		if (errc == 0 && getbufsz < bufsz)
		{
#ifdef BLOCXX_NCR
			::setsockopt(sockfd, SOL_SOCKET, optname, (char*)&bufsz, sizeof(bufsz));
#else
			::setsockopt(sockfd, SOL_SOCKET, optname, &bufsz, sizeof(bufsz));
#endif
		}
	}

	void setDefaultKernelBufsz(Descriptor sockfd_read, Descriptor sockfd_write)
	{
		int const BUFSZ = 64 * 1024;
		setKernelBufferSize(sockfd_read, BUFSZ, E_READ_PIPE);
		setKernelBufferSize(sockfd_write, BUFSZ, E_WRITE_PIPE);
	}

	GlobalString COMPONENT_NAME = BLOCXX_GLOBAL_STRING_INIT("blocxx.PosixUnnamedPipe");

#if defined(BLOCXX_DARWIN)
	// Mac OS X < 10.5 has a kernel bug related to passing descriptors. As a workaround, descriptors are passed synchronously.
	// This variable determines whether the workaround will be used. It will be set to false by detectDescriptorPassingBug()
	bool needDescriptorPassingWorkaround = true;

	// This is the control to ensure that detectDescriptorPassingBug() is only called once.
	OnceFlag detectDescriptorPassingBugFlag = BLOCXX_ONCE_INIT;

	// This function can not have logging statements or they will be sent over the pipe before sending the ACK.
	void detectDescriptorPassingBug()
	{
		// until OS X 10.5 is actually released, assume it will be broken (even though Apple said it is fixed)
		needDescriptorPassingWorkaround = true;
		return;
#if 0
		// if uname() reports the version as < 9.0.0 then we'll need the workaround.
		struct utsname unamerv;
		if (::uname(&unamerv) == -1)
		{
			needDescriptorPassingWorkaround = true; // unknown, so just assume it's necessary.
			return;
		}
		String release(unamerv.release);
		PosixRegEx re("([^.]*)\\..*");
		StringArray releaseCapture = re.capture(release);
		if (releaseCapture.size() < 2)
		{
			needDescriptorPassingWorkaround = true; // unknown, so just assume it's necessary.
			return;
		}
		String majorRelease = releaseCapture[1];
		try
		{
			needDescriptorPassingWorkaround = (majorRelease.toInt32() < 9);
		} 
		catch (StringConversionException& e)
		{
			needDescriptorPassingWorkaround = true; // unknown, so just assume it's necessary.
			return;
		}
#endif
	}
#endif

}

#ifdef BLOCXX_NETWARE
namespace
{
class AcceptThread
{
public:
	AcceptThread(int serversock)
		: m_serversock(serversock)
		, m_serverconn(-1)
	{
	}

	void acceptConnection();
	int getConnectFD() { return m_serverconn; }
private:
	int m_serversock;
	int m_serverconn;
};

void
AcceptThread::acceptConnection()
{
	struct sockaddr_in sin;
	size_t val;
	int tmp = 1;

	tmp = 1;
	::setsockopt(m_serversock, IPPROTO_TCP, 1,		// #define TCP_NODELAY 1
		(char*) &tmp, sizeof(int));

	val = sizeof(struct sockaddr_in);
	if ((m_serverconn = upaccept(m_serversock, (struct sockaddr*)&sin, &val))
	   == -1)
	{
		return;
	}
	tmp = 1;
	::setsockopt(m_serverconn, IPPROTO_TCP, 1, // #define TCP_NODELAY 1
		(char *) &tmp, sizeof(int));
	tmp = 0;
	::setsockopt(m_serverconn, SOL_SOCKET, SO_KEEPALIVE,
				 (char*) &tmp, sizeof(int));
}

void*
runConnClass(void* arg)
{
	AcceptThread* acceptThread = (AcceptThread*)(arg);
	acceptThread->acceptConnection();
	::pthread_exit(NULL);
	return 0;
}

int
_pipe(int *fds)
{
	int svrfd, lerrno, connectfd;
	size_t val;
	struct sockaddr_in sin;

	svrfd = socket( AF_INET, SOCK_STREAM, 0 );
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl( 0x7f000001 ); // loopback
	sin.sin_port = 0;
	memset(sin.sin_zero, 0, 8 );
	if (bind(svrfd, (struct sockaddr * )&sin, sizeof( struct sockaddr_in ) ) == -1)
	{
		int lerrno = errno;
		upclose(svrfd);
		fprintf(stderr, "CreateSocket(): Failed to bind on socket" );
		return -1;
	}
	if (listen(svrfd, 1) == -1)
	{
		int lerrno = errno;
		upclose(svrfd);
		return -1;
	}
	val = sizeof(struct sockaddr_in);
	if (getsockname(svrfd, ( struct sockaddr * )&sin, &val ) == -1)
	{
		int lerrno = errno;
		fprintf(stderr, "CreateSocket(): Failed to obtain socket name" );
		upclose(svrfd);
		return -1;
	}

	AcceptThread* pat = new AcceptThread(svrfd);
	pthread_t athread;
	// Start thread that will accept connection on svrfd.
	// Once a connection is made the thread will exit.
	pthread_create(&athread, NULL, runConnClass, pat);

	int clientfd = socket(AF_INET, SOCK_STREAM, 0);
	if (clientfd == -1)
	{
		delete pat;
		return -1;
	}

	// Connect to server
	struct sockaddr_in csin;
	csin.sin_family = AF_INET;
	csin.sin_addr.s_addr = htonl(0x7f000001); // loopback
	csin.sin_port = sin.sin_port;
	if (::connect(clientfd, (struct sockaddr*)&csin, sizeof(csin)) == -1)
	{
		delete pat;
		return -1;
	}

#define TCP_NODELAY 1
	int tmp = 1;
	//
	// Set for Non-blocking writes and disable keepalive
	//
	::setsockopt(clientfd, IPPROTO_TCP, TCP_NODELAY, (char*)&tmp, sizeof(int));
	tmp = 0;
	::setsockopt(clientfd, SOL_SOCKET, SO_KEEPALIVE, (char*)&tmp, sizeof(int));

	void* threadResult;
	// Wait for accept thread to terminate
	::pthread_join(athread, &threadResult);

	upclose(svrfd);
	fds[0] = pat->getConnectFD();
	fds[1] = clientfd;
	delete pat;
	return 0;
}
}
#endif // BLOCXX_NETWARE

//////////////////////////////////////////////////////////////////////////////
PosixUnnamedPipe::PosixUnnamedPipe(EOpen doOpen)
{
	m_fds[0] = m_fds[1] = BLOCXX_INVALID_HANDLE;
	if (doOpen)
	{
		open();
	}
	setTimeouts(Timeout::relative(60 * 10)); // 10 minutes. This helps break deadlocks when using safePopen()
	setBlocking(E_BLOCKING); // necessary to set the pipes up right.
}

//////////////////////////////////////////////////////////////////////////////
PosixUnnamedPipe::PosixUnnamedPipe(AutoDescriptor inputfd, AutoDescriptor outputfd)
{
	m_fds[0] = inputfd.get();
	m_fds[1] = outputfd.get();
	setTimeouts(Timeout::relative(60 * 10)); // 10 minutes. This helps break deadlocks when using safePopen()
	setBlocking(E_BLOCKING);
	setDefaultKernelBufsz(m_fds[0], m_fds[1]);
	inputfd.release();
	outputfd.release();
}

//////////////////////////////////////////////////////////////////////////////
PosixUnnamedPipe::~PosixUnnamedPipe()
{
	close();
}
//////////////////////////////////////////////////////////////////////////////
namespace
{
	typedef UnnamedPipe::EBlockingMode EBlockingMode;

	void set_desc_blocking(
		int d, EBlockingMode & bmflag, EBlockingMode blocking_mode)
	{
		BLOCXX_ASSERT(d != BLOCXX_INVALID_HANDLE);
		int fdflags = fcntl(d, F_GETFL, 0);
		if (fdflags == -1)
		{
			BLOCXX_THROW_ERRNO_MSG(IOException, "Failed to set pipe blocking mode");
		}
		if (blocking_mode == UnnamedPipe::E_BLOCKING)
		{
			fdflags &= ~O_NONBLOCK;
		}
		else
		{
			fdflags |= O_NONBLOCK;
		}
		if (fcntl(d, F_SETFL, fdflags) == -1)
		{
			BLOCXX_THROW_ERRNO_MSG(IOException, "Failed to set pipe blocking mode");
		}
		bmflag = blocking_mode;
	}
}
//////////////////////////////////////////////////////////////////////////////
void
PosixUnnamedPipe::setBlocking(EBlockingMode blocking_mode)
{
	BLOCXX_ASSERT(m_fds[0] != BLOCXX_INVALID_HANDLE || m_fds[1] != BLOCXX_INVALID_HANDLE);

	for (size_t i = 0; i < 2; ++i)
	{
		if (m_fds[i] != -1)
		{
			set_desc_blocking(m_fds[i], m_blocking[i], blocking_mode);
		}
	}
}
//////////////////////////////////////////////////////////////////////////////
void
PosixUnnamedPipe::setWriteBlocking(EBlockingMode blocking_mode)
{
	set_desc_blocking(m_fds[1], m_blocking[1], blocking_mode);
}
//////////////////////////////////////////////////////////////////////////////
void
PosixUnnamedPipe::setReadBlocking(EBlockingMode blocking_mode)
{
	set_desc_blocking(m_fds[0], m_blocking[0], blocking_mode);
}
//////////////////////////////////////////////////////////////////////////////
void
PosixUnnamedPipe::open()
{
	if (m_fds[0] != BLOCXX_INVALID_HANDLE)
	{
		close();
	}
#if defined(BLOCXX_NETWARE)
	if (_pipe(m_fds) == BLOCXX_INVALID_HANDLE)
	{
		m_fds[0] = m_fds[1] = BLOCXX_INVALID_HANDLE;
		BLOCXX_THROW_ERRNO_MSG(UnnamedPipeException, "PosixUnamedPipe::open(): soketpair()");
	}

#else
	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_fds) == -1)
	{
		m_fds[0] = m_fds[1] = -1;
		BLOCXX_THROW_ERRNO_MSG(UnnamedPipeException, "PosixUnamedPipe::open(): soketpair()");
	}
	::shutdown(m_fds[0], SHUT_WR);
	::shutdown(m_fds[1], SHUT_RD);
	setDefaultKernelBufsz(m_fds[0], m_fds[1]);
#endif
}
//////////////////////////////////////////////////////////////////////////////
int
PosixUnnamedPipe::close()
{
	int rc = -1;

	// handle the case where both input and output are the same descriptor.  It can't be closed twice.
	if (m_fds[0] == m_fds[1])
	{
		m_fds[1] = BLOCXX_INVALID_HANDLE;
	}

	if (m_fds[0] != BLOCXX_INVALID_HANDLE)
	{

		rc = upclose(m_fds[0]);
		m_fds[0] = BLOCXX_INVALID_HANDLE;
	}

	if (m_fds[1] != BLOCXX_INVALID_HANDLE)
	{
		rc = upclose(m_fds[1]);
		m_fds[1] = BLOCXX_INVALID_HANDLE;
	}

	return rc;
}
//////////////////////////////////////////////////////////////////////////////
bool
PosixUnnamedPipe::isOpen() const
{
	return (m_fds[0] != BLOCXX_INVALID_HANDLE) || (m_fds[1] != BLOCXX_INVALID_HANDLE);
}

//////////////////////////////////////////////////////////////////////////////
int
PosixUnnamedPipe::closeInputHandle()
{
	int rc = -1;
	if (m_fds[0] != BLOCXX_INVALID_HANDLE)
	{
		if (m_fds[0] != m_fds[1])
		{
			rc = upclose(m_fds[0]);
		}
		m_fds[0] = BLOCXX_INVALID_HANDLE;
	}
	return rc;
}
//////////////////////////////////////////////////////////////////////////////
int
PosixUnnamedPipe::closeOutputHandle()
{
	int rc = -1;
	if (m_fds[1] != BLOCXX_INVALID_HANDLE)
	{
		if (m_fds[0] != m_fds[1])
		{
			rc = upclose(m_fds[1]);
		}
		m_fds[1] = BLOCXX_INVALID_HANDLE;
	}
	return rc;
}
//////////////////////////////////////////////////////////////////////////////
int
PosixUnnamedPipe::write(const void* data, int dataLen, ErrorAction errorAsException)
{
	int rc = -1;
	if (m_fds[1] != BLOCXX_INVALID_HANDLE)
	{
		if (m_blocking[1] == E_BLOCKING)
		{
			rc = SocketUtils::waitForIO(m_fds[1], getWriteTimeout(), SocketFlags::E_WAIT_FOR_OUTPUT);
			if (rc != 0)
			{
				if (rc == ETIMEDOUT)
				{
					errno = ETIMEDOUT;
				}
				if (errorAsException == E_THROW_ON_ERROR)
				{
					BLOCXX_THROW_ERRNO_MSG(IOException, "SocketUtils::waitForIO failed.");
				}
				else
				{
					return -1;
				}
			}
		}
		rc = upwrite(m_fds[1], data, dataLen);
	}
	if (errorAsException == E_THROW_ON_ERROR && rc == -1)
	{
		if (m_fds[1] == BLOCXX_INVALID_HANDLE)
		{
			BLOCXX_THROW(IOException, "pipe write failed because pipe is closed");
		}
		else
		{
			BLOCXX_THROW_ERRNO_MSG(IOException, "pipe write failed");
		}
	}
	return rc;
}
//////////////////////////////////////////////////////////////////////////////
int
PosixUnnamedPipe::read(void* buffer, int bufferLen, ErrorAction errorAsException)
{
	int rc = -1;
	if (m_fds[0] != BLOCXX_INVALID_HANDLE)
	{
		if (m_blocking[0] == E_BLOCKING)
		{
			rc = SocketUtils::waitForIO(m_fds[0], getReadTimeout(), SocketFlags::E_WAIT_FOR_INPUT);
			if (rc != 0)
			{
				if (rc == ETIMEDOUT)
				{
					errno = ETIMEDOUT;
				}
				if (errorAsException == E_THROW_ON_ERROR)
				{
					BLOCXX_THROW_ERRNO_MSG(IOException, "SocketUtils::waitForIO failed.");
				}
				else
				{
					return -1;
				}
			}
		}
		rc = upread(m_fds[0], buffer, bufferLen);
	}

	if (rc == 0)
	{
		closeInputHandle();
	}

	if (errorAsException == E_THROW_ON_ERROR && rc == -1)
	{
		if (m_fds[0] == BLOCXX_INVALID_HANDLE)
		{
			BLOCXX_THROW(IOException, "pipe read failed because pipe is closed");
		}
		else
		{
			BLOCXX_THROW_ERRNO_MSG(IOException, "pipe read failed");
		}
	}
	return rc;
}
//////////////////////////////////////////////////////////////////////////////
Select_t
PosixUnnamedPipe::getReadSelectObj() const
{
	return m_fds[0];
}

//////////////////////////////////////////////////////////////////////////////
Select_t
PosixUnnamedPipe::getWriteSelectObj() const
{
	return m_fds[1];
}

//////////////////////////////////////////////////////////////////////////////
void
PosixUnnamedPipe::passDescriptor(Descriptor descriptor, const UnnamedPipeRef& ackPipe, const ProcessRef& targetProcess)
{
	int rc = -1;
	if (m_fds[1] != BLOCXX_INVALID_HANDLE)
	{
		if (m_blocking[1] == E_BLOCKING)
		{
			rc = SocketUtils::waitForIO(m_fds[1], getWriteTimeout(), SocketFlags::E_WAIT_FOR_OUTPUT);

			if (rc != 0)
			{
				if (rc == ETIMEDOUT)
				{
					errno = ETIMEDOUT;
				}
				BLOCXX_THROW_ERRNO_MSG(IOException, "SocketUtils::waitForIO failed.");
			}
		}
 
		rc = blocxx::passDescriptor(m_fds[1], descriptor);
		if (rc == -1)
		{
			BLOCXX_THROW_ERRNO_MSG(IOException, "sendDescriptor() failed: passDescriptor()");
		}

#if defined(BLOCXX_DARWIN)
		callOnce(detectDescriptorPassingBugFlag, detectDescriptorPassingBug);
		if (rc != -1 && needDescriptorPassingWorkaround)
		{
			// This ignores the blocking and timeouts, because this ACK shouldn't timeout.
			rc = SocketUtils::waitForIO(ackPipe->getInputDescriptor(), Timeout::infinite, SocketFlags::E_WAIT_FOR_INPUT);
			if (rc != -1)
			{
				char ack = 'Z';
				rc = ackPipe->read(&ack, sizeof(ack), E_RETURN_ON_ERROR);
				if (rc == -1)
				{
					BLOCXX_THROW_ERRNO_MSG(IOException, "sendDescriptor() failed: ackPipe->read()");
				}
				if (ack != 'A')
				{
					BLOCXX_THROW(IOException, Format("sendDescriptor() failed: ackPipe->read() didn't get 'A', got %1", static_cast<int>(ack)).c_str());
				}
			}
			else
			{
				BLOCXX_THROW_ERRNO_MSG(IOException, "sendDescriptor() failed: waitForIO()");
			}
		}
#endif
	}
	if (rc == -1)
	{
		if (m_fds[1] == BLOCXX_INVALID_HANDLE)
		{
			BLOCXX_THROW(IOException, "sendDescriptor() failed because pipe is closed");
		}
		else
		{
			BLOCXX_THROW_ERRNO_MSG(IOException, "sendDescriptor() failed");
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
AutoDescriptor
PosixUnnamedPipe::receiveDescriptor(const UnnamedPipeRef& ackPipe)
{
	int rc = -1;
	AutoDescriptor descriptor;
	if (m_fds[0] != BLOCXX_INVALID_HANDLE)
	{
		if (m_blocking[0] == E_BLOCKING)
		{
			rc = SocketUtils::waitForIO(m_fds[0], getReadTimeout(), SocketFlags::E_WAIT_FOR_INPUT);

			if (rc != 0)
			{
				if (rc == ETIMEDOUT)
				{
					errno = ETIMEDOUT;
				}
				BLOCXX_THROW_ERRNO_MSG(IOException, "SocketUtils::waitForIO failed.");
			}
		}
		descriptor = blocxx::receiveDescriptor(m_fds[0]);

#if defined(BLOCXX_DARWIN)
		callOnce(detectDescriptorPassingBugFlag, detectDescriptorPassingBug);
		if (needDescriptorPassingWorkaround)
		{
			// This ignores the blocking and timeouts, because this ACK shouldn't timeout.
			rc = SocketUtils::waitForIO(ackPipe->getOutputDescriptor(), Timeout::infinite, SocketFlags::E_WAIT_FOR_OUTPUT);
			if (rc != -1)
			{
				char ack = 'A';
				ackPipe->write(&ack, sizeof(ack), E_THROW_ON_ERROR);
			}
		}
#endif
	}
	else
	{
		BLOCXX_THROW(IOException, "receiveDescriptor() failed because pipe is closed");
	}
	return descriptor;
}

//////////////////////////////////////////////////////////////////////////////
Descriptor
PosixUnnamedPipe::getInputDescriptor() const
{
	return m_fds[0];
}

//////////////////////////////////////////////////////////////////////////////
Descriptor
PosixUnnamedPipe::getOutputDescriptor() const
{
	return m_fds[1];
}

//////////////////////////////////////////////////////////////////////////////
EBlockingMode
PosixUnnamedPipe::getReadBlocking() const
{
	return m_blocking[0];
}

//////////////////////////////////////////////////////////////////////////////
EBlockingMode
PosixUnnamedPipe::getWriteBlocking() const
{
	return m_blocking[1];
}

} // end namespace BLOCXX_NAMESPACE

#endif
