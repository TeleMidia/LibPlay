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

AM_CFLAGS= $(WERROR_CFLAGS) $(WARN_CFLAGS)

EXTRA_DIST= play.pc.in playconf.h.in
DISTCLEANFILES+= play.pc playconf.h

pkgconfigdir= $(libdir)/pkgconfig
pkgconfig_DATA= play.pc

include_HEADERS= play.h playconf.h
noinst_HEADERS= macros.h
lib_LTLIBRARIES= libplay.la

LP_BUILDING= -DLP_BUILDING
if HAVE_VISIBILITY
LP_BUILDING+= -DLP_HAVE_VISIBILITY
endif

libplay_la_DEPENDENCIES= $(include_HEADERS) $(noinst_HEADERS)
libplay_la_CPPFLAGS= $(LP_BUILDING)
libplay_la_CFLAGS= $(AM_CFLAGS) $(CFLAG_VISIBILITY)\
  $(GLIB_CFLAGS) $(GSTREAMER_CFLAGS)
libplay_la_LDFLAGS= -version-info $(LIBPLAY_LIBTOOL_STRING) -no-undefined\
  $(GLIB_LIBS) $(GSTREAMER_LIBS) -fPIC
libplay_la_SOURCES=\
  gstx-macros.h\
  gx-macros.h\
  lp-clock.c\
  lp-error.c\
  lp-event-error.c\
  lp-event-key.c\
  lp-event-pointer-click.c\
  lp-event-pointer-move.c\
  lp-event-start.c\
  lp-event-stop.c\
  lp-event-tick.c\
  lp-event.c\
  lp-media.c\
  lp-scene.c\
  lp-version.c\
  play.h\
  playconf.h\
  $(NULL)
