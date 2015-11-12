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
  ASSERT_MEDIA_IS_EMPTY (child, NULL);
  ASSERT (lp_media_remove_child (NULL, child) == FALSE);
  lp_media_destroy (child);

  /* no-op: invalid parent */
  parent = lp_media_create_for_parent (NULL, NULL);
  ASSERT (parent != NULL);
  child = lp_media_create (NULL);
  ASSERT_MEDIA_IS_EMPTY (child, NULL);
  ASSERT (lp_media_remove_child (parent, child) == FALSE);
  lp_media_destroy (parent);
  lp_media_destroy (child);

  /* no-op: child->parent != parent  */
  parent = lp_media_create (NULL);
  ASSERT_MEDIA_IS_EMPTY (parent, NULL);
  ASSERT (lp_media_remove_child (parent, parent) == FALSE);
  lp_media_destroy (parent);

  /* no-op: child is not in parent's children list */
  parent = lp_media_create (NULL);
  ASSERT_MEDIA_IS_EMPTY (parent, NULL);
  parent->parent = parent;
  ASSERT (lp_media_remove_child (parent, parent) == FALSE);
  parent->parent = NULL;
  lp_media_destroy (parent);

  /* success */
  P = lp_media_create (NULL);
  ASSERT_MEDIA_IS_EMPTY (P, NULL);
  p1 = lp_media_create_for_parent (P, NULL);
  ASSERT (p1 != NULL);
  p2 = lp_media_create_for_parent (P, NULL);
  ASSERT (p2 != NULL);
  c1 = lp_media_create_for_parent (p1, NULL);
  ASSERT (c1 != NULL);
  c2 = lp_media_create_for_parent (p1, NULL);
  ASSERT (c2 != NULL);
  c3 = lp_media_create_for_parent (p2, NULL);
  ASSERT (c3 != NULL);

  ASSERT (lp_media_remove_child (P, p1));
  ASSERT (lp_media_remove_child (p2, c3));

  ASSERT (lp_media_get_parent (P) == NULL);
  ASSERT (lp_media_get_parent (p1) == NULL);
  ASSERT (lp_media_get_parent (c3) == NULL);

  ASSERT (g_list_length (P->children) == 1);
  ASSERT (g_list_length (p1->children) == 2);
  ASSERT (g_list_length (p2->children) == 0);
  ASSERT (g_list_length (c1->children) == 0);
  ASSERT (g_list_length (c2->children) == 0);
  ASSERT (g_list_length (c3->children) == 0);

  lp_media_destroy (P);
  lp_media_destroy (p1);
  lp_media_destroy (c3);

  exit (EXIT_SUCCESS);
}
