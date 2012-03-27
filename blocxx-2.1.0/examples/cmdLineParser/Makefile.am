PROGS=cmdLineParser 

noinst_PROGRAMS = $(PROGS)
#check_PROGRAMS = unitMain


cmdLineParser_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

cmdLineParser_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o cmdLineParser$(EXEEXT)

cmdLineParser_SOURCES = \
cmdLineParser.cpp

INCLUDES = \
-I$(top_srcdir)/src 

REPLIBFLAGS=


