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
  lp_Media *media;
  lp_Event *event;

  gdouble volume = G_MAXDOUBLE;

  scene = lp_scene_new (800, 600);
  g_assert_nonnull (scene);
  g_object_set (scene, "pattern", 0, NULL);

  media = lp_media_new (scene, SAMPLE_NIGHT);
  g_assert_nonnull (media);

  g_object_set (media, "text", "100%", "text-font", "sans 40", NULL);
  g_object_get (media, "volume", &volume, NULL);
  g_assert (volume == 1.0);     /* default */

  g_assert (lp_media_start (media));
  event = await_filtered (scene, 1, LP_EVENT_MASK_START);
  g_assert_nonnull (event);
  g_object_unref (event);

  g_object_get (media, "volume", &volume, NULL);
  g_assert (volume == 1.0);
  await_ticks (scene, 1);

  g_object_set (media, "text", "25%", NULL);
  g_object_set (media, "volume", .25, NULL);
  g_object_get (media, "volume", &volume, NULL);
  g_assert (volume == .25);
  await_ticks (scene, 1);

  g_object_set (media, "text", "500%", NULL);
  g_object_set (media, "volume", 5.0, NULL);
  g_object_get (media, "volume", &volume, NULL);
  g_assert (volume == 5.0);
  await_ticks (scene, 1);

  g_object_set (media, "text", "0%", NULL);
  g_object_set (media, "volume", .0, NULL);
  g_object_get (media, "volume", &volume, NULL);
  g_assert (volume == .0);
  await_ticks (scene, 1);

  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
