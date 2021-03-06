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
  lp_EventTick *event;
  gchar *str = NULL;

  lp_Scene *source = NULL;
  lp_EventMask mask = 0;
  guint64 serial = 0;

  scene = LP_SCENE (g_object_new (LP_TYPE_SCENE, "lockstep", TRUE, NULL));
  g_assert_nonnull (scene);

  event = _lp_event_tick_new (scene, G_MAXUINT64);
  g_assert_nonnull (event);

  g_object_get (event,
                "source", &source,
                "mask", &mask,
                "serial", &serial, NULL);

  str = lp_event_to_string (LP_EVENT (event));
  g_assert_nonnull (str);
  g_print ("%s\n", str);
  g_free (str);

  g_assert (source == scene);
  g_assert (mask == LP_EVENT_MASK_TICK);
  g_assert (serial == G_MAXUINT64);

  g_object_unref (event);
  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
