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
  lp_value_t v1;
  lp_value_t v2;

  /* bad type */
  lp_value_init_int (&v1, 0);
  lp_value_init_double (&v2, 0.0);
  ASSERT (lp_value_equals (&v1, &v2) == FALSE);

  /* int */
  lp_value_init_int (&v2, 1);
  ASSERT (lp_value_equals (&v1, &v2) == FALSE);

  lp_value_init_int (&v2, 0);
  ASSERT (lp_value_equals (&v1, &v2) == TRUE);

  /* double */
  lp_value_init_double (&v1, 0.0);
  lp_value_init_double (&v2, 1.0);
  ASSERT (lp_value_equals (&v1, &v2) == FALSE);

  lp_value_init_double (&v2, 0.0);
  ASSERT (lp_value_equals (&v1, &v2) == TRUE);

  /* string */
  lp_value_init_string (&v1, NULL);
  lp_value_init_string (&v2, "");
  ASSERT (lp_value_equals (&v1, &v2) == FALSE);

  lp_value_init_string (&v2, NULL);
  ASSERT (lp_value_equals (&v1, &v2) == TRUE);

  lp_value_init_string (&v1, "");
  ASSERT (lp_value_equals (&v1, &v2) == FALSE);

  lp_value_init_string (&v2, "x");
  ASSERT (lp_value_equals (&v1, &v2) == FALSE);

  lp_value_init_string (&v1, "x");
  ASSERT (lp_value_equals (&v1, &v2) == TRUE);

  exit (EXIT_SUCCESS);
}
