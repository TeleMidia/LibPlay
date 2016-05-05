/* play-internal.h -- Internal declarations.
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

#ifndef PLAY_INTERNAL_H
#define PLAY_INTERNAL_H

#include "macros.h"
#include "gx-macros.h"
#include "gstx-macros.h"

GX_INCLUDE_PROLOGUE
#include "play.h"
GX_INCLUDE_EPILOGUE

/* debugging */

#if defined DEBUG && DEBUG
# define _lp_debug(fmt, ...)                            \
  g_print (G_STRLOC " [thread %p] " fmt "\n",           \
           (void *) g_thread_self (), ## __VA_ARGS__)

# define _lp_debug_dump_gst_message(msg)                                \
  STMT_BEGIN                                                            \
  {                                                                     \
    const GstStructure *__st = gst_message_get_structure ((msg));       \
    if (likely (__st != NULL))                                          \
      {                                                                 \
        gchar *__str = gst_structure_to_string (__st);                  \
        _lp_debug ("%s", __str);                                        \
        g_free (__str);                                                 \
      }                                                                 \
  }                                                                     \
  STMT_END
#else
# define _lp_debug_init()                /* nothing */
# define _lp_debug(fmt, ...)             /* nothing */
# define _lp_debug_dump_gst_message(msg) /* nothing */
#endif

/* checks */

#define _lp_error(fmt, ...)\
  g_error (G_STRLOC ": " fmt, ## __VA_ARGS__)

#define _lp_warn(fmt, ...)\
  g_warning (G_STRLOC ": " fmt, ## __VA_ARGS__)

#define _lp_eltmap_alloc_check(obj, map)                                \
  STMT_BEGIN                                                            \
  {                                                                     \
    const gchar *__missing;                                             \
    if (unlikely (!gstx_eltmap_alloc ((obj), (map), &__missing)))       \
      g_error (G_STRLOC ": missing GStreamer plugin: %s", __missing);   \
  }                                                                     \
  STMT_END

/* clock */

#define LP_TYPE_CLOCK (lp_clock_get_type ())
GX_DECLARE_FINAL_TYPE (lp_Clock, lp_clock, LP, CLOCK, GstSystemClock)

gboolean
_lp_clock_advance (lp_Clock *, GstClockTime);

/* scene */

GstElement *
_lp_scene_get_pipeline (const lp_Scene *);

GstElement *
_lp_scene_get_audio_mixer (const lp_Scene *);

GstElement *
_lp_scene_get_video_mixer (const lp_Scene *);

GstClockTime
_lp_scene_get_clock_time (const lp_Scene *);

gboolean
_lp_scene_has_video (const lp_Scene *);

void
_lp_scene_add_media (lp_Scene *, lp_Media *);

void
_lp_scene_step (lp_Scene *, gboolean);

void
_lp_scene_dispatch (lp_Scene *, lp_Event *);

/* media */

void
_lp_media_finish_start (lp_Media *);

void
_lp_media_finish_stop (lp_Media *);

/* event */
lp_EventTick *
lp_event_tick_new (lp_Scene *, guint64);

lp_EventError *
lp_event_error_new (lp_Media *, GError *);

lp_EventStart *
lp_event_start_new (lp_Media *, gboolean);

lp_EventStop *
lp_event_stop_new (lp_Media *, gboolean);

lp_EventMouseButton *
lp_event_mouse_button_new (lp_Scene *, double, double, int,
                           gboolean);

lp_EventKey *
lp_event_key_new (lp_Scene *, const char *key, gboolean);

guint64
lp_event_tick_get_serial (lp_EventTick *);

GError *
lp_event_error_get_error (lp_EventError *);

gboolean
lp_event_start_is_resume (lp_EventStart *);

gboolean
lp_event_stop_is_eos (lp_EventStop *);

#endif /* PLAY_INTERNAL_H */
