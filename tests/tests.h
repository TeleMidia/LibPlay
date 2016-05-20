/* tests.h -- Common declarations for tests.
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

#ifndef TESTS_H
#define TESTS_H

#include <config.h>
#include "macros.h"
#include "gx-macros.h"
#include "gstx-macros.h"

GX_INCLUDE_PROLOGUE
#include "play.h"
#include "play-internal.h"
GX_INCLUDE_EPILOGUE

PRAGMA_DIAG_IGNORE (-Wfloat-equal)
PRAGMA_DIAG_IGNORE (-Winline)

/* Expands to the path of the given sample file.  */
#define SAMPLES_DIR(file) TOP_SRCDIR"/tests/samples/"G_STRINGIFY (file)

/* Audio samples. */
#define SAMPLE_ARCADE     SAMPLES_DIR (arcade.mp3)
#define SAMPLE_EVILEYE    SAMPLES_DIR (evileye.mp3)

/* Image samples.  */
#define SAMPLE_EARTH      SAMPLES_DIR (earth.gif)
#define SAMPLE_FELIS      SAMPLES_DIR (felis.jpg)
#define SAMPLE_GNU        SAMPLES_DIR (gnu.png)

/* Video samples.  */
#define SAMPLE_DIODE      SAMPLES_DIR (diode.mp4)
#define SAMPLE_NIGHT      SAMPLES_DIR (night.avi)
#define SAMPLE_ROAD       SAMPLES_DIR (road.ogv)
#define SAMPLE_SYNC       SAMPLES_DIR (sync.m4v)

/* All samples.  */
static ATTR_UNUSED const gchar *samples_all[] =
{
  SAMPLE_ARCADE,
  SAMPLE_EVILEYE,
  SAMPLE_EARTH,
  SAMPLE_FELIS,
  SAMPLE_GNU,
  SAMPLE_DIODE,
  SAMPLE_NIGHT,
  SAMPLE_ROAD,
  SAMPLE_SYNC
};

/* By format.  */
#define SAMPLE_AVI SAMPLE_NIGHT
#define SAMPLE_GIF SAMPLE_EARTH
#define SAMPLE_JPG SAMPLE_FELIS
#define SAMPLE_M4V SAMPLE_SYNC
#define SAMPLE_MP3 SAMPLE_ARCADE
#define SAMPLE_MP4 SAMPLE_DIODE
#define SAMPLE_OGV SAMPLE_ROAD
#define SAMPLE_PNG SAMPLE_GNU

/* Random sample.  */
#define random_sample()\
  samples_all[g_random_int_range (0, nelementsof (samples_all))]

/* Sleeps for @n seconds.  */
#define SLEEP(n) g_usleep ((n) * 1000000)

/* Waits for @n events that matches @mask in @scene.
   Returns the last matched event.  */

static ATTR_UNUSED lp_Event *
await_filtered (lp_Scene *scene, guint n, guint mask)
{
  lp_Event *event;

  if (unlikely (n == 0))
    return NULL;

  for (;;)
    {
      event = lp_scene_receive (scene, TRUE);
      g_assert_nonnull (event);

      if ((lp_event_get_mask (event) & mask) && --n == 0)
        break;

      g_object_unref (event);
    }
  return event;
}

/* Waits for @n ticks in @scene.  */

static ATTR_UNUSED void
await_ticks (lp_Scene *scene, guint n)
{
  lp_Event *event;

  if (n == 0)
    return;

  event = await_filtered (scene, n, LP_EVENT_MASK_TICK
                          | LP_EVENT_MASK_ERROR);
  g_assert_nonnull (event);
  g_assert (LP_IS_EVENT_TICK (event));
  g_object_unref (event);
}

/* Sends a key to @scene.  */

static ATTR_UNUSED void
send_key (lp_Scene *scene, const gchar *key, gboolean press)
{
  GstElement *sink;
  sink = _lp_scene_get_real_video_sink (scene);
  g_assert_nonnull (sink);
  gst_navigation_send_key_event (GST_NAVIGATION (sink),
                                 press ? "key-press" : "key-release", key);
  gst_object_unref (sink);
}

/* Send a pointer click to @scene.  */

static ATTR_UNUSED void
send_pointer_click (lp_Scene *scene, gint button, gdouble x, gdouble y,
                    gboolean press)
{
  GstElement *sink = _lp_scene_get_real_video_sink (scene);
  g_assert_nonnull (sink);
  gst_navigation_send_mouse_event (GST_NAVIGATION (sink),
                                   press
                                   ? "mouse-button-press"
                                   : "mouse-button-release",
                                   button, x, y);
  gst_object_unref (sink);
}

/* Send a pointer move to @scene.  */

static ATTR_UNUSED void
send_pointer_move (lp_Scene *scene, gdouble x, gdouble y)
{
  GstElement *sink = _lp_scene_get_real_video_sink (scene);
  g_assert_nonnull (sink);
  gst_navigation_send_mouse_event (GST_NAVIGATION (sink),
                                   "mouse-move", 0, x, y);
  gst_object_unref (sink);
}

#endif /* TESTS_H */
