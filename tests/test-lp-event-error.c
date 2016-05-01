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
  lp_EventError *event;

  lp_Media *source = NULL;
  GError *error = NULL;

  scene = LP_SCENE (g_object_new (LP_TYPE_SCENE, "lockstep", TRUE, NULL));
  g_assert_nonnull (scene);

  media = lp_media_new (scene, NULL);
  g_assert_nonnull (media);

  error = g_error_new_literal (G_MARKUP_ERROR, G_MARKUP_ERROR_EMPTY, "e1");
  g_assert_nonnull (error);

  event = LP_EVENT_ERROR (g_object_new (LP_TYPE_EVENT_ERROR,
                                        "source", media,
                                        "error", error, NULL));
  g_assert_nonnull (event);

  source = LP_MEDIA (lp_event_get_source (LP_EVENT (event)));
  g_assert (source == media);

  error = NULL;
  error = lp_event_error_get_error (event);
  g_assert_nonnull (error);
  g_object_unref (event);

  error = g_error_new_literal (G_MARKUP_ERROR, G_MARKUP_ERROR_EMPTY, "e1");
  g_assert_nonnull (error);

  event = lp_event_error_new (media, error);
  g_assert_nonnull (event);

  source = LP_MEDIA (lp_event_get_source (LP_EVENT (event)));
  g_assert (source == media);

  error = NULL;
  error = lp_event_error_get_error (event);
  g_assert_nonnull (error);
  g_object_unref (event);

  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
