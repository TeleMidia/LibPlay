# cfg.mk -- Setup maintainer's makefile.
# Copyright (C) 2015-2016 PUC-Rio/Laboratorio TeleMidia
#
# This file is part of LibPlay.
#
# LibPlay is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# LibPlay is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
# License for more details.
#
# You should have received a copy of the GNU General Public License
# along with LibPlay.  If not, see <http://www.gnu.org/licenses/>.

COPYRIGHT_YEAR= 2016
COPYRIGHT_HOLDER= PUC-Rio/Laboratorio TeleMidia

INDENT_OPTIONS=\
  $(NULL)

INDENT_EXCLUDE=\
  $(NULL)

INDENT_JOIN_EMPTY_LINES_EXCLUDE=\
  $(NULL)

INDENT_TYPES=\
  $(NULL)

SC_USELESS_IF_BEFORE_FREE_ALIASES=\
  g_free\
  $(NULL)

SYNTAX_CHECK_EXCLUDE=\
  $(NULL)

SC_COPYRIGHT_EXCLUDE=\
  $(REMOTE_FILES)\
  play/luax-macros.h\
  $(NULL)

UPDATE_COPYRIGHT_EXCLUDE=\
  $(NULL)

SC_RULES+= sc-copyright
sc-copyright:
	$(V_at)$(build_aux)/syntax-check-copyright\
	  -b='/*' -e='*/' -t=cfg.mk\
	  $(call vc_list_exclude, $(VC_LIST_C), $(SC_COPYRIGHT_EXCLUDE))
	$(V_at)$(build_aux)/syntax-check-copyright\
	  -b='#' -t=cfg.mk\
	  $(call vc_list_exclude,\
	    $(VC_LIST_AC)\
	    $(VC_LIST_AM)\
	    $(VC_LIST_MK)\
	    $(VC_LIST_PL)\
	    $(VC_LIST_SH),\
	    $(SC_COPYRIGHT_EXCLUDE))

# Files copied from the NCLua project.
nclua:= https://github.com/gflima/nclua/raw/master
NCLUA_FILES+= build-aux/Makefile.am.common
NCLUA_FILES+= build-aux/Makefile.am.coverage
NCLUA_FILES+= build-aux/Makefile.am.env
NCLUA_FILES+= build-aux/Makefile.am.gitlog
NCLUA_FILES+= build-aux/Makefile.am.link
NCLUA_FILES+= build-aux/Makefile.am.valgrind
NCLUA_FILES+= build-aux/util.m4
NCLUA_FILES+= lib/luax-macros.h
NCLUA_FILES+= lib/macros.h
NCLUA_FILES+= maint.mk
NCLUA_FILES+= tests/lua.c
NCLUA_SCRIPTS+= bootstrap
NCLUA_SCRIPTS+= build-aux/syntax-check
NCLUA_SCRIPTS+= build-aux/syntax-check-copyright
REMOTE_FILES+= $(NCLUA_FILES)
REMOTE_SCRIPTS+= $(NCLUA_SCRIPTS)
fetch-remote-local:
	$(V_at)for path in $(NCLUA_FILES) $(NCLUA_SCRIPTS); do\
	  if test "$$path" = "lib/luax-macros.h"; then\
	    dir=play;\
	  else\
	    dir=`dirname "$$path"`;\
	  fi;\
	  $(FETCH) -dir="$$dir" "$(nclua)/$$path" || exit 1;\
	done

# Build dependencies locally.
glib_dir:= deps/glib
glib_git:= git://git.gnome.org/glib
glib_configure:=\
  --prefix=$(PWD)/deps/tree\
  --enable-debug=yes\
  --enable-gc-friendly\
  --disable-mem-pools\
  --disable-installed-tests\
  --disable-always-build-tests\
  --disable-man\
  --disable-compile-warnings\
  CFLAGS='-O0 -g'\
  $(NULL)

gstreamer_dir:= deps/gstreamer
gstreamer_git_root:= git://anongit.freedesktop.org/gstreamer
gstreamer_git:= $(gstreamer_git_root)/gstreamer
gstreamer_configure:=\
  --prefix=$(PWD)/deps/tree\
  --disable-fatal-warnings\
  --disable-examples\
  --disable-tests\
  --disable-failing-tests\
  --disable-benchmarks\
  --disable-docbook\
  --disable-gtk-doc\
  --disable-check\
  --with-pkg-config-path=$(PWD)/deps/tree/lib/pkgconfig\
  $(NULL)
