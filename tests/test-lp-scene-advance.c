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
  /* get/set lockstep */
  STMT_BEGIN
    {
      lp_Scene *scene;
      gboolean lockstep;
      guint64 time;
      guint64 last;
      gint i, j;

      scene = lp_scene_new (800, 600);
      g_assert_nonnull (scene);

      g_object_get (scene, "lockstep", &lockstep, NULL);
      g_assert_false (lockstep);

      g_object_set (scene, "pattern", 18, "wave", 0, NULL);
      await_ticks (scene, 2);

      g_assert_false (lp_scene_advance (scene, GST_SECOND));

      for (i = 0; i < 3; i++)
        {
          g_object_set (scene, "lockstep", TRUE, NULL);
          g_object_get (scene, "lockstep", &lockstep, NULL);
          g_assert_true (lockstep);

          g_object_get (scene, "time", &time, NULL);
          last = time;

          for (j = 0; j < 2000; j++) /* 2s */
            {
              g_assert_true (lp_scene_advance (scene, 1 * GST_MSECOND));
              g_object_get (scene, "time", &time, NULL);
              g_assert (((time - last) / GST_MSECOND) == 1);
              last = time;
              g_usleep (1 * 1000);
            }

          for (j = 0; j < 4; j++) /* 2s */
            {
              g_assert_true (lp_scene_advance (scene, 500 * GST_MSECOND));
              g_object_get (scene, "time", &time, NULL);
              g_assert (((time - last) / GST_MSECOND) == 500);
              last = time;
              SLEEP (.5);
            }

          g_object_set (scene, "lockstep", FALSE, NULL);
          g_object_get (scene, "lockstep", &lockstep, NULL);
          g_assert_false (lockstep);
          g_assert_false (lp_scene_advance (scene, GST_SECOND));

          SLEEP (2);            /* 2s */
        }

      g_object_unref (scene);
    }
  STMT_END;

  exit (EXIT_SUCCESS);
}
