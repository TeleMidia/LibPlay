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
#include <unistd.h>

int
main (int argc, char *argv[])
{
  lp_media_t *m;
  lp_media_t *parent;
  lp_event_t evt;

  if (argc < 2)
  {
    printf ("Usage: %s <uri>\n", argv[0]);
    exit (EXIT_FAILURE);
  }

  m = lp_media_create (argv[1]);
  parent = lp_media_get_parent (m);

  lp_event_init_start (&evt);
  ASSERT (evt.type == LP_EVENT_START);

  lp_media_post (m, &evt);

  sleep (1000);
  
  lp_media_destroy (m);
  lp_media_destroy (parent);

  exit (EXIT_SUCCESS);
}
