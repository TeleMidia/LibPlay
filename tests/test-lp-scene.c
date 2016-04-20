/* Copyright (C) 2015 PUC-Rio/Laboratorio TeleMidia

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

      scene = g_object_new (LP_TYPE_SCENE, NULL);
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

      scene = g_object_new (LP_TYPE_SCENE, NULL);
      assert (scene != NULL);
      g_object_get (scene, "width", &width, "height", &height, NULL);
      assert (width == 0 && height == 0);
      g_object_unref (scene);

      scene = g_object_new (LP_TYPE_SCENE, "width", 800, NULL);
      assert (scene != NULL);
      g_object_get (scene, "width", &width, "height", &height, NULL);
      assert (width == 800 && height == 0);
      g_object_unref (scene);

      scene = g_object_new (LP_TYPE_SCENE, "height", 600, NULL);
      assert (scene != NULL);
      g_object_get (scene, "width", &width, "height", &height, NULL);
      assert (width == 0 && height == 600);
      g_object_unref (scene);
    }
  STMT_END;

  /* get/set pattern */
  STMT_BEGIN
    {
      /* TODO */
    }
  STMT_END;

  /* get/set wave */
  STMT_BEGIN
    {
    }
  STMT_END;

  exit (EXIT_SUCCESS);
}
