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
  lp_Event *event;
  guint64 interval;
  gsize i;

  scene = SCENE_NEW (800, 600, 2);
  g_object_get (scene, "interval", &interval, NULL);

  media[0] = lp_media_new (scene, SAMPLE_SYNC);
  g_assert_nonnull (media[0]);

  media[1] = lp_media_new (scene, SAMPLE_SYNC);
  g_assert_nonnull (media[1]);
  g_object_set (media[1], "x", 400, "y", 400, NULL);

  g_assert (lp_media_start (media[0]));
  g_assert (lp_media_start (media[1]));

  event = await_filtered (scene, 2, LP_EVENT_MASK_START);
  g_assert_nonnull (event);
  g_object_unref (event);

  for (i = 0; i < 20; i++)
    {
      await_ticks (scene, 1);
      g_assert (lp_media_seek (media[0], 5 * interval));
      /* g_assert (lp_media_seek (media[1], 2 * interval)); */
    }

  g_object_unref (scene);
  exit (EXIT_SUCCESS);
}
