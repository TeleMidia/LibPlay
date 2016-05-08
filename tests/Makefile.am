# Makefile.am -- Template for generating Makefile via Automake.
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

include $(top_srcdir)/build-aux/Makefile.am.common

AM_CPPFLAGS= -I$(top_srcdir)/lib -I$(top_builddir)/lib\
  -DTOP_SRCDIR=\"$(top_srcdir)\" -DTOP_BUILDDIR=\"$(top_builddir)\"

AM_CFLAGS= $(WERROR_CFLAGS) $(WARN_CFLAGS)\
  $(GLIB_CFLAGS) $(GSTREAMER_CFLAGS)
AM_LDFLAGS= -static $(GLIB_LIBS) $(GSTREAMER_LIBS)
LDADD= $(top_builddir)/lib/libplay.la

# Auxiliary programs.
noinst_PROGRAMS=
if WITH_LUA
noinst_PROGRAMS+= lua
lua_CFLAGS= $(AM_CFLAGS) $(LUA_CFLAGS)
lua_LDFLAGS= $(AM_LDFLAGS) $(LUA_LIBS)
endif

# Auxiliary scripts.
noinst_SCRIPTS=

# Generate env.sh, env.csh, and gdb.cfg.
include $(top_srcdir)/build-aux/Makefile.am.env

# Configure Valgrind and generate suppfile from suppfile.m4.
include $(top_srcdir)/build-aux/Makefile.am.valgrind

# Setup test framework.
AM_COLOR_TESTS= always
DISABLE_HARD_ERRORS= true
TEST_EXTENSIONS= .lua .sh
LOG_COMPILER= $(_VALGRIND)
LUA_LOG_COMPILER= $(_VALGRIND) $(builddir)/lua
SH_LOG_COMPILER= $(SHELL)

# Setup tests environment.
LD_PRELOAD=
if WITH_LUA
LD_PRELOAD+= play0$(SOEXT)
endif

AM_TESTS_ENVIRONMENT=\
  export GST_DEBUG="*:WARN";\
  export G_DEBUG="all";\
  export G_SLICE="always-malloc";\
  export LD_LIBRARY_PATH="$(top_builddir)/lib/.libs:$(top_builddir)/play:$$LD_LIBRARY_PATH";\
  export MALLOC_CHECK_=1;\
  export LD_PRELOAD="$(LD_PRELOAD)";\
  export LUA_CPATH="$(builddir)/.libs/?$(SOEXT);$(top_builddir)/?$(SOEXT);;";\
  export LUA_PATH="$(top_srcdir)/?.lua;$(top_srcdir)/?/init.lua;$(top_srcdir)/tests/?.lua;;";\
  $(NULL)

# Test scripts.
scripts=
if WITH_LUA
scripts+= lua-play0-scene.lua
endif
check_SCRIPTS= $(scripts)

# Test programs.
programs=
programs+= test-lp-version
programs+= test-lp-version-string
programs+= test-lp-event-xfail
programs+= test-lp-event-tick
programs+= test-lp-event-key
programs+= test-lp-event-pointer-click
programs+= test-lp-event-pointer-move
programs+= test-lp-event-error
programs+= test-lp-event-start
programs+= test-lp-event-stop
programs+= test-lp-clock
programs+= test-lp-scene-new
programs+= test-lp-scene-prop-mask
programs+= test-lp-scene-prop-width-height
programs+= test-lp-scene-prop-pattern
programs+= test-lp-scene-prop-wave
programs+= test-lp-scene-prop-ticks
programs+= test-lp-scene-prop-interval
programs+= test-lp-scene-prop-time
programs+= test-lp-scene-prop-lockstep
programs+= test-lp-scene-advance
programs+= test-lp-media-new
programs+= test-lp-media-new-xfail-1
programs+= test-lp-media-new-xfail-2
programs+= test-lp-media-start
check_PROGRAMS= $(programs)

TESTS=\
  $(check_PROGRAMS)\
  $(check_SCRIPTS)\
  $(NULL)

# Tests that should fail.
XFAIL_TESTS=\
  test-lp-event-xfail\
  test-lp-media-new-xfail-1\
  test-lp-media-new-xfail-2\
  $(NULL)

EXTRA_DIST=\
  $(check_SCRIPTS)\
  gst.supp\
  suppfile.m4\
  tests.h\
  $(NULL)

CLEANFILES+= *.log *.trs
