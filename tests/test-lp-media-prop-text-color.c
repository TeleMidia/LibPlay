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

  guint color = 0;

  scene = SCENE_NEW (800, 600, 0);
  media = lp_media_new (scene, SAMPLE_GNU);
  g_assert_nonnull (media);

  g_object_set (media,
                "text", "Nullius in verba",
                "text-font", "mono 60", NULL);

  g_object_get (media, "text-color", &color, NULL);
  g_assert (color == 0xffffffff); /* default */

  g_assert (lp_media_start (media));
  event = await_filtered (scene, 1, LP_EVENT_MASK_START);
  g_assert_nonnull (event);
  g_object_unref (event);

  g_object_get (media, "text-color", &color, NULL);
  g_assert (color == 0xffffffff);
  await_ticks (scene, 1);

  g_object_set (media, "text-color", 0xffff0000, NULL);
  g_object_get (media, "text-color", &color, NULL);
  g_assert (color == 0xffff0000);
  await_ticks (scene, 1);

  g_object_set (media, "text-color", 0x4400ff00, NULL);
  g_object_get (media, "text-color", &color, NULL);
  g_assert (color == 0x4400ff00);
  await_ticks (scene, 1);

  g_object_set (media, "text-color", 0, NULL);
  g_object_get (media, "text-color", &color, NULL);
  g_assert (color == 0);
  await_ticks (scene, 1);

  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
