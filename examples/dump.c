/* dump.c -- Dump all events to stdout.
   Copyright (C) 2015-2016 PUC-Rio/Laboratorio TeleMidia

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

#include <config.h>
#include "gx-macros.h"

GX_INCLUDE_PROLOGUE
#include "play.h"
GX_INCLUDE_EPILOGUE

int
main (void)
{
  lp_Scene *scene;
  gboolean quit = FALSE;

  scene = lp_scene_new (800, 600);
  g_assert_nonnull (scene);

  while (!quit)
    {
      lp_Event *event;
      gchar *str;

      event = lp_scene_receive (scene, TRUE);
      g_assert_nonnull (event);

      str = lp_event_to_string (event);
      g_assert_nonnull (str);

      g_print ("%s\n", str);
      g_free (str);

      if (LP_IS_EVENT_ERROR (event))
        {
          quit = TRUE;
        }
      else if (LP_IS_EVENT_KEY (event))
        {
          gchar *key;

          g_object_get (LP_EVENT_KEY (event), "key", &key, NULL);
          if (g_str_equal (key, "q"))
            quit = TRUE;
        }

      g_object_unref (event);
    }

  g_object_unref (scene);
  exit (EXIT_SUCCESS);
}
