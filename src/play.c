/* play.c -- Simple media player.
   Copyright (C) 2015 PUC-Rio/Laboratorio TeleMidia

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

#include <config.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "play.h"
#include "macros.h"

/* *INDENT-OFF* */
PRAGMA_DIAG_PUSH ()
PRAGMA_DIAG_IGNORE (-Wvariadic-macros)
#include <glib.h>
PRAGMA_DIAG_POP ()
/* *INDENT-ON* */

static GMainLoop *loop;
static int count = 0;

static lp_bool_t
handler (lp_media_t *media, lp_media_t *target, lp_event_t *event)
{
  assert (media != NULL);
  assert (target != NULL);
  assert (event != NULL);

  printf ("%p, %p, %p:%s\n",
          (void *) media,
          (void *) target,
          (void *) event,
          (event->type == LP_EVENT_START) ? "start"
          : (event->type == LP_EVENT_STOP) ? "stop"
          : (event->type == LP_EVENT_USER) ? "user"
          : "unknown");

  switch (event->type)
    {
    case LP_EVENT_START:
      break;
    case LP_EVENT_STOP:
      g_main_loop_quit (loop);
      break;
    case LP_EVENT_TICK:
      if (++count > 4)
        {
          lp_event_init_stop (event);
          assert (lp_media_post (media, event));
        }
      break;
    default:
      ASSERT_NOT_REACHED;
    }

  return TRUE;                  /* consume event */
}

int
main (int argc, char **argv)
{
  lp_media_t *media;
  lp_event_t start;

  if (argc != 2)
    {
      fprintf (stderr, "usage: %s URI\n", argv[0]);
      exit (EXIT_FAILURE);
    }

  loop = g_main_loop_new (NULL, FALSE);
  assert (loop != NULL);

  media = lp_media_create (argv[1]);
  assert (media != NULL);
  assert (lp_media_register (media, handler));
  lp_event_init_start (&start);
  assert (lp_media_post (media, &start));

  g_main_loop_run (loop);

  lp_media_destroy (media);
  g_main_loop_unref (loop);

  exit (EXIT_SUCCESS);
}