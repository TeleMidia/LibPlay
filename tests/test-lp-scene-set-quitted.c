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
  guint64 interval;

  scene = SCENE_NEW (0, 0, 0);

  await_ticks (scene, 1);
  g_object_set (scene, "interval", 1, NULL);
  g_object_get (scene, "interval", &interval, NULL);
  g_assert (interval == 1);

  lp_scene_quit (scene);
  g_object_set (scene, "interval", 200, NULL);
  g_object_get (scene, "interval", &interval, NULL);
  g_assert (interval == 200);

  g_object_set (scene, "interval", 400, NULL);
  g_object_get (scene, "interval", &interval, NULL);
  g_assert (interval == 400);

  g_object_unref (scene);
  exit (EXIT_SUCCESS);
}
