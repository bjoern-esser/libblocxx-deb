PROGS= MemTracerTest exitWithSignal fillOutputBuffer runMultiProcessLogger

noinst_PROGRAMS = $(PROGS) $(BUILT_TESTS)

COMMON_PROJECT_LIBS = \
-L$(top_builddir)/src/blocxx -lblocxx \
-L$(top_builddir)/src/blocxx_test -lblocxx_test

# CppUnit is listed here because as a static object, it must occur *AFTER* the
# objects that use it.
COMMON_EXTERNAL_LIBS = \
$(SSL_LIBS) $(Z_LIBS) $(ICONV_LIB) $(PCRE_LIB) $(REALTIME_LIBS)

MemTracerTest_LDFLAGS =
MemTracerTest_SOURCES = \
MemTracerTest.cpp

runMultiProcessLogger_LDFLAGS=
runMultiProcessLogger_SOURCES = \
runMultiProcessLogger.cpp

BUILT_TESTS = \
AtomicOpsTestCases \
CmdLineParserTestCases \
ConditionTestCases \
CompareTestCases \
COWIntrusiveReferenceTestCases \
CryptographicRandomNumberTestCases \
DateTimeTestCases \
DelayedFormatTestCases \
DivRemTestCases \
EnumerationTestCases \
ExceptionTestCases \
ExecTestCases \
FileSystemTestCases \
FileTestCases \
FormatTestCases \
GlobalPtrTestCases \
InetAddressTestCases \
IPCMutexTestCases \
IstreamBufIteratorTestCases \
LazyGlobalTestCases \
LogAppenderTestCases \
LoggerTestCases \
MD5TestCases \
MemTracerTestCases \
MTQueueTestCases \
MutexTestCases \
PathSecurityTestCases \
ProcessTestCases \
RandomNumberTestCases \
ReferenceTestCases \
RWLockerTestCases \
ScopeLoggerTestCases \
SocketUtilsTestCases \
StackTraceTestCases \
StringStreamTestCases \
StringTestCases \
TestFrameworkTestCases \
ThreadBarrierTestCases \
ThreadOnceTestCases \
ThreadPoolTestCases \
ThreadTestCases \
TimeDurationTestCases \
TimeoutTimerTestCases \
TimePeriodTestCases \
TmUtils_normTmTestCases \
TmUtils_timeGmTestCases \
UnnamedPipeTestCases \
UserUtilsTestCases \
UTF8UtilsTestCases \
UUIDTestCases \
\
FedPipeTestCases \
MockFileSystemTestCases

## add tests to makefile -- DO NOT EDIT THIS COMMENT
AtomicOpsTestCases_LDFLAGS =
AtomicOpsTestCases_SOURCES = \
AtomicOpsTestCases.cpp

CmdLineParserTestCases_LDFLAGS =
CmdLineParserTestCases_SOURCES = \
CmdLineParserTestCases.cpp

CompareTestCases_LDFLAGS =
CompareTestCases_SOURCES = \
CompareTestCases.cpp

ConditionTestCases_LDFLAGS =
ConditionTestCases_SOURCES = \
ConditionTestCases.cpp

COWIntrusiveReferenceTestCases_LDFLAGS =
COWIntrusiveReferenceTestCases_SOURCES = \
COWIntrusiveReferenceTestCases.cpp

CryptographicRandomNumberTestCases_LDFLAGS =
CryptographicRandomNumberTestCases_SOURCES = \
CryptographicRandomNumberTestCases.cpp

DateTimeTestCases_LDFLAGS =
DateTimeTestCases_SOURCES = \
DateTimeTestCases.cpp

DelayedFormatTestCases_LDFLAGS =
DelayedFormatTestCases_SOURCES = \
DelayedFormatTestCases.cpp

DivRemTestCases_LDFLAGS =
DivRemTestCases_SOURCES = \
DivRemTestCases.cpp

EnumerationTestCases_LDFLAGS =
EnumerationTestCases_SOURCES = \
EnumerationTestCases.cpp

ExceptionTestCases_LDFLAGS =
ExceptionTestCases_SOURCES = \
ExceptionTestCases.cpp

ExecTestCases_LDFLAGS =
ExecTestCases_SOURCES = \
ExecTestCases.cpp

FileSystemTestCases_LDFLAGS =
FileSystemTestCases_SOURCES = \
FileSystemTestCases.cpp

FileTestCases_LDFLAGS =
FileTestCases_SOURCES = \
FileTestCases.cpp

FormatTestCases_LDFLAGS =
FormatTestCases_SOURCES = \
FormatTestCases.cpp

GlobalPtrTestCases_LDFLAGS =
GlobalPtrTestCases_SOURCES = \
GlobalPtrTestCases.cpp

InetAddressTestCases_LDFLAGS =
InetAddressTestCases_SOURCES = \
InetAddressTestCases.cpp

IPCMutexTestCases_LDFLAGS =
IPCMutexTestCases_SOURCES = \
IPCMutexTestCases.cpp

IstreamBufIteratorTestCases_LDFLAGS =
IstreamBufIteratorTestCases_SOURCES = \
IstreamBufIteratorTestCases.cpp

LazyGlobalTestCases_LDFLAGS =
LazyGlobalTestCases_SOURCES = \
LazyGlobalTestCases.cpp

LogAppenderTestCases_LDFLAGS =
LogAppenderTestCases_SOURCES = \
LogAppenderTestCases.cpp

LoggerTestCases_LDFLAGS =
LoggerTestCases_SOURCES = \
LoggerTestCases.cpp

MD5TestCases_LDFLAGS =
MD5TestCases_SOURCES = \
MD5TestCases.cpp

