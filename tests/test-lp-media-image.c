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
  /* start/stop */
  STMT_BEGIN
    {
      lp_Scene *scene;
      lp_Media *media;
      lp_Scene *sc = NULL;
      gchar *uri = NULL;

      scene = lp_scene_new (0, 0);
      g_assert_nonnull (scene);

      media = lp_media_new (scene, "samples/felis.jpg");
      g_assert_nonnull (media);

      g_object_get (media, "scene", &sc, NULL);
      g_assert (sc == scene);

      g_object_get (media, "uri", &uri, NULL);
      g_assert_nonnull (uri);
      g_str_equal (uri, "samples/felis.jpg");
      g_free (uri);

      g_assert (lp_media_start (media));
      AWAIT (scene, 1);

      g_object_unref (scene);
    }
  STMT_END;

  exit (EXIT_SUCCESS);
}
