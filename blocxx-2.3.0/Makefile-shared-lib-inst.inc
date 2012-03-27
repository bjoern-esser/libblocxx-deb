# Make sure you define STATIC_COMMON_LIBRARY before including this file, if it
# should be anything other than the default.
STATIC_COMMON_LIBRARY ?= $(libdir)/libblocxx.a

# This is necessary so that it can be used as a variable in the following script. If make expands it, non-bash shells puke on "for linkdir in ;"
export BLOCXX_EXTRA_LINK_DIRS

# Common install rules.
define install-create-extra-links
	$(if $(BLOCXX_EXTRA_LINK_DIRS), \
		$(foreach linkdir,$(BLOCXX_EXTRA_LINK_DIRS), \
			$(INSTALL_PROGRAM) -d $(DESTDIR)$(linkdir) && \
			ln -sf $(thelibdir)/$(1) $(DESTDIR)$(linkdir)/$(2) && \
		) \
		true \
	)
endef

#
# Darwin (MacOS X) install rules
#
define install-darwin-lib
	rm -f $(DESTDIR)$(thelibdir)/$(1:%.$(LIB_EXT)=%.$(2))
	$(INSTALL_PROGRAM) $(1:%.$(LIB_EXT)=%.$(SO_EXT).$(2)) $(DESTDIR)$(thelibdir)
	ln -s $(1:%.$(LIB_EXT)=%.$(SO_EXT).$(2)) $(DESTDIR)$(thelibdir)/$(1:%.$(LIB_EXT)=%.$(2))
	$(call install-create-extra-links,$(1:%.$(LIB_EXT)=%.$(SO_EXT).$(2)),$(1:%.$(LIB_EXT)=%.$(2)))
endef

define install-check-dylib
	$(if $(MAKEDYLIB:0=)$(MAKEDYLIB:1=1),$(call install-darwin-lib,$(1),dylib),$(warning Not installing $(1:%.$(LIB_EXT)=%.dylib): MAKEDYLIB is disabled.))
endef
define install-check-bundle
	$(if $(MAKEBUNDLE:0=)$(MAKEBUNDLE:1=1),$(call install-darwin-lib,$(1),dylib.bundle),$(warning Not installing $(1): MAKEBUNDLE is disabled.))
endef

define install-lib-dylib.bundle
	$(call install-check-dylib,$(1))
	$(call install-check-bundle,$(1))
endef


#
# Netware install rules
#
define install-lib-nlm
	$(INSTALL_PROGRAM) $(1).$(SO_EXT) $(DESTDIR)$(thelibdir)
	rm -f $(DESTDIR)$(thelibdir)/$(1)
	ln -sf $(1).$(SO_EXT) $(DESTDIR)$(thelibdir)/$(1)
endef


#
# Linux, Solaris install rules
#
define install-lib-so
	$(INSTALL_PROGRAM) $(1).$(SO_EXT) $(DESTDIR)$(thelibdir)
	rm -f $(DESTDIR)$(thelibdir)/$(1)
	ln -sf $(1).$(SO_EXT) $(DESTDIR)$(thelibdir)/$(1)
	$(call install-create-extra-links,$(1).$(SO_EXT),$(1))
endef


#
# HP-UX install rules (just calls the normal "so" install function)
#
define install-lib-sl
	$(call install-lib-so,$(1))
endef


#
# Static install rules.
#
define install-fake-library
	@echo Installing fake library for $(1)
	$(INSTALL_PROGRAM) $(1:%.$(LIB_EXT)=%.fsl) $(DESTDIR)$(thelibdir)/$(1)
	$(call install-create-extra-links,$(1),$(1))
endef

install-fake-lib-%:
	$(call install-fake-library,$(@:install-fake-lib-%=%))

