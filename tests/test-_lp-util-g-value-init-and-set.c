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
  GValue value = G_VALUE_INIT;
  int i = 10;
  double d = 13.37;
  const char *s = "s";
  void *p = pointerof (main);

  assert (_lp_util_g_value_init_and_set (&value, G_TYPE_INT, &i) == &value);
  assert (g_value_get_int (&value) == i);
  g_value_unset (&value);

  assert (_lp_util_g_value_init_and_set
          (&value, G_TYPE_DOUBLE, &d) == &value);
  assert (g_value_get_double (&value) == d);
  g_value_unset (&value);

  assert (_lp_util_g_value_init_and_set
          (&value, G_TYPE_STRING, &s) == &value);
  assert (streq (g_value_get_string (&value), s));
  g_value_unset (&value);

  assert (_lp_util_g_value_init_and_set
          (&value, G_TYPE_POINTER, &p) == &value);
  assert (g_value_get_pointer (&value) == p);
  g_value_unset (&value);

  exit (EXIT_SUCCESS);
}
