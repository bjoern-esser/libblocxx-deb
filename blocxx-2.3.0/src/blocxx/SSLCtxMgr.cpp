/*******************************************************************************
* Copyright (C) 2001-2004 Quest Software, Inc. All rights reserved.
* Copyright (C) 2004 Novell, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
*  - Neither the name of Quest Software, Inc. nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Quest Software, Inc. OR THE CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/**
 * @author Bart Whiteley
 * @author Dan Nuffer
 * @author Mat Bess
 */

#include "blocxx/BLOCXX_config.h"
#ifdef BLOCXX_HAVE_OPENSSL
#include "blocxx/SSLCtxMgr.hpp"
#include "blocxx/GetPass.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/FileSystem.hpp"
#include "blocxx/ThreadImpl.hpp"
#include "blocxx/Mutex.hpp"
#include "blocxx/GlobalMutex.hpp"
#include "blocxx/MutexLock.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/MD5.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/SecureRand.hpp"
#include "blocxx/SignalScope.hpp"
#include "blocxx/LazyGlobal.hpp"

#include <openssl/rand.h>
#include <openssl/err.h>
#include <cstring>
#include <csignal>
#include <cerrno>
#ifndef BLOCXX_WIN32
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <fcntl.h>

#ifdef BLOCXX_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef BLOCXX_HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef BLOCXX_DEBUG
#include <iostream>
#endif

#include <fstream>

// This struct has to be in the global namespace
extern "C"
{
struct CRYPTO_dynlock_value
{
	BLOCXX_NAMESPACE::Mutex mutex;
};
}

namespace BLOCXX_NAMESPACE
{

namespace
{

BLOCXX_NAMESPACE::Mutex* mutex_buf = 0;

extern "C"
{

static struct CRYPTO_dynlock_value * dyn_create_function(const char *,int)
{
	return new CRYPTO_dynlock_value;
}

// these need to still be static, since they get exported because of extern "C"
static void dyn_lock_function(int mode, struct CRYPTO_dynlock_value *l,
				  const char *, int)
{
	if (mode & CRYPTO_LOCK)
	{
		l->mutex.acquire();
	}
	else
	{
		l->mutex.release();
	}
}

static void dyn_destroy_function(struct CRYPTO_dynlock_value *l,
				 const char *, int)
{
	delete l;
}

static unsigned long id_function()
{
	return static_cast<unsigned long>(BLOCXX_NAMESPACE::ThreadImpl::thread_t_ToUInt64(BLOCXX_NAMESPACE::ThreadImpl::currentThread()));
}

static void locking_function(int mode, int n, const char*, int)
{
	if (mode & CRYPTO_LOCK)
	{
		mutex_buf[n].acquire();
	}
	else
	{
		mutex_buf[n].release();
	}
}
} // end extern "C"

class X509Freer
{
public:
	X509Freer(X509* x509)
		: m_x509(x509)
	{
	}
	~X509Freer()
	{
		if (m_x509 != 0)
		{
			X509_free(m_x509);
		}
	}
private:
	X509* m_x509;
};

enum SSLLibraryInit_t {
	BLOCXX_SSL_LIBRARY_NOT_INITIALIZED,
	BLOCXX_SSL_LIBRARY_INITIALIZED,
	BLOCXX_SSL_LIBRARY_INITIALIZATION_DISABLED
};
SSLLibraryInit_t m_initState = BLOCXX_SSL_LIBRARY_NOT_INITIALIZED;
GlobalMutex m_initStateGuard = BLOCXX_GLOBAL_MUTEX_INIT();


enum SSLLocks_t {
	BLOCXX_SSL_LOCKS_NOT_USED,
	BLOCXX_SSL_LOCKS_USED,
	BLOCXX_SSL_LOCKS_DISABLED
};
SSLLocks_t m_locksState = BLOCXX_SSL_LOCKS_NOT_USED;
GlobalMutex m_locksStateGuard = BLOCXX_GLOBAL_MUTEX_INIT();

//////////////////////////////////////////////////////////////////////////////
struct SSLGlobalWork
{
	SSLGlobalWork()
	{
		MutexLock initLock(m_initStateGuard);
		if (m_initState == BLOCXX_SSL_LIBRARY_NOT_INITIALIZED)
		{
			m_initState = BLOCXX_SSL_LIBRARY_INITIALIZED;

			SSL_library_init();
			SSL_load_error_strings();
		}
		initLock.release();

		MutexLock locksLock(m_locksStateGuard);
		if (m_locksState == BLOCXX_SSL_LOCKS_NOT_USED)
		{
			m_locksState = BLOCXX_SSL_LOCKS_USED;

			if (!mutex_buf)
			{
				mutex_buf = new Mutex[CRYPTO_num_locks()];
			}

			CRYPTO_set_id_callback(id_function);
			CRYPTO_set_locking_callback(locking_function);

			// The following three CRYPTO_... functions are the OpenSSL functions
			// for registering the callbacks we implemented above
			CRYPTO_set_dynlock_create_callback(dyn_create_function);
			CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
			CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
		}
	}

