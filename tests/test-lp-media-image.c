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
  /* start error (bad URI) */
  STMT_BEGIN
    {
      lp_Scene *scene;
      lp_Media *media;

      scene = lp_scene_new (800, 600);
      g_assert_nonnull (scene);

      media = lp_media_new (scene, "nonexistent");
      g_assert_nonnull (media);

      g_assert_false (lp_media_start (media));
      g_object_unref (scene);
    }
  STMT_END;

  /* start async error (no active pads) */
  STMT_BEGIN
    {
      lp_Scene *scene;
      lp_Media *media;

      lp_Event *event;
      GError *error = NULL;

      scene = lp_scene_new (0, 0); /* no video output */
      g_assert_nonnull (scene);

      media = lp_media_new (scene, "samples/felis.jpg");
      g_assert_nonnull (media);

      g_assert (lp_media_start (media));

      event = await_filtered (scene, 1, LP_EVENT_MASK_ERROR);
      g_assert_nonnull (event);

      g_object_get (event, "error", &error, NULL);
      g_object_unref (event);

      g_assert_nonnull (error);
      g_assert_true (g_error_matches (error, LP_ERROR, LP_ERROR_START));
      g_error_free (error);

      g_object_unref (scene);
    }
  STMT_END;

  /* start/stop (PNG, JPEG, GIF) */
  STMT_BEGIN
    {
      lp_Scene *scene;
      gsize i;

      const char *uri[] = {
        "samples/gnu.png",
        "samples/felis.jpg",
        "samples/earth.gif",
      };

      scene = lp_scene_new (800, 600);
      g_assert_nonnull (scene);
      g_object_set (scene, "pattern", 0, NULL);

      for (i = 0; i < nelementsof (uri); i++)
        {
          lp_Media *media;
          lp_Event *event;

          media = lp_media_new (scene, uri[i]);
          g_assert_nonnull (media);

          g_assert (lp_media_start (media));

          event = await_filtered (scene, 1, LP_EVENT_MASK_START
                                  | LP_EVENT_MASK_ERROR);
          g_assert_nonnull (event);
          g_assert (LP_IS_EVENT_START (event));
          g_object_unref (event);

          event = await_filtered (scene, 2, LP_EVENT_MASK_TICK
                                  | LP_EVENT_MASK_ERROR);
          g_assert_nonnull (event);
          g_assert (LP_IS_EVENT_TICK (event));
          g_object_unref (event);

          g_assert (lp_media_stop (media));

          event = await_filtered (scene, 1, LP_EVENT_MASK_STOP
                                  | LP_EVENT_MASK_ERROR);
          g_assert_nonnull (event);
          g_assert (LP_IS_EVENT_STOP (event));
          g_object_unref (event);
        }

      g_object_unref (scene);
    }
  STMT_END;

  /* get/set width and height */
  STMT_BEGIN
    {
      lp_Scene *scene;
      lp_Media *media;

      scene = lp_scene_new (800, 600);
      g_assert_nonnull (scene);
      g_object_set (scene, "pattern", 0, NULL);

      media = lp_media_new (scene, "samples/gnu.png");
      g_assert_nonnull (media);
      g_assert (lp_media_start (media));

      await_ticks (scene, 2);

      g_object_unref (scene);
    }
  STMT_END;

  exit (EXIT_SUCCESS);
}
