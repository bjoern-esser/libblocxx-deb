PROGS= threads 

noinst_PROGRAMS = $(PROGS)
#check_PROGRAMS = unitMain


threads_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

threads_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o threads$(EXEEXT)

threads_SOURCES = \
threads.cpp \
demoRunnable.cpp \
demoRunnable.hpp \
barrierRunnable.cpp \
barrierRunnable.hpp \
conditionRunnable.cpp \
conditionRunnable.hpp

INCLUDES = \
-I$(top_srcdir)/src 

REPLIBFLAGS=

EXTRA_DIST = \
demoThread.cpp \
demoThread.hpp


