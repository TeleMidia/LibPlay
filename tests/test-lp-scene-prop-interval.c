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
  guint64 v[] = {500, 250, 100, 1000};
  guint64 t = 0;
  gsize i;

  guint64 interval = G_MAXUINT64;
  guint64 ticks = G_MAXUINT64;

  scene = SCENE_NEW (640, 480, 3);
  g_object_get (scene, "interval", &interval, NULL);
  g_assert (interval == GST_SECOND);

  for (i = 0; i < nelementsof (v); i++)
    {
      gint background = 0;
      gint wave = 0;
      gint n;

      g_object_set (scene, "interval", v[i] * GST_MSECOND, NULL);
      g_object_get (scene, "interval", &interval, "ticks", &ticks, NULL);

      g_assert (interval == v[i] * GST_MSECOND);
      g_assert (ticks == t);

      for (n = 0; n < 10; n++)
        {
          await_ticks (scene, 1);
          t++;
          g_object_set (scene,
                        "background", background++ % 4,
                        "wave", wave++ % 13, NULL);
        }
    }
  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