	~SSLGlobalWork()
	{
		if (SSLCtxMgr::isClient() || SSLCtxMgr::isServer())
		{
			Secure::rand_save_state();
		}
		SSLCtxMgr::uninit();

		MutexLock locksLock(m_locksStateGuard);
		if (m_locksState == BLOCXX_SSL_LOCKS_USED)
		{
			CRYPTO_set_id_callback(NULL);
			CRYPTO_set_locking_callback(NULL);
			CRYPTO_set_dynlock_create_callback(NULL);
			CRYPTO_set_dynlock_lock_callback(NULL);
			CRYPTO_set_dynlock_destroy_callback(NULL);
			delete[] mutex_buf;
			mutex_buf = 0;
			m_locksState = BLOCXX_SSL_LOCKS_NOT_USED;
		}
	}
};


DefaultConstructedLazyGlobal<SSLGlobalWork>::type g_SSLGlobalWork = BLOCXX_LAZY_GLOBAL_DEFAULT_INIT;

} // end unnamed namespace

SSL_CTX* SSLCtxMgr::m_ctxClient = 0;
SSL_CTX* SSLCtxMgr::m_ctxServer = 0;
certVerifyFuncPtr_t SSLCtxMgr::m_clientCertVerifyCB = 0;
certVerifyFuncPtr_t SSLCtxMgr::m_serverCertVerifyCB = 0;

/////////////////////////////////////////////////////////////////////////////
// static
String
SSLCtxMgr::getOpenSSLErrorDescription()
{
	BIO* bio = BIO_new(BIO_s_mem());
	if (!bio)
	{
		return String();
	}
	ERR_print_errors(bio);
	char* p = 0;
	long len = BIO_get_mem_data(bio, &p);
	String rval(p, len);
	int freerv = BIO_free(bio);
	BLOCXX_ASSERT(freerv == 1);
	return rval;
}

/////////////////////////////////////////////////////////////////////////////
// static
void SSLCtxMgr::disableSSLInit()
{
	MutexLock lock(m_initStateGuard);
	if (m_initState == BLOCXX_SSL_LIBRARY_INITIALIZED)
	{
		BLOCXX_THROW(SSLException, "SSLCtxMgr::disableSSLInit(): init() cannot be disabled as it has already been called");
	}
	m_initState = BLOCXX_SSL_LIBRARY_INITIALIZATION_DISABLED;
}

/////////////////////////////////////////////////////////////////////////////
// static
void SSLCtxMgr::disableLocks()
{
	MutexLock lock(m_locksStateGuard);
	if (m_locksState == BLOCXX_SSL_LOCKS_USED)
	{
		BLOCXX_THROW(SSLException, "SSLCtxMgr::disableSSLLocks(): locks cannot be disabled as they are already in use");
	}
	m_locksState = BLOCXX_SSL_LOCKS_DISABLED;
}

/////////////////////////////////////////////////////////////////////////////
// static
Bool SSLCtxMgr::getSSLInitDisabled()
{
	MutexLock lock(m_initStateGuard);
	return m_initState == BLOCXX_SSL_LIBRARY_INITIALIZATION_DISABLED;
}

/////////////////////////////////////////////////////////////////////////////
// static
Bool SSLCtxMgr::getSSLLocksDisabled()
{
	MutexLock lock(m_locksStateGuard);
	return m_locksState == BLOCXX_SSL_LOCKS_DISABLED;
}

//////////////////////////////////////////////////////////////////////////////
SSL_CTX*
SSLCtxMgr::initCtx(const String& certfile, const String& keyfile, EVP_PKEY* pkey)
{
	g_SSLGlobalWork.get();  // trigger initialization of lazy global stuff

	ERR_clear_error();
	SSL_CTX* ctx = SSL_CTX_new(SSLv23_method());
	if (ctx == 0)
	{
		BLOCXX_THROW(SSLException, Format("SSLCtxMgr::initCtx(): SSL_CTX_new returned 0: %1", getOpenSSLErrorDescription()).c_str());
	}
	SSL_CTX_set_default_passwd_cb(ctx, pem_passwd_cb);
	if (!certfile.empty())
	{
		if (SSL_CTX_use_certificate_chain_file(ctx, certfile.c_str()) != 1)
		{
			SSL_CTX_free(ctx);
			BLOCXX_THROW(SSLException, Format("SSLCtxMgr::initCtx(): Couldn't read certificate from file: %1: %2",
				certfile, getOpenSSLErrorDescription()).c_str());
		}
		if (pkey)
		{
			int rv = SSL_CTX_use_PrivateKey(ctx, pkey); 
			if (rv != 1)
			{
				SSL_CTX_free(ctx);
				BLOCXX_THROW(SSLException, Format("SSLCtxMgr::initCtx(): Unable to set private key: %1",
					getOpenSSLErrorDescription()).c_str());
			}
		}
		else if (SSL_CTX_use_PrivateKey_file(ctx, keyfile.empty()?certfile.c_str():keyfile.c_str(), SSL_FILETYPE_PEM) != 1)
		{
			SSL_CTX_free(ctx);
			BLOCXX_THROW(SSLException, Format("SSLCtxMgr::initCtx(): Couldn't read key from file: %1: %2",
				keyfile.empty()?certfile:keyfile, getOpenSSLErrorDescription()).c_str());
		}
	}

	SSL_CTX_set_options(ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2 |
			SSL_OP_SINGLE_DH_USE); 
	if (SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH") != 1)
	{
		SSL_CTX_free(ctx);
		BLOCXX_THROW(SSLException, "SSLCtxMgr::initCtx(): Error setting cipher list (no valid ciphers)"); 
	}

	Secure::rand_init();

	return ctx;
}

//////////////////////////////////////////////////////////////////////////////
void
SSLCtxMgr::loadDHParams(SSL_CTX* ctx, const String& file)
{
	BLOCXX_ASSERT(ctx != 0);
	ERR_clear_error();
	BIO* bio = BIO_new_file(file.c_str(), "r");
	if (bio == NULL)
	{
		BLOCXX_THROW(SSLException, Format("SSLCtxMgr::loadDHParams(): Couldn't open DH file %1: %2", file, getOpenSSLErrorDescription()).c_str());
	}
	DH* ret = PEM_read_bio_DHparams(bio, NULL, NULL, NULL);
	BIO_free(bio);
	if (ret == 0)
	{
		BLOCXX_THROW(SSLException, Format("SSLCtxMgr::loadDHParams(): PEM_read_bio_DHparams failed: %1", getOpenSSLErrorDescription()).c_str());
	}
	if (SSL_CTX_set_tmp_dh(ctx, ret) != 1)
	{
		BLOCXX_THROW(SSLException, Format("SSLCtxMgr::loadDHParams(): Couldn't set DH parameters because SSL_CTX_set_tmp_dh failed: %1", getOpenSSLErrorDescription()).c_str());
	}
}
//////////////////////////////////////////////////////////////////////////////
void
SSLCtxMgr::generateEphRSAKey(SSL_CTX* ctx)
{
	BLOCXX_ASSERT(ctx != 0);
	ERR_clear_error();
	RSA* rsa = RSA_generate_key(512, RSA_F4, NULL, NULL);
	if (rsa == 0)
	{
		BLOCXX_THROW(SSLException, Format("SSLCtxMgr::generateEphRSAKey(): RSA_generate_key failed: %1", getOpenSSLErrorDescription()).c_str());
	}
	if (SSL_CTX_set_tmp_rsa(ctx, rsa) != 1)
	{
		RSA_free(rsa);
		BLOCXX_THROW(SSLException, Format("SSLCtxMgr::generateEphRSAKey(): SSL_CTX_set_tmp_rsa failed. Couldn't set RSA key: %1", getOpenSSLErrorDescription()).c_str());
	}
	RSA_free(rsa);
}
//////////////////////////////////////////////////////////////////////////////
void
SSLCtxMgr::initClient(const String& certfile, const String& keyfile)
{
	if (m_ctxClient)
	{
		uninitClient();
	}
	m_ctxClient = initCtx(certfile,keyfile);
}
//////////////////////////////////////////////////////////////////////////////
void
SSLCtxMgr::initServer(const String& certfile, const String& keyfile)
{
	if (certfile.empty())
	{
		BLOCXX_THROW(SSLException, "SSLCtxMgr::initCtx(): no certificate file specified");
	}
	if (m_ctxServer)
	{
		uninitServer();
	}
	m_ctxServer = initCtx(certfile,keyfile);
	//loadDHParams(m_ctx, dhfile);
	generateEphRSAKey(m_ctxServer);
	String sessID("SSL_SESSION_");
	sessID += String(Secure::rand_range<UInt16>(0, 10000));
	int sessIDLen =
		(SSL_MAX_SSL_SESSION_ID_LENGTH < (sessID.length())) ?
		SSL_MAX_SSL_SESSION_ID_LENGTH : (sessID.length());
	ERR_clear_error();
	if (SSL_CTX_set_session_id_context(m_ctxServer, reinterpret_cast<const unsigned char*>(sessID.c_str()), sessIDLen) != 1)
	{
		BLOCXX_THROW(SSLException, Format("SSLCtxMgr::initServer(): SSL_CTX_set_session_id_context failed: %1", getOpenSSLErrorDescription()).c_str());
	}
	SSL_CTX_set_verify(m_ctxServer, SSL_VERIFY_PEER /*| SSL_VERIFY_FAIL_IF_NO_PEER_CERT*/, NULL);
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
int
SSLCtxMgr::pem_passwd_cb(char* buf, int size, int /*rwflag*/,
	void* /*userData*/)
{
	String passwd = GetPass::getPass("Enter the password for the SSL certificate: ");

	strncpy(buf, passwd.c_str(), size);
	buf[size - 1] = '\0';

	return passwd.length();
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
bool
SSLCtxMgr::checkClientCert(SSL* ssl, const String& hostName)
{
	return checkCert(ssl, hostName, m_clientCertVerifyCB);
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
bool
SSLCtxMgr::checkServerCert(SSL* ssl, const String& hostName)
{
	return checkCert(ssl, hostName, m_serverCertVerifyCB);
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
bool
SSLCtxMgr::checkCert(SSL* ssl, const String& hostName,
	certVerifyFuncPtr_t certVerifyCB)
{
	BLOCXX_ASSERT(ssl != 0);

	/** @todo this isn't working.
	if (SSL_get_verify_result(ssl)!=X509_V_OK)
	{
		cout << "SSL_get_verify_results failed." << endl;
		return false;
	}
	*/
	/*Check the cert chain. The chain length
	  is automatically checked by OpenSSL when we
	  set the verify depth in the ctx */
	/*Check the common name*/
	if (certVerifyCB)
	{
		X509 *peer = SSL_get_peer_certificate(ssl);
		X509Freer x509freer(peer);
		if (peer == 0)
		{
			return false;
		}
		if (certVerifyCB(peer, hostName) == 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
int
SSLCtxMgr::sslRead(SSL* ssl, char* buf, int len)
{
	int cc = SSL_ERROR_WANT_READ;
	int r = -1; 
	int retries = 0;
	while (cc == SSL_ERROR_WANT_READ && retries < BLOCXX_SSL_RETRY_LIMIT)
	{
		r = SSL_read(ssl, buf, len);
		cc = SSL_get_error(ssl, r);
		retries++;
	}
	
	switch (cc)
	{
		case SSL_ERROR_NONE:
			return r;
		case SSL_ERROR_ZERO_RETURN:
			return -1;
		default:
			return -1;
	}
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
int
SSLCtxMgr::sslWrite(SSL* ssl, const char* buf, int len)
{
	int r = 0; 
	int cc;
	int retries;
	int myLen = len;
	int offset = 0;
#ifndef BLOCXX_WIN32
	// block SIGPIPE so we don't kill the process if the socket is closed.
	SignalScope ss(SIGPIPE, SIG_IGN);
#endif
	while (myLen > 0)
	{
		cc = SSL_ERROR_WANT_WRITE;
		retries = 0;
		while(cc == SSL_ERROR_WANT_WRITE && retries < BLOCXX_SSL_RETRY_LIMIT)
		{
			r = SSL_write(ssl, buf + offset, myLen);
			cc = SSL_get_error(ssl, r);
			retries++;
		}

		if (cc == SSL_ERROR_NONE)
		{
			myLen -= r;
			offset += r;
		}
		else
		{
			return -1;
		}
	}
	return len;
}
//////////////////////////////////////////////////////////////////////////////
void
SSLCtxMgr::uninit()
{
	uninitClient();
	uninitServer();
		
	MutexLock initLock(m_initStateGuard);
	if (m_initState == BLOCXX_SSL_LIBRARY_INITIALIZED)
	{
		// free up memory allocated in SSL_library_init()
		EVP_cleanup();
		// free up memory allocated in SSL_load_error_strings()
		ERR_free_strings();

		m_initState = BLOCXX_SSL_LIBRARY_NOT_INITIALIZED;
	}
}
//////////////////////////////////////////////////////////////////////////////
void
SSLCtxMgr::uninitClient()
{
	if (m_ctxClient)
	{
		SSL_CTX_free(m_ctxClient);
		m_ctxClient = NULL;
	}
}
//////////////////////////////////////////////////////////////////////////////
void
SSLCtxMgr::uninitServer()
{
	if (m_ctxServer)
	{
		SSL_CTX_free(m_ctxServer);
		m_ctxServer = NULL;
	}
}

namespace
{

//////////////////////////////////////////////////////////////////////////////
extern "C"
{
static int verify_callback(int ok, X509_STORE_CTX *store)
{
	int index = SSL_get_ex_data_X509_STORE_CTX_idx();
	if (index < 0)
	{
		return 0;
	}
	SSL* ssl = static_cast<SSL*>(X509_STORE_CTX_get_ex_data(store, index));
	if (ssl == 0)
	{
		return 0;
	}
	OWSSLContext* owctx = static_cast<OWSSLContext*>(SSL_get_ex_data(ssl, SSLServerCtx::SSL_DATA_INDEX));
	BLOCXX_ASSERT(owctx);
	if (owctx == 0)
	{
		return 0;
	}

	/**
	 * This callback is called multiple times while processing a cert.
	 * Supposedly, it is called at each depth of the cert chain.
	 * However, even with a self-signed cert, it is called twice, both times
	 * at depth 0.  Since we will change the status from NOT_OK to OK, the
	 * next time through the preverify status will also be changed.  Therefore
	 * we need to make sure that we set the status on owctx to failed with
	 * any failure, and then that we don't inadvertantly change it to PASS
	 * on subsequent calls.
	 */
	if (!ok)
	{
		owctx->peerCertPassedVerify = OWSSLContext::VERIFY_FAIL;
	}
	else
	{
		// if the cert failed on a previous call, we don't want to change
		// the status.
		if (owctx->peerCertPassedVerify != OWSSLContext::VERIFY_FAIL)
		{
			owctx->peerCertPassedVerify = OWSSLContext::VERIFY_PASS;
		}
	}

#ifdef BLOCXX_DEBUG
	if (!ok)
	{
		char data[256];
		X509 *cert = X509_STORE_CTX_get_current_cert(store);
		int  depth = X509_STORE_CTX_get_error_depth(store);
		int  err = X509_STORE_CTX_get_error(store);

		fprintf(stderr, "-Error with certificate at depth: %i\n", depth);
		X509_NAME_oneline(X509_get_issuer_name(cert), data, 256);
		fprintf(stderr, "  issuer   = %s\n", data);
		X509_NAME_oneline(X509_get_subject_name(cert), data, 256);
		fprintf(stderr, "  subject  = %s\n", data);
		fprintf(stderr, "  err %i:%s\n", err, X509_verify_cert_error_string(err));
	}
#endif

	return 1;
}
} // end extern "C"

} // end unnamed namespace

//////////////////////////////////////////////////////////////////////////////
SSLCtxBase::SSLCtxBase(const SSLOpts& opts)
	: m_ctx(0)
{
	m_ctx = SSLCtxMgr::initCtx(opts.certfile, opts.keyfile, opts.pkey);
	
	SSLCtxMgr::generateEphRSAKey(m_ctx); /// @todo Identify the reason for and side effects of this
	String sessID("SSL_SESSION_");
	sessID += String(Secure::rand_range<UInt16>(0, 10000));
	int sessIDLen =
		(SSL_MAX_SSL_SESSION_ID_LENGTH < (sessID.length())) ?
		SSL_MAX_SSL_SESSION_ID_LENGTH : (sessID.length());
	ERR_clear_error();
	if (SSL_CTX_set_session_id_context(m_ctx, reinterpret_cast<const unsigned char*>(sessID.c_str()), sessIDLen) != 1)
	{
		SSL_CTX_free(m_ctx);
		BLOCXX_THROW(SSLException, Format("SSLCtxMgr::initServer(): SSL_CTX_set_session_id_context failed: %1", SSLCtxMgr::getOpenSSLErrorDescription()).c_str());
	}

	if (opts.verifyMode != SSLOpts::MODE_DISABLED)
	{
		if (!opts.trustStore.empty())
		{
			if (!FileSystem::exists(opts.trustStore))
			{
				SSL_CTX_free(m_ctx);
				BLOCXX_THROW(SSLException, Format("Error loading truststore %1", opts.trustStore).c_str());
			}
			if (SSL_CTX_load_verify_locations(m_ctx,0,opts.trustStore.c_str()) != 1)
			{
				SSL_CTX_free(m_ctx);
				BLOCXX_THROW(SSLException, Format("Error loading truststore %1: %2", opts.trustStore, SSLCtxMgr::getOpenSSLErrorDescription()).c_str());
			}
		}
	
		for (size_t i = 0; i < opts.inMemoryTrustStore.size(); ++i)
		{
			X509* cert = opts.inMemoryTrustStore[i];
			if (X509_STORE_add_cert(m_ctx->cert_store, cert) != 1)
			{
				BLOCXX_THROW(SSLException, Format("Error adding trusted certificate %1", cert->name).c_str());
			}
		}
	}

	switch (opts.verifyMode)
	{
	case SSLOpts::MODE_DISABLED:
		SSL_CTX_set_verify(m_ctx, SSL_VERIFY_NONE, 0);
		break;
	case SSLOpts::MODE_REQUIRED:
		SSL_CTX_set_verify(m_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, 0);
		break;
	case SSLOpts::MODE_OPTIONAL:
	case SSLOpts::MODE_AUTOUPDATE:
		SSL_CTX_set_verify(m_ctx, SSL_VERIFY_PEER, verify_callback);
		break;
	default:
		BLOCXX_ASSERTMSG(false, "Bad option, shouldn't happen");
		break;
	}

	SSL_CTX_set_verify_depth(m_ctx, 9);
}

//////////////////////////////////////////////////////////////////////////////
SSLCtxBase::~SSLCtxBase()
{
	if (m_ctx)
	{
		SSL_CTX_free(m_ctx);
	}
	ERR_clear_error();
	ERR_remove_state(0);
}

//////////////////////////////////////////////////////////////////////////////
SSL_CTX*
SSLCtxBase::getSSLCtx() const
{
	return m_ctx;
}

SSLOpts::SSLOpts()
	: verifyMode(MODE_DISABLED)
	, pkey(0)
{
}

SSLOpts::~SSLOpts()
{
	if (pkey != 0)
	{
		EVP_PKEY_free(pkey); 
		pkey = 0; 
	}
}



//////////////////////////////////////////////////////////////////////////////
SSLServerCtx::SSLServerCtx(const SSLOpts& opts)
	: SSLCtxBase(opts)
{
}

//////////////////////////////////////////////////////////////////////////////
SSLServerCtx::~SSLServerCtx()
{
}

//////////////////////////////////////////////////////////////////////////////
SSLClientCtx::SSLClientCtx(const SSLOpts& opts)
	: SSLCtxBase(opts)
{
}

//////////////////////////////////////////////////////////////////////////////
SSLClientCtx::~SSLClientCtx()
{
}

static GlobalMutex m_mapGuard = BLOCXX_GLOBAL_MUTEX_INIT();

//////////////////////////////////////////////////////////////////////////////
SSLTrustStore::SSLTrustStore(const String& storeLocation)
	: m_store(storeLocation)
{
	m_mapfile = m_store + "/map";
	if (FileSystem::exists(m_mapfile))
	{
		MutexLock mlock(m_mapGuard);
		readMap();
	}
}

//////////////////////////////////////////////////////////////////////////////
SSLTrustStore::~SSLTrustStore()
{
}

//////////////////////////////////////////////////////////////////////////////
bool
SSLTrustStore::getUser(const String& certhash, String& user, String& uid)
{
	MutexLock mlock(m_mapGuard);
	Map<String, UserInfo>::const_iterator iter = m_map.find(certhash);
	if (iter == m_map.end())
	{
		return false;
	}
	user = iter->second.user;
	uid = iter->second.uid;
	return true;
}

//////////////////////////////////////////////////////////////////////////////
void
SSLTrustStore::addCertificate(X509* cert, const String& user, const String& uid)
{
	static const int numtries = 1000;
	BLOCXX_ASSERT(cert);
	OStringStream ss;
	unsigned long hash = X509_subject_name_hash(cert);
	ss << std::hex << hash;
	String filename = m_store + "/" + ss.toString() + ".";
	int i = 0;
	for (i = 0; i < numtries; ++i)
	{
		String temp = filename + String(i);
		if (FileSystem::exists(temp))
		{
			continue;
		}
		filename = temp;
		break;
	}
	if (i == numtries)
	{
		BLOCXX_THROW(SSLException, "Unable to find a valid filename to store cert");
	}
	FILE* fp = fopen(filename.c_str(), "w");
	if (!fp)
	{
		BLOCXX_THROW_ERRNO_MSG(SSLException, Format("Unable to open new cert file for writing: %1", filename).c_str());
	}

	ERR_clear_error();
	// Undocumented function in OpenSSL.  We assume it returns 1 on success
	// like most OpenSSL funcs.
	if (PEM_write_X509(fp, cert) != 1)
	{
		fclose(fp);
		BLOCXX_THROW(SSLException, Format("SSL error while writing certificate to %1: %2", filename, SSLCtxMgr::getOpenSSLErrorDescription()).c_str());
	}
	fclose(fp);

	String digest = getCertMD5Fingerprint(cert);
	MutexLock mlock(m_mapGuard);
	UserInfo info;
	info.user = user;
	info.uid = uid;
	m_map[digest] = info;
	writeMap();
}

//////////////////////////////////////////////////////////////////////////////
String
SSLTrustStore::getCertMD5Fingerprint(X509* cert)
{
	unsigned char digest[16];
	unsigned int len = 16;
	X509_digest(cert, EVP_md5(), digest, &len);
	return MD5::convertBinToHex(digest);
}

//////////////////////////////////////////////////////////////////////////////
void
SSLTrustStore::writeMap()
{
	std::ofstream f(m_mapfile.c_str(), std::ios::out);
	if (!f)
	{
		BLOCXX_THROW_ERRNO_MSG(SSLException, Format("SSL error opening map file: %1", m_mapfile).c_str());
	}
	for (Map<String, UserInfo>::const_iterator iter = m_map.begin();
		iter != m_map.end(); ++iter)
	{
		f << iter->first << " " << iter->second.user
			<< " " << iter->second.uid << "\n";
	}
	f.close();
}

//////////////////////////////////////////////////////////////////////////////
void
SSLTrustStore::readMap()
{
	std::ifstream f(m_mapfile.c_str(), std::ios::in);
	if (!f)
	{
		BLOCXX_THROW_ERRNO_MSG(SSLException, Format("SSL error opening map file: %1", m_mapfile).c_str());
	}
	int lineno = 0;
	while (f)
	{
		String line = String::getLine(f);
		if (!f)
		{
			break;
		}
		++lineno;
		StringArray toks = line.tokenize();
		if (toks.size() != 3 && toks.size() != 2)
		{
			BLOCXX_THROW(SSLException, Format("Error processing user map %1 at line %2", m_mapfile, lineno).c_str());
		}
		UserInfo info;
		info.user = toks[1];
		if (toks.size() == 3)
		{
			info.uid = toks[2];
		}
		m_map.insert(std::make_pair(toks[0], info));
	}
#ifdef BLOCXX_DEBUG
	std::cerr << "cert<>user map initizialized with " << m_map.size() << " users" << std::endl;
#endif
	f.close();
}

//////////////////////////////////////////////////////////////////////////////

OWSSLContext::OWSSLContext()
	: peerCertPassedVerify(VERIFY_NONE)
{
}
//////////////////////////////////////////////////////////////////////////////
OWSSLContext::~OWSSLContext()
{
}


} // end namespace BLOCXX_NAMESPACE

#endif // #ifdef BLOCXX_HAVE_OPENSSL

