/*******************************************************************************
* Copyright (C) 2001-2004 Vintela, Inc. All rights reserved.
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
*  - Neither the name of Vintela, Inc. nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Vintela, Inc. OR THE CONTRIBUTORS
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
 */

#ifndef BLOCXX_SSLCtxMgr_HPP_INCLUDE_GUARD_
#define BLOCXX_SSLCtxMgr_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/SSLException.hpp"
#include "blocxx/IntrusiveCountableBase.hpp"
#include "blocxx/IntrusiveReference.hpp"
#include "blocxx/Map.hpp"
#include "blocxx/Bool.hpp"
#ifdef BLOCXX_HAVE_OPENSSL
#include "blocxx/String.hpp"
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#define BLOCXX_SSLCTX_MAX_CN_LEN 256
#define BLOCXX_SSL_RETRY_LIMIT 20

namespace BLOCXX_NAMESPACE
{

/**
 * Verify a X509 certificate.
 * @param cert a pointer to the certificate to verify.
 * @return 1 if the certificate is good, 0 if the certificate is bad.
 * 	If 0 is returned, the SSL handshake will abort.
 */
typedef int (*certVerifyFuncPtr_t)(X509* cert, const String& hostName);

// TODO: Make this class be a singleton.
class BLOCXX_COMMON_API SSLCtxMgr
{
public:
	/**
	 * The callback for getting a passphrase on a certificate.
	 */
	static int pem_passwd_cb(char* buf, int size, int rwflag, void *userData);
	/**
	 * Check a certificate based on the callback function for client cert
	 * verification. 
	 * @param ssl A pointer to the SSL context
	 * @param hostname the hostname of the client machine
	 * @return True if the certificate is good, false otherwise
	 */
	static bool checkClientCert(SSL* ssl, const String& hostName);
	/**
	 * Check a certificate based on the callback function for server cert
	 * verification. 
	 * @param ssl A pointer to the SSL context
	 * @param hostname the hostname of the server machine
	 * @return True if the certificate is good, false otherwise
	 */
	static bool checkServerCert(SSL* ssl, const String& hostName);
	/**
	 * Initialize for a client
	 * @param certFile the path to the file containing the certificate.
	 * @param keyFile the path to the file containing the key.  If a certificate is
	 *  specified but a key is not, the certificate file will also be searched for a key.
	 * @exception SSLException
	 */
	static void initClient(const String& certFile = String(), const String& keyFile = String());
	/**
	 * Initialize for a server
	 * @param certFile the path to the file containing the certificate.
	 * @param keyFile the path to the file containing the key.  If a certificate is
	 *  specified but a key is not, the certificate file will also be searched for a key.
	 * @exception SSLException
	 */
	static void initServer(const String& certFile, const String& keyFile = String());
	/** 
	 * get the Server SSL Context
	 * @return the server SSL_CTX
	 */
	static SSL_CTX* getSSLCtxServer()
	{
		return m_ctxServer;
	}
	/**
	 * get the Client SSL Context
	 * @return the client SSL_CTX
	 */
	static SSL_CTX* getSSLCtxClient()
	{
		return m_ctxClient;
	}
	/**
	 * Read from a SSL connection
	 * @param ssl a pointer to the SSL Context for the connection
	 * @param buf a pointer to a buffer where data should be copied.
	 * @param len the number of bytes to read.
	 * @return the number of bytes read.
	 * @exception SSLException
	 */
	static int sslRead(SSL* ssl, char* buf, int len); 
	/**
	 * Write to a SSL connection
	 * @param ssl a pointer to a SSL Context for the connection
	 * @param buf the buffer containing the data to write.
	 * @param len the number of bytes to write.
	 * @return the number of bytes written.
	 * @exception SSLException
	 */
	static int sslWrite(SSL* ssl, const char* buf, int len);
	/**
	 * Have we been initialized as a client?
	 * @return true if initialized as a client
	 */
	static bool isClient() { return m_ctxClient != NULL; }
	/**
	 * Have we been initialized as a server?
	 * @return true if initialized as a server
	 */
	static bool isServer() { return m_ctxServer != NULL; }
	/**
	 * Assign a callback function to be used to verify SSL certificates.
	 * @param cbfunc the callback function.  Signature:
	 *		typedef void (*certVerifyFuncPtr_t)(X509* cert);
	 */
	static void setClientCertVerifyCallback(certVerifyFuncPtr_t cbfunc)
		{ m_clientCertVerifyCB = cbfunc; }
	/**
	 * Assign a callback function to be used to verify SSL certificates.
	 * @param cbfunc the callback function.  Signature:
	 *		typedef void (*certVerifyFuncPtr_t)(X509* cert);
	 */
	static void setServerCertVerifyCallback(certVerifyFuncPtr_t cbfunc)
		{ m_serverCertVerifyCB = cbfunc; }
	// set type to NOT_INIT and free memory.
	static void uninit(); 
	/**
	 * @throws SSLException
	 */
	static void generateEphRSAKey(SSL_CTX* ctx);

