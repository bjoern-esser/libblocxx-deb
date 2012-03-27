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
 * @author Dan Nuffer
 * @author Kevin S. Van Horn
 */

#include "blocxx/BLOCXX_config.h"

#ifdef BLOCXX_HAVE_OPENSSL
// If you don't have SSL, you don't have cryptographically secure random
// numbers.  Don't try to fall back to a weaker PRNG, as this violates the
// security principle of "fail safe".

#include "blocxx/Array.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/Exec.hpp"
#include "blocxx/FileSystem.hpp"
#include "blocxx/Mutex.hpp"
#include "blocxx/MutexLock.hpp"
#include "blocxx/GlobalMutex.hpp"
#include "blocxx/Secure.hpp"
#include "blocxx/SecureRand.hpp"
#include "blocxx/SSLCtxMgr.hpp"
#include "blocxx/String.hpp"
#include "blocxx/Thread.hpp"
#include "blocxx/ThreadOnce.hpp"
#include "blocxx/UnnamedPipe.hpp"
#include "blocxx/UserUtils.hpp"
#include "blocxx/Process.hpp"

#include <cmath>
#include <csignal>
#include <cstring>  // for std::memset
#include <limits>
#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <fcntl.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#ifdef BLOCXX_HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifndef BLOCXX_WIN32
#include <sys/time.h>
#endif

#ifdef BLOCXX_WIN32
#include <wincrypt.h>
#endif

using namespace blocxx;

namespace
{
	unsigned const RESEED_BYTES = 16; // 128 bits
	unsigned const SEED_BYTES   = 16; // 128 bits

	template <typename T> struct unsigned_equivalent
	{
		typedef T type;
	};

	template <> struct unsigned_equivalent<char>
	{
		typedef unsigned char type;
	};

	template <> struct unsigned_equivalent<signed char>
	{
		typedef unsigned char type;
	};

	template <> struct unsigned_equivalent<short>
	{
		typedef unsigned short type;
	};

	template <> struct unsigned_equivalent<int>
	{
		typedef unsigned int type;
	};

	template <> struct unsigned_equivalent<long>
	{
		typedef unsigned long type;
	};

	template <> struct unsigned_equivalent<long long>
	{
		typedef unsigned long long type;
	};

	blocxx::OnceFlag guard = BLOCXX_ONCE_INIT;

	void rand_init_impl();
}

namespace BLOCXX_NAMESPACE
{
BLOCXX_DEFINE_EXCEPTION(SecureRand);

namespace Secure
{
	void rand_init()
	{
		callOnce(guard, &rand_init_impl);
	}

	unsigned char * rand(unsigned char * buf, std::size_t n)
	{
		callOnce(guard, &rand_init_impl);
		ERR_clear_error();
		if (!RAND_bytes(buf, n))
		{
			BLOCXX_THROW(SecureRandException,
				SSLCtxMgr::getOpenSSLErrorDescription().c_str());
		}
		return buf;
	}

	::pid_t fork_reseed()
	{
#ifdef BLOCXX_WIN32
#pragma message(Reminder "TODO: implement it for Win!")

		return BLOCXX_INVALID_HANDLE;
#else
		unsigned char seed[2][RESEED_BYTES];
		rand(seed[0], sizeof(seed[0]));
		rand(seed[1], sizeof(seed[1]));

		::pid_t rv = ::fork();
		if (rv < 0)
		{
			return rv;
		}

		std::size_t idx = rv > 0;  // 0 or 1
		RAND_seed(seed[idx], sizeof(seed[idx]));
		// forget other process's seed
		std::memset(seed[1 - idx], 0, sizeof(seed[1- idx]));

		return rv;
#endif
	}

