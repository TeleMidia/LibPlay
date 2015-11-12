# Makefile.am -- Template for generating Makefile via Automake.
# Copyright (C) 2015 PUC-Rio/Laboratorio TeleMidia
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

# Auxiliary scripts.
noinst_SCRIPTS=

# Generate env.sh, env.csh, and gdb.cfg.
include $(top_srcdir)/build-aux/Makefile.am.env

# Configure Valgrind and generate suppfile from suppfile.m4.
include $(top_srcdir)/build-aux/Makefile.am.valgrind

# Setup test framework.
AM_COLOR_TESTS= always
DISABLE_HARD_ERRORS= true
TEST_EXTENSIONS= .sh
LOG_COMPILER= $(_VALGRIND)
VALGRIND_OPTIONS+= --show-reachable=yes --errors-for-leak-kinds=all
SH_LOG_COMPILER= $(SHELL)

AM_TESTS_ENVIRONMENT=\
  export G_DEBUG="all";\
  export G_SLICE="always-malloc";\
  export LD_LIBRARY_PATH="$(top_builddir)/lib/.libs";\
  export LD_PRELOAD="libplay$(SOEXT)";\
  export MALLOC_CHECK_=1;\
  $(NULL)

# Test scripts.
scripts=
check_SCRIPTS= $(scripts)

# Test programs.
programs=
# _lp-util
programs+= test-_lp-util-g-value-alloc
programs+= test-_lp-util-g-value-free
programs+= test-_lp-util-g-value-dup
# _lp-properties
programs+= test-_lp-properties-alloc
programs+= test-_lp-properties-free
programs+= test-_lp-properties-size
programs+= test-_lp-properties-get
programs+= test-_lp-properties-set
programs+= test-_lp-properties-reset
programs+= test-_lp-properties-reset-all
# lp-status
programs+= test-lp-status-to-string
# lp-event
programs+= test-lp-event-init-start
programs+= test-lp-event-init-stop
programs+= test-lp-event-equals
# lp-media
programs+= test-lp-media-create
programs+= test-lp-media-create-for-parent
programs+= test-lp-media-destroy
programs+= test-lp-media-status
programs+= test-lp-media-reference
programs+= test-lp-media-get-reference-count
# programs+= test-lp-media-register-unregister
# programs+= test-lp-media-set-get-property-double
# programs+= test-lp-media-set-get-property-int
# programs+= test-lp-media-set-get-property-pointer
# programs+= test-lp-media-set-get-property-string
check_PROGRAMS= $(programs)

TESTS=\
  $(check_SCRIPTS)\
  $(check_PROGRAMS)\
  $(NULL)

# Tests that should fail.
XFAIL_TESTS=\
  $(NULL)

EXTRA_DIST=\
  suppfile.m4\
  tests.h\
  $(NULL)
