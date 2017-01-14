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
  lp_Media *media;
  lp_Event *event;

  gchar *font = NULL;

  scene = SCENE_NEW (800, 600, 0);
  media = lp_media_new (scene, SAMPLE_GNU);
  g_assert_nonnull (media);

  g_object_set (media, "text", "Nullius in verba", NULL);

  g_object_get (media, "text-font", &font, NULL);
  g_assert (font == NULL);      /* default */

  g_assert (lp_media_start (media));
  event = await_filtered (scene, 1, LP_EVENT_MASK_START);
  g_assert_nonnull (event);
  g_object_unref (event);

  g_object_get (media, "text-font", &font, NULL);
  g_assert (font == NULL);
  await_ticks (scene, 1);

  g_object_set (media, "text-font", "sans 80", NULL);
  g_object_get (media, "text-font", &font, NULL);
  g_assert (g_str_equal (font, "sans 80"));
  g_free (font);
  await_ticks (scene, 1);

  g_object_set (media, "text-font", "serif bold 40", NULL);
  g_object_get (media, "text-font", &font, NULL);
  g_assert (g_str_equal (font, "serif bold 40"));
  g_free (font);
  await_ticks (scene, 1);

  g_object_set (media, "text-font", NULL, NULL);
  g_object_get (media, "text-font", &font, NULL);
  g_assert (font == NULL);
  await_ticks (scene, 1);

  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
