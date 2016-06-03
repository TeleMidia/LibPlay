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
  gboolean done;

  scene = lp_scene_new (800, 600);
  g_assert_nonnull (scene);
  g_object_set (scene, "text-font", "sans 16", NULL);

  done = FALSE;
  do
    {
      lp_Event *event;
      gchar *str;

      event = lp_scene_receive (scene, TRUE);
      g_assert_nonnull (event);

      str = lp_event_to_string (event);
      g_assert_nonnull (str);

      g_object_set (scene, "text", str, NULL);
      g_free (str);

      switch (lp_event_get_mask (event))
        {
        case LP_EVENT_MASK_ERROR:
          {
            GError *error = NULL;

            g_object_get (LP_EVENT_ERROR (event), "error", &error, NULL);
            g_assert_nonnull (error);
            g_assert_no_error (error);
            g_error_free (error);
            done = TRUE;
            break;
          }
        case LP_EVENT_MASK_KEY:
          {
            gchar *key;

            g_object_get (LP_EVENT_KEY (event), "key", &key, NULL);
            if (g_str_equal (key, "q") || g_str_equal (key, "Escape"))
              done = TRUE;
            g_free (key);
            break;
          }
        default:
          break;
        }

      g_object_unref (event);
    }
  while (!done);

  g_object_unref (scene);
  exit (EXIT_SUCCESS);
}
