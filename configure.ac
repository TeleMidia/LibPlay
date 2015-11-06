# configure.ac -- Configure template for LibPlay.
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

AC_PREREQ([2.62])
AU_GIT_VERSION_GEN([libplay])
AC_INIT([LibPlay],
  libplay_version_string,
 [bugs@telemidia.puc-rio.br],
 [libplay],
 [http://github.com/TeleMidia/LibPlay])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([build-aux])
AC_USE_SYSTEM_EXTENSIONS
AC_CONFIG_SRCDIR([lib/play.h])
AC_CONFIG_HEADERS([lib/config.h])
AU_GITLOG_TO_CHANGELOG([2015-11-05])

AM_INIT_AUTOMAKE([1.14 -Wall -Werror dist-xz gnu no-dist-gzip])
AM_SILENT_RULES([yes])
AM_PROG_AR

LT_PREREQ([2.2])
LT_INIT([win32-dll])
AU_LIBTOOL_MODULE_LDFLAGS

# LibPlay (package) version.
AC_SUBST([LIBPLAY_VERSION_MAJOR], libplay_version_major)
AC_SUBST([LIBPLAY_VERSION_MINOR], libplay_version_minor)
AC_SUBST([LIBPLAY_VERSION_MICRO], libplay_version_micro)
AC_SUBST([LIBPLAY_VERSION_STRING], libplay_version_string)

# LibPlay (library) version.
# - library code modified:            REVISION++
# - interfaces changed/added/removed: REVISION=0, CURRENT++
# - interfaces added:                 AGE++
# - interfaces removed:               AGE=0
AU_LIBTOOL_VERSION([LIBPLAY], [0], [0], [0])

# Dependencies version.
m4_define([glib_required_version], [2.32.4])
m4_define([gstreamer_required_version], [1.6.0])
AC_SUBST([GLIB_REQUIRED_VERSION], glib_required_version)
AC_SUBST([GSTREAMER_REQUIRED_VERSION], gstreamer_required_version)

# Tool checks.
AU_PROG_CC_VISIBILITY
AU_PROG_PKG_CONFIG
AU_PROG_UNIX_TOOLS

# System checks.
AU_SYSTEM
AU_SYSTEM_MINGW

# Configure options.
AU_ARG_ENABLE_ANSI
AU_ARG_ENABLE_COVERAGE
AU_ARG_ENABLE_DEBUG
AU_ARG_ENABLE_VALGRIND
AU_ARG_ENABLE_WARNINGS

# Library functions.
AC_CHECK_LIBM
AU_CHECK_MACROS_H

# Helper macro: Breaks version into major, minor, and micro.
m4_define([_my_break_version],[dnl
AC_DEFINE(m4_toupper([$1])[_REQUIRED_]m4_toupper([$3]),
  m4_default(au_version_$3([$2]), [0]),
 [$1 required $3 version])])
m4_define([my_break_version],[dnl
_my_break_version([$1], [$2], [major])
_my_break_version([$1], [$2], [minor])
_my_break_version([$1], [$2], [micro])])

# Check for GLib.
my_break_version([glib], glib_required_version)
AU_CHECK_PKG([GLIB],
  glib-2.0 >= glib_required_version,
 [AC_LANG_PROGRAM([[
#include <glib.h>
#if !GLIB_CHECK_VERSION  \
    (GLIB_REQUIRED_MAJOR,\
     GLIB_REQUIRED_MINOR,\
     GLIB_REQUIRED_MICRO)
# error "glib is too old"
#endif
 ]])],
 [AC_LANG_PROGRAM([[]], [[glib_check_version (0, 0, 0);]])])

# Check for GStreamer (optional).
my_break_version([gstreamer], gstreamer_required_version)
AU_CHECK_PKG([GSTREAMER],
  gstreamer-1.0 >= gstreamer_required_version
  gstreamer-video-1.0 >= gstreamer_required_version,
 [AC_LANG_PROGRAM([[
#include <gst/gst.h>
#if !GST_CHECK_VERSION  \
    (GST_REQUIRED_MAJOR,\
     GST_REQUIRED_MINOR,\
     GST_REQUIRED_MICRO)
# error "gstreamer is too old"
#endif
 ]])],
 [AC_LANG_PROGRAM([[]], [[
gst_init (0, 0);
gst_video_info_from_caps (0, 0);
 ]])])

AC_CONFIG_FILES([
Makefile
lib/Makefile
lib/play.pc
lib/playconf.h
tests/Makefile
])

AC_OUTPUT
AC_MSG_NOTICE([summary of main build options:

  Version:            ${VERSION}
  Host type:          ${host}
  Install prefix:     ${prefix}
  Compiler:           ${CC}
  Cflags:             ${CFLAGS}
  Cppflags:           ${CPPFLAGS}
  Warning flags:      ${WERROR_CFLAGS} ${WARN_CFLAGS}
  Library types:      Shared=${enable_shared}, Static=${enable_static}
  Valgrind:           ${VALGRIND}
])
