PROGS = \
cmdLineParser \
dynamicCmdLineParser

noinst_PROGRAMS = $(PROGS)

COMMON_LDADD =  \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) $(PCRE_LIB) \
$(BINLINKAPPENDFLAGS)

COMMON_LDFLAGS = $(CXXBINLINKFLAGS) -o $@

cmdLineParser_LDADD = $(COMMON_LDADD)
cmdLineParser_LDFLAGS = $(COMMON_LDFLAGS)
cmdLineParser_SOURCES = cmdLineParser.cpp
dynamicCmdLineParser_LDADD = $(COMMON_LDADD)
dynamicCmdLineParser_LDFLAGS = $(COMMON_LDFLAGS)
dynamicCmdLineParser_SOURCES = dynamicCmdLineParser.cpp

INCLUDES = -I$(top_srcdir)/src


REPLIBFLAGS=


