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

static int
h1 (arg_unused (lp_media_t *m), arg_unused (lp_event_t *e))
{
  return TRUE;
}

static int
h2 (arg_unused (lp_media_t *m), arg_unused (lp_event_t *e))
{
  return TRUE;
}

static int
h3 (arg_unused (lp_media_t *m), arg_unused (lp_event_t *e))
{
  return TRUE;
}

int
main (void)
{
  lp_media_t *media;

  /* no-op: NULL media */
  assert (lp_media_unregister (NULL, h1) == FALSE);

  /* no-op: invalid media */
  media = lp_media_create_for_parent (NULL, NULL);
  assert (media != NULL);
  assert (lp_media_unregister (media, h1) == FALSE);
  lp_media_destroy (media);

  /* no-op: NULL func */
  media = lp_media_create (NULL);
  assert_media_is_empty (media, NULL);
  assert (lp_media_unregister (media, NULL) == FALSE);
  lp_media_destroy (media);

  /* success */
  media = lp_media_create (NULL);
  assert_media_is_empty (media, NULL);
  assert (g_list_length (media->handlers) == 0);

  assert (lp_media_register (media, h1));
  assert (lp_media_register (media, h2));
  assert (lp_media_register (media, h3));
  assert (g_list_length (media->handlers) == 3);

  assert (lp_media_unregister (media, h2));
  assert (lp_media_unregister (media, h2) == FALSE);
  assert (g_list_length (media->handlers) == 2);

  assert (g_list_nth_data (media->handlers, 0) == pointerof (h1));
  assert (g_list_nth_data (media->handlers, 1) == pointerof (h3));

  assert (lp_media_unregister (media, h1));
  assert (lp_media_unregister (media, h1) == FALSE);
  assert (g_list_length (media->handlers) == 1);

  assert (g_list_nth_data (media->handlers, 0) == pointerof (h3));

  assert (lp_media_unregister (media, h3));
  assert (lp_media_unregister (media, h3) == FALSE);
  assert (g_list_length (media->handlers) == 0);

  assert (lp_media_register (media, h3));
  assert (lp_media_register (media, h2));
  assert (lp_media_register (media, h1));
  assert (g_list_length (media->handlers) == 3);

  assert (lp_media_unregister (media, h3));
  assert (lp_media_unregister (media, h3) == FALSE);
  assert (g_list_length (media->handlers) == 2);

  assert (g_list_nth_data (media->handlers, 0) == pointerof (h2));
  assert (g_list_nth_data (media->handlers, 1) == pointerof (h1));
  lp_media_destroy (media);

  exit (EXIT_SUCCESS);
}
