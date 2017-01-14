/* Copyright (C) 2015-2017 PUC-Rio/Laboratorio TeleMidia

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
  lp_Media *media[10];
  gint w, h;
  gsize i;

  w = 800;
  h = 600;
  scene = SCENE_NEW (w, h, 0);
  for (i = 0; i < nelementsof (media); i++)
    {
      media[i] = LP_MEDIA
        (g_object_new (LP_TYPE_MEDIA,
                       "scene", scene,
                       "uri", random_sample (),
                       "x", g_random_int_range (0, w),
                       "y", g_random_int_range (0, h),
                       "z", g_random_int_range (1, 10),
                       "width", g_random_int_range (0, w),
                       "height", g_random_int_range (0, h),
                       "alpha", g_random_double_range (0., 1.),
                       "mute", g_random_boolean (),
                       "volume", g_random_double_range (0., 2.),
                       NULL));
      g_assert_nonnull (media[i]);
      g_assert (lp_media_start (media[i]));
    }

  await_ticks (scene, 5);

  g_timeout_add_seconds (2, (GSourceFunc) abort, NULL);
  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
