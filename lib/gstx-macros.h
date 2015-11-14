/* gstx-macros.h -- GStreamer auxiliary macros.
   Copyright (C) 2015 PUC-Rio/Laboratorio TeleMidia

This file is part of LibPlay.

LibPlay is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

LibPlay is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License
along with LibPlay.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef GSTX_MACROS_H
#define GSTX_MACROS_H

#include "macros.h"

#define GSTX_INCLUDE_PROLOGUE                   \
  PRAGMA_DIAG_IGNORE (-Wbad-function-cast)      \
  PRAGMA_DIAG_PUSH ()                           \
  PRAGMA_DIAG_IGNORE (-Wcast-align)             \
  PRAGMA_DIAG_IGNORE (-Wcast-qual)              \
  PRAGMA_DIAG_IGNORE (-Wconversion)             \
  PRAGMA_DIAG_IGNORE (-Wpedantic)               \
  PRAGMA_DIAG_IGNORE (-Wsign-conversion)        \
  PRAGMA_DIAG_IGNORE (-Wvariadic-macros)

#define GSTX_INCLUDE_EPILOGUE\
  PRAGMA_DIAG_POP ()

/* *INDENT-OFF* */
GSTX_INCLUDE_PROLOGUE
#include <glib.h>
#include <gst/gst.h>
GSTX_INCLUDE_EPILOGUE
/* *INDENT-ON* */

static inline void
gstx_dump_message (GstMessage *message)
{
  const GstStructure *st;

  g_print ("%s (thread: %p): ",
           GST_STR_NULL (GST_ELEMENT_NAME (GST_MESSAGE_SRC (message))),
           (void *) g_thread_self ());

  st = gst_message_get_structure (message);
  if (st == NULL)
    {
      g_print ("(empty)\n\n");
    }
  else
    {
      gchar *s = gst_structure_to_string (st);
      g_print ("%s\n\n", s);
      g_free (s);
    }
}

#endif /* GSTX_MACROS_H */
