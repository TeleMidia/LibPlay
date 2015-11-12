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
  lp_properties_t *props;
  GValue value = G_VALUE_INIT;

  /* no-op: wrong type for known property */
  props = _lp_properties_alloc ();
  assert (props != NULL);
  _lp_properties_reset_all (props);

  g_value_init (&value, G_TYPE_DOUBLE);
  g_value_set_double (&value, 13.37);
  assert (_lp_properties_set (props, "x", &value) == FALSE);
  assert (_lp_properties_set (props, "x", &value) == FALSE);
  g_value_unset (&value);
  _lp_properties_free (props);

  /* success: known property  */
  props = _lp_properties_alloc ();
  assert (props != NULL);
  _lp_properties_reset_all (props);

  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, 50);
  assert (_lp_properties_set (props, "x", &value));
  g_value_unset (&value);

  assert (_lp_properties_get (props, "x", &value));
  assert (g_value_get_int (&value) == 50);

  g_value_set_int (&value, 80);
  assert (_lp_properties_set (props, "y", &value));
  g_value_unset (&value);

  assert (_lp_properties_get (props, "y", &value));
  assert (g_value_get_int (&value) == 80);
  g_value_unset (&value);
  _lp_properties_free (props);

  /* success: unknown property */
  props = _lp_properties_alloc ();
  assert (props != NULL);

  g_value_init (&value, G_TYPE_POINTER);
  g_value_set_pointer (&value, pointerof (main));
  assert (_lp_properties_set (props, "main", &value));
  g_value_unset (&value);

  assert (_lp_properties_get (props, "main", &value));
  assert (g_value_get_pointer (&value) == pointerof (main));
  g_value_unset (&value);
  _lp_properties_free (props);

  exit (EXIT_SUCCESS);
}
