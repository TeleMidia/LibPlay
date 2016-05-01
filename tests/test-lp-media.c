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
  /* new/unref */
  STMT_BEGIN
    {
      lp_Scene *scene;
      lp_Media *media;
      lp_Scene *sc = NULL;
      const char *uri = NULL;

      scene = lp_scene_new (0, 0);
      g_assert_nonnull (scene);

      media = LP_MEDIA (g_object_new (LP_TYPE_MEDIA, "scene", scene, NULL));
      g_assert_nonnull (media);

      g_object_get (media, "scene", &sc, NULL);
      g_assert (sc == scene);

      g_object_get (media, "uri", &uri, NULL);
      g_assert_null (uri);
      g_object_unref (scene);

      scene = lp_scene_new (0, 0);
      g_assert_nonnull (scene);

      media = lp_media_new (scene, "test");
      g_assert_nonnull (media);

      g_object_get (media, "scene", &sc, NULL);
      g_assert (sc == scene);

      g_object_get (media, "uri", &uri, NULL);
      g_assert_true (g_str_equal (uri, "test"));
      g_object_unref (scene);
    }
  STMT_END;

  /* start */
  STMT_BEGIN
    {
      lp_Scene *scene;
      lp_Media *m1, *m2, *m3;
      int n = 3;

      scene = lp_scene_new (1024, 768);
      g_assert_nonnull (scene);

      m1 = lp_media_new (scene, "samples/sync.m4v");
      g_assert_nonnull (m1);
      g_assert_true (lp_media_start (m1));

      m2 = lp_media_new (scene, "samples/gnu.png");
      g_assert_nonnull (m2);
      g_object_set (m2, "y", 300, "z", 2, NULL);
      g_assert_true (lp_media_start (m2));

      AWAIT (scene, 2);
      m3 = lp_media_new (scene, "samples/bars.ogg");
      g_assert_nonnull (m3);
      g_object_set (m3, "alpha", .75, "z", 3, NULL);
      g_assert_true (lp_media_start (m3));

      while (n-- > 0)
        {
          gboolean done = FALSE;
          int x, y;
          do
            {
              lp_Event *event;
              GObject *source;

              event = lp_scene_receive (scene, TRUE);
              g_assert_nonnull (event);

              source = lp_event_get_source (event);
              g_assert_nonnull (source);

              if (LP_IS_EVENT_STOP (event) && LP_MEDIA (source) == m3)
                done = TRUE;

              g_object_unref (event);
            }
          while (!done);

          g_assert_true (lp_media_start (m3));
          g_object_get (m3, "x", &x, "y", &y, NULL);
          g_object_set (m3, "x", x + 100, "y", y + 100, NULL);
        }
      AWAIT (scene, 2);
      g_object_unref (scene);
    }
  STMT_END;

#if 0

  /* get/set scene*/
  STMT_BEGIN
    {
      lp_Scene *scene;
      lp_Scene *tmp;
      lp_Media *media;

      scene = g_object_new (LP_TYPE_SCENE, NULL);
      g_assert_nonnull (scene);

      media = g_object_new (LP_TYPE_MEDIA, NULL);
      g_assert_nonnull (media);
      g_object_get (media, "scene", &tmp, NULL);
      g_assert_null (tmp);
      g_object_unref (media);

      media = g_object_new (LP_TYPE_MEDIA, "scene", scene, NULL);
      g_assert_nonnull (media);
      g_object_get (media, "scene", &tmp, NULL);
      g_assert_null (tmp);
      g_object_unref (media);
      g_object_unref (scene);
    }
  STMT_END;

  /* get/set scene, width and height */
  STMT_BEGIN
    {
      lp_Media *media;
      int width = -1;
      int height = -1;

      media = g_object_new (LP_TYPE_MEDIA, NULL);
      g_assert_nonnull (media);
      g_object_get (media, "width", &width, "height", &height, NULL);
      g_assert (width == 0 && height == 0);
      g_object_unref (media);

      media = g_object_new (LP_TYPE_MEDIA, "width", 800, NULL);
      g_assert_nonnull (media);
      g_object_get (media, "width", &width, "height", &height, NULL);
      g_assert (width == 800 && height == 0);
      g_object_unref (media);

      media = g_object_new (LP_TYPE_MEDIA, "height", 600, NULL);
      g_assert_nonnull (media);
      g_object_get (media, "width", &width, "height", &height, NULL);
      g_assert (width == 0 && height == 600);
      g_object_unref (media);

      media = g_object_new (LP_TYPE_MEDIA, "width", 800,
          "height", 600, NULL);
      g_assert_nonnull (media);
      g_object_get (media, "width", &width, "height", &height, NULL);
      g_assert (width == 800 && height == 600);
      g_object_unref (media);
    }
  STMT_END;

  /* get/set zorder */
  STMT_BEGIN
    {
      lp_Media *media;
      int zorder = -1;

      media = g_object_new (LP_TYPE_MEDIA, NULL);
      g_assert_nonnull (media);
      g_object_get (media, "zorder", &zorder, NULL);
      g_assert (zorder == 0);
      g_object_unref (media);

      media = g_object_new (LP_TYPE_MEDIA, "zorder", 10, NULL);
      g_assert_nonnull (media);
      g_object_get (media, "zorder", &zorder, NULL);
      g_assert (zorder == 10);
      g_object_unref (media);
    }
  STMT_END;

  /* get/set alpha and volume */
  STMT_BEGIN
    {
      lp_Media *media;
      double alpha = -1.0;
      double volume = -1.0;

      media = g_object_new (LP_TYPE_MEDIA, NULL);
      g_assert_nonnull (media);
      g_object_get (media, "alpha", &alpha, "volume", &volume, NULL);
      g_assert (alpha == 1.0 && volume == 1.0);
      g_object_unref (media);

      alpha = -1.0;
      volume = -1.0;

      media = g_object_new (LP_TYPE_MEDIA, "alpha", 0.5, NULL);
      g_assert_nonnull (media);
      g_object_get (media, "alpha", &alpha, "volume", &volume, NULL);
      g_assert (alpha == 0.5 && volume == 1.0);
      g_object_unref (media);

      alpha = -1.0;
      volume = -1.0;

      media = g_object_new (LP_TYPE_MEDIA, "volume", 0.5, NULL);
      g_assert_nonnull (media);
      g_object_get (media, "alpha", &alpha, "volume", &volume, NULL);
      g_assert (alpha == 1.0 && volume == 0.5);
      g_object_unref (media);

      alpha = -1.0;
      volume = -1.0;

      media = g_object_new (LP_TYPE_MEDIA, "alpha", 0.5,
          "volume", 0.5, NULL);
      g_assert_nonnull (media);
      g_object_get (media, "alpha", &alpha, "volume", &volume, NULL);
      g_assert (alpha == 0.5 && volume == 0.5);
      g_object_unref (media);
    }
  STMT_END;

  /* get/set uri*/
  STMT_BEGIN
    {
      lp_Media *media;
      const char *uri = "file:///dev/null";
      char *tmp;

      media = g_object_new (LP_TYPE_MEDIA, NULL);
      g_assert_nonnull (media);
      g_object_get (media, "uri", &tmp, NULL);
      g_assert_null (tmp);
      g_object_unref (media);

      media = g_object_new (LP_TYPE_MEDIA, "uri", uri, NULL);
      g_assert_nonnull (media);
      g_object_get (media, "uri", &tmp, NULL);
      g_assert_true (g_str_equal (tmp, uri));
      g_object_unref (media);
    }
  STMT_END;
#endif
  exit (EXIT_SUCCESS);
}
