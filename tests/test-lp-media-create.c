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
  lp_media_t *parent;

  m = lp_media_create ("xyz");
  ASSERT (m != NULL);

  parent = _lp_media_get_default_parent ();
  ASSERT (parent != NULL);

  ASSERT (m->parent == parent);
  ASSERT (m->refcount == 1);
  ASSERT (streq (m->uri, "xyz"));
  ASSERT (m->properties != NULL);

  lp_media_destroy (m);
  lp_media_destroy (parent);

  exit (EXIT_SUCCESS);
}
