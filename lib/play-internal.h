/* play-internal.h -- Internal declarations.
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

#ifndef PLAY_INTERNAL_H
#define PLAY_INTERNAL_H

#include "gstx-macros.h"

/* checks */
#define _lp_scene_check(scene)                                          \
  STMT_BEGIN                                                            \
  {                                                                     \
    if (unlikely (scene == NULL))                                       \
      g_error (G_STRLOC ": bad scene: %s", (scene));                    \
  }                                                                     \
  STMT_END

#define _lp_eltmap_alloc_check(obj, map)                                \
  STMT_BEGIN                                                            \
  {                                                                     \
    const char *missing;                                                \
    if (unlikely (!gstx_eltmap_alloc ((obj), (map), &missing)))         \
      g_error (G_STRLOC": missing GStreamer plugin: %s", missing);      \
  }                                                                     \
  STMT_END

/* scene */
gboolean
_lp_scene_add (lp_Scene *, lp_Media *);

GstElement *
_lp_scene_pipeline (lp_Scene *);

gboolean
_lp_scene_has_video(lp_Scene *);

#endif /* PLAY_INTERNAL_H */
