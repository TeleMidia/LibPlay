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
  void *p;
  char *s;

  /* no-op: NULL media */
  assert (lp_media_get_property_string (NULL, "s", &s) == FALSE);

  /* no-op: invalid media */
  media = lp_media_create_for_parent (NULL, NULL);
  assert (media != NULL);
  assert (lp_media_get_property_string (media, "s", &s) == FALSE);
  lp_media_destroy (media);

  /* no-op: NULL name */
  media = lp_media_create (NULL);
  assert (media != NULL);
  assert (lp_media_get_property_string (media, NULL, &s) == FALSE);
  lp_media_destroy (media);

  /* no-op: unset property, unknown */
  media = lp_media_create (NULL);
  assert (media != NULL);
  assert (lp_media_get_property_string (media, "unknown", &s) == FALSE);
  lp_media_destroy (media);

  /* TODO: success: unset property, with default */
  /* TODO: success: unset property, inherited */

  /* success: set property */
  media = lp_media_create (NULL);
  assert (media != NULL);
  assert (lp_media_set_property_string (media, "s", "russell"));
  assert (lp_media_get_property_pointer (media, "s", &p) == FALSE);
  assert (lp_media_get_property_string (media, "s", &s));
  assert (streq (s, "russell"));
  free (s);
  lp_media_destroy (media);

  exit (EXIT_SUCCESS);
}
