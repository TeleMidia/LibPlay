/* Copyright (C) 2015-2016 PUC-Rio/Laboratorio TeleMidia

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

#include "tests.h"

int
main (void)
{
  lp_Scene *scene;
  lp_Media *media[2];
  gint w = 800;
  gint h = 600;

  scene = SCENE_NEW (w, h, 0);
  media[0] = LP_MEDIA
    (g_object_new (LP_TYPE_MEDIA,
                   "scene", scene,
                   "uri", SAMPLE_AVI,
                   "x", g_random_int_range (0, w),
                   "y", g_random_int_range (0, h),
                   "z", g_random_int_range (1, 10),
                   NULL));
  g_assert_nonnull (media[0]);
  g_assert (lp_media_start (media[0]));

  media[0] = LP_MEDIA
    (g_object_new (LP_TYPE_MEDIA,
                   "scene", scene,
                   "uri", SAMPLE_PNG,
                   "x", g_random_int_range (0, w),
                   "y", g_random_int_range (0, h),
                   "z", g_random_int_range (1, 10),
                   NULL));
  g_assert_nonnull (media[0]);
  g_assert (lp_media_start (media[0]));

  await_ticks (scene, 8);
  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