	namespace Impl
	{
		template <typename UnsignedInt>
		UnsignedInt rand_uint_lt(UnsignedInt n)
		{
			BLOCXX_ASSERT(n > 0);
			if ((n & (n - 1)) == 0) // n is a power of two
			{
				return rand_uint<UnsignedInt>() % n;
			}
			UnsignedInt const uint_max = static_cast<UnsignedInt>(-1);
			UnsignedInt const bound = uint_max - (uint_max % n);
			UnsignedInt rn;
			do
			{
				rn = rand_uint<UnsignedInt>();
			} while (rn >= bound);
			return rn % n;
		}

		// Explicit instantiation
		template unsigned char
			BLOCXX_COMMON_API rand_uint_lt<unsigned char>(unsigned char);
		template unsigned short
			BLOCXX_COMMON_API rand_uint_lt<unsigned short>(unsigned short);
		template unsigned int
			BLOCXX_COMMON_API rand_uint_lt<unsigned int>(unsigned int);
		template unsigned long
			BLOCXX_COMMON_API rand_uint_lt<unsigned long>(unsigned long);
		template unsigned long long
			BLOCXX_COMMON_API rand_uint_lt<unsigned long long>(unsigned long long);

		template <typename Integer>
		Integer rand_range(Integer min_value, Integer max_value)
		{
			BLOCXX_ASSERT(max_value >= min_value);

			// The following code uses these properties of C++:
			// - Conversions from a signed integer to an unsigned integer
			//   of the same size are always well-defined.
			// - Arithmetic for unsigned integers is module 2^n, where n
			//   is the number of bits.
			// - If signed integer k is negative, then converting it to
			//   the equivalent unsigned integer yields 2^n + k.

			typedef typename unsigned_equivalent<Integer>::type UnsignedInt;
			UnsignedInt const umax = static_cast<UnsignedInt>(max_value);
			UnsignedInt const umin = static_cast<UnsignedInt>(min_value);
			UnsignedInt const diff = umax - umin;

			// diff is the mathematical difference between max_value
			// and min_value, which may not be representable as an Integer,
			// but is guaranteed to be representable as an UnsignedInt.

			UnsignedInt const range = diff + static_cast<UnsignedInt>(1);

			// range == 0 iff every UnsignedInt value corresponds to
			// a unique Integer value (e.g., two's complement representation
			// instead of sign-magnitude), min_value is the smallest possible
			// Integer value, and max_value is the largest possible Integer
			// value.

			UnsignedInt rv;
			if (range == 0)
			{
				// All Integer values are allowed return values, and there
				// is a one-to-one mapping from UnsignedInt values to
				// Integer values.
				rv = rand_uint<UnsignedInt>();
			}
			else
			{
				// Compute the UnsignedInt value corresponding to the desired
				// Integer value.  This works even if min_value < 0 and
				// max_value >= 0, because the arithmetic is module 2^n.
				rv = umin + rand_uint_lt(range);
			}
			return static_cast<Integer>(rv);
		}

		// explicit instantiations
		template char
			BLOCXX_COMMON_API rand_range(char, char);
		template signed char
			BLOCXX_COMMON_API rand_range(signed char, signed char);
		template unsigned char
			BLOCXX_COMMON_API rand_range(unsigned char, unsigned char);
		template short
			BLOCXX_COMMON_API rand_range(short, short);
		template unsigned short
			BLOCXX_COMMON_API rand_range(unsigned short, unsigned short);
		template int
			BLOCXX_COMMON_API rand_range(int, int);
		template unsigned int
			BLOCXX_COMMON_API rand_range(unsigned int, unsigned int);
		template long
			BLOCXX_COMMON_API rand_range(long, long);
		template unsigned long
			BLOCXX_COMMON_API rand_range(unsigned long, unsigned long);
		template long long
			BLOCXX_COMMON_API rand_range(long long, long long);
		template unsigned long long
			BLOCXX_COMMON_API rand_range(unsigned long long, unsigned long long);
		
		template <unsigned int N>
		struct log2
		{
			enum { value = 1 + log2<N/2>::value };
		};

		template <>
		struct log2<1>
		{
			enum { value = 0 };
		};
		
