/* seek.c -- Seek in media objects.
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
main (int argc, char *const *argv)
{
  lp_Scene *scene;
  lp_Media *media[2];
  int w = 1024;
  int h = 768;
  gboolean done = FALSE;

  if (argc != 2)
    {
      gchar *me = g_path_get_basename (argv[0]);
      g_printerr ("usage: %s FILE\n", me);
      g_free (me);
      exit (EXIT_FAILURE);
    }

  scene = lp_scene_new (w, h);
  g_assert_nonnull (scene);
  g_object_set (scene, "pattern", 18,
                "text",
                "seek 1s in media\n"
                "←/→ (relative) or BACKSPACE/SPACE (absolute)",
                "text-color", 0xffffff00,
                "text-font", "sans 16", NULL);

  media[0] = lp_media_new (scene, argv[1]);
  g_assert_nonnull (media[0]);
  g_object_set (media[0], "width", w/2, "height", h/2, "alpha", .75, NULL);
  g_assert (lp_media_start (media[0]));

  media[1] = lp_media_new (scene, argv[1]);
  g_assert_nonnull (media[1]);
  g_object_set (media[1], "y", h/2, "width", w/2, "height", h/2,
                "alpha", .75, NULL);
  g_assert (lp_media_start (media[1]));

  while (!done)
    {
      lp_Event *event;

      event = lp_scene_receive (scene, TRUE);
      g_assert_nonnull (event);

      switch (lp_event_get_mask (event))
        {
        case LP_EVENT_MASK_QUIT:
          {
            done = TRUE;
            break;
          }
        case LP_EVENT_MASK_STOP:
          {
            lp_Media *source = LP_MEDIA (lp_event_get_source (event));
            if (source == media[0])
              done = TRUE;
            break;
          }
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
            gboolean press;

            g_object_get (LP_EVENT_KEY (event),
                          "key", &key, "press", &press, NULL);

            if (!press)
              {
                g_free (key);
                break;
              }

            if (g_str_equal (key, "q")
                || g_str_equal (key, "Escape"))
              {
                done = TRUE;
              }
            else if (g_str_equal (key, "Left")
                     || g_str_equal (key, "comma"))
              {
                g_assert (lp_media_seek (media[0], TRUE, -1000000000));
              }
            else if (g_str_equal (key, "Right")
                     || g_str_equal (key, "period"))
              {
                g_assert (lp_media_seek (media[0], TRUE, 1000000000));
              }
            else if (g_str_equal (key, "BackSpace"))
              {
                g_assert (lp_media_seek (media[0], FALSE, 1000000000));
              }
            else if (g_str_equal (key, "space"))
              {
                g_assert (lp_media_seek (media[0], FALSE, -1000000000));
              }
            g_free (key);
            break;
          }
        case LP_EVENT_MASK_SEEK:
          {
            gchar *str = lp_event_to_string (event);
            g_print ("%s\n", str);
            g_free (str);
            break;
          }
        default:
          break;                /* nothing to do */
        }
      g_object_unref (event);
    }

  g_object_unref (scene);
  exit (EXIT_SUCCESS);
}
