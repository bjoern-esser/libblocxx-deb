
noinst_PROGRAMS = execute capture replace grep split match

INCLUDES = \
-I$(top_srcdir)/src 

REPLIBFLAGS=

execute_SOURCES = execute.cpp

execute_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

execute_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o execute$(EXEEXT)


capture_SOURCES = capture.cpp

capture_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

capture_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o capture$(EXEEXT)


replace_SOURCES = replace.cpp

replace_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

replace_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o replace$(EXEEXT)


grep_SOURCES    = grep.cpp

grep_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

grep_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o grep$(EXEEXT)


split_SOURCES   = split.cpp

split_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

split_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o split$(EXEEXT)


match_SOURCES   = match.cpp

match_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

match_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o match$(EXEEXT)

