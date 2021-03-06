/* Copyright (C) 2015-2018 PUC-Rio/Laboratorio TeleMidia

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
  char *str;

  scene = LP_SCENE (g_object_new (LP_TYPE_SCENE, NULL));
  g_assert_nonnull (scene);
  str = lp_scene_to_string (scene);
  g_assert_nonnull (str);
  g_print ("%s\n", str);
  g_free (str);
  g_object_unref (scene);

  scene = SCENE_NEW (0, 0, 0);
  str = lp_scene_to_string (scene);
  g_assert_nonnull (str);
  g_print ("%s\n", str);
  g_free (str);
  g_object_unref (scene);

  scene = SCENE_NEW (200, 200, 0);
  str = lp_scene_to_string (scene);
  g_assert_nonnull (str);
  g_print ("%s\n", str);
  g_free (str);
  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
