PROGS= utils

noinst_PROGRAMS = $(PROGS)
#check_PROGRAMS = unitMain


utils_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) $(PCRE_LIB) \
$(BINLINKAPPENDFLAGS)

utils_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o utils$(EXEEXT)

utils_SOURCES = \
utils.cpp

INCLUDES = \
-I$(top_srcdir)/src 

REPLIBFLAGS=


