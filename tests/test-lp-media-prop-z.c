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
  lp_Media *m1;
  lp_Media *m2;
  lp_Event *event;

  gint z1 = G_MAXINT;
  gint z2 = G_MAXINT;

  scene = lp_scene_new (800, 600);
  g_assert_nonnull (scene);

  m1 = lp_media_new (scene, SAMPLE_GNU);
  g_assert_nonnull (m1);

  m2 = lp_media_new (scene, SAMPLE_EARTH);
  g_assert_nonnull (m2);

  g_object_get (m1, "z", &z1, NULL);
  g_assert (z1 == 1);           /* default */

  g_object_get (m2, "z", &z2, NULL);
  g_assert (z2 == 1);           /* default */

  g_object_set (m1, "text", "0", "text-font", "sans 40", NULL);
  g_object_set (m2, "text", "0", "text-font", "sans 40",
                "y", 100, NULL);

  g_assert (lp_media_start (m1));
  g_assert (lp_media_start (m2));
  event = await_filtered (scene, 2, LP_EVENT_MASK_START);
  g_assert_nonnull (event);
  g_object_unref (event);

  await_ticks (scene, 1);

  g_object_set (m1, "text", "2", "text-font", "sans 40", NULL);
  g_object_set (m1, "z", 2, NULL);
  await_ticks (scene, 1);

  g_object_set (m2, "text", "100", "text-font", "sans 40", NULL);
  g_object_set (m2, "z", 100, NULL);
  await_ticks (scene, 1);

  g_object_set (m1, "text", "0", "text-font", "sans 40", NULL);
  g_object_set (m1, "z", 0, NULL);
  await_ticks (scene, 1);

  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
