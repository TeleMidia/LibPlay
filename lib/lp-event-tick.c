/* lp-event-tick.c -- Tick event.
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

/* Tick event.  */
struct _lp_EventTick
{
  lp_Event parent;              /* parent object */
  struct
  {
    guint64 serial;             /* serial number */
  } prop;
};

/* Tick event properties.  */
enum
{
  PROP_0,
  PROP_SERIAL,
  PROP_LAST
};

/* Property defaults.  */
#define DEFAULT_SERIAL  0       /* first tick */

/* Define the lp_EventTick type.  */
GX_DEFINE_TYPE (lp_EventTick, lp_event_tick, LP_TYPE_EVENT)


/* methods */

static void
lp_event_tick_init (lp_EventTick *event)
{
  event->prop.serial = DEFAULT_SERIAL;
}

static void
lp_event_tick_get_property (GObject *object, guint prop_id,
                            GValue *value, GParamSpec *pspec)
{
  lp_EventTick *event;

  event = LP_EVENT_TICK (object);
  switch (prop_id)
    {
      case PROP_SERIAL:
        g_value_set_uint64 (value, event->prop.serial);
        break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_tick_set_property (GObject *object, guint prop_id,
                            const GValue *value, GParamSpec *pspec)
{
  lp_EventTick *event;

  event = LP_EVENT_TICK (object);
  switch (prop_id)
    {
    case PROP_SERIAL:
      event->prop.serial = g_value_get_uint64 (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_tick_finalize (GObject *object)
{
  lp_EventTick *event;

  event = LP_EVENT_TICK (object);

  _lp_debug ("finalizing tick event %p", event);
  G_OBJECT_CLASS (lp_event_tick_parent_class)->finalize (object);
}

static void
lp_event_tick_class_init (lp_EventTickClass *cls)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->get_property = lp_event_tick_get_property;
  gobject_class->set_property = lp_event_tick_set_property;
  gobject_class->finalize = lp_event_tick_finalize;

  g_object_class_install_property
    (gobject_class, PROP_SERIAL, g_param_spec_uint64
     ("serial", "serial", "serial number",
      0, G_MAXUINT64, DEFAULT_SERIAL,
      (GParamFlags) (G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));
}


/* methods */

/**
 * lp_event_tick_new:
 * @source: (transfer none): the source #lp_Scene
 * @serial: serial number
 *
 * Creates a new tick event.
 *
 * Returns: (transfer full): a new #lp_EventTick
 */
lp_EventTick *
lp_event_tick_new (lp_Scene *source, guint64 serial)
{
  return LP_EVENT_TICK (g_object_new (LP_TYPE_EVENT_TICK,
                                      "source", source,
                                      "serial", serial, NULL));
}

/**
 * lp_event_tick_get_serial:
 * @event: an #lp_EventTick
 *
 * Gets the event serial number.
 *
 * Returns: the event serial number
 */
guint64
lp_event_tick_get_serial (lp_EventTick *event)
{
  guint64 serial;

  g_object_get (event, "serial", &serial, NULL);
  return serial;
}
