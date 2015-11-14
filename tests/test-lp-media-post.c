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

static unsigned int counter = 0;
static int
handler (lp_media_t *media, lp_media_t *target, lp_event_t *event)
{
  assert (media != NULL);
  assert (target != NULL);
  assert (event != NULL);
  assert (event->type == LP_EVENT_USER);
  counter++;
  return FALSE;
}

int
main (void)
{
  lp_media_t *media;
  lp_event_t event;

  /* no-op: NULL media */
  lp_event_init_start (&event);
  assert (lp_media_post (NULL, &event) == FALSE);

  /* no-op: invalid media */
  media = lp_media_create_for_parent (NULL, NULL);
  assert (media != NULL);
  lp_event_init_start (&event);
  assert (lp_media_post (media, &event) == FALSE);
  lp_media_destroy (media);

  /* no-op: NULL event */
  media = lp_media_create (NULL);
  assert (media != NULL);
  assert (lp_media_post (media, NULL) == FALSE);
  lp_media_destroy (media);

  /* success: user event */
  media = lp_media_create (NULL);
  assert (media != NULL);

  lp_event_init_user (&event);
  assert (counter == 0);

  assert (lp_media_post (media, &event));
  assert (counter == 0);

  assert (lp_media_register (media, handler));
  assert (lp_media_post (media, &event));
  assert (counter == 1);

  assert (lp_media_post (media, &event));
  assert (counter == 2);

  assert (lp_media_unregister (media, handler));
  assert (lp_media_post (media, &event));
  assert (counter == 2);

  lp_media_destroy (media);

  exit (EXIT_SUCCESS);
}