	static String getOpenSSLErrorDescription();

	/**
	 * Calling this function before using any SSLCtxMgr instance
	 * will prevent blocxx from initializing the SSL library.
	 * This call should only be made if the SSL library has already
	 * been initialized, including cryptographic libraries,
	 * algorithms and error strings, if applicable.
	 * SSL library cleanup will also be skipped.
	 * @throws SSLException if an SSLCtxMgr instance has already been used.
	 */
	static void disableSSLInit();
	/**
	 * Calling this function before using any SSLCtxMgr instance
	 * will prevent blocxx from providing a dynamic locks
	 * implementation for the SSL library.  This call should
	 * only be made if an alternate locks implementation is
	 * being used.
	 * @throws SSLException if an SSLCtxMgr instance has already been used.
	 */
	static void disableLocks();

	static Bool getSSLInitDisabled();
	static Bool getSSLLocksDisabled();

private:

	friend class SSLCtxBase;

	static SSL_CTX* m_ctxClient;
	static SSL_CTX* m_ctxServer;
	static certVerifyFuncPtr_t m_clientCertVerifyCB;
	static certVerifyFuncPtr_t m_serverCertVerifyCB;

	/**
	 * @throws SSLException
	 */
	static SSL_CTX* initCtx(const String& certfile, const String& keyfile, 
							EVP_PKEY* pkey = 0);
	/**
	 * @throws SSLException
	 */
	static void loadDHParams(SSL_CTX* ctx, const String& file);
	static void uninitServer();
	static void uninitClient();

	// don't allow instantiation
	SSLCtxMgr();
	SSLCtxMgr(const SSLCtxMgr&);
	SSLCtxMgr& operator=(const SSLCtxMgr&);

	/** 
	 * This probably needs to say something useful.
	 */
	static bool checkCert(SSL* ssl, const String& hostName, certVerifyFuncPtr_t cbFunc);
};

//////////////////////////////////////////////////////////////////////////////
struct BLOCXX_COMMON_API SSLOpts
{
	SSLOpts(); 
	~SSLOpts(); 
	String certfile; 
	String keyfile; 
	String trustStore; 
	enum VerifyMode_t
	{
		MODE_DISABLED, 
		MODE_REQUIRED, 
		MODE_OPTIONAL, 
		MODE_AUTOUPDATE
	}; 
	VerifyMode_t verifyMode; 
	EVP_PKEY* pkey; 
}; 


//////////////////////////////////////////////////////////////////////////////
class BLOCXX_COMMON_API SSLCtxBase
{
public: 
	SSL_CTX* getSSLCtx() const; 

protected: 
	SSLCtxBase(const SSLOpts& opts); 
	virtual ~SSLCtxBase(); 
	SSL_CTX* m_ctx; 
};

//////////////////////////////////////////////////////////////////////////////
class BLOCXX_COMMON_API SSLServerCtx : public SSLCtxBase, public IntrusiveCountableBase
{
public: 
	SSLServerCtx(const SSLOpts& opts); 
	static const int SSL_DATA_INDEX = 0; 
};

//////////////////////////////////////////////////////////////////////////////
class BLOCXX_COMMON_API SSLClientCtx : public SSLCtxBase, public IntrusiveCountableBase
{
public: 
	SSLClientCtx(const SSLOpts& opts = SSLOpts()); 
};

//////////////////////////////////////////////////////////////////////////////
class BLOCXX_COMMON_API SSLTrustStore: public IntrusiveCountableBase
{
public: 
	SSLTrustStore(const String& storeLocation); 
	void addCertificate(X509* cert, const String& user, const String& uid); 
	bool getUser(const String& certhash, String& user, String& uid); 

	static String getCertMD5Fingerprint(X509* cert); 
private: 
	String m_store; 
	String m_mapfile; 
	struct UserInfo
	{
		String user; 
		String uid; 
	};

#ifdef BLOCXX_WIN32
#pragma warning (push)
#pragma warning (disable: 4251)
#endif

	Map<String, UserInfo> m_map; 

#ifdef BLOCXX_WIN32
#pragma warning (pop)
#endif

	void readMap(); 
	void writeMap(); 

};

//////////////////////////////////////////////////////////////////////////////

struct BLOCXX_COMMON_API OWSSLContext
{
	enum CertVerifyState_t
	{
		VERIFY_NONE, 
		VERIFY_PASS, 
		VERIFY_FAIL
	}; 
	OWSSLContext(); 
	~OWSSLContext(); 
	CertVerifyState_t peerCertPassedVerify; 
};

//////////////////////////////////////////////////////////////////////////////


#else // ifdef BLOCXX_HAVE_OPENSSL

namespace BLOCXX_NAMESPACE
{

class BLOCXX_COMMON_API SSLServerCtx : public IntrusiveCountableBase
{
};

class BLOCXX_COMMON_API SSLClientCtx : public IntrusiveCountableBase
{
};

#endif // ifdef BLOCXX_HAVE_OPENSSL

} // end namespace BLOCXX_NAMESPACE


#endif
