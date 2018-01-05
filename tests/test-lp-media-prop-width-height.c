/* Copyright (C) 2015-2018 PUC-Rio/Laboratorio TeleMidia

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

  gint width = G_MAXINT;
  gint height = G_MAXINT;

  scene = SCENE_NEW (800, 600, 0);
  media = lp_media_new (scene, SAMPLE_GNU);
  g_assert_nonnull (media);

  g_object_set (media, "text", "0x0", "text-font", "sans 40", NULL);
  g_object_get (media, "width", &width, "height", &height, NULL);
  g_assert (width == 0 && height == 0);  /* default */

  g_assert (lp_media_start (media));
  event = await_filtered (scene, 1, LP_EVENT_MASK_START);
  g_assert_nonnull (event);
  g_object_unref (event);

  g_object_get (media, "width", &width, "height", &height, NULL);
  g_assert (width == 0 && height == 0);
  await_ticks (scene, 1);

  g_object_set (media, "text", "200x200", NULL);
  g_object_set (media, "width", 200, "height", 200, NULL);
  g_object_get (media, "width", &width, "height", &height, NULL);
  g_assert (width == 200 && height == 200);
  await_ticks (scene, 1);

  g_object_set (media, "text", "600x600", NULL);
  g_object_set (media, "width", 600, "height", 600, NULL);
  g_object_get (media, "width", &width, "height", &height, NULL);
  g_assert (width == 600 && height == 600);
  await_ticks (scene, 1);

  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
