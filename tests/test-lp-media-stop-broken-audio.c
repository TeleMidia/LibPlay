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

  scene = SCENE_NEW (800, 600, 2);
  media = lp_media_new (scene, SAMPLES_DIR (clock-broken.ogv));
  g_assert_nonnull (media);

  g_assert (lp_media_start (media));
  await_ticks (scene, 2);

  /* FIXME: The unref instruction should stop the media object but that
     doesn't happen.  The audio stream of "clock-broken.ogv" is broken; the
     stop-probe never gets called on its pad, which causes the media object
     to run "forever".  */

  g_timeout_add_seconds (2, (GSourceFunc) abort, NULL);
  g_object_unref (scene);

  exit (EXIT_SUCCESS);
}
