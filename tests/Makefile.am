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
include $(top_srcdir)/build-aux/Makefile.am.link

AM_CPPFLAGS= -I$(top_srcdir)/lib -I$(top_builddir)/lib\
  -DTOP_SRCDIR=\"$(top_srcdir)\" -DTOP_BUILDDIR=\"$(top_builddir)\"

AM_CFLAGS= $(WERROR_CFLAGS) $(WARN_CFLAGS)\
  $(GLIB_CFLAGS) $(GSTREAMER_CFLAGS)
AM_LDFLAGS= -static $(GLIB_LIBS) $(GSTREAMER_LIBS)
LDADD= $(top_builddir)/lib/libplay.la

# Auxiliary libraries.
noinst_LTLIBRARIES=
if WITH_LUA
noinst_LTLIBRARIES+= tests0.la
tests0_la_CPPFLAGS= $(AM_CPPFLAGS) -I$(top_srcdir)/play
tests0_la_CFLAGS= $(AM_CFLAGS) $(GLIB_CFLAGS) $(LUA_CFLAGS)
tests0_la_LDFLAGS= -module $(LT_MODULE_LDFLAGS) -rpath '/force-shared'\
  $(GLIB_LIBS) $(LUA_LIBS)
endif

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
  export LD_LIBRARY_PATH="$(top_builddir)/deps/tree/lib:$(top_builddir)/lib/.libs:$(top_builddir)/play:$$LD_LIBRARY_PATH";\
  export MALLOC_CHECK_=1;\
  export LD_PRELOAD="$(LD_PRELOAD)";\
  export LUA_CPATH="$(builddir)/.libs/?$(SOEXT);$(top_builddir)/?$(SOEXT);;";\
  export LUA_PATH="$(top_srcdir)/?.lua;$(top_srcdir)/?/init.lua;$(top_srcdir)/tests/?.lua;;";\
  $(NULL)

# Test scripts.
scripts=
if WITH_LUA
scripts+= test-lua-play0-scene-api.lua
scripts+= test-lua-play0-scene-new.lua
scripts+= test-lua-play0-scene-get.lua
scripts+= test-lua-play0-scene-set.lua
scripts+= test-lua-play0-scene-receive.lua
scripts+= test-lua-play0-scene-tostring.lua
scripts+= test-lua-play0-media-api.lua
scripts+= test-lua-play0-media-new.lua
scripts+= test-lua-play0-media-tostring.lua
endif
check_SCRIPTS= $(scripts)

.PHONY: check-scripts
check-scripts:
	$(MAKE) $(AM_MAKEFLAGS) check TESTS='$(check_SCRIPTS)'