MemTracerTestCases_LDFLAGS =
MemTracerTestCases_SOURCES = \
MemTracerTestCases.cpp

MTQueueTestCases_LDFLAGS =
MTQueueTestCases_SOURCES = \
MTQueueTestCases.cpp

MutexTestCases_LDFLAGS =
MutexTestCases_SOURCES = \
MutexTestCases.cpp

PathSecurityTestCases_LDFLAGS =
PathSecurityTestCases_SOURCES = \
PathSecurityTestCases.cpp

ProcessTestCases_LDFLAGS =
ProcessTestCases_SOURCES = \
ProcessTestCases.cpp

RandomNumberTestCases_LDFLAGS =
RandomNumberTestCases_SOURCES = \
RandomNumberTestCases.cpp

ReferenceTestCases_LDFLAGS =
ReferenceTestCases_SOURCES = \
ReferenceTestCases.cpp

RWLockerTestCases_LDFLAGS =
RWLockerTestCases_SOURCES = \
RWLockerTestCases.cpp

ScopeLoggerTestCases_LDFLAGS =
ScopeLoggerTestCases_SOURCES = \
ScopeLoggerTestCases.cpp

SocketUtilsTestCases_LDFLAGS =
SocketUtilsTestCases_SOURCES = \
SocketUtilsTestCases.cpp

StackTraceTestCases_LDFLAGS =
StackTraceTestCases_SOURCES = \
StackTraceTestCases.cpp

StringStreamTestCases_LDFLAGS =
StringStreamTestCases_SOURCES = \
StringStreamTestCases.cpp

StringTestCases_LDFLAGS =
StringTestCases_SOURCES = \
StringTestCases.cpp

TestFrameworkTestCases_LDFLAGS =
TestFrameworkTestCases_SOURCES = \
TestFrameworkTestCases.cpp

ThreadBarrierTestCases_LDFLAGS =
ThreadBarrierTestCases_SOURCES = \
ThreadBarrierTestCases.cpp

ThreadOnceTestCases_LDFLAGS =
ThreadOnceTestCases_SOURCES = \
ThreadOnceTestCases.cpp

ThreadPoolTestCases_LDFLAGS =
ThreadPoolTestCases_SOURCES = \
ThreadPoolTestCases.cpp

ThreadTestCases_LDFLAGS =
ThreadTestCases_SOURCES = \
ThreadTestCases.cpp

TimeDurationTestCases_LDFLAGS =
TimeDurationTestCases_SOURCES = \
TimeDurationTestCases.cpp

TimeoutTimerTestCases_LDFLAGS =
TimeoutTimerTestCases_SOURCES = \
TimeoutTimerTestCases.cpp

TimePeriodTestCases_LDFLAGS =
TimePeriodTestCases_SOURCES = \
TimePeriodTestCases.cpp

TmUtils_normTmTestCases_LDFLAGS =
TmUtils_normTmTestCases_SOURCES = \
TmUtils_normTmTestCases.cpp

TmUtils_timeGmTestCases_LDFLAGS =
TmUtils_timeGmTestCases_SOURCES = \
TmUtils_timeGmTestCases.cpp

UnnamedPipeTestCases_LDFLAGS =
UnnamedPipeTestCases_SOURCES = \
UnnamedPipeTestCases.cpp

UserUtilsTestCases_LDFLAGS =
UserUtilsTestCases_SOURCES = \
UserUtilsTestCases.cpp

UTF8UtilsTestCases_LDFLAGS =
UTF8UtilsTestCases_SOURCES = \
UTF8UtilsTestCases.cpp

UUIDTestCases_LDFLAGS =
UUIDTestCases_SOURCES = \
UUIDTestCases.cpp

FedPipeTestCases_LDFLAGS =
FedPipeTestCases_SOURCES = FedPipeTestCases.cpp
MockFileSystemTestCases_LDFLAGS =
MockFileSystemTestCases_SOURCES = MockFileSystemTestCases.cpp

INCLUDES = \
-I$(top_srcdir)/src/blocxx_test/CppUnit \
-I$(top_srcdir)/src -I$(top_builddir)/src

check_SCRIPTS = run_make_check.sh set_test_libpath.sh
EXTRA_DIST = $(check_SCRIPTS)

TESTS = $(BUILT_TESTS)

exitWithSignal_LDFLAGS =
exitWithSignal_SOURCES = \
exitWithSignal.cpp

fillOutputBuffer_LDFLAGS =
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

local-scripts: $(check_SCRIPTS)
	chmod +x $^

all-local: all-blocxx-shared-libs local-scripts
clean-local: clean-blocxx-shared-libs

# For each program we run as a test, execute it with this script.
TESTS_ENVIRONMENT = $(top_srcdir)/test/unit/set_test_libpath.sh


# This will set the PROJECT_LIBS and EXTERNAL_LIBS variables for each of the
# built tests if they did not define it above.  This should help keep this
# makefile simple.  This may be a good thing to put in the normal executable
# hackery of Makefile-executable.inc
define TEST_TEMPLATE
$(if $(value $(1)_PROJECT_LIBS),,$(1)_PROJECT_LIBS=$(COMMON_PROJECT_LIBS))
$(if $(value $(1)_EXTERNAL_LIBS),,$(1)_EXTERNAL_LIBS=$(COMMON_EXTERNAL_LIBS))
endef
$(foreach prog,$(PROGS) $(BUILT_TESTS),$(eval $(call TEST_TEMPLATE,$(prog))))

# A hack to include the specific include flags for a particular target in the
# includes.  The need for hacks like this is part of why automake sucks so bad.
AM_CPPFLAGS = $($(@:%.o=%)_INCLUDES)

include $(top_srcdir)/Makefile-replacement.inc
include $(top_srcdir)/Makefile-executable.inc
