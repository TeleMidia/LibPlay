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
  lp_properties_t *meta;

  props = _lp_properties_alloc ();
  assert (props != NULL);
  assert (_lp_properties_get_metatable (props) == NULL);

  meta = _lp_properties_alloc ();
  assert (meta != NULL);
  assert (_lp_properties_get_metatable (meta) == NULL);
  assert (_lp_properties_set_metatable (props, meta) == NULL);
  assert (_lp_properties_get_metatable (props) == meta);
  assert (_lp_properties_get_metatable (meta) == NULL);

  assert (_lp_properties_set_metatable (props, NULL) == meta);
  assert (_lp_properties_get_metatable (props) == NULL);
  assert (_lp_properties_get_metatable (meta) == NULL);

  _lp_properties_free (props);
  _lp_properties_free (meta);

  exit (EXIT_SUCCESS);
}
