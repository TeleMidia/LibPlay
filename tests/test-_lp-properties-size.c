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
  ASSERT (_lp_properties_size (NULL) == 0);

  /* success */
  props = _lp_properties_alloc ();
  ASSERT (props != NULL);
  ASSERT (_lp_properties_size (props) == 0);

  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, 0);
  ASSERT (_lp_properties_set (props, "i", &value));
  g_value_unset (&value);

  ASSERT (_lp_properties_size (props) == 1);
  _lp_properties_free (props);

  exit (EXIT_SUCCESS);
}
