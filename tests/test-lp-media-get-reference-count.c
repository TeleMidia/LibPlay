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
  ASSERT (lp_media_get_reference_count (NULL) == 0);

  /* no-op: invalid media */
  media = lp_media_create_for_parent (NULL, NULL);
  ASSERT (media != NULL);
  ASSERT (lp_media_get_reference_count (NULL) == 0);
  lp_media_destroy (NULL);

  /* success */
  media = lp_media_create (NULL);
  ASSERT_MEDIA_IS_EMPTY (media, NULL);
  ASSERT (media->ref_count == 1);
  ASSERT (lp_media_get_reference_count (media) == 1);

  ASSERT (lp_media_reference (media) == media);
  ASSERT (media->ref_count == 2);
  ASSERT (lp_media_get_reference_count (media) == 2);

  lp_media_destroy (media);
  ASSERT (media->ref_count == 1);
  ASSERT (lp_media_get_reference_count (media) == 1);

  lp_media_destroy (media);

  exit (EXIT_SUCCESS);
}
