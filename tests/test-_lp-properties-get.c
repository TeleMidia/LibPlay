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
  lp_properties_t *m1;
  lp_properties_t *m2;
  lp_properties_t *props;
  GValue value = G_VALUE_INIT;

  /* success: known property, with default */
  props = _lp_properties_alloc ();
  assert (props != NULL);

  assert (_lp_properties_get (props, "x", &value));
  assert (g_value_get_int (&value) == LP_PROPERTY_DEFAULT_X);
  g_value_unset (&value);
  _lp_properties_free (props);

  /* success: known property, inherited */
  m1 = _lp_properties_alloc ();
  assert (m1 != NULL);
  m2 = _lp_properties_alloc ();
  assert (m2 != NULL);
  props = _lp_properties_alloc ();
  assert (props != NULL);

  assert (_lp_properties_set_metatable (m2, m1) == NULL);
  assert (_lp_properties_set_metatable (props, m2) == NULL);

  assert (_lp_properties_get (props, "width", &value) == FALSE);

  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, 237);
  assert (_lp_properties_set (m1, "width", &value));
  g_value_unset (&value);

  assert (_lp_properties_get (props, "width", &value));
  assert (g_value_get_int (&value) == 237);
  g_value_unset (&value);

  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, 1981);
  assert (_lp_properties_set (m2, "width", &value));
  g_value_unset (&value);

  assert (_lp_properties_get (props, "width", &value));
  assert (g_value_get_int (&value) == 1981);
  g_value_unset (&value);

  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, -33);
  assert (_lp_properties_set (props, "width", &value));
  g_value_unset (&value);

  assert (_lp_properties_get (props, "width", &value));
  assert (g_value_get_int (&value) == -33);
  g_value_unset (&value);

  _lp_properties_free (m1);
  _lp_properties_free (m2);
  _lp_properties_free (props);

  /* success: unknown property */
  props = _lp_properties_alloc ();
  assert (props != NULL);
  assert (_lp_properties_get (props, "foo", &value) == FALSE);
  _lp_properties_free (props);

  props = _lp_properties_alloc ();
  assert (props != NULL);

  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, "bar");
  assert (_lp_properties_set (props, "foo", &value));
  g_value_unset (&value);

  assert (_lp_properties_get (props, "foo", &value));
  assert (streq (g_value_get_string (&value), "bar"));
  g_value_unset (&value);

  _lp_properties_free (props);

  exit (EXIT_SUCCESS);
}
