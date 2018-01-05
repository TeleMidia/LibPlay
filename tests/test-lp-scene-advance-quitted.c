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

  scene = SCENE_NEW (800, 600, 0);

  g_object_set (scene, "lockstep", TRUE, NULL);
  g_assert (lp_scene_advance (scene, 1000));

  lp_scene_quit (scene);
  g_assert (!lp_scene_advance (scene, 1000));
  g_assert (!lp_scene_advance (scene, 1000));
  g_assert (!lp_scene_advance (scene, 1000));

  g_object_unref (scene);
  exit (EXIT_SUCCESS);
}
