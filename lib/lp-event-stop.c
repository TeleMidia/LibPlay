/* lp-event-stop.c -- Stop event.
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

/* Stop event.  */
struct _lp_EventStop
{
  lp_Event parent;              /* parent object */
  struct
  {
    gboolean eos;               /* indicates an eos */
  } prop;
};

/* Stop event properties.  */
enum
{
  PROP_0,
  PROP_EOS,
  PROP_LAST
};

/* Property defaults.  */
#define DEFAULT_EOS  FALSE      /* not an eos */

/* Define the lp_EventStop type.  */
GX_DEFINE_TYPE (lp_EventStop, lp_event_stop, LP_TYPE_EVENT)


/* methods */

static void
lp_event_stop_init (lp_EventStop *event)
{
  event->prop.eos = DEFAULT_EOS;
}

static void
lp_event_stop_get_property (GObject *object, guint prop_id,
                            GValue *value, GParamSpec *pspec)
{
  lp_EventStop *event;

  event = LP_EVENT_STOP (object);
  switch (prop_id)
    {
    case PROP_EOS:
      g_value_set_boolean (value, event->prop.eos);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_stop_set_property (GObject *object, guint prop_id,
                            const GValue *value, GParamSpec *pspec)
{
  lp_EventStop *event;

  event = LP_EVENT_STOP (object);
  switch (prop_id)
    {
    case PROP_EOS:
      event->prop.eos = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_stop_constructed (GObject *object)
{
  lp_Event *event;
  GObject *source;
  lp_EventMask mask;

  event = LP_EVENT (object);
  g_object_get (event, "source", &source, "mask", &mask, NULL);
  g_assert (LP_IS_MEDIA (source));
  g_assert (mask == LP_EVENT_MASK_STOP);
}

static void
lp_event_stop_finalize (GObject *object)
{
  G_OBJECT_CLASS (lp_event_stop_parent_class)->finalize (object);
}

static gchar *
lp_event_stop_to_string (lp_Event *event)
{
  return _lp_event_to_string (event, "\
  eos: %s\n\
",                            LP_EVENT_STOP (event)->prop.eos
                              ? "true" : "false");
}

static void
lp_event_stop_class_init (lp_EventStopClass *cls)
{
  GObjectClass *gobject_class;
  lp_EventClass *lp_event_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->get_property = lp_event_stop_get_property;
  gobject_class->set_property = lp_event_stop_set_property;
  gobject_class->constructed = lp_event_stop_constructed;
  gobject_class->finalize = lp_event_stop_finalize;

  lp_event_class = LP_EVENT_CLASS (cls);
  lp_event_class->to_string = lp_event_stop_to_string;

  g_object_class_install_property
    (gobject_class, PROP_EOS, g_param_spec_boolean
     ("eos", "eos", "true if stop was triggered by end-of-stream",
      DEFAULT_EOS,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));
}


/* internal */

/* Creates a new stop event.  */

lp_EventStop *
_lp_event_stop_new (lp_Media *source, gboolean eos)
{
  return LP_EVENT_STOP (g_object_new (LP_TYPE_EVENT_STOP,
                                      "source", source,
                                      "mask", LP_EVENT_MASK_STOP,
                                      "eos", eos, NULL));
}
