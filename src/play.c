/* play.c -- Simple media player.
   Copyright (C) 2015-2018 PUC-Rio/Laboratorio TeleMidia

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
main (int argc, char **argv)
{
  lp_Scene *scene;
  lp_Media *media;
  gboolean done;
  int status;

  if (unlikely (argc != 2))
  {
    gchar *me = g_path_get_basename (argv[0]);
    g_printerr ("usage: %s FILE\n", me);
    g_free (me);
    exit (EXIT_FAILURE);
  }

  scene = lp_scene_new (800, 600);
  g_assert_nonnull (scene);

  media = lp_media_new (scene, argv[1]);
  g_assert_nonnull (media);

  if (unlikely (!lp_media_start (media)))
    {
      g_printerr ("error: cannot start media at '%s'\n", argv[1]);
      g_object_unref (scene);
      exit (EXIT_FAILURE);
    }

  done = FALSE;
  status = EXIT_SUCCESS;
  do
    {
      lp_Event *event;

      event = lp_scene_receive (scene, TRUE);
      g_assert_nonnull (event);

      switch (lp_event_get_mask (event))
        {
        case LP_EVENT_MASK_TICK:
          break;
        case LP_EVENT_MASK_KEY:
          {
            gchar *key = NULL;

            g_object_get (LP_EVENT_KEY (event), "key", &key, NULL);
            g_assert_nonnull (key);
            if (g_str_equal (key, "q") || g_str_equal (key, "Escape"))
              done = TRUE;
            g_free (key);
            break;
          }
        case LP_EVENT_MASK_POINTER_CLICK:
          break;
        case LP_EVENT_MASK_POINTER_MOVE:
          break;
        case LP_EVENT_MASK_ERROR:
          {
            GError *error = NULL;

            g_object_get (event, "error", &error, NULL);
            g_assert_nonnull (error);
            g_printerr ("error: %s\n", error->message);
            g_error_free (error);
            status = EXIT_FAILURE;
            done = TRUE;
            break;
          }
        case LP_EVENT_MASK_START:
          break;
        case LP_EVENT_MASK_STOP:
          {
            done = TRUE;
            break;
          }
        case LP_EVENT_MASK_SEEK:
          break;
        default:
          g_assert_not_reached ();
        }
      g_object_unref (event);
    }
  while (!done);

  g_object_unref (scene);
  exit (status);
}
