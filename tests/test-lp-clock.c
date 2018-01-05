/* Copyright (C) 2015-2018 PUC-Rio/Laboratorio TeleMidia

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
  GstClock *clock;
  gboolean lockstep = TRUE;

  clock = GST_CLOCK (g_object_new (LP_TYPE_CLOCK, NULL));
  g_assert_nonnull (clock);

  g_object_get (clock, "lockstep", &lockstep, NULL);
  g_assert (!lockstep);

  g_object_set (clock, "lockstep", TRUE, NULL);
  g_object_get (clock, "lockstep", &lockstep, NULL);
  g_assert (lockstep);

  lockstep = FALSE;
  g_object_set (clock, "lockstep", TRUE, NULL); /* vacuous set */
  g_object_get (clock, "lockstep", &lockstep, NULL);
  g_assert (lockstep);

  g_object_unref (clock);

  exit (EXIT_SUCCESS);
}
