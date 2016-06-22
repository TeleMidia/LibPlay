/* lp-event-pause.c -- Pause event.
   Copyright (C) 2015-2016 PUC-Rio/Laboratorio TeleMidia

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

/* Pause event.  */
struct _lp_EventPause
{
  lp_Event parent;              /* parent object */
};

/* Define the lp_EventPause type.  */
GX_DEFINE_TYPE (lp_EventPause, lp_event_pause, LP_TYPE_EVENT)


/* methods */

static void
lp_event_pause_init (arg_unused (lp_EventPause *event))
{
}

static void
lp_event_pause_constructed (GObject *object)
{
  lp_Event *event;
  lp_EventMask mask;

  event = LP_EVENT (object);
  g_object_get (event, "mask", &mask, NULL);
  g_assert (mask == LP_EVENT_MASK_PAUSE);

  G_OBJECT_CLASS (lp_event_pause_parent_class)->constructed (object);
}

static void
lp_event_pause_finalize (GObject *object)
{
  G_OBJECT_CLASS (lp_event_pause_parent_class)->finalize (object);
}

static gchar *
lp_event_pause_to_string (lp_Event *event)
{
  return _lp_event_to_string (event, "");
}

static void
lp_event_pause_class_init (lp_EventPauseClass *cls)
{
  GObjectClass *gobject_class;
  lp_EventClass *lp_event_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->constructed = lp_event_pause_constructed;
  gobject_class->finalize = lp_event_pause_finalize;

  lp_event_class = LP_EVENT_CLASS (cls);
  lp_event_class->to_string = lp_event_pause_to_string;
}


/* internal */

/* Creates a new pause event.  */

lp_EventPause *
_lp_event_pause_new (GObject *source)
{
  return LP_EVENT_PAUSE (g_object_new (LP_TYPE_EVENT_PAUSE,
                                      "source", source,
                                      "mask", LP_EVENT_MASK_PAUSE, NULL));
}
