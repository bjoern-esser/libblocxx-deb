#
# This file is used to help in defining some flags needed for compiling
# executables.
#
# It is suggested that this file should be included at the *END* of your
# Makefile.am instead of the beginning.  This is because it dynamically creates
# some makefile rules and will not function properly if you are defining
# OW_EXECUTABLES or relying on the default of using <foo>_PROGRAMS for rule
# autogeneration.
#
# If you have static libraries that you need to link to, even if they are
# internal to the project, you are probably better off putting them in the
# EXTERNAL_LIBS than in the PROJECT_LIBS.  The reason for this is that they
# normally must occur after the objects that use them.
#

# This define creates a few variables that are used by automake.  They are
# defined in a non-standard way to prevent automake from puking on anything
# that uses '$' on the left side of an '='.
#
# NOTE: Anything that needs a $ in it to be evaluated after the initial call
# needs to use $$.  This would be needed for $$ or any other environment
# variables used.  Pretty much the only thing that should have a single $ are
# whatever is passed as positional arguments ($(1)).
define EXECUTABLE_FLAG_TEMPLATE

# Define <foo>_LDFLAGS in terms of the predefined values requested.  The
# resulting LDFLAGS are applied *before* any object files listed on a link line
# and so should not contain any library references.
define $(1)_LDFLAGS
-o $(1)$$(EXEEXT) $$(CXXBINLINKFLAGS_START) $$(COMMON_LDFLAGS)
endef

# Define <foo>_LDADD in terms of any PROJECT_LIBS, EXTERNAL_LIBS and any
# systemwide predefined append flags
define $(1)_LDADD
$$($(1)_PROJECT_LIBS) $$($(1)_EXTERNAL_LIBS) $$(BINLINKAPPENDFLAGS)
endef

# Define <foo>_DEPENCIES to be *.a files listed in _EXTERNAL_LIBS and the
# objects generated for the target.
define $(1)_DEPENDENCIES
$$(filter %.a,$$($(1)_EXTERNAL_LIBS)) $$($(1)_OBJECTS)
endef

# A simple makefile testing target.  Note that @ is replaced with $(1) or it
# will not print properly outside of a real rule.
showflags-$(1):
	@echo '$(1)_LDFLAGS=$$($(1)_LDFLAGS:@=$(1))'
	@echo '$(1)_LDADD=$$($(1)_LDADD:@=$(1))'
	@echo '$(1)_DEPENDENCIES=$$($(1)_DEPENDENCIES:@=$(1))'
	@echo '$(1)_EXTERNAL_LIBS=$$($(1)_EXTERNAL_LIBS:@=$(1))'
	@echo '$(1)_PROJECT_LIBS=$$($(1)_PROJECT_LIBS:@=$(1))'
	@echo '$(1)_SOURCES=$$($(1)_SOURCES:@=$(1))'
	@echo '$(1)_LINK=$$($(1)_LINK:@=$(1))'
	@echo '$(1)_OBJECTS=$$($(1)_OBJECTS:@=$(1))'
	@echo 'LIBS=$(LIBS)'

endef # end of EXECUTABLE_FLAG_TEMPLATE

# Conditionally define OW_EXECUTABLES to a few standard <foo>_PROGRAMS.
OW_EXECUTABLES ?= \
$(bin_PROGRAMS) \
$(sbin_PROGRAMS) \
$(noinst_PROGRAMS)

# Generate compilation rules for all executables listed in $(OW_EXECUTABLES).
# This uses a few tricky items.  First, the call will produce the text for the
# above template.  Second, the eval will dynamically create makefile rules or
# assignments from whatever text is given as its input.
$(foreach prog,$(OW_EXECUTABLES),$(eval $(call EXECUTABLE_FLAG_TEMPLATE,$(prog))))
