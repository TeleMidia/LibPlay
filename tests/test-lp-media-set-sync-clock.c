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
  lp_sync_clock_t *clock;
  lp_event_t initevt;
  
  assert (lp_media_set_sync_clock (NULL, NULL) == FALSE);
  
  media = lp_media_create (NULL);
  clock = lp_sync_clock_create ();

  assert (media != NULL);
  assert (clock != NULL);
  
  assert (lp_media_set_sync_clock (media, NULL) == FALSE);
  assert (lp_media_set_sync_clock (NULL, clock) == FALSE);
  assert (lp_media_set_sync_clock (media, clock));

  lp_event_init_start(&initevt);
  lp_media_post (media, &initevt);

  lp_sync_clock_tick_for (clock, 1000);
  assert (lp_media_get_time (media) == 1000);
  
  lp_sync_clock_tick_for (clock, 2000);
  assert (lp_media_get_time (media) == 3000);
  
  lp_sync_clock_advance_time (clock, 1000);
  assert (lp_media_get_time (media) == 4000);
  
  sleep (1);
  assert (lp_media_get_time (media) == 4000);

  lp_media_destroy (media);
  lp_sync_clock_destroy (clock);

  exit (EXIT_SUCCESS);
}
