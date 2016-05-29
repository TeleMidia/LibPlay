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
  guint64 interval;
  gsize i;

  scene = SCENE_NEW (800, 600, 2);
  g_object_get (scene, "interval", &interval, NULL);

  media = lp_media_new (scene, SAMPLE_LEGO);
  g_assert_nonnull (media);

  g_assert (lp_media_start (media));
  event = await_filtered (scene, 1, LP_EVENT_MASK_START);
  g_assert_nonnull (event);
  g_object_unref (event);

  for (i = 0; i < 3; i++)
    {
      await_ticks (scene, 2);
      g_assert (lp_media_seek (media, -2 * interval));
    }

  g_object_unref (scene);
  exit (EXIT_SUCCESS);
}