		// # of mantissa bits if Number is a floating-point type.
		template <typename Number>
		struct bits_precision
		{
			typedef std::numeric_limits<Number> lim_t;
			enum { value = lim_t::digits * log2<lim_t::radix>::value };
		};

		template <typename Real>
		Real rand_unit_interval()
		{
			typedef UInt32 uint_t;
			int const UINT_BITS = 32;
			int const NUINT =
				(bits_precision<Real>::value + UINT_BITS - 1) / UINT_BITS;
			Real rv = 0.0;
			for (int i = 1; i <= NUINT; ++i)
			{
				Real r = static_cast<Real>(rand_uint<uint_t>());
				rv += std::ldexp(r, -UINT_BITS * i);
			}
			return rv;
		}

		// explicit instantiations
		template float rand_unit_interval<float>();
		template double rand_unit_interval<double>();
		template long double rand_unit_interval<long double>();

	} // namespace Impl

	void rand_save_state()
	{
		char randFile[MAXPATHLEN];
		char const * rval = RAND_file_name(randFile, MAXPATHLEN);
		if (rval)
		{
			// we only create this file is there's no chance an attacker
			// could read or write it. see Network Security with OpenSSL p. 101
			using namespace FileSystem::Path;
			if (security(dirname(randFile)).first == E_SECURE_DIR)
			{
				if (RAND_write_file(randFile) <= 0)
				{
					// in case "the bytes written were generated without
					// appropriate seed.", we don't want to load it up next
					// time.
					FileSystem::removeFile(randFile);
				}
			}
		}

	}

} // namespace Secure
} // namespace BLOCXX_NAMESPACE

namespace
{
	// These are used to generate random data via signal delivery timing
	// differences.  We have to use global data since it's modified from a
	// signal handler.
	volatile sig_atomic_t g_counter;
	volatile unsigned char* g_data;
	volatile sig_atomic_t g_dataIdx;
	int g_dataSize;
}

extern "C"
{
	// this needs to still be static, since it gets exported because of
	// extern "C"
#ifdef BLOCXX_NCR
	static void randomALRMHandler()
#else
	static void randomALRMHandler(int sig)
#endif
	{
		if (g_dataIdx < g_dataSize)
		{
			g_data[g_dataIdx++] ^= g_counter & 0xFF;
		}
	}
}

namespace
{
	GlobalMutex g_randomTimerGuard = BLOCXX_GLOBAL_MUTEX_INIT();

#ifndef BLOCXX_WIN32
	// This function will continue to iterate until *iterations <= 0.
	// *iterations may be set by another thread. *iterations should not be < 8.
	void generateRandomTimerData(unsigned char* data, int size, int* iterations)
	{
		BLOCXX_ASSERT(data != 0);
		BLOCXX_ASSERT(size > 0);
		BLOCXX_ASSERT(iterations != 0);

		// make sure we only have one thread running this at a time.
		MutexLock l(g_randomTimerGuard);

		// set up the global data for the signal handler
		g_data = data;
		g_dataSize = size;
		g_dataIdx = 0;

		// install our ALRM handler
		struct sigaction sa, osa;
		sa.sa_handler = randomALRMHandler;
		sa.sa_flags = 0;
		sigemptyset(&sa.sa_mask);
		sigaction(SIGALRM, &sa, &osa);

		// Start timer
		struct ::itimerval tv, otv;
		tv.it_value.tv_sec = 0;
		tv.it_value.tv_usec = 10 * 1000; // 10 ms
		tv.it_interval = tv.it_value;
		setitimer(ITIMER_REAL, &tv, &otv);
	
		while ((*iterations)-- > 0)
		{
			// g_dataIdx++ in sigALRM
			for (g_dataIdx = 0; g_dataIdx < g_dataSize;)
			{
				++g_counter;
			}
			// rotate the bits to accomodate for a possible lack of
			// low-bit entropy
			for (int j = 0; j < g_dataSize; j++)
			{
				g_data[j] = (g_data[j]>>3) | (g_data[j]<<5);
			}
		}
		setitimer(ITIMER_REAL, &otv, 0);

		// reset signal handler
		sigaction(SIGALRM, &osa, 0);

	}

