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
  lp_Media *media[10];
  gint w, h;
  gsize i, n;

  lp_Event *event;

  w = 800;
  h = 600;
  scene = SCENE_NEW (w, h, 0);
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

  event = await_filtered (scene, nelementsof (media),
                          LP_EVENT_MASK_START
                          | LP_EVENT_MASK_ERROR
                          | LP_EVENT_MASK_STOP);
  g_assert_nonnull (event);
  g_assert (LP_IS_EVENT_START (event));
  g_object_unref (event);

  n = nelementsof (media);
  while (n > 0)
    {
      event = lp_scene_receive (scene, TRUE);
      g_assert_nonnull (event);
      switch (lp_event_get_mask (event))
        {
        case LP_EVENT_MASK_TICK:
          {
            for (i = 0; i < nelementsof (media); i++)
              {
                if (media[i] != NULL)
                  {
                    g_assert (lp_media_stop (media[i]));
                    break;
                  }
              }
            break;
          }
        case LP_EVENT_MASK_STOP:
          {
            lp_Media *source;
            source = LP_MEDIA (lp_event_get_source (event));
            g_assert_nonnull (source);
            for (i = 0; i < nelementsof (media); i++)
              {
                if (media[i] == source)
                  {
                    media[i] = NULL;
                    n--;
                    break;
                  }
              }
            break;
          }
        case LP_EVENT_MASK_KEY:
        case LP_EVENT_MASK_POINTER_CLICK:
        case LP_EVENT_MASK_POINTER_MOVE:
          {
            break;
          }
        default:
          {
            gchar *str;
            str = lp_event_to_string (event);
            g_assert_nonnull (str);
            g_print ("%s\n", str);
            g_free (str);
            g_assert_not_reached ();
          }
        }
      g_object_unref (event);
    }

  g_timeout_add_seconds (2, (GSourceFunc) abort, NULL);
  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