define install-static-lib-helper
	@true \
		# This is really ugly.  I want to be able to use a *MUCH* cleaner \
		# $ (if ...) here, but it will have the condition evaluated once \
		# before execution begins.  Thus, the results would depend on a \
		# previous invokation of $(MAKE) and not the current one.  Note \
		# that the recursive use of make here is used to allow make-based \
		# function calls from within the shell script without having the \
		# function called when it should not be (the false case). 
	if test -f $(1:%.$(LIB_EXT)=%.fsl); then \
		$(MAKE) --no-print-directory install-fake-lib-$(1); \
	else \
		echo "Warning: Not installing $(1) because no exports were found ($(1:%.$(LIB_EXT)=%.fsl))"; \
	fi
	@true \
		# This can be safely executed here, because all preconditions are \
		# set before the initial evaluation (before execution).
	$(if $(wildcard $(DESTDIR)/$(STATIC_COMMON_LIBRARY)), \
		$(warning Warning: Adding the static version of $(1) to $(DESTDIR)/${STATIC_COMMON_LIBRARY}) \
		$(AR) cru $(DESTDIR)/${STATIC_COMMON_LIBRARY} $(1:%.$(LIB_EXT)=%.a), \
		$(warning Warning: Not adding $(1) to static common library -- Does not exist: $(DESTDIR)$(STATIC_COMMON_LIBRARY)) \
	)
endef

define install-static-lib
	$(if $(strip $(STATIC_COMMON_LIBRARY)),,$(error STATIC_COMMON_LIBRARY is not defined in this makefile.  Cannot install.))
	@echo Checking $(1) for fake library exports...
	$(SHELL) $(top_srcdir)/fake_library_creator.sh $(EXPORT_PREFIX) $(1)
	$(call install-static-lib-helper,$(1))
endef




install-create-libdir:
	$(INSTALL_PROGRAM) -d $(DESTDIR)$(thelibdir)

install-blocxx-shared-lib-%: % install-create-libdir
	@echo "Executing install rule for $(filter-out install-create-libdir, $^)"
	$(if $(shell test x$(STATIC_SERVICES) = x1 -a x$(NO_FAKE_LIB_INSTALL) = x && echo STATIC), $(call install-static-lib,$(filter-out install-create-libdir, $^)), $(call install-lib-$(LIB_EXT),$(filter-out install-create-libdir, $^)))

#
# Install all shared libraries listed in $(BLOCXX_SHARED_LIBS) using the above install functions
#
# Note that this does "install as side-effect" because the $(foreach) command
# isn't well-suited to complex loops (individual commands would need trailing
# semicolons or &&s).
#
install-blocxx-shared-libs: $(BLOCXX_SHARED_LIBS:%=install-blocxx-shared-lib-%)



# The uninstall rules are much shorter than the install rules were (and
# executed less frequently), so they will be left in a single glob for now.
#
# NOTE: Static members added to the common library CAN NOT be uninstalled.
uninstall-blocxx-shared-libs:
	for i in $(BLOCXX_SHARED_LIBS); do \
		if [ "$(LIB_EXT)" = "dylib.bundle" ]; then \
			LONG_FILE_NAME=`echo $$i|cut -f1 -d'.'`.$(SO_EXT).$(LIB_EXT:%.bundle=%); \
			SHORT_FILE_NAME=`echo $$i|cut -f1 -d'.'`.$(LIB_EXT:%.bundle=%); \
			rm -f $(DESTDIR)$(thelibdir)/$$LONG_FILE_NAME; \
			rm -f $(DESTDIR)$(thelibdir)/$$LONG_FILE_NAME.bundle; \
			rm -f $(DESTDIR)$(thelibdir)/$$SHORT_FILE_NAME; \
			rm -f $(DESTDIR)$(thelibdir)/$$SHORT_FILE_NAME.bundle; \
			for linkdir in $$BLOCXX_EXTRA_LINK_DIRS; do \
				rm -f $(DESTDIR)$$linkdir/$$SHORT_FILE_NAME; \
				rm -f $(DESTDIR)$$linkdir/$$SHORT_FILE_NAME.bundle; \
			done; \
		elif [ "$(LIB_EXT)" = "nlm" ]; then \
			rm -f $(DESTDIR)$(thelibdir)/$$i; \
			for linkdir in $$BLOCXX_EXTRA_LINK_DIRS; do \
				rm -f $(DESTDIR)$$linkdir/$$i; \
			done; \
		else \
			rm -f $(DESTDIR)$(thelibdir)/$$i.$(SO_EXT); \
			rm -f $(DESTDIR)$(thelibdir)/$$i; \
			for linkdir in $$BLOCXX_EXTRA_LINK_DIRS; do \
				rm -f $(DESTDIR)$$linkdir/$$i; \
			done; \
		fi; \
	done;
