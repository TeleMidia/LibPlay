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
  /* sync error: bad URI */
  STMT_BEGIN
    {
      lp_Scene *scene;
      lp_Media *media;

      scene = lp_scene_new (800, 600);
      g_assert_nonnull (scene);

      media = lp_media_new (scene, "nonexistent");
      g_assert_nonnull (media);

      g_assert (!lp_media_start (media));
      g_object_unref (scene);
    }
  STMT_END;

  /* async error: no active pads */
  STMT_BEGIN
    {
      lp_Scene *scene;
      lp_Media *media;

      lp_Event *event;
      GError *error = NULL;

      scene = lp_scene_new (0, 0); /* no video output */
      g_assert_nonnull (scene);

      media = lp_media_new (scene, SAMPLE_FELIS);
      g_assert_nonnull (media);

      g_assert (lp_media_start (media));

      event = await_filtered (scene, 1, LP_EVENT_MASK_ERROR);
      g_assert_nonnull (event);

      g_object_get (event, "error", &error, NULL);
      g_object_unref (event);

      g_assert_nonnull (error);
      g_assert (g_error_matches (error, LP_ERROR, LP_ERROR_START));
      g_print ("%s\n", error->message);
      g_error_free (error);

      g_object_unref (scene);
    }
  STMT_END;

  /* async error: stream error (wrong type) */
  STMT_BEGIN
    {
      lp_Scene *scene;
      lp_Media *media;

      lp_Event *event;
      GError *error = NULL;

      scene = lp_scene_new (0, 0);
      g_assert_nonnull (scene);

      media = lp_media_new (scene, __FILE__);
      g_assert_nonnull (media);

      g_assert (lp_media_start (media));

      event = await_filtered (scene, 1, LP_EVENT_MASK_ERROR);
      g_assert_nonnull (event);

      g_object_get (event, "error", &error, NULL);
      g_object_unref (event);

      g_assert_nonnull (error);
      g_assert (g_error_matches (error, LP_ERROR, LP_ERROR_START));
      g_print ("%s\n", error->message);
      g_error_free (error);

      g_object_unref (scene);
    }
  STMT_END;

  /* success: start random samples */
  STMT_BEGIN
    {
      lp_Scene *scene;
      lp_Media *media[10];
      gint w, h;
      gsize i;

      w = 800;
      h = 600;
      scene = lp_scene_new (w, h);
      g_assert_nonnull (scene);
      g_object_set (scene, "pattern", 0, NULL);

      for (i = 0; i < nelementsof (media); i++)
        {
          media[i] = LP_MEDIA
            (g_object_new (LP_TYPE_MEDIA,
                           "scene", scene,
                           "uri", random_sample (),
                           "x", g_random_int_range (0, w),
                           "y", g_random_int_range (0, h),
                           "z", g_random_int_range (1, 10),
                           "width", g_random_int_range (0, w),
                           "height", g_random_int_range (0, h),
                           "alpha", g_random_double_range (0., 1.),
                           "mute", g_random_boolean (),
                           "volume", g_random_double_range (0., 2.),
                           NULL));
          g_assert_nonnull (media[i]);
          g_assert (lp_media_start (media[i]));
        }

      await_ticks (scene, 8);
      g_object_unref (scene);
    }
  STMT_END;

  exit (EXIT_SUCCESS);
}
