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
#include <stdio.h>
#include <assert.h>

#include "gx-macros.h"

GX_INCLUDE_PROLOGUE
#include "play.h"
GX_INCLUDE_EPILOGUE

int
main (int argc, char **argv)
{
  lp_Scene *scene;
  lp_Media *media;
  GObject *obj;
  lp_Event evt;

  if (argc < 2)
  {
    fprintf (stderr, "usage: %s <uri>\n", argv[0]);
    exit (EXIT_FAILURE);
  }

  scene = lp_scene_new (1080, 720);
  assert (scene != NULL);

  media = lp_media_new (scene, argv[1]);
  assert (media != NULL);

  lp_media_start (media);

  while (1)
  {
    lp_scene_pop(scene, TRUE, &obj, &evt);
    if (evt == LP_ERROR || evt == LP_EOS || evt == LP_STOP)
      break;
  }

  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
