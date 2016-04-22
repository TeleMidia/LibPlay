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
      assert (scene != NULL);

      g_object_unref (scene);
    }
  STMT_END;

  /* get/set width and height */
  STMT_BEGIN
    {
      lp_Scene *scene;
      int width = -1;
      int height = -1;

      scene = LP_SCENE (g_object_new (LP_TYPE_SCENE, NULL));
      assert (scene != NULL);
      g_object_get (scene, "width", &width, "height", &height, NULL);
      assert (width == 0 && height == 0);
      g_object_unref (scene);

      scene = LP_SCENE (g_object_new (LP_TYPE_SCENE, "width", 800, NULL));
      assert (scene != NULL);
      g_object_get (scene, "width", &width, "height", &height, NULL);
      assert (width == 800 && height == 0);
      g_object_unref (scene);

      scene = LP_SCENE (g_object_new (LP_TYPE_SCENE, "height", 600, NULL));
      assert (scene != NULL);
      g_object_get (scene, "width", &width, "height", &height, NULL);
      assert (width == 0 && height == 600);
      g_object_unref (scene);

      scene = lp_scene_new (0, 0);
      assert (scene != NULL);
      g_object_get (scene, "width", &width, "height", &height, NULL);
      assert (width == 0 && height == 0);
      g_object_unref (scene);

      scene = lp_scene_new (100, 100);
      assert (scene != NULL);
      g_object_get (scene, "width", &width, "height", &height, NULL);
      assert (width == 100 && height == 100);

      AWAIT (scene, 1);
      g_object_get (scene, "width", &width, "height", &height, NULL);
      assert (width == 100 && height == 100);
      g_object_unref (scene);
    }
  STMT_END;

  /* get ticks */
  STMT_BEGIN
    {
      lp_Scene *scene;
      guint64 ticks = G_MAXUINT64;

      scene = lp_scene_new (0, 0);
      assert (scene != NULL);
      g_object_get (scene, "ticks", &ticks, NULL);
      assert (ticks == 0);

      AWAIT (scene, 1);
      g_object_get (scene, "ticks", &ticks, NULL);
      assert (ticks == 1);

      AWAIT (scene, 1);
      g_object_get (scene, "ticks", &ticks, NULL);
      assert (ticks == 2);
      g_object_unref (scene);
    }
  STMT_END;

  /* get/set pattern */
  STMT_BEGIN
    {
      lp_Scene *scene;
      int pattern = -1;

      scene = lp_scene_new (800, 600);
      assert (scene != NULL);

      g_object_get (scene, "pattern", &pattern, NULL);
      assert (pattern == 2);

      AWAIT (scene, 1);
      g_object_set (scene, "pattern", 8, NULL);
      g_object_get (scene, "pattern", &pattern, NULL);
      assert (pattern == 8);

      AWAIT (scene, 1);
      g_object_get (scene, "pattern", &pattern, NULL);
      assert (pattern == 8);
      g_object_unref (scene);
    }
  STMT_END;

  /* get/set wave */
  STMT_BEGIN
    {
      lp_Scene *scene;
      int wave = -1;

      scene = LP_SCENE (g_object_new (LP_TYPE_SCENE, NULL));
      assert (scene != NULL);
      g_object_get (scene, "wave", &wave, NULL);
      assert (wave == 4);

      AWAIT (scene, 1);
      g_object_set (scene, "wave", 0, NULL);
      g_object_get (scene, "wave", &wave, NULL);
      assert (wave == 0);

      AWAIT (scene, 1);
      g_object_get (scene, "wave", &wave, NULL);
      assert (wave == 0);
      g_object_unref (scene);
    }
  STMT_END;

  exit (EXIT_SUCCESS);
}
