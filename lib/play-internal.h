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
#include "gstx-macros.h"

#include "play.h"

/* debugging */
#if defined DEBUG && DEBUG
# define _lp_debug(fmt, ...)                                    \
  STMT_BEGIN                                                    \
  {                                                             \
    g_print ("%s:%d:%s (thread %p): " fmt "\n",                 \
             __FILE__, __LINE__, __FUNCTION__,                  \
             (void *) g_thread_self (), ## __VA_ARGS__);        \
  }                                                             \
  STMT_END

# define _lp_debug_dump_gst_message(msg)                                \
  STMT_BEGIN                                                            \
  {                                                                     \
    const GstStructure *_st = gst_message_get_structure ((msg));        \
    if (_st != NULL)                                                    \
      {                                                                 \
        gchar *_str = gst_structure_to_string (_st);                    \
        _lp_debug ("%s", _str);                                         \
        g_free (_str);                                                  \
      }                                                                 \
  }                                                                     \
  STMT_END
#else
# define _lp_debug(tag, fmt, ...)         /* nothing */
# define _lp_debug_dump_message(tag, msg) /* nothing */
#endif

/* errors and checks */
#define _lp_error(fmt, ...)\
  g_error (G_STRLOC ": " fmt, ## __VA_ARGS__)

#define _lp_warn(fmt, ...)\
  g_warning (G_STRLOC ": " fmt, ## __VA_ARGS__)

#define _lp_eltmap_alloc_check(obj, map)                                \
  STMT_BEGIN                                                            \
  {                                                                     \
    const char *missing;                                                \
    if (unlikely (!gstx_eltmap_alloc ((obj), (map), &missing)))         \
      g_error (G_STRLOC ": missing GStreamer plugin: %s", missing);     \
  }                                                                     \
  STMT_END

/* scene */
GstElement *
_lp_scene_get_pipeline (const lp_Scene *);

GstElement *
_lp_scene_get_audio_mixer (const lp_Scene *);

GstElement *
_lp_scene_get_video_mixer (const lp_Scene *);

gboolean
_lp_scene_has_video (const lp_Scene *);

void
_lp_scene_add_media (lp_Scene *, lp_Media *);

void
_lp_scene_dispatch (lp_Scene *, GObject *, lp_Event);

void
_lp_scene_step (lp_Scene *, gboolean);

/* media */
gboolean
_lp_media_is_starting (lp_Media *);

gboolean
_lp_media_has_started (lp_Media *);

gboolean
_lp_media_is_stopping (lp_Media *);

gboolean
_lp_media_has_stopped (lp_Media *);

gboolean
_lp_media_has_drained (lp_Media *);

void
_lp_media_finish_start (lp_Media *);

void
_lp_media_finish_stop (lp_Media *);

#endif /* PLAY_INTERNAL_H */
