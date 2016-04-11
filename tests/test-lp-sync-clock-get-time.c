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
  lp_sync_clock_t *clock;
  
  assert (lp_sync_clock_get_time (NULL) == 0);

  clock = lp_sync_clock_create ();
  assert (clock != NULL);
  
  assert (lp_sync_clock_get_time (clock) == 0);

  assert (lp_sync_clock_advance_time (clock, 100));
  assert (lp_sync_clock_get_time (clock) == 100);

  sleep (1);
  assert (lp_sync_clock_get_time (clock) == 100);

  lp_sync_clock_tick_for (clock, 100);
  assert (lp_sync_clock_get_time (clock) == 200);
  
  assert (lp_sync_clock_advance_time (clock, 100));
  assert (lp_sync_clock_get_time (clock) == 300);

  lp_sync_clock_destroy (clock);

  exit (EXIT_SUCCESS);
}
