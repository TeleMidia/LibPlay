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
  lp_Media *media;
  lp_EventStart *event;

  lp_Media *source = NULL;

  scene = LP_SCENE (g_object_new (LP_TYPE_SCENE, "lockstep", TRUE, NULL));
  g_assert_nonnull (scene);

  media = lp_media_new (scene, NULL);
  g_assert_nonnull (media);

  event = LP_EVENT_START (g_object_new (LP_TYPE_EVENT_START,
                                        "source", media, NULL));
  g_assert_nonnull (event);

  source = LP_MEDIA (lp_event_get_source (LP_EVENT (event)));
  g_assert (source == media);

  g_assert_false (lp_event_start_is_resume (event));
  g_object_unref (event);

  event = lp_event_start_new (media, TRUE);
  g_assert_nonnull (event);

  source = LP_MEDIA (lp_event_get_source (LP_EVENT (event)));
  g_assert (source == media);

  g_assert_true (lp_event_start_is_resume (event));
  g_object_unref (event);

  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
