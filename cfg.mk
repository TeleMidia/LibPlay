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

# Build dependencies.
glib_git:= git://git.gnome.org/glib
glib_configure:=\
  --prefix=$(PWD)/deps/tree\
  --enable-debug\
  --enable-gc-friendly\
  --disable-mem-pools\
  --disable-gtk-doc\
  --disable-man\
  --disable-dtrace\
  --with-libiconv=gnu\
  CFLAGS="-O0 -g"\
  $(NULL)
GLIB_CLONE= test -d deps/glib || git clone $(deps_git_glib) deps/glib
GLIB_SYNC= (cd deps/glib && git pull) || exit 1
GLIB_CONFIGURE= (cd deps/glib && ./autogen.sh $(glib_configure)) || exit 1
GLIB_MAKE= $(MAKE) -C deps/glib

.PHONY: deps-sync
deps-sync:
	$(GLIB_TRY_CLONE)
	$(GLIB_SYNC)

.PHONY: deps-force-configure
deps-force-configure:
	$(GLIB_CONFIGURE)

.PHONY: deps-configure
deps-configure:
	test -f deps/glib/configure || { $(GLIB_CONFIGURE); } && :

.PHONY: deps-build
deps-build: deps-configure
	$(GLIB_MAKE)

.PHONY: deps-clean
deps-clean:
	$(GLIB_MAKE) clean

.PHONY: deps-install
deps-install: deps-build
	$(GLIB_MAKE) install
