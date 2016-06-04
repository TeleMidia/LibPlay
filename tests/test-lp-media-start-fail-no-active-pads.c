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

  lp_Event *event;
  GError *error = NULL;

  scene = SCENE_NEW (0, 0, 0); /* no video output */
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

  exit (EXIT_SUCCESS);
}
