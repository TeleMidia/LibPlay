/* Copyright (C) 2015 PUC-Rio/Laboratorio TeleMidia

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

      scene = g_object_new (LP_TYPE_SCENE, NULL);
      assert (scene != NULL);
      media = g_object_new (LP_TYPE_MEDIA, "scene", scene, "uri", NULL, NULL);
      assert (media != NULL);
      g_object_unref (media);
      g_object_unref (scene);
    }
  STMT_END;
  
  /* get/set scene*/
  STMT_BEGIN
    {
      lp_Scene *scene;
      lp_Scene *tmp;
      lp_Media *media;
     
      scene = g_object_new (LP_TYPE_SCENE, NULL);
      assert (scene != NULL);

      media = g_object_new (LP_TYPE_MEDIA, NULL);
      assert (media != NULL);
      g_object_get (media, "scene", &tmp, NULL);
      assert (tmp == NULL);
      g_object_unref (media);

      media = g_object_new (LP_TYPE_MEDIA, "scene", scene, NULL);
      assert (media != NULL);
      g_object_get (media, "scene", &tmp, NULL);
      assert (tmp == scene);
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
      assert (media != NULL);
      g_object_get (media, "width", &width, "height", &height, NULL);
      assert (width == 0 && height == 0);
      g_object_unref (media);

      media = g_object_new (LP_TYPE_MEDIA, "width", 800, NULL);
      assert (media != NULL);
      g_object_get (media, "width", &width, "height", &height, NULL);
      assert (width == 800 && height == 0);
      g_object_unref (media);

      media = g_object_new (LP_TYPE_MEDIA, "height", 600, NULL);
      assert (media != NULL);
      g_object_get (media, "width", &width, "height", &height, NULL);
      assert (width == 0 && height == 600);
      g_object_unref (media);

      media = g_object_new (LP_TYPE_MEDIA, "width", 800, 
          "height", 600, NULL);
      assert (media != NULL);
      g_object_get (media, "width", &width, "height", &height, NULL);
      assert (width == 800 && height == 600);
      g_object_unref (media);
    }
  STMT_END;

  /* get/set zorder */
  STMT_BEGIN
    {
      lp_Media *media;
      int zorder = -1;
      
      media = g_object_new (LP_TYPE_MEDIA, NULL);
      assert (media != NULL);
      g_object_get (media, "zorder", &zorder, NULL);
      assert (zorder == 0);
      g_object_unref (media);

      media = g_object_new (LP_TYPE_MEDIA, "zorder", 10, NULL);
      assert (media != NULL);
      g_object_get (media, "zorder", &zorder, NULL);
      assert (zorder == 10);
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
      assert (media != NULL);
      g_object_get (media, "alpha", &alpha, "volume", &volume, NULL);
      assert (alpha == 1.0 && volume == 1.0);
      g_object_unref (media);

      alpha = -1.0;
      volume = -1.0;

      media = g_object_new (LP_TYPE_MEDIA, "alpha", 0.5, NULL);
      assert (media != NULL);
      g_object_get (media, "alpha", &alpha, "volume", &volume, NULL);
      assert (alpha == 0.5 && volume == 1.0);
      g_object_unref (media);
      
      alpha = -1.0;
      volume = -1.0;

      media = g_object_new (LP_TYPE_MEDIA, "volume", 0.5, NULL);
      assert (media != NULL);
      g_object_get (media, "alpha", &alpha, "volume", &volume, NULL);
      assert (alpha == 1.0 && volume == 0.5);
      g_object_unref (media);
      
      alpha = -1.0;
      volume = -1.0;

      media = g_object_new (LP_TYPE_MEDIA, "alpha", 0.5, 
          "volume", 0.5, NULL);
      assert (media != NULL);
      g_object_get (media, "alpha", &alpha, "volume", &volume, NULL);
      assert (alpha == 0.5 && volume == 0.5);
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
      assert (media != NULL);
      g_object_get (media, "uri", &tmp, NULL);
      assert (tmp == NULL);
      g_object_unref (media);

      media = g_object_new (LP_TYPE_MEDIA, "uri", uri, NULL);
      assert (media != NULL);
      g_object_get (media, "uri", &tmp, NULL);
      assert (strcmp (tmp, uri) == 0);
      g_object_unref (media);
    }
  STMT_END;
  exit (EXIT_SUCCESS);
}
