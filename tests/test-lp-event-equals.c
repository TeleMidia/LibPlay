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
  lp_event_t e1;
  lp_event_t e2;

  lp_event_init_start (&e1);
  lp_event_init_stop (&e2);
  ASSERT (lp_event_equals (&e1, &e2) == FALSE);

  lp_event_init_start (&e2);
  ASSERT (lp_event_equals (&e1, &e2));

  exit (EXIT_SUCCESS);
}
