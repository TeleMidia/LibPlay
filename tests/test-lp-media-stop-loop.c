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
  gsize i;

  lp_Event *event;
  gboolean eos;
  gchar *str;

  scene = SCENE_NEW (800, 600, 2);
  media = lp_media_new (scene, SAMPLE_NIGHT);
  g_assert_nonnull (media);

  for (i = 0; i < 3; i++)
    {
      g_assert (lp_media_start (media));
      event = await_filtered (scene, 1, LP_EVENT_MASK_STOP);
      g_assert_nonnull (event);

      eos = FALSE;
      g_assert (LP_IS_EVENT_STOP (event));
      g_object_get (event, "eos", &eos, NULL);
      g_assert (eos);

      str = lp_event_to_string (event);
      g_print ("%s\n", str);
      g_free (str);
      g_object_unref (event);

      g_object_set (media,
                    "x", g_random_int_range (0, 800/2),
                    "y", g_random_int_range (0, 600/2), NULL);
    }

  g_object_unref (scene);
  exit (EXIT_SUCCESS);
}
