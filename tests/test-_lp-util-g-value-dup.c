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
  GValue *v1;
  GValue *v2;

  /* no-op: NULL pointer */
  v1 = _lp_util_g_value_dup (NULL);
  _lp_util_g_value_free (v1);

  /* success */
  v1 = _lp_util_g_value_alloc (G_TYPE_STRING);
  assert (G_VALUE_TYPE (v1) == G_TYPE_STRING);
  g_value_set_string (v1, "abc");

  v2 = _lp_util_g_value_dup (v1);
  ASSERT (v2 != NULL);
  ASSERT (G_VALUE_TYPE (v1) == G_VALUE_TYPE (v2));
  ASSERT (streq (g_value_get_string (v1), g_value_get_string (v2)));

  _lp_util_g_value_free (v1);
  _lp_util_g_value_free (v2);

  exit (EXIT_SUCCESS);
}
