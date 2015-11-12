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

  /* no-op: invalid media */
  media = lp_media_create_for_parent (NULL, NULL);
  assert (media != NULL);
  assert (lp_media_status (media) == LP_STATUS_NULL_POINTER);
  lp_media_destroy (media);

  /* success */
  media = lp_media_create (NULL);
  assert_media_is_empty (media, NULL);
  assert (lp_media_status (media) == LP_STATUS_SUCCESS);
  lp_media_destroy (media);

  exit (EXIT_SUCCESS);
}
