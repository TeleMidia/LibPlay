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
  lp_media_t *m;
  void *p;

  m = lp_media_create (NULL);
  ASSERT (m != NULL);

  ASSERT (!lp_media_get_property_pointer (m, "p", NULL));

  ASSERT (lp_media_set_property_pointer (m, "p", pointerof (main)));
  ASSERT (lp_media_get_property_pointer (m, "p", &p));
  ASSERT (p == pointerof (main));

  ASSERT (!lp_media_get_property_int (m, "p", NULL));

  lp_media_destroy (m);
  _lp_media_destroy_default_parent ();

  exit (EXIT_SUCCESS);
}