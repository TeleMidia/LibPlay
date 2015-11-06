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

/* *INDENT-OFF* */
PRAGMA_DIAG_IGNORE (-Wfloat-equal)
/* *INDENT-ON* */

int
main (void)
{
  lp_value_t value;

  lp_value_init_double (&value, 0.0);
  ASSERT (value.type == LP_VALUE_DOUBLE);
  ASSERT (value.u.d == 0.0);

  lp_value_init_double (&value, G_MINDOUBLE);
  ASSERT (value.type == LP_VALUE_DOUBLE);
  ASSERT (value.u.d == G_MINDOUBLE);

  exit (EXIT_SUCCESS);
}