gstreamer_build:=\
  CFLAGS='-O0 -g'\
  $(NULL)

gstbase_dir:= deps/gstbase
gstbase_git:= $(gstreamer_git_root)/gst-plugins-base
gstbase_configure= $(gstreamer_configure)
gstbase_build= $(gstreamer_build)

gstgood_dir:= deps/gstgood
gstgood_git:= $(gstreamer_git_root)/gst-plugins-good
gstgood_configure= $(gstreamer_configure)
gstgood_build= $(gstreamer_build)

gstbad_dir:= deps/gstbad
gstbad_git:= $(gstreamer_git_root)/gst-plugins-bad
gstbad_configure= $(gstreamer_configure)
gstbad_build= $(gstreamer_build)

DEPS= glib gstreamer gstbase gstgood gstbad

# Bootstraps project with local deps.
.PHONY: deps-bootstrap
deps-bootstrap:
	$(V_at)./$(BOOTSTRAP)
	$(V_at)./configure $(bootstrap_default_options) $(BOOTSTRAP_EXTRA)\
	  PKG_CONFIG_PATH=$(PWD)/deps/tree/lib/pkgconfig

# Sync (cloning, if necessary) local deps.
.PHONY: deps-sync
deps-sync: $(foreach dep,$(DEPS),deps-sync-$(dep))

define deps_sync_tpl=
.PHONY: deps-sync-$(1)
deps-sync-$(1):
	test -d $(2) || git clone $(3) $(2)
	(cd $(2) && git pull) || exit 1
endef
$(foreach dep,$(DEPS),\
  $(eval $(call deps_sync_tpl,$(dep),$($(dep)_dir),$($(dep)_git))))

# Configure (forcefully) local deps.
.PHONY: deps-force-configure
deps-force-configure: $(foreach dep,$(DEPS),deps-force-configure-$(dep))
	$(GLIB_CONFIGURE)
	$(GSTREAMER_CONFIGURE)

define deps_force_configure_tpl=
.PHONY: deps-force-configure-$(1)
deps-force-configure-$(1):
	cd $(2) && ./autogen.sh $(3)
endef
$(foreach dep,$(DEPS),\
  $(eval $(call deps_force_configure_tpl,$(dep),$($(dep)_dir),$($(dep)_configure))))

# Configure local deps.
.PHONY: deps-configure
deps-configure: $(foreach dep,$(DEPS),deps-configure-$(dep))
define deps_configure_tpl=
.PHONY: deps-configure-$(1)
deps-configure-$(1):
	test -f $(2)/configure || (cd $(2) && ./autogen.sh $(3))
endef
$(foreach dep,$(DEPS),\
  $(eval $(call deps_configure_tpl,$(dep),$($(dep)_dir),$($(dep)_configure))))

# Build local deps.
.PHONY: deps-build
deps-build: deps-configure $(foreach dep,$(DEPS),deps-build-$(dep))
define deps_build_tpl=
.PHONY: deps-build-$(1)
deps-build-$(1):
	$(MAKE) -C $(2) $(3)
endef
$(foreach dep,$(DEPS),\
  $(eval $(call deps_build_tpl,$(dep),$($(dep)_dir),$($(dep)_build))))

# Clean local deps.
.PHONY: deps-clean
deps-clean: $(foreach dep,$(DEPS),deps-clean-$(dep))
define deps_clean_tpl=
.PHONY: deps-clean-$(1)
deps-clean-$(1):
	$(MAKE) -C $(2) $(3) clean
endef
$(foreach dep,$(DEPS),\
  $(eval $(call deps_clean_tpl,$(dep),$($(dep)_dir),$($(dep)_clean))))

# Install local deps.
.PHONY: deps-install
deps-install: $(foreach dep,$(DEPS),deps-install-$(dep))
define deps_install_tpl=
.PHONY: deps-install-$(1)
deps-install-$(1):
	$(MAKE) -C $(2) $(3) install
endef
$(foreach dep,$(DEPS),\
  $(eval $(call deps_install_tpl,$(dep),$($(dep)_dir),$($(dep)_install))))
