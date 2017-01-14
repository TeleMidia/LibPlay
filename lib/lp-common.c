/* lp-common.c -- Common functions
   Copyright (C) 2015-2017 PUC-Rio/Laboratorio TeleMidia

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

#define DEFAULT_VIDEO_BUFFER_DUR      33 /* ≅ 30 fps */
#define VIDEO_FORMAT_ARGB_SIZE         4 /* 8 bits per channel == 4 bytes */

/* Callback called whenever an appsrc needs to push a transparent buffer */
void
_lp_common_appsrc_transparent_data (GstElement *src, guint size, gpointer data)
{
  static GstClockTime timestamp = 0;
  GstBuffer *buffer;
  GstFlowReturn ret;
  GObject *object;
  GstCaps *caps;
  GstStructure *structure;
  const GValue *framerate;
  guint width;
  guint height;
  guint64 duration;
  gint numerator;
  gint denominator;

  object = G_OBJECT (data);

  g_object_get (object, 
                "width", &width,
                "height", &height,
                NULL);

  g_object_get (src, 
                "caps", &caps,
                NULL);

  g_assert_nonnull (caps);
  structure = gst_caps_get_structure (caps, 0);
  gst_caps_unref (caps);

  framerate = gst_structure_get_value (structure, "framerate");

  if (framerate == NULL || 
      (numerator =  gst_value_get_fraction_numerator (framerate), 
       denominator = gst_value_get_fraction_denominator(framerate),
       numerator == 0 || denominator == 0))
  {
    /* 
     * We consider 30 fps as default framerate. Note this only works 
     * if the appsrc is generating video buffers.
     */
    duration = DEFAULT_VIDEO_BUFFER_DUR * GST_MSECOND; 
  }
  else
    duration = (denominator * 1000 /*ms*/ /  numerator) * GST_MSECOND;
  
  /* We assume the video format ARGB (4 bytes) */
  size = width * height * VIDEO_FORMAT_ARGB_SIZE;

  buffer = gst_buffer_new_allocate (NULL, size, NULL);

  /* transparent buffer */
  gst_buffer_memset (buffer, 0, 0x0, size);

  GST_BUFFER_PTS (buffer) = timestamp;
  GST_BUFFER_DURATION (buffer) = duration;

  timestamp += GST_BUFFER_DURATION (buffer);

  g_signal_emit_by_name (src, "push-buffer", buffer, &ret);
  gst_buffer_unref (buffer);
}
