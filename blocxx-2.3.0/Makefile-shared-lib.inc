
SO_EXT = $(LIBRARY_BINARY_VERSION)
SOFILENAME = $@.$(SO_EXT)
# This DYFILENAME definition looks confusing.  It takes the target of the
# Darwin rule, makes sure there is a library version in it (only one), and
# forces the suffix as .dylib.
DYFILENAME = $(patsubst %.$(SO_EXT).$(SO_EXT).dylib,%.$(SO_EXT).dylib,$(sort $(patsubst %.dylib,%,$(patsubst %.dylib.bundle,%.dylib,$@))).$(SO_EXT).dylib)
TMPLIBDIR = .$@.libs


# Here's what this does:  It finds all symbols in each object file that are one
# of T,D,B (Global text symbol, Global data symbol, Global BSS symbol), where
# the first character is not a '.', and puts them in a list, which is then
# sorted (unique), and dumped in LIBNAME.exp 
# 
# The nm options are: Berleky style (3 column), no demangling, unsorted, global
# symbols only. 
CREATE_EXPORTS=nm -BCpg $(TMPLIBDIR)/*.o | \
			$(AWK) '{ if((($$2 == "T") || \
				   ($$2 == "D") || \
				   ($$2 == "B")) && \
				  (substr($$3,1,1) != ".")) \
				{ print $$3 } }' | \
			sort -u > $(@:%.so=%.exp)


# This exists to make the body of the shared library rule more readable.
# Note the use of the override keyword to keep automake from barfing about redefining a value.
DO_CREATE_EXPORTS=
DO_INCLUDE_EXPORTS=
if AIX
if BLOCXX_STATIC_LINKING
override DO_CREATE_EXPORTS=$(CREATE_EXPORTS)
override DO_INCLUDE_EXPORTS=-Wl,-bexport:$(@:%.so=%.exp)
endif # BLOCXX_STATIC_LINKING
endif # AIX

%.so %.sl : %.a
	rm -rf $(TMPLIBDIR)
	mkdir $(TMPLIBDIR)
	cd $(TMPLIBDIR) && \
	$(AR) x ../$<
	$(DO_CREATE_EXPORTS)
	$(CXXLINK) $(CXXSHAREDLIBLINKFLAGS) $(DO_INCLUDE_EXPORTS) -o $(SOFILENAME) $(TMPLIBDIR)/*.o $(SHAREDLIBADD)
	rm -rf $@ $(TMPLIBDIR) $(@:%.so=%.exp)
	ln -s $(SOFILENAME) $@
	chmod +x $(SOFILENAME)

# Rules for NetWare

#	Look only for symbols in the text "T" section because that is all the 
#	fake library stuff needs to be concerned with.
CREATE_IMPORTS=nm -Bpg $(TMPLIBDIR)/*.o | \
			$(AWK) '{ if( ($$2 == "T") && \
				      (substr($$3,1,1) != ".") && \
				      (substr($$3,1,1) != "_") ) \
				{ print $$3 } }' > $(@:%.nlm=%.imp)

#	Removed the following line
#	$(CXXLINK) $(CXXSHAREDLIBLINKFLAGS) -o $@ $(TMPLIBDIR)/*.o $(SHAREDLIBADD)
#	and changed it to a 'touch' command because we no longer need dynamic
#	libraries.  The 'touch' command speeds up the compile significantly
%.nlm : %.a
	rm -rf $(TMPLIBDIR)
	mkdir $(TMPLIBDIR)
	cd $(TMPLIBDIR) && \
	$(AR) x ../$<
	$(CREATE_IMPORTS)
	touch $@
	cp $@ $(SOFILENAME)
	rm -rf $(TMPLIBDIR)

DO_NOTHING=
VALID_CONDITION=$(1:0=)$(1:1=1)

BUILD_DYLIB=$(if $(call VALID_CONDITION,$(MAKEDYLIB)), \
$(CXXLINK) $(CXXSHAREDLIBLINKFLAGS) -o $(DYFILENAME) $(TMPLIBDIR)/*.o $(SHAREDLIBADD), \
$(DO_NOTHING))
BUILD_BUNDLE=$(if $(call VALID_CONDITION,$(MAKEBUNDLE)), \
$(CXXLINK) $(CXXBUNDLELIBLINKFLAGS) -o $(DYFILENAME).bundle $(TMPLIBDIR)/*.o $(SHAREDLIBADD), \
$(DO_NOTHING))


TOUCH_FILE_IF_NEEDED=$(if $(wildcard $(1)),$(DO_NOTHING),echo "The target for the rule was not created. Touching $(1)"; touch "$(1)")

define TOUCH_UNCREATED_LIBS
	$(call TOUCH_FILE_IF_NEEDED,$(DYFILENAME))
	$(call TOUCH_FILE_IF_NEEDED,$(DYFILENAME).bundle)
endef

# Rules for OSX (Darwin) so we can get the plugable modules built
# at the same time. Requires that either MAKEDYLIB or MAKEBUNDLE
# be inserted into Makefile.am in the appropriate directory.
%.$(SO_EXT).dylib.bundle %.$(SO_EXT).dylib : %.a
	rm -f $(@:%.dylib=%.dylib.bundle) $@ $(DYFILENAME)
	rm -rf $(TMPLIBDIR)
	mkdir $(TMPLIBDIR)
	cd $(TMPLIBDIR) && \
	$(AR) x ../$<
	$(BUILD_DYLIB)
	$(BUILD_BUNDLE)
	$(TOUCH_UNCREATED_LIBS)
	rm -rf $(TMPLIBDIR)
	@echo "$@ done"

%.dylib.bundle %.dylib : %.$(SO_EXT).dylib.bundle %.$(SO_EXT).dylib
	ln -f -v -s $(DYFILENAME) $(DYFILENAME:%.$(SO_EXT).dylib=%.dylib)
	ln -f -v -s $(DYFILENAME).bundle $(DYFILENAME:%.$(SO_EXT).dylib=%.dylib).bundle

# Prevent the generated libraries (and symlinks) from being removed as 'side
# effects'.  This improves recompilation (or installation) time in some cases.
.PRECIOUS: %.$(SO_EXT).dylib.bundle %.dylib.bundle %.$(SO_EXT).dylib %.dylib
.PRECIOUS: %.so.$(SO_EXT) %.so
.PRECIOUS: %.sl.$(SO_EXT) %.sl
.PRECIOUS: %.a

all-blocxx-shared-libs: $(BLOCXX_SHARED_LIBS)

clean-blocxx-shared-libs:
	rm -rf $(BLOCXX_SHARED_LIBS:%=.%.libs)
	rm -f $(BLOCXX_SHARED_LIBS:%=%*)
	rm -f $(BLOCXX_SHARED_LIBS:%.dylib.bundle=%.*.dylib*) $(BLOCXX_SHARED_LIBS:%.dylib.bundle=%.dylib*)
	rm -f *.fsl
