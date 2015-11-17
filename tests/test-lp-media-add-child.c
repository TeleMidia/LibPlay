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
  lp_media_t *child;

  lp_media_t *P;
  lp_media_t *p1;
  lp_media_t *p2;
  lp_media_t *c1;
  lp_media_t *c2;
  lp_media_t *c3;

  /* no-op: NULL parent */
  child = lp_media_create (NULL);
  assert (child != NULL);
  assert (lp_media_add_child (NULL, child) == FALSE);
  lp_media_destroy (child);

  /* no-op: NULL child */
  parent = lp_media_create (NULL);
  assert (parent != NULL);
  assert (lp_media_add_child (parent, NULL) == FALSE);
  lp_media_destroy (parent);

  /* no-op: invalid parent */
  parent = lp_media_create_for_parent (NULL, NULL);
  assert (parent != NULL);
  child = lp_media_create (NULL);
  assert (child != NULL);
  assert (lp_media_add_child (parent, child) == FALSE);
  lp_media_destroy (parent);
  lp_media_destroy (child);

  /* no-op: invalid child */
  child = lp_media_create_for_parent (NULL, NULL);
  assert (child != NULL);
  parent = lp_media_create (NULL);
  assert (parent != NULL);
  assert (lp_media_add_child (parent, child) == FALSE);
  lp_media_destroy (parent);
  lp_media_destroy (child);

  /* no-op: parent == child */
  parent = lp_media_create (NULL);
  assert (parent != NULL);
  assert (lp_media_add_child (parent, parent) == FALSE);
  lp_media_destroy (parent);

  /* no-op: child->parent != NULL  */
  p1 = lp_media_create (NULL);
  assert (p1 != NULL);
  c1 = lp_media_create_for_parent (p1, NULL);
  assert (c1 != NULL);

  p2 = lp_media_create (NULL);
  assert (p2 != NULL);
  assert (lp_media_add_child (p2, c1) == FALSE);
  lp_media_destroy (p1);
  lp_media_destroy (p2);

  /* no-op: child is already in parent's children list */
  parent = lp_media_create (NULL);
  assert (parent != NULL);
  child = lp_media_create_for_parent (parent, NULL);
  assert (child != NULL);
  assert (lp_media_add_child (parent, child) == FALSE);
  lp_media_destroy (parent);

  /* success */
  P = lp_media_create (NULL);
  assert (P != NULL);
  p1 = lp_media_create (NULL);
  assert (p1 != NULL);
  p2 = lp_media_create (NULL);
  assert (p2 != NULL);
  c1 = lp_media_create (NULL);
  assert (c1 != NULL);
  c2 = lp_media_create (NULL);
  assert (c2 != NULL);
  c3 = lp_media_create (NULL);
  assert (c3 != NULL);

  assert (lp_media_get_parent (P) == NULL);

  assert (lp_media_add_child (P, p1));
  assert (lp_media_get_parent (p1) == P);

  assert (lp_media_add_child (P, p2));
  assert (lp_media_get_parent (p2) == P);

  assert (lp_media_add_child (p1, c1));
  assert (lp_media_get_parent (c1) == p1);

  assert (lp_media_add_child (p1, c2));
  assert (lp_media_get_parent (c2) == p1);

  assert (lp_media_add_child (p2, c3));
  assert (lp_media_get_parent (c3) == p2);

  lp_media_destroy (P);

  exit (EXIT_SUCCESS);
}
