/*******************************************************************************
* Copyright (C) 2009-2010, Quest Software, Inc. All rights reserved.
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
 * @name		BLOCXX_SSLSocketImpl.cpp
 * @author J. Bart Whiteley
 * @author Dan Nuffer
 * @author Kevin Harris
 *
 * @description
 *		Implementation file for the SSLSocketImpl class.
 */

#include "blocxx/BLOCXX_config.h"

#ifdef BLOCXX_HAVE_OPENSSL

#include "blocxx/SSLSocketImpl.hpp"
#include "blocxx/SSLException.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/Timeout.hpp"
#include <openssl/err.h>
#include "blocxx/Format.hpp"
#include "blocxx/SocketUtils.hpp"


namespace BLOCXX_NAMESPACE
{
//////////////////////////////////////////////////////////////////////////////
SSLSocketImpl::SSLSocketImpl(SSLClientCtxRef sslCtx)
	: SocketBaseImpl()
	, m_ssl(0)
	, m_sslIsConnected(false)
	, m_sslCtx(sslCtx)
{
}

namespace
{

	void cleanupSSL(SSL*& ssl)
	{
		SSL_free(ssl);
		ssl = 0;
	}

void sslWaitForIO(SocketBaseImpl& s, int type)
{
	if(type == SSL_ERROR_WANT_READ)
	{
		s.waitForInput(Timeout::infinite);
	}
	else
	{
		s.waitForOutput(Timeout::infinite);
	}
}

void shutdownSSL(SSL* ssl)
{
	BLOCXX_ASSERT(ssl != 0);
	if (SSL_shutdown(ssl) == -1)
	{
		// do nothing, since we're probably cleaning up.  If we had a logger we should log the reason why this failed....
	}
	// we're not going to reuse the SSL context, so we do a
	// unidirectional shutdown, and don't need to call it twice
}

void connectWithSSL(SSL* ssl, SocketBaseImpl& s)
{
	BLOCXX_ASSERT(ssl != 0);
	int retries = 0;
	ERR_clear_error();
	int cc = SSL_connect(ssl);
	cc = SSL_get_error(ssl, cc);
	while((cc == SSL_ERROR_WANT_READ
		|| cc == SSL_ERROR_WANT_WRITE)
		&& retries < BLOCXX_SSL_RETRY_LIMIT)
	{
		sslWaitForIO(s, cc);
		ERR_clear_error();
		cc = SSL_connect(ssl);
		cc = SSL_get_error(ssl, cc);
		retries++;
	}

	if (cc != SSL_ERROR_NONE)
	{
		BLOCXX_THROW(SSLException, Format("SSL connect error: %1", SSLCtxMgr::getOpenSSLErrorDescription()).c_str());
	}
}

int acceptSSL(SSL* ssl, SocketBaseImpl& s, String& errorDescription)
{
	BLOCXX_ASSERT(ssl != 0);
	int retries = 0;
	int cc = SSL_ERROR_WANT_READ;
	while((cc == SSL_ERROR_WANT_READ || cc == SSL_ERROR_WANT_WRITE)
		&& retries < BLOCXX_SSL_RETRY_LIMIT)
	{
		sslWaitForIO(s, cc);
		ERR_clear_error();
		cc = SSL_accept(ssl);
		cc = SSL_get_error(ssl, cc);
		retries++;
	}
	if (cc == SSL_ERROR_NONE)
	{
		return 0;
	}
	else
	{
		errorDescription = SSLCtxMgr::getOpenSSLErrorDescription();
		return -1;
	}
}

}	// End of unnamed namespace

//////////////////////////////////////////////////////////////////////////////
SSLSocketImpl::SSLSocketImpl(SocketHandle_t fd,
	SocketAddress::AddressType addrType, const SSLServerCtxRef& sslCtx)
	: SocketBaseImpl(fd, addrType)
{
	m_sslIsConnected = false;
	BLOCXX_ASSERT(sslCtx);
	ERR_clear_error();
	m_ssl = SSL_new(sslCtx->getSSLCtx());
	if (!m_ssl)
	{
		BLOCXX_THROW(SSLException, Format("SSL_new failed: %1", SSLCtxMgr::getOpenSSLErrorDescription()).c_str());
	}

	if (SSL_set_ex_data(m_ssl, SSLServerCtx::SSL_DATA_INDEX, &m_owctx) == 0)
	{
		BLOCXX_THROW(SSLException, Format("SSL_set_ex_data failed: %1", SSLCtxMgr::getOpenSSLErrorDescription()).c_str());
	}

	BIO* bio = BIO_new_socket(fd, BIO_NOCLOSE);
	if (!bio)
	{
		cleanupSSL(m_ssl);
		BLOCXX_THROW(SSLException, Format("BIO_new_socket failed: %1", SSLCtxMgr::getOpenSSLErrorDescription()).c_str());
	}

	SSL_set_bio(m_ssl, bio, bio);
	String errorDescription;
	if (acceptSSL(m_ssl, *this, errorDescription) != 0)
	{
		shutdownSSL(m_ssl);
		cleanupSSL(m_ssl);
		ERR_remove_state(0); // cleanup memory SSL may have allocated
		BLOCXX_THROW(SSLException, Format("SSLSocketImpl ctor: SSL accept error while connecting to %1: %2", getPeerAddress().toString(), errorDescription).c_str());
	}
	if (!SSLCtxMgr::checkClientCert(m_ssl, getPeerAddress().getName()))
	{
		shutdownSSL(m_ssl);
		cleanupSSL(m_ssl);
		ERR_remove_state(0); // cleanup memory SSL may have allocated
		BLOCXX_THROW(SSLException, "SSL failed to authenticate client");
	}
	m_sslIsConnected = true;
}

/// @todo Get rid of this one later.
//////////////////////////////////////////////////////////////////////////////
SSLSocketImpl::SSLSocketImpl(SocketHandle_t fd,
	SocketAddress::AddressType addrType)
	: SocketBaseImpl(fd, addrType)
{
	m_sslIsConnected = false;
	ERR_clear_error();
	m_ssl = SSL_new(SSLCtxMgr::getSSLCtxServer());
	if (!m_ssl)
	{
		BLOCXX_THROW(SSLException, Format("SSL_new failed: %1", SSLCtxMgr::getOpenSSLErrorDescription()).c_str());
	}

	m_sbio = BIO_new_socket(fd, BIO_NOCLOSE);
	if (!m_sbio)
	{
		cleanupSSL(m_ssl);
		BLOCXX_THROW(SSLException, Format("BIO_new_socket failed: %1", SSLCtxMgr::getOpenSSLErrorDescription()).c_str());
	}

	SSL_set_bio(m_ssl, m_sbio, m_sbio);
	String errorDescription;
	if (acceptSSL(m_ssl, *this, errorDescription) != 0)
	{
		shutdownSSL(m_ssl);
		cleanupSSL(m_ssl);
		ERR_remove_state(0); // cleanup memory SSL may have allocated
		BLOCXX_THROW(SSLException, Format("SSLSocketImpl ctor: SSL accept error while connecting to %1: %2", getPeerAddress().toString(), errorDescription).c_str());
	}
	if (!SSLCtxMgr::checkClientCert(m_ssl, getPeerAddress().getName()))
	{
		shutdownSSL(m_ssl);
		cleanupSSL(m_ssl);
		ERR_remove_state(0); // cleanup memory SSL may have allocated
		BLOCXX_THROW(SSLException, "SSL failed to authenticate client");
	}
	m_sslIsConnected = true;
}
//////////////////////////////////////////////////////////////////////////////
SSLSocketImpl::SSLSocketImpl(const SocketAddress& addr)
	: SocketBaseImpl(addr)
{
	connectSSL();
}
//////////////////////////////////////////////////////////////////////////////
SSLSocketImpl::~SSLSocketImpl()
{
	try
	{
		disconnect();
		if (m_ssl)
		{
			cleanupSSL(m_ssl);
		}
		ERR_remove_state(0); // cleanup memory SSL may have allocated
	}
	catch (...)
	{
		// no exceptions allowed out of destructors.
	}
}
//////////////////////////////////////////////////////////////////////////////
Select_t
SSLSocketImpl::getSelectObj() const
{
	return SocketBaseImpl::getSelectObj();
}
//////////////////////////////////////////////////////////////////////////////
void
SSLSocketImpl::connect(const SocketAddress& addr)
{
	SocketBaseImpl::connect(addr);
	connectSSL();
}
//////////////////////////////////////////////////////////////////////////////
void
SSLSocketImpl::connectSSL()
{
	m_sslIsConnected = false;
	BLOCXX_ASSERT(m_sslCtx);
	if (m_ssl)
	{
		cleanupSSL(m_ssl);
	}
	ERR_clear_error();
	m_ssl = SSL_new(m_sslCtx->getSSLCtx());

	if (!m_ssl)
	{
		BLOCXX_THROW(SSLException, Format("SSL_new failed: %1", SSLCtxMgr::getOpenSSLErrorDescription()).c_str());
	}
	m_sbio = BIO_new_socket(getfd(), BIO_NOCLOSE);
	if (!m_sbio)
	{
		cleanupSSL(m_ssl);
		BLOCXX_THROW(SSLException, Format("BIO_new_socket failed: %1", SSLCtxMgr::getOpenSSLErrorDescription()).c_str());
	}
	SSL_set_bio(m_ssl, m_sbio, m_sbio);

	connectWithSSL(m_ssl, *this);

	if (!SSLCtxMgr::checkServerCert(m_ssl, getPeerAddress().getName()))
	{
		BLOCXX_THROW(SSLException, "Failed to validate peer certificate");
	}
	m_sslIsConnected = true;
}
//////////////////////////////////////////////////////////////////////////////
void
SSLSocketImpl::disconnect()
{
	if (SocketBaseImpl::isConnected())
	{
		if (m_ssl)
		{
			shutdownSSL(m_ssl);
		}
	}
	SocketBaseImpl::disconnect();
}
//////////////////////////////////////////////////////////////////////////////
bool SSLSocketImpl::isConnected() const
{
	return SocketBaseImpl::isConnected() && m_sslIsConnected;
}
//////////////////////////////////////////////////////////////////////////////
int
SSLSocketImpl::writeAux(const void* dataOut, int dataOutLen)
{
	return SSLCtxMgr::sslWrite(m_ssl, static_cast<const char*>(dataOut),
			dataOutLen);
}
//////////////////////////////////////////////////////////////////////////////
int
SSLSocketImpl::readAux(void* dataIn, int dataInLen)
{
	return SSLCtxMgr::sslRead(m_ssl, static_cast<char*>(dataIn),
			dataInLen);
}
//////////////////////////////////////////////////////////////////////////////
SSL*
SSLSocketImpl::getSSL() const
{
	return m_ssl;
}

//////////////////////////////////////////////////////////////////////////////
bool
SSLSocketImpl::peerCertVerified() const
{
    return (m_owctx.peerCertPassedVerify == OWSSLContext::VERIFY_PASS);
}

//////////////////////////////////////////////////////////////////////////////
// SSL buffer can contain the data therefore select
// does not work without checking SSL_pending() first.
bool
SSLSocketImpl::waitForInput(const Timeout& timeout)
{
   // SSL buffer contains data -> read them
   if (SSL_pending(m_ssl))
   {
	   return false;
   }
   return SocketBaseImpl::waitForInput(timeout);
}
//////////////////////////////////////////////////////////////////////////////

} // end namespace BLOCXX_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif // #ifdef BLOCXX_HAVE_OPENSSL

