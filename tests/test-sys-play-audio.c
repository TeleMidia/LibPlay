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
#include <unistd.h>
#define BUFFSIZE 256

static GMainLoop *loop;
static unsigned int total_runs = 0;
static unsigned int total_ticks = 0;

static lp_bool_t
handler (lp_media_t *media, lp_media_t *target, lp_event_t *event)
{
  assert (media != NULL);
  assert (target != NULL);
  assert (event != NULL);

  printf ("media=%p\ttarget=%p\tevent=%p:%s\n",
          (void *) media,
          (void *) target,
          (void *) event,
          (event->type == LP_EVENT_START) ? "start"
          : (event->type == LP_EVENT_STOP) ? "stop"
          : (event->type == LP_EVENT_USER) ? "user" : "unknown");

  switch (event->type)
  {
    case LP_EVENT_START:
      break;
    case LP_EVENT_STOP:
      if (total_runs >= 1)
      {
        g_main_loop_quit (loop);
      }
      else
      {
        total_runs++;
        total_ticks = 0;
        lp_event_init_start (event);
        assert (lp_media_post (media, event));
      }
      break;
    case LP_EVENT_TICK:
      total_ticks++;
      if (total_ticks >= 4)
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
main (void)
{
  lp_media_t *parent;
  lp_media_t *media;
  lp_event_t start;
  char uri[BUFFSIZE];
  size_t len;

  /* This test assumes the executable dir as current working directory  */
  getcwd (&uri[0], BUFFSIZE);
  len = strlen ("file://");
  strcat (&uri[0], "/media/audiotest.ogg");
  memmove (&uri[0] + len, &uri[0], strlen (&uri[0]) + 2);
  strncpy (&uri[0], "file://", len);

  loop = g_main_loop_new (NULL, FALSE);
  assert (loop != NULL);

  parent = lp_media_create (NULL);
  assert (parent != NULL);

  media = lp_media_create_for_parent (parent, uri);
  assert (media != NULL);
  assert (lp_media_register (media, handler));

  lp_event_init_start (&start);
  assert (lp_media_post (media, &start));

  g_main_loop_run (loop);

  lp_media_destroy (parent);
  lp_media_destroy (media);
  g_main_loop_unref (loop);

  exit (EXIT_SUCCESS);
}
