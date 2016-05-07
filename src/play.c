/* play.c -- Simple media player.
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
main (int argc, char **argv)
{
  lp_Scene *scene;
  lp_Media *media;
  GType type;

  if (unlikely (argc < 2))
  {
    g_printerr ("usage: %s <uri>\n", argv[0]);
    exit (EXIT_FAILURE);
  }

  scene = lp_scene_new (1080, 720);
  g_assert_nonnull (scene);

  media = lp_media_new (scene, argv[1]);
  g_assert_nonnull (media);

  g_assert (lp_media_start (media));
  do
    {
      lp_Event *event;

      event = lp_scene_receive (scene, TRUE);
      g_assert_nonnull (event);

      type = G_OBJECT_TYPE (event);
      g_assert (type != LP_TYPE_EVENT_ERROR);
      if (type == LP_TYPE_EVENT_POINTER_CLICK)
        {
          gdouble x, y;
          gint button;
          gboolean press;
          lp_EventPointerClick *click = LP_EVENT_POINTER_CLICK (event);

          g_object_get (G_OBJECT (click),
                        "x", &x,
                        "y", &y,
                        "button", &button,
                        "press", &press, NULL);

          g_print ("(x, y): %.0f, %.0f\n", x, y);
          g_print ("button: %d\n", button);
          g_print ("type: %s\n", press ? "press" : "release");
        }
      g_object_unref (event);
    }
  while (type != LP_TYPE_EVENT_STOP);

  g_object_unref (scene);
  exit (EXIT_SUCCESS);
}
