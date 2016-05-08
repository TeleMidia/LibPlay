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

  gint width = G_MAXINT;
  gint height = G_MAXINT;

  scene = LP_SCENE (g_object_new (LP_TYPE_SCENE, NULL));
  g_assert_nonnull (scene);
  g_object_get (scene, "width", &width, "height", &height, NULL);
  g_assert (width == 0 && height == 0); /* default */
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
  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
