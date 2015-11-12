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

  /* no-op: NULL props */
  ASSERT (_lp_properties_set (NULL, NULL, NULL) == FALSE);

  /* no-op: NULL name */
  props = _lp_properties_alloc ();
  ASSERT (props != NULL);
  ASSERT (_lp_properties_set (props, NULL, NULL) == FALSE);
  _lp_properties_free (props);

  /* no-op: NULL value */
  props = _lp_properties_alloc ();
  ASSERT (props != NULL);
  ASSERT (_lp_properties_set (props, "@unknown", NULL) == FALSE);
  _lp_properties_free (props);

  /* no-op: wrong type for known property */
  props = _lp_properties_alloc ();
  ASSERT (props != NULL);
  _lp_properties_reset_all (props);

  g_value_init (&value, G_TYPE_DOUBLE);
  g_value_set_double (&value, 13.37);
  ASSERT (_lp_properties_set (props, "width", &value) == FALSE);
  ASSERT (_lp_properties_set (props, "height", &value) == FALSE);
  g_value_unset (&value);
  _lp_properties_free (props);

  /* success: known property  */
  props = _lp_properties_alloc ();
  ASSERT (props != NULL);
  _lp_properties_reset_all (props);

  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, 1920);
  ASSERT (_lp_properties_set (props, "width", &value));
  g_value_unset (&value);

  ASSERT (_lp_properties_get (props, "width", &value));
  ASSERT (g_value_get_int (&value) == 1920);

  g_value_set_int (&value, 1080);
  ASSERT (_lp_properties_set (props, "height", &value));
  g_value_unset (&value);

  ASSERT (_lp_properties_get (props, "height", &value));
  ASSERT (g_value_get_int (&value) == 1080);
  g_value_unset (&value);
  _lp_properties_free (props);

  /* success: unknown property */
  props = _lp_properties_alloc ();
  ASSERT (props != NULL);

  g_value_init (&value, G_TYPE_POINTER);
  g_value_set_pointer (&value, pointerof (main));
  ASSERT (_lp_properties_set (props, "main", &value));
  g_value_unset (&value);

  ASSERT (_lp_properties_get (props, "main", &value));
  ASSERT (g_value_get_pointer (&value) == pointerof (main));
  g_value_unset (&value);
  _lp_properties_free (props);

  exit (EXIT_SUCCESS);
}
