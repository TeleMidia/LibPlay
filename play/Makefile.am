# Makefile.am -- Template for generating Makefile via Automake.
# Copyright (C) 2015-2018 PUC-Rio/Laboratorio TeleMidia
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

AM_CPPFLAGS= -I$(top_srcdir)/lib -I$(top_builddir)/lib
AM_CFLAGS= $(WERROR_CFLAGS) $(WARN_CFLAGS) $(GLIB_CFLAGS) $(LUA_CFLAGS)
AM_LDFLAGS= $(GLIB_LIBS) $(LUA_LIBS) -module $(LT_MODULE_LDFLAGS)

luaplaylib_LTLIBRARIES= play0.la
play0_la_LIBADD= $(top_builddir)/lib/libplay.la
play0_la_SOURCES=\
  luax-macros.h\
  play0.c\
  $(NULL)