# Test programs.
programs=
programs+= test-lp-version
programs+= test-lp-version-string
programs+= test-lp-event-xfail-new
programs+= test-lp-event-quit
programs+= test-lp-event-quit-xfail-get
programs+= test-lp-event-quit-xfail-set
programs+= test-lp-event-tick
programs+= test-lp-event-tick-xfail-get
programs+= test-lp-event-tick-xfail-set
programs+= test-lp-event-key
programs+= test-lp-event-key-xfail-get
programs+= test-lp-event-key-xfail-set
programs+= test-lp-event-pointer-click
programs+= test-lp-event-pointer-click-xfail-get
programs+= test-lp-event-pointer-click-xfail-set
programs+= test-lp-event-pointer-move
programs+= test-lp-event-pointer-move-xfail-get
programs+= test-lp-event-pointer-move-xfail-set
programs+= test-lp-event-error
programs+= test-lp-event-error-xfail-get
programs+= test-lp-event-error-xfail-set
programs+= test-lp-event-start
programs+= test-lp-event-start-xfail-get
programs+= test-lp-event-start-xfail-set
programs+= test-lp-event-stop
programs+= test-lp-event-stop-xfail-get
programs+= test-lp-event-stop-xfail-set
programs+= test-lp-event-seek
programs+= test-lp-event-seek-xfail-get
programs+= test-lp-event-seek-xfail-set
programs+= test-lp-clock
programs+= test-lp-clock-xfail-get
programs+= test-lp-clock-xfail-set
programs+= test-lp-scene-new
programs+= test-lp-scene-xfail-get
programs+= test-lp-scene-get-quitted
programs+= test-lp-scene-xfail-set
programs+= test-lp-scene-set-quitted
programs+= test-lp-scene-prop-mask
programs+= test-lp-scene-prop-width-height
programs+= test-lp-scene-prop-pattern
programs+= test-lp-scene-prop-wave
programs+= test-lp-scene-prop-ticks
programs+= test-lp-scene-prop-interval
programs+= test-lp-scene-prop-time
programs+= test-lp-scene-prop-lockstep
programs+= test-lp-scene-advance
programs+= test-lp-scene-advance-quitted
programs+= test-lp-scene-quit
programs+= test-lp-scene-quit-quitted
programs+= test-lp-media-new
programs+= test-lp-media-new-xfail-bad-scene
programs+= test-lp-media-new-xfail-bad-uri
programs+= test-lp-media-xfail-get
programs+= test-lp-media-xfail-set
programs+= test-lp-media-start-fail-bad-uri
programs+= test-lp-media-start-fail-no-active-pads
programs+= test-lp-media-start-fail-no-decoder
programs+= test-lp-media-start-avi
programs+= test-lp-media-start-gif
programs+= test-lp-media-start-jpg
programs+= test-lp-media-start-m4v
programs+= test-lp-media-start-mp3
programs+= test-lp-media-start-mp4
programs+= test-lp-media-start-oga
programs+= test-lp-media-start-ogv
programs+= test-lp-media-start-png
programs+= test-lp-media-start-duo
programs+= test-lp-media-start-random
programs+= test-lp-media-stop-broken-audio
programs+= test-lp-media-stop-fail-not-started
programs+= test-lp-media-stop-loop
programs+= test-lp-media-stop-random
programs+= test-lp-media-seek-fail-not-started
programs+= test-lp-media-seek-loop-abs
programs+= test-lp-media-seek-loop-rel
programs+= test-lp-media-seek-duo
programs+= test-lp-media-prop-x-y
programs+= test-lp-media-prop-z
programs+= test-lp-media-prop-width-height
programs+= test-lp-media-prop-alpha
programs+= test-lp-media-prop-mute
programs+= test-lp-media-prop-volume
programs+= test-lp-media-prop-text
programs+= test-lp-media-prop-text-font
programs+= test-lp-media-prop-text-color
check_PROGRAMS= $(programs)

TESTS=\
  $(check_PROGRAMS)\
  $(check_SCRIPTS)\
  $(NULL)

# Tests that should fail.
XFAIL_TESTS=\
  test-lp-clock-xfail-get\
  test-lp-clock-xfail-set\
  test-lp-event-error-xfail-get\
  test-lp-event-error-xfail-set\
  test-lp-event-key-xfail-get\
  test-lp-event-key-xfail-set\
  test-lp-event-pointer-click-xfail-get\
  test-lp-event-pointer-click-xfail-set\
  test-lp-event-pointer-move-xfail-get\
  test-lp-event-pointer-move-xfail-set\
  test-lp-event-quit-xfail-get\
  test-lp-event-quit-xfail-set\
  test-lp-event-seek-xfail-get\
  test-lp-event-seek-xfail-set\
  test-lp-event-start-xfail-get\
  test-lp-event-start-xfail-set\
  test-lp-event-stop-xfail-get\
  test-lp-event-stop-xfail-set\
  test-lp-event-tick-xfail-get\
  test-lp-event-tick-xfail-set\
  test-lp-event-xfail-new\
  test-lp-media-new-xfail-bad-scene\
  test-lp-media-new-xfail-bad-uri\
  test-lp-media-xfail-get\
  test-lp-media-xfail-set\
  test-lp-scene-xfail-get\
  test-lp-scene-xfail-set\
  $(NULL)

EXTRA_DIST=\
  $(check_SCRIPTS)\
  gst.supp\
  suppfile.m4\
  test-samples.h\
  test-templates.h\
  tests.h\
  $(NULL)

CLEANFILES+= *.log *.trs
