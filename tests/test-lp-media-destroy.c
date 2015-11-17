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
  lp_media_t *media;

  /* no-op: NULL media */
  lp_media_destroy (NULL);

  /* no-op: invalid media */
  media = lp_media_create_for_parent (NULL, NULL);
  assert (media != NULL);
  lp_media_destroy (media);

  /* TODO: no-op: invalid ref_count */

  /* success: single reference */
  media = lp_media_create (NULL);
  assert (media != NULL);
  lp_media_destroy (media);

  /* success: multiple references */
  media = lp_media_create (NULL);
  assert (media != NULL);
  assert (lp_media_reference (media) == media);
  assert (lp_media_reference (media) == media);
  lp_media_destroy (media);
  lp_media_destroy (media);
  lp_media_destroy (media);

  exit (EXIT_SUCCESS);
}
