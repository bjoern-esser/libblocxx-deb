PROGS= MemTracerTest unitMain exitWithSignal fillOutputBuffer runMultiProcessLogger

noinst_PROGRAMS = $(PROGS)
#check_PROGRAMS = unitMain


MemTracerTest_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

MemTracerTest_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o MemTracerTest$(EXEEXT)

MemTracerTest_SOURCES = \
MemTracerTest.cpp

runMultiProcessLogger_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

runMultiProcessLogger_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o runMultiProcessLogger$(EXEEXT)

runMultiProcessLogger_SOURCES = \
runMultiProcessLogger.cpp

check_PROGRAMS = unitMain
# Note: Do not change the following line, unless you also update the newtest.sh script
unitMain_SOURCES = \
IstreamBufIteratorTestCases.cpp \
IstreamBufIteratorTestCases.hpp \
PathSecurityTestCases.cpp \
PathSecurityTestCases.hpp \
AtomicOpsTestCases.cpp \
AtomicOpsTestCases.hpp \
LazyGlobalTestCases.cpp \
LazyGlobalTestCases.hpp \
LogAppenderTestCases.cpp \
LogAppenderTestCases.hpp \
DelayedFormatTestCases.cpp \
DelayedFormatTestCases.hpp \
ScopeLoggerTestCases.cpp \
ScopeLoggerTestCases.hpp \
GlobalPtrTestCases.cpp \
GlobalPtrTestCases.hpp \
IStringStreamTestCases.cpp \
IStringStreamTestCases.hpp \
UnnamedPipeTestCases.cpp \
UnnamedPipeTestCases.hpp \
ProcessTestCases.cpp \
ProcessTestCases.hpp \
ConditionTestCases.cpp \
ConditionTestCases.hpp \
TimeoutTimerTestCases.cpp \
TimeoutTimerTestCases.hpp \
ThreadOnceTestCases.cpp \
ThreadOnceTestCases.hpp \
CryptographicRandomNumberTestCases.cpp \
CryptographicRandomNumberTestCases.hpp \
CmdLineParserTestCases.cpp \
CmdLineParserTestCases.hpp \
COWIntrusiveReferenceTestCases.cpp \
COWIntrusiveReferenceTestCases.hpp \
DateTimeTestCases.cpp \
DateTimeTestCases.hpp \
EnumerationTestCases.cpp \
EnumerationTestCases.hpp \
ExceptionTestCases.cpp \
ExceptionTestCases.hpp \
ExecTestCases.cpp \
ExecTestCases.hpp \
FileSystemTestCases.cpp \
FileSystemTestCases.hpp \
FormatTestCases.cpp \
FormatTestCases.hpp \
InetAddressTestCases.cpp \
InetAddressTestCases.hpp \
IPCMutexTestCases.cpp \
IPCMutexTestCases.hpp \
LoggerTestCases.cpp \
LoggerTestCases.hpp \
MD5TestCases.cpp \
MD5TestCases.hpp \
MemTracerTestCases.cpp \
MemTracerTestCases.hpp \
MutexTestCases.cpp \
MutexTestCases.hpp \
RandomNumberTestCases.cpp \
RandomNumberTestCases.hpp \
ReferenceTestCases.cpp \
ReferenceTestCases.hpp \
RWLockerTestCases.cpp \
RWLockerTestCases.hpp \
SocketUtilsTestCases.cpp \
SocketUtilsTestCases.hpp \
StackTraceTestCases.cpp \
StackTraceTestCases.hpp \
StringStreamTestCases.cpp \
StringStreamTestCases.hpp \
StringTestCases.cpp \
StringTestCases.hpp \
ThreadBarrierTestCases.cpp \
ThreadBarrierTestCases.hpp \
ThreadPoolTestCases.cpp \
ThreadPoolTestCases.hpp \
ThreadTestCases.cpp \
ThreadTestCases.hpp \
UTF8UtilsTestCases.cpp \
UTF8UtilsTestCases.hpp \
UUIDTestCases.cpp \
UUIDTestCases.hpp \
testSharedLibraryLoader.hpp \
UnitMain.cpp 


INCLUDES = \
-I$(top_srcdir)/test/CppUnit \
-I$(top_srcdir)/src

unitMain_LDADD = \
-L$(top_builddir)/test/CppUnit -lCppUnit \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(Z_LIBS) $(ICONV_LIB) \
$(REALTIME_LIBS) \
$(BINLINKAPPENDFLAGS)


unitMain_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o unitMain$(EXEEXT)

check_SCRIPTS = run_make_check.sh
EXTRA_DIST = $(check_SCRIPTS)

TESTS = run_make_check.sh

exitWithSignal_LDADD = \
$(BINLINKAPPENDFLAGS)

exitWithSignal_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o exitWithSignal$(EXEEXT)

exitWithSignal_SOURCES = \
exitWithSignal.cpp

fillOutputBuffer_LDADD = \
$(BINLINKAPPENDFLAGS)

fillOutputBuffer_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o fillOutputBuffer$(EXEEXT)

fillOutputBuffer_SOURCES = \
fillOutputBuffer.cpp

include $(top_srcdir)/Makefile-shared-lib.inc
# We don't need a dylib, as we only want to load and not link this library.
MAKEDYLIB=0
MAKEBUNDLE=1
BLOCXX_SHARED_LIBS = libLazyGlobalTestLibrary.$(LIB_EXT)

LIBRARY_BINARY_VERSION = $(BLOCXX_LIBRARY_VERSION)
SHAREDLIBADD = -L$(top_builddir)/src/blocxx -lblocxx

noinst_LIBRARIES = libLazyGlobalTestLibrary.a
libLazyGlobalTestLibrary_a_SOURCES = LazyGlobalTestLibrary.cpp

all-local: all-blocxx-shared-libs
clean-local: clean-blocxx-shared-libs

