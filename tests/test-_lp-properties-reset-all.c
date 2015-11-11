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

  /* no-op: NULL pointer */
  _lp_properties_reset_all (NULL);

  /* success */
  props = _lp_properties_alloc ();
  ASSERT (props != NULL);

  _lp_properties_reset_all (props);

  ASSERT (_lp_properties_get (props, "width", &value));
  ASSERT (G_VALUE_TYPE (&value) == G_TYPE_INT);
  ASSERT (g_value_get_int (&value) == LP_PROPERTY_DEFAULT_WIDTH);
  g_value_unset (&value);

  ASSERT (_lp_properties_get (props, "height", &value));
  ASSERT (G_VALUE_TYPE (&value) == G_TYPE_INT);
  ASSERT (g_value_get_int (&value) == LP_PROPERTY_DEFAULT_HEIGHT);
  g_value_unset (&value);

  _lp_properties_free (props);

  exit (EXIT_SUCCESS);
}
