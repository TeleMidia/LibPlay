/* lp-common.c -- Common functions
   Copyright (C) 2015-2016 PUC-Rio/Laboratorio TeleMidia

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

#include <config.h>
#include "play-internal.h"

/* Callback called whenever an appsrc needs to push a transparent buffer */
void
_lp_common_appsrc_transparent_data (GstElement *src, guint size, gpointer data)
{
  static GstClockTime timestamp = 0;
  GstBuffer *buffer;
  GstFlowReturn ret;
  GObject *object;
  guint width;
  guint height;

  object = G_OBJECT (data);

  g_object_get (object, 
                "width", &width,
                "height", &height,
                NULL);

  size = width * height * 32;

  buffer = gst_buffer_new_allocate (NULL, size, NULL);

  /* transparent buffer */
  gst_buffer_memset (buffer, 0, 0x0, size);

  GST_BUFFER_PTS (buffer) = timestamp;
  GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 2);

  timestamp += GST_BUFFER_DURATION (buffer);

  g_signal_emit_by_name (src, "push-buffer", buffer, &ret);
  gst_buffer_unref (buffer);
}
