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
  gsize i;

  gint mask = LP_EVENT_MASK_NONE;
  lp_Event *event = NULL;

  gchar *key = NULL;
  gboolean press = FALSE;

  gint button = -1;
  gdouble x = 0.;
  gdouble y = 0.;

  scene = lp_scene_new (800, 600);
  g_assert_nonnull (scene);

  g_object_get (scene, "mask", &mask, NULL);
  g_assert (mask == LP_EVENT_MASK_ANY);

  /* tick */

  g_object_set (scene, "mask", LP_EVENT_MASK_TICK, NULL);
  g_object_get (scene, "mask", &mask, NULL);
  g_assert (mask == LP_EVENT_MASK_TICK);

  await_ticks (scene, 1);

  /* key */

  g_object_set (scene, "mask", LP_EVENT_MASK_KEY, NULL);
  send_key (scene, "q", TRUE);  /* key */

  event = lp_scene_receive (scene, TRUE);
  g_assert_nonnull (event);
  g_assert (LP_IS_EVENT_KEY (event));
  g_object_get (event, "key", &key, "press", &press, NULL);
  g_assert (g_str_equal (key, "q"));
  g_assert_true (press);
  g_object_unref (event);

  /* click */

  g_object_set (scene, "mask", LP_EVENT_MASK_POINTER_CLICK, NULL);
  send_pointer_click (scene, 3, 10., 10., TRUE);

  event = lp_scene_receive (scene, TRUE);
  g_assert_nonnull (event);
  g_assert (LP_IS_EVENT_POINTER_CLICK (event));
  g_object_get (event, "button", &button, "x", &x, "y", &y,
                "press", &press, NULL);
  g_assert (button == 3);
  g_assert (x == 10.);
  g_assert (y == 10.);
  g_assert_true (press);
  g_object_unref (event);

  /* move */

  g_object_set (scene, "mask", LP_EVENT_MASK_POINTER_MOVE, NULL);
  send_pointer_move (scene, 20., 20.);

  event = lp_scene_receive (scene, TRUE);
  g_assert_nonnull (event);
  g_assert (LP_IS_EVENT_POINTER_MOVE (event));
  g_object_get (event, "x", &x, "y", &y, NULL);
  g_assert (x == 20.);
  g_assert (y == 20.);
  g_object_unref (event);

  /* none */

  g_object_set (scene, "mask", LP_EVENT_MASK_NONE, NULL);
  send_key (scene, "q", TRUE);
  send_pointer_click (scene, 3, 10., 10., TRUE);
  send_pointer_move (scene, 20., 20.);

  for (i = 0; i < 50; i++)
    {
      event = lp_scene_receive (scene, FALSE);
      g_assert (event == NULL);
    }

  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