	void generateRandomDataFromFile(const char* name, int len)
	{
		int fd = ::open(name, O_RDONLY);
		if (fd == -1)
		{
			return;
		}

		std::vector<char> buf(len);
		int bytesRead = ::read(fd, &buf[0], len);
		if (bytesRead == -1)
		{
			return;
		}
		buf.resize(bytesRead);
		::RAND_add(&buf[0], buf.size(), 0.0);
		// 0 entropy, since this could all be observable by someone else.
	}

	void generateRandomDataFromTime(double entropy)
	{
		struct timeval tv;
		::gettimeofday(&tv, 0);
		::RAND_add(&tv, sizeof(tv), entropy);

		clock_t c(::clock());
		::RAND_add(&c, sizeof(c), entropy);

		struct rusage ru;
		::getrusage(RUSAGE_SELF, &ru);
		::RAND_add(&ru, sizeof(ru), entropy);

		::getrusage(RUSAGE_CHILDREN, &ru);
		::RAND_add(&ru, sizeof(ru), entropy);
	}

	struct cmd
	{
		const char* command;

		// estimated number of bytes of entropy per 1K of output
		double usefulness;
	};

	// This list of sources comes from gnupg, prngd and egd.
	const cmd randomSourceCommands[] =
	{
		{ "advfsstat -b usr_domain", 0.01 },
		{ "advfsstat -l 2 usr_domain", 0.5 },
		{ "advfsstat -p usr_domain", 0.01 },
		{ "arp -a -n", 0.5 },
		{ "df", 0.5 },
		{ "df -i", 0.5 },
		{ "df -a", 0.5 },
		{ "df -in", 0.5 },
		{ "dmesg", 0.5 },
		{ "errpt -a", 0.5 },
		{ "ifconfig -a", 0.5 },
		{ "iostat", 0.5 },
		{ "ipcs -a", 0.5 },
		{ "last", 0.5 },
		{ "lastlog", 0.5 },
		{ "lpstat -t", 0.1 },
		{ "ls -alniR /var/log", 1.0 },
		{ "ls -alniR /var/adm", 1.0 },
		{ "ls -alni /var/spool/mail", 1.0 },
		{ "ls -alni /proc", 1.0 },
		{ "ls -alniR /tmp", 1.0 },
		{ "ls -alniR /var/tmp", 1.0 },
		{ "ls -alni /var/mail", 1.0 },
		{ "ls -alniR /var/db", 1.0 },
		{ "ls -alniR /etc", 1.0 },
		{ "ls -alniR /private/var/log", 1.0 },
		{ "ls -alniR /private/var/db", 1.0 },
		{ "ls -alniR /private/etc", 1.0 },
		{ "ls -alniR /private/tmp", 1.0 },
		{ "ls -alniR /private/var/tmp", 1.0 },
		{ "mpstat", 1.5 },
		{ "netstat -s", 1.5 },
		{ "netstat -n", 1.5 },
		{ "netstat -a -n", 1.5 },
		{ "netstat -anv", 1.5 },
		{ "netstat -i -n", 0.5 },
		{ "netstat -r -n", 0.1 },
		{ "netstat -m", 0.5 },
		{ "netstat -ms", 0.5 },
		{ "nfsstat", 0.5 },
		{ "ps laxww", 1.5 },
		{ "ps -laxww", 1.5 },
		{ "ps -al", 1.5 },
		{ "ps -el", 1.5 },
		{ "ps -efl", 1.5 },
		{ "ps -efly", 1.5 },
		{ "ps aux", 1.5 },
		{ "ps -A", 1.5 },
		{ "pfstat", 0.5 },
		{ "portstat", 0.5 },
		{ "pstat -p", 0.5 },
		{ "pstat -S", 0.5 },
		{ "pstat -A", 0.5 },
		{ "pstat -t", 0.5 },
		{ "pstat -v", 0.5 },
		{ "pstat -x", 0.5 },
		{ "pstat -t", 0.5 },
		{ "ripquery -nw 1 127.0.0.1", 0.5 },
		{ "sar -A 1 1", 0.5 },
		{ "snmp_request localhost public get 1.3.6.1.2.1.7.1.0", 0.5 },
		{ "snmp_request localhost public get 1.3.6.1.2.1.7.4.0", 0.5 },
		{ "snmp_request localhost public get 1.3.6.1.2.1.4.3.0", 0.5 },
		{ "snmp_request localhost public get 1.3.6.1.2.1.6.10.0", 0.5 },
		{ "snmp_request localhost public get 1.3.6.1.2.1.6.11.0", 0.5 },
		{ "snmp_request localhost public get 1.3.6.1.2.1.6.13.0", 0.5 },
		{ "snmp_request localhost public get 1.3.6.1.2.1.5.1.0", 0.5 },
		{ "snmp_request localhost public get 1.3.6.1.2.1.5.3.0", 0.5 },
		{ "tail -c 1024 /var/log/messages", 1.0 },
		{ "tail -c 1024 /var/log/syslog", 1.0 },
		{ "tail -c 1024 /var/log/system.log", 1.0 },
		{ "tail -c 1024 /var/log/debug", 1.0 },
		{ "tail -c 1024 /var/adm/messages", 1.0 },
		{ "tail -c 1024 /var/adm/syslog", 1.0 },
		{ "tail -c 1024 /var/adm/syslog/mail.log", 1.0 },
		{ "tail -c 1024 /var/adm/syslog/syslog.log", 1.0 },
		{ "tail -c 1024 /var/log/maillog", 1.0 },
		{ "tail -c 1024 /var/adm/maillog", 1.0 },
		{ "tail -c 1024 /var/adm/SPlogs/SPdaemon.log", 1.0 },
		{ "tail -c 1024 /usr/es/adm/cluster.log", 1.0 },
		{ "tail -c 1024 /usr/adm/cluster.log", 1.0 },
		{ "tail -c 1024 /var/adm/cluster.log", 1.0 },
		{ "tail -c 1024 /var/adm/ras/conslog", 1.0 },
		{ "tcpdump -c 100 -efvvx", 1 },
		{ "uptime", 0.5 },
		{ "vmstat", 2.0 },
		{ "vmstat -c", 2.0 },
		{ "vmstat -s", 2.0 },
		{ "vmstat -i", 2.0 },
		{ "vmstat -f", 2.0 },
		{ "w", 2.5 },
		{ "who -u", 0.5 },
		{ "who -i", 0.5 },
		{ "who -a", 0.5 },

		{ 0, 0 }
	};

