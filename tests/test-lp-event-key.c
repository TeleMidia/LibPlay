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
  lp_EventKey *event;

  lp_Scene *source = NULL;
  lp_EventMask mask = 0;
  gchar *key = NULL;
  gboolean press = TRUE;

  scene = LP_SCENE (g_object_new (LP_TYPE_SCENE, "lockstep", TRUE, NULL));
  g_assert_nonnull (scene);

  event = _lp_event_key_new (scene, "a", FALSE);
  g_assert_nonnull (event);

  g_object_get (event,
                "source", &source,
                "mask", &mask,
                "key", &key,
                "press", &press, NULL);

  g_assert (source == scene);
  g_assert (mask == LP_EVENT_MASK_KEY);
  g_assert (g_str_equal (key, "a"));
  g_assert (press == FALSE);

  g_free (key);
  g_object_unref (event);
  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
