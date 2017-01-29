/* play-internal.h -- Internal declarations.
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

#ifndef PLAY_INTERNAL_H
#define PLAY_INTERNAL_H

#include <stdarg.h>

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

#define _lp_critical(fmt, ...)\
  g_critical (G_STRLOC ": " fmt, ## __VA_ARGS__)

#define _lp_error(fmt, ...)\
  g_error (G_STRLOC ": " fmt, ## __VA_ARGS__)

#define _lp_warn(fmt, ...)\
  g_warning (G_STRLOC ": " fmt, ## __VA_ARGS__)

#define _lp_eltmap_alloc_check(obj, map)                                \
  STMT_BEGIN                                                            \
  {                                                                     \
    const gchar *__missing;                                             \
    if (unlikely (!gstx_eltmap_alloc ((obj), (map), &__missing)))       \
      _lp_error ("missing GStreamer plugin: %s", __missing);            \
  }                                                                     \
  STMT_END

/* clock */

#define LP_TYPE_CLOCK (lp_clock_get_type ())
GX_DECLARE_FINAL_TYPE (lp_Clock, lp_clock, LP, CLOCK, GstSystemClock)

gboolean
_lp_clock_advance (lp_Clock *, GstClockTime);

/* event */

gchar *
_lp_event_to_string (lp_Event *, const gchar *fmt, ...);

lp_EventQuit *
_lp_event_quit_new (lp_Scene *);

lp_EventTick *
_lp_event_tick_new (lp_Scene *, guint64);

lp_EventError *
_lp_event_error_new (GObject *, lp_Error, const gchar *, ...);

lp_EventError *
_lp_event_error_new_custom (GObject *, GError *);

#define _lp_event_error_new_start_no_pads(media)\
  _lp_event_error_new ((media), LP_ERROR_START, "could not activate pads")

lp_EventKey *
_lp_event_key_new (lp_Scene *, const gchar *, gboolean);

lp_EventPointerClick *
_lp_event_pointer_click_new (GObject *, double, double, int, gboolean);

lp_EventPointerMove *
_lp_event_pointer_move_new (lp_Scene *, double, double);

lp_EventStart *
_lp_event_start_new (lp_Media *, gboolean);

lp_EventStop *
_lp_event_stop_new (lp_Media *, gboolean);

lp_EventSeek *
_lp_event_seek_new (lp_Media *, gboolean, gint64);

lp_EventPause *
_lp_event_pause_new (GObject *);
/* media */

lp_Media *
_lp_media_find_media (GstObject *);

void
_lp_media_finish_error (lp_Media *);

void
_lp_media_finish_start (lp_Media *);

void
_lp_media_finish_stop (lp_Media *);

void
_lp_media_finish_pause (lp_Media *);

void
_lp_media_finish_seek (lp_Media *);

void
_lp_media_finish_resume (lp_Media *);
/* scene */

GstElement *
_lp_scene_get_pipeline (lp_Scene *);

GstElement *
_lp_scene_get_audio_mixer (lp_Scene *);

GstElement *
_lp_scene_get_video_mixer (lp_Scene *);

GstElement *
_lp_scene_get_real_audio_sink (lp_Scene *); /* transfer-full */

GstElement *
_lp_scene_get_real_video_sink (lp_Scene *); /* transfer-full */

GstClockTime
_lp_scene_get_running_time (lp_Scene *);

GstClockTime
_lp_scene_get_start_time (lp_Scene *);

gboolean
_lp_scene_has_video (lp_Scene *);

void
_lp_scene_add_media (lp_Scene *, lp_Media *);

void
_lp_scene_step (lp_Scene *, gboolean);

void
_lp_scene_dispatch (lp_Scene *, lp_Event *);

gboolean
_lp_scene_is_paused (lp_Scene *);

/* common */
void
_lp_common_appsrc_transparent_data (GstElement *, guint, gpointer);

#endif /* PLAY_INTERNAL_H */