	class RandomOutputGatherer : public Exec::OutputCallback
	{
	private:
		virtual void doHandleData(
			const char* data, size_t dataLen, Exec::EOutputSource outputSource,
			const ProcessRef& theProc, size_t streamIndex,
			Array<char>& inputBuffer
		)
		{
			if (outputSource == Exec::E_STDERR)
			{
				// for all the commands we run, anything output to stderr
				// doesn't have any entropy.
				::RAND_add(data, dataLen, 0.0);
			}
			else
			{
				// streamIndex is the index into the PopenStreams array which
				// correlates to randomSourceCommands
				::RAND_add(
					data, dataLen,
					randomSourceCommands[streamIndex].usefulness * 
					static_cast<double>(dataLen) / 1024.0
				);
			}
			// the actual length of stuff we got could be random, but we can't
			// say for sure, so it gets 0.0 entropy.
			::RAND_add(&dataLen, sizeof(dataLen), 0.0);
			::RAND_add(&outputSource, sizeof(outputSource), 0.0);
			// The timing is random too.
			generateRandomDataFromTime(0.1);
		}

	};

	class RandomInputCallback : public Exec::InputCallback
	{
	private:
		virtual void doGetData(
			Array<char>& inputBuffer, const ProcessRef& theProcess,
			size_t streamIndex
		)
		{
			// none of the processes we run need data from stdin
			if (theProcess->in()->isOpen())
			{
				theProcess->in()->close();
			}
		}
	};

