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
  lp_media_t *parent;
  lp_media_t *m1;
  lp_media_t *m2;
  lp_media_t *m3;

  /* no-op: NULL parent */
  m1 = lp_media_create_for_parent (NULL, NULL);
  assert (lp_media_status (m1) == LP_STATUS_NULL_POINTER);
  lp_media_destroy (m1);

  /* no-op: invalid parent */
  parent = lp_media_create_for_parent (NULL, NULL);
  assert (lp_media_status (parent) == LP_STATUS_NULL_POINTER);

  m1 = lp_media_create_for_parent (parent, NULL);
  assert (lp_media_status (m1) == LP_STATUS_INVALID_PARENT);

  lp_media_destroy (parent);
  lp_media_destroy (m1);

  /* success */
  parent = lp_media_create (NULL);
  assert (lp_media_status (parent) == LP_STATUS_SUCCESS);
  assert (parent->parent == NULL);

  m1 = lp_media_create_for_parent (parent, "m1");
  assert (lp_media_status (m1) == LP_STATUS_SUCCESS);
  assert (m1->parent == parent);

  m2 = lp_media_create_for_parent (NULL, "m2");
  assert (lp_media_status (m2) == LP_STATUS_NULL_POINTER);
  lp_media_destroy (m2);

  m3 = lp_media_create_for_parent (parent, NULL);
  assert (lp_media_status (m1) == LP_STATUS_SUCCESS);
  assert (m1->parent == parent);

  assert (g_list_find (parent->children, m1) != NULL);
  assert (g_list_find (parent->children, m3) != NULL);
  lp_media_destroy (parent);

  exit (EXIT_SUCCESS);
}
