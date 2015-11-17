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
  lp_media_t *m1;
  lp_media_t *m2;
  lp_media_t *m3;

  m1 = lp_media_create (NULL);
  assert (m1 != NULL);
  assert (lp_media_get_parent (m1) == NULL);
  assert (_lp_media_get_root_ancestor (m1) == m1);

  m2 = lp_media_create (NULL);
  assert (m2 != NULL);
  assert (_lp_media_get_root_ancestor (m2) == m2);
  assert (lp_media_add_child (m1, m2));
  assert (lp_media_get_parent (m2) == m1);
  assert (_lp_media_get_root_ancestor (m1) == m1);
  assert (_lp_media_get_root_ancestor (m2) == m1);

  m3 = lp_media_create_for_parent (m2, NULL);
  assert (m3 != NULL);
  assert (lp_media_get_parent (m3) == m2);
  assert (_lp_media_get_root_ancestor (m1) == m1);
  assert (_lp_media_get_root_ancestor (m2) == m1);
  assert (_lp_media_get_root_ancestor (m3) == m1);

  lp_media_destroy (m1);

  exit (EXIT_SUCCESS);
}