	String locateInPath(const String& cmd, const String& path)
	{
		StringArray pathElements(path.tokenize(":"));
		for (size_t i = 0; i < pathElements.size(); ++i)
		{
			String testCmd(pathElements[i] + '/' + cmd);
			if (FileSystem::exists(testCmd))
			{
				return testCmd;
			}
		}
		return cmd;
	}

	class RandomTimerThread : public Thread
	{
		virtual Int32 run()
		{
			// don't initialize to anything, as we may pick up some good
			// random junk off the stack.
			unsigned char buf[256];
			int iterations = 8;
			generateRandomTimerData(buf, sizeof(buf), &iterations);
			::RAND_add(buf, sizeof(buf), 32);
			// 32 is if we assume 1 bit per byte, and most systems should have
			// something better than that.

			generateRandomDataFromTime(0.1);
		
			return 0;
		}
	};
#endif

	void rand_init_impl()
	{
#ifdef BLOCXX_WIN32
		// There are issues on win32 with calling RAND_status() w/out sufficient
		// entropy in a threaded environment, so we'll just add some before
		// calling RAND_status()
		HCRYPTPROV hProvider = 0;
		BYTE buf[64];

		if (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL,
				CRYPT_VERIFYCONTEXT))
		{
			if (CryptGenRandom(hProvider, sizeof(buf), buf))
			{
				RAND_add(buf, sizeof(buf), sizeof(buf));
			}
			CryptReleaseContext(hProvider, 0);
		}
		// provided by OpenSSL. Try doing something in addition to
		// CryptGenRandom(), since we can't trust closed source.
		::RAND_screen();
#endif

		// try a egd socket that OpenSSL wouldn't by default.
		RAND_egd(BLOCXX_DEFAULT_STATE_DIR"/egd-pool");

		// with OpenSSL 0.9.7 calling RAND_status() will try to load
		// sufficient randomness, so hopefully we won't have to do anything.
		if (::RAND_status() == 1)
		{
			return;
		}

#ifndef BLOCXX_WIN32
		// OpenSSL 0.9.7 does this automatically, so only try if we've got an
		// older version of OpenSSL.
		if (::SSLeay() < 0x00907000L)
		{
			// now try adding in /dev/random
			int loadedBytes = RAND_load_file("/dev/random", 1024);
			if (loadedBytes == 0)
			{
				// okay, no /dev/random... try adding in /dev/urandom
				RAND_load_file("/dev/urandom", 1024);
			}

			if (RAND_status() == 1)
			{
				return;
			}

			// now try adding in data from an entropy gathering daemon (egd)
			const char *names[] =
			{
				"/var/run/egd-pool", 
				"/dev/egd-pool", 
				"/etc/egd-pool",
				"/etc/entropy", 
				NULL
			};

			for (int i = 0; names[i]; i++)
			{
				if (RAND_egd(names[i]) != -1)
				{
					break;
				}
			}

			if (RAND_status() == 1)
			{
				return;
			}
		}

		// try loading up randomness from a previous run.
		char randFile[MAXPATHLEN];
		const char* rval = ::RAND_file_name(randFile, MAXPATHLEN);
		if (rval)
		{
			using namespace FileSystem::Path;
			try
			{
				if (security(randFile).first == E_SECURE_FILE)
				{
					::RAND_load_file(randFile, -1);
				}
			}
			catch (FileSystemException& e)
			{
				// ignore
			}
		}

		// don't check RAND_status() again, since we don't really trust the
		// random file to be very secure--there are too many ways an attacker
		// could get or change it, so we'll do this other stuff as well.

		// we're on a really broken system.  We'll try to get some random data
		// by:
		// - running commands that reflect random system activity.
		//   This is the same approach a egd daemon would do, but we do it only
		//   once to seed the randomness.
		//   The list of sources comes from gnupg, prngd and egd.
		// - use a timing based approach which gives decent randomness.
		// - use other variable things, such as pid, execution times, etc.
		//   most of these values have an entropy of 0, since they are
		//   observable to any other user on the system, so even though they
		//   are random, they're observable, and we can't count them as entropy.

		// do the time based ones before we start, after the timing tests,
		// and then again after running commands.
		generateRandomDataFromTime(0.0);

		RandomTimerThread randomTimerThread;
		randomTimerThread.start();

		// - read some portions of files and dirs (e.g. /dev/mem) if possible
		const char* files[] = {	"/dev/mem", 0 };
		for (const char** p = files; *p; ++p)
		{
			generateRandomDataFromFile(*p, 1024*1024*2);
		}

		generateRandomDataFromTime(0.1);

		pid_t myPid(::getpid());
		::RAND_add(&myPid, sizeof(myPid), 0.0);

		pid_t parentPid(::getppid());
		::RAND_add(&parentPid, sizeof(parentPid), 0.0);
	
		uid_t myUid(::getuid());
		::RAND_add(&myUid, sizeof(myUid), 0.0);

		gid_t myGid(::getgid());
		::RAND_add(&myGid, sizeof(myGid), 0.0);

		// now run commands
		Array<ProcessRef> procs;
		for (size_t i = 0; randomSourceCommands[i].command != 0; ++i)
		{
			StringArray cmd = String(randomSourceCommands[i].command).tokenize();
			if( cmd.empty() )
			{
				// This shouldn't happen unless someone messes up the command table above.
				continue;
			}
			// If it isn't an absolute path, search for it.
			if (cmd[0][0] != '/')
			{
				const char * RANDOM_COMMAND_PATH =
					"/bin:/sbin:/usr/bin:/usr/sbin:/usr/ucb:/usr/etc:/usr/bsd:"
					"/etc:/usr/local/bin:/usr/local/sbin";

				// Attempt to locate the command in our chosen "secure" path
				String locatedCmd = locateInPath(cmd[0], RANDOM_COMMAND_PATH);
				if( locatedCmd == cmd[0] )
				{
					// This command must not exist.  Skip it to avoid long delays of attempted execution.
					continue;
				}

				try
				{
					using namespace FileSystem::Path;
					if (security(locatedCmd).first != E_SECURE_FILE)
					{
						// This command is not in a secure location, we are going to skip it.
						continue;
					}
				}
				catch (FileSystemException& e)
				{
					// ignore the error and continue
					continue;
				}

				cmd[0] = locatedCmd;
			}
			try
			{
				// This may throw an exception before the command is executed.  We
				// can't let it cause the random init to fail.
				procs.push_back(Exec::spawn(cmd));
			}
			catch(const ExecErrorException& e)
			{
				// Ignore it.  We'll get random data from the other commands.
			}
		}

		RandomOutputGatherer randomOutputGatherer;
		RandomInputCallback randomInputCallback;
		const Timeout RANDOM_COMMAND_TIMEOUT = Timeout::relative(10.0);
		try
		{
			Exec::processInputOutput(
				randomOutputGatherer, procs,
				randomInputCallback, RANDOM_COMMAND_TIMEOUT
			);
		}
		catch (ExecTimeoutException&)
		{
			// ignore it.
		}

		// terminate all the processes and add their return code to the pool.
		for (size_t i = 0; i < procs.size(); ++i)
		{
			procs[i]->waitCloseTerm(Timeout::relative(0.001), Timeout::relative(0.002), Timeout::relative(0.003));
			Process::Status status = procs[i]->processStatus();
			if (!status.terminatedSuccessfully())
			{
				int rv = status.exitStatus();
				::RAND_add(&rv, sizeof(rv), 0.0);
			}
		}

		randomTimerThread.join();

		generateRandomDataFromTime(0.1);
#endif
	} // rand_init_impl

} // anonymous namespace
#endif // #ifdef BLOCXX_HAVE_OPENSSL
