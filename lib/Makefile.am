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

AM_CFLAGS= $(WERROR_CFLAGS) $(WARN_CFLAGS)

EXTRA_DIST= play.pc.in ncluaconf.h.in
DISTCLEANFILES+= play.pc ncluaconf.h

pkgconfigdir= $(libdir)/pkgconfig
pkgconfig_DATA= play.pc

include_HEADERS= play.h playconf.h
noinst_HEADERS= macros.h
lib_LTLIBRARIES= libplay.la

libplay_la_DEPENDENCIES= $(include_HEADERS) $(noinst_HEADERS)
libplay_la_CPPFLAGS= -DLP_BUILDING
libplay_la_CFLAGS= $(AM_CFLAGS) $(CFLAGS_VISIBILITY)\
  $(GLIB_CFLAGS) $(GSTREAMER_CFLAGS)
libplay_la_LDFLAGS= -version-info $(LIBPLAY_LIBTOOL_STRING) -no-undefined\
  $(GLIB_CFLAGS) $(GSTREAMER_LIBS)
libplay_la_SOURCES=\
  event.c\
  macros.h\
  media.c\
  play.h\
  playconf.h\
  $(NULL)
