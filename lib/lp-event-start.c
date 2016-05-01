/* lp-event-start.c -- Start event.
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

/* Start event.  */
struct _lp_EventStart
{
  lp_Event parent;              /* parent object */
  struct
  {
    gboolean resume;            /* indicates a resume */
  } prop;
};

/* Start event properties.  */
enum
{
  PROP_0,
  PROP_RESUME,
  PROP_LAST
};

/* Property defaults.  */
#define DEFAULT_RESUME  FALSE   /* not a resume */

/* Define the lp_EventStart type.  */
GX_DEFINE_TYPE (lp_EventStart, lp_event_start, LP_TYPE_EVENT)


/* methods */

static void
lp_event_start_init (lp_EventStart *event)
{
  event->prop.resume = DEFAULT_RESUME;
}

static void
lp_event_start_get_property (GObject *object, guint prop_id,
                            GValue *value, GParamSpec *pspec)
{
  lp_EventStart *event;

  event = LP_EVENT_START (object);
  switch (prop_id)
    {
      case PROP_RESUME:
        g_value_set_boolean (value, event->prop.resume);
        break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_start_set_property (GObject *object, guint prop_id,
                            const GValue *value, GParamSpec *pspec)
{
  lp_EventStart *event;

  event = LP_EVENT_START (object);
  switch (prop_id)
    {
    case PROP_RESUME:
      event->prop.resume = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_start_constructed (GObject *object)
{
  lp_Event *event;
  GObject *source;

  event = LP_EVENT (object);
  g_object_get (event, "source", &source, NULL);
  g_assert (LP_IS_MEDIA (source));
}

static void
lp_event_start_finalize (GObject *object)
{
  lp_EventStart *event;

  event = LP_EVENT_START (object);

  _lp_debug ("finalizing start event %p", event);
  G_OBJECT_CLASS (lp_event_start_parent_class)->finalize (object);
}

static void
lp_event_start_class_init (lp_EventStartClass *cls)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->get_property = lp_event_start_get_property;
  gobject_class->set_property = lp_event_start_set_property;
  gobject_class->constructed = lp_event_start_constructed;
  gobject_class->finalize = lp_event_start_finalize;

  g_object_class_install_property
    (gobject_class, PROP_RESUME, g_param_spec_boolean
     ("resume", "resume", "true if start is a resume",
      DEFAULT_RESUME,
      (GParamFlags) (G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));
}


/* public */

/**
 * lp_event_start_new:
 * @source: (transfer none): the source #lp_Media
 * @resume: %TRUE if event signals a resume
 *
 * Creates a new start event.
 *
 * Returns: (transfer full): a new #lp_EventStart
 */
lp_EventStart *
lp_event_start_new (lp_Media *source, gboolean resume)
{
  return LP_EVENT_START (g_object_new (LP_TYPE_EVENT_START,
                                       "source", source,
                                       "resume", resume, NULL));
}

/**
 * lp_event_start_is_resume:
 * @event: an #lp_EventStart
 *
 * Checks if event is resume.
 *
 * Returns: %TRUE if event indicates resume
 */
gboolean
lp_event_start_is_resume (lp_EventStart *event)
{
  gboolean resume;

  g_object_get (event, "resume", &resume, NULL);
  return resume;
}
