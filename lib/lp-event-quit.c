/* lp-event-quit.c -- Quit event.
   Copyright (C) 2015-2017 PUC-Rio/Laboratorio TeleMidia

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
#include "play-internal.h"

/* Quit event.  */
struct _lp_EventQuit
{
  lp_Event parent;              /* parent object */
};

/* Define the lp_EventQuit type.  */
GX_DEFINE_TYPE (lp_EventQuit, lp_event_quit, LP_TYPE_EVENT)


/* methods */

static void
lp_event_quit_init (arg_unused (lp_EventQuit *event))
{
}

static void
lp_event_quit_constructed (GObject *object)
{
  lp_Event *event;
  GObject *source;
  lp_EventMask mask;

  event = LP_EVENT (object);
  g_object_get (event, "source", &source, "mask", &mask, NULL);
  g_assert (LP_IS_SCENE (source));
  g_assert (mask == LP_EVENT_MASK_QUIT);

  G_OBJECT_CLASS (lp_event_quit_parent_class)->constructed (object);
}

static void
lp_event_quit_finalize (GObject *object)
{
  G_OBJECT_CLASS (lp_event_quit_parent_class)->finalize (object);
}

static gchar *
lp_event_quit_to_string (lp_Event *event)
{
  return _lp_event_to_string (event, "");
}

static void
lp_event_quit_class_init (lp_EventQuitClass *cls)
{
  GObjectClass *gobject_class;
  lp_EventClass *lp_event_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->constructed = lp_event_quit_constructed;
  gobject_class->finalize = lp_event_quit_finalize;

  lp_event_class = LP_EVENT_CLASS (cls);
  lp_event_class->to_string = lp_event_quit_to_string;
}


/* internal */

/* Creates a new quit event.  */

lp_EventQuit *
_lp_event_quit_new (lp_Scene *source)
{
  return LP_EVENT_QUIT (g_object_new (LP_TYPE_EVENT_QUIT,
                                      "source", source,
                                      "mask", LP_EVENT_MASK_QUIT, NULL));
}
