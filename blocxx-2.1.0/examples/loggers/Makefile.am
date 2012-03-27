PROGS= loggers

noinst_PROGRAMS = $(PROGS)
#check_PROGRAMS = unitMain


loggers_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

loggers_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o loggers$(EXEEXT)

loggers_SOURCES = \
loggers.cpp \
MutexedLogger.cpp \
MutexedLogger.hpp

INCLUDES = \
-I$(top_srcdir)/src 

REPLIBFLAGS=


