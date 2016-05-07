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
  /* new/unref */
  STMT_BEGIN
    {
      lp_Scene *scene;

      scene = LP_SCENE (g_object_new (LP_TYPE_SCENE, NULL));
      g_assert_nonnull (scene);
      g_object_unref (scene);
    }
  STMT_END;

  /* get/set width and height */
  STMT_BEGIN
    {
      lp_Scene *scene;
      gint width = -1;
      gint height = -1;

      scene = LP_SCENE (g_object_new (LP_TYPE_SCENE, NULL));
      g_assert_nonnull (scene);
      g_object_get (scene, "width", &width, "height", &height, NULL);
      g_assert (width == 0 && height == 0);
      g_object_unref (scene);

      scene = LP_SCENE (g_object_new (LP_TYPE_SCENE, "width", 800, NULL));
      g_assert_nonnull (scene);
      g_object_get (scene, "width", &width, "height", &height, NULL);
      g_assert (width == 800 && height == 0);
      g_object_unref (scene);

      scene = LP_SCENE (g_object_new (LP_TYPE_SCENE, "height", 600, NULL));
      g_assert_nonnull (scene);
      g_object_get (scene, "width", &width, "height", &height, NULL);
      g_assert (width == 0 && height == 600);
      g_object_unref (scene);

      scene = lp_scene_new (0, 0);
      g_assert_nonnull (scene);
      g_object_get (scene, "width", &width, "height", &height, NULL);
      g_assert (width == 0 && height == 0);
      g_object_unref (scene);

      scene = lp_scene_new (100, 100);
      g_assert_nonnull (scene);
      g_object_get (scene, "width", &width, "height", &height, NULL);
      g_assert (width == 100 && height == 100);

      AWAIT (scene, 1);
      g_object_get (scene, "width", &width, "height", &height, NULL);
      g_assert (width == 100 && height == 100);
      g_object_unref (scene);
    }
  STMT_END;

  /* get ticks */
  STMT_BEGIN
    {
      lp_Scene *scene;
      guint64 ticks = G_MAXUINT64;

      scene = lp_scene_new (0, 0);
      g_assert_nonnull (scene);
      g_object_get (scene, "ticks", &ticks, NULL);
      g_assert (ticks == 0);

      AWAIT (scene, 1);
      g_object_get (scene, "ticks", &ticks, NULL);
      g_assert (ticks == 1);

      AWAIT (scene, 1);
      g_object_get (scene, "ticks", &ticks, NULL);
      g_assert (ticks == 2);
      g_object_unref (scene);
    }
  STMT_END;

  /* get/set pattern */
  STMT_BEGIN
    {
      lp_Scene *scene;
      gint pattern = -1;

      scene = lp_scene_new (800, 600);
      g_assert_nonnull (scene);

      g_object_get (scene, "pattern", &pattern, NULL);
      g_assert (pattern == 2);

      AWAIT (scene, 1);
      g_object_set (scene, "pattern", 8, NULL);
      g_object_get (scene, "pattern", &pattern, NULL);
      g_assert (pattern == 8);

      AWAIT (scene, 1);
      g_object_get (scene, "pattern", &pattern, NULL);
      g_assert (pattern == 8);
      g_object_unref (scene);
    }
  STMT_END;

  /* get/set wave */
  STMT_BEGIN
    {
      lp_Scene *scene;
      gint wave = -1;

      scene = LP_SCENE (g_object_new (LP_TYPE_SCENE, NULL));
      g_assert_nonnull (scene);
      g_object_get (scene, "wave", &wave, NULL);
      g_assert (wave == 4);

      AWAIT (scene, 1);
      g_object_set (scene, "wave", 0, NULL);
      g_object_get (scene, "wave", &wave, NULL);
      g_assert (wave == 0);

      AWAIT (scene, 1);
      g_object_get (scene, "wave", &wave, NULL);
      g_assert (wave == 0);
      g_object_unref (scene);
    }
  STMT_END;

  /* get/set interval */
  STMT_BEGIN
    {
      lp_Scene *scene;
      guint64 interval = G_MAXUINT64;
      guint64 v[] = {500, 250, 100, 1000};
      size_t i;

      scene = lp_scene_new (200, 200);
      g_assert_nonnull (scene);
      g_object_get (scene, "interval", &interval, NULL);
      g_assert (interval == GST_SECOND);

      for (i = 0; i < nelementsof (v); i++)
        {
          gint pattern = 0;
          gint wave = 0;
          gint n;

          g_object_set (scene, "interval", v[i] * GST_MSECOND, NULL);
          g_object_get (scene, "interval", &interval, NULL);
          g_assert (interval == v[i] * GST_MSECOND);
          for (n = 0; n < 10; n++)
            {
              AWAIT (scene, 1);
              g_object_set (scene,
                            "pattern", pattern++ % 25,
                            "wave", wave++ % 13, NULL);
            }
        }
      g_object_unref (scene);
    }
  STMT_END;

  exit (EXIT_SUCCESS);
}
