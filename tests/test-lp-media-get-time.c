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
main (void)
{
  lp_media_t *media;
  lp_event_t initevt;
  
  media = lp_media_create (NULL);

  assert (media != NULL);
  assert (lp_media_get_time (media) == 0);
  
  lp_event_init_start(&initevt);
  lp_media_post (media, &initevt);

  sleep (1);
  assert (lp_media_get_time (media) > 0);
  
  lp_media_destroy (media);
  exit (EXIT_SUCCESS);
}
