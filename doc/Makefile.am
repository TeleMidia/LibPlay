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

DOC_MODULE= libplay
DOC_MAIN_SGML_FILE= libplay-docs.xml
DOC_SOURCE_DIR= $(top_srcdir)/lib
CFILE_GLOB= $(top_srcdir)/lib/*.c
HFILE_GLOB= $(top_srcdir)/lib/*.h
IGNORE_HFILES=\
  config.h\
  gstx-macros.h\
  gx-macros.h\
  macros.h\
  play-internal.h\
  $(NULL)

# The CFLAGS and LDFLAGS for compiling scan program.
AM_CPPFLAGS= $(GLIB_CFLAGS) $(GSTREAMER_CFLAGS)
GTKDOC_LIBS= $(top_builddir)/lib/libplay.la

# Extra options to supply to gtkdoc-mkdb.
MKDB_OPTIONS= --output-format=xml --name-space=g\
  --ignore-files='$(MKDB_IGNORE_FILES)'

# Images to copy into HTML directory.
HTML_IMAGES=

content_files=\
  version.xml\
  $(NULL)

include $(top_srcdir)/gtk-doc.make

EXTRA_DIST+= version.xml.in
CLEANFILES+= *.txt *.types *.stamp
DISTCLEANFILES = version.xml
MAINTAINERCLEANFILES= Makefile.in

.PHONY: doc
doc: docs

.PHONY:
doc-clean:
	cd $(srcdir) && rm -rf ./xml ./html ./tmpl

clean-local: doc-clean
