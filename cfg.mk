# cfg.mk -- Setup maintainer's makefile.
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

COPYRIGHT_YEAR= 2015
COPYRIGHT_HOLDER= PUC-Rio/Laboratorio TeleMidia

INDENT_OPTIONS=\
  --brace-indent0\
  --case-brace-indentation0\
  --case-indentation2\
  --else-endif-column0\
  --gnu-style\
  --indent-label-1\
  --leave-preprocessor-space\
  --no-tabs\
  -l76\
  $(NULL)

INDENT_EXCLUDE=\
  lib/gstx-macros.h\
  lib/play-internal.h\
  lib/play.h\
  lua/luax-macros.h\
  tests/tests.h\
  $(NULL)

INDENT_JOIN_EMPTY_LINES_EXCLUDE=\
  $(NULL)

INDENT_TYPES=\
  GValue\
  GstBus\
  GstClock\
  GstMessage\
  GstPad\
  lp_event_t\
  lp_media_backend_t\
  lp_media_gst_t\
  lp_media_t\
  lp_properties_desc_t\
  lp_properties_t\
  lp_value_t\
  $(NULL)

SC_USELESS_IF_BEFORE_FREE_ALIASES=\
  _lp_properties_free\
  _lp_util_g_value_free\
  g_free\
  g_hash_table_destroy\
  g_list_free\
  lp_media_destroy\
  $(NULL)

SYNTAX_CHECK_EXCLUDE=\
  $(NULL)

SC_COPYRIGHT_EXCLUDE=\
  build-aux/Makefile.am.common\
  build-aux/Makefile.am.coverage\
  build-aux/Makefile.am.env\
  build-aux/Makefile.am.gitlog\
  build-aux/Makefile.am.link\
  build-aux/Makefile.am.valgrind\
  lib/macros.h\
  lua/luax-macros.h\
  maint.mk\
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
NCLUA_FILES+= lib/macros.h
NCLUA_FILES+= lib/luax-macros.h
NCLUA_FILES+= maint.mk
NCLUA_SCRIPTS+= bootstrap
NCLUA_SCRIPTS+= build-aux/syntax-check
NCLUA_SCRIPTS+= build-aux/syntax-check-copyright
REMOTE_FILES+= $(NCLUA_FILES)
REMOTE_SCRIPTS+= $(NCLUA_SCRIPTS)
fetch-remote-local:
	$(V_at)for path in $(NCLUA_FILES) $(NCLUA_SCRIPTS); do\
	  if test "$$path" = "lib/luax-macros.h"; then\
	    dir=lua;\
	  else\
	    dir=`dirname "$$path"`;\
	  fi;\
	  $(FETCH) -dir="$$dir" "$(nclua)/$$path" || exit 1;\
	done
