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

#define DEFINE_HANDLER(name)                                            \
  static lp_media_t *name##_media = NULL;                               \
  static lp_media_t *name##_target = NULL;                              \
  static lp_event_t *name##_event = NULL;                               \
  static int name##_return = FALSE;                                     \
  static unsigned int name##_count = 0;                                 \
  static ATTR_UNUSED void                                               \
  name##_reset (void)                                                   \
  {                                                                     \
    name##_media = NULL;                                                \
    name##_target = NULL;                                               \
    name##_event = NULL;                                                \
    name##_return = FALSE;                                              \
    name##_count = 0;                                                   \
  }                                                                     \
  static ATTR_UNUSED lp_bool_t                                          \
  name (lp_media_t *media, lp_media_t *target, lp_event_t *event)       \
  {                                                                     \
    assert (_lp_media_is_valid (media));                                \
    assert (_lp_media_is_valid (target));                               \
    assert (event != NULL);                                             \
    name##_media = media;                                               \
    name##_target = target;                                             \
    name##_event = event;                                               \
    name##_count++;                                                     \
    return name##_return++;                                             \
  }

/* *INDENT-OFF* */
DEFINE_HANDLER (h1)
DEFINE_HANDLER (h2)
DEFINE_HANDLER (h3)
/* INDENT-ON* */

int
main (void)
{
  lp_media_t *media;
  lp_event_t event;

  lp_media_t *m1;
  lp_media_t *m2;
  lp_media_t *m3;

  /* success: NULL parent */
  media = lp_media_create (NULL);
  assert_media_is_empty (media, NULL);
  assert (lp_media_register (media, h1));
  assert (lp_media_register (media, h2));
  assert (lp_media_register (media, h3));

  lp_event_init_start (&event);
  assert (_lp_media_dispatch (media, &event) == 3);
  assert (h1_count == 1);
  assert (h2_count == 1);
  assert (h3_count == 1);
  assert (h1_media == media);
  assert (h2_media == media);
  assert (h3_media == media);
  assert (h1_target == media);
  assert (h2_target == media);
  assert (h3_target == media);
  assert (h1_event == &event);
  assert (h2_event == &event);
  assert (h3_event == &event);
  h1_reset ();
  h2_reset ();
  h3_reset ();

  h1_return = TRUE;
  assert (_lp_media_dispatch (media, &event) == 1);
  assert (h1_count == 1);
  assert (h2_count == 0);
  assert (h3_count == 0);
  assert (h1_media == media);
  assert (h2_media == NULL);
  assert (h3_media == NULL);
  assert (h1_target == media);
  assert (h2_target == NULL);
  assert (h3_target == NULL);
  assert (h1_event == &event);
  assert (h2_event == NULL);
  assert (h3_event == NULL);
  h1_reset ();
  h2_reset ();
  h3_reset ();
  lp_media_destroy (media);

  /* success: three-level hierarchy */
  m1 = lp_media_create (NULL);
  assert_media_is_empty (m1, NULL);
  m2 = lp_media_create_for_parent (m1, NULL);
  assert (m2 != NULL);
  m3 = lp_media_create_for_parent (m2, NULL);
  assert (m3 != NULL);

  assert (lp_media_register (m1, h1));
  assert (lp_media_register (m2, h2));
  assert (lp_media_register (m3, h3));

  lp_event_init_start (&event);
  assert (_lp_media_dispatch (m3, &event) == 3);
  assert (h1_count == 1);
  assert (h2_count == 1);
  assert (h3_count == 1);
  assert (h1_media == m1);
  assert (h2_media == m2);
  assert (h3_media == m3);
  assert (h1_target == m3);
  assert (h2_target == m3);
  assert (h3_target == m3);
  assert (h1_event == &event);
  assert (h2_event == &event);
  assert (h3_event == &event);
  h1_reset ();
  h2_reset ();
  h3_reset ();

  h2_return = TRUE;
  assert (_lp_media_dispatch (m3, &event) == 2);
  assert (h1_count == 0);
  assert (h2_count == 1);
  assert (h3_count == 1);
  assert (h1_media == NULL);
  assert (h2_media == m2);
  assert (h3_media == m3);
  assert (h1_target == NULL);
  assert (h2_target == m3);
  assert (h3_target == m3);
  assert (h1_event == NULL);
  assert (h2_event == &event);
  assert (h3_event == &event);
  h1_reset ();
  h2_reset ();
  h3_reset ();

  lp_media_destroy (m1);

  exit (EXIT_SUCCESS);
}
