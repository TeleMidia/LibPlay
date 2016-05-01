/* lp-event.c -- Generic event.
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

/* Event private data.  */
struct _lp_EventPrivate
{
  GObject *source;              /* source object */
};

/* Event properties.  */
enum
{
  PROP_0,
  PROP_SOURCE,
  PROP_LAST
};

/* Property defaults.  */
#define DEFAULT_SOURCE  NULL    /* not initialized */

/* Define the lp_Event type.  */
GX_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (lp_Event, lp_event, G_TYPE_OBJECT)


/* methods */

static void
lp_event_init (lp_Event *event)
{
  event->priv = (lp_EventPrivate *) lp_event_get_instance_private (event);
  event->priv->source = DEFAULT_SOURCE;
}

static void
lp_event_get_property (GObject *object, guint prop_id,
                       GValue *value, GParamSpec *pspec)
{
  lp_Event *event;

  event = LP_EVENT (object);
  switch (prop_id)
    {
      case PROP_SOURCE:
        g_value_take_object (value, event->priv->source);
        break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_set_property (GObject *object, guint prop_id,
                       const GValue *value, GParamSpec *pspec)
{
  lp_Event *event;

  event = LP_EVENT (object);
  switch (prop_id)
    {
    case PROP_SOURCE:
      {                         /* don't take ownership */
        GObject *obj = (GObject *) g_value_dup_object (value);
        g_assert (LP_IS_SCENE (obj) || LP_IS_MEDIA (obj));
        g_assert (event->priv->source == DEFAULT_SOURCE);
        event->priv->source = obj;
        break;
      }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_finalize (GObject *object)
{
  lp_Event *event;

  event = LP_EVENT (object);

  g_assert_nonnull (event->priv->source);
  g_object_unref (event->priv->source);

  _lp_debug ("finalizing event %p", event);
  G_OBJECT_CLASS (lp_event_parent_class)->finalize (object);
}

static void
lp_event_class_init (lp_EventClass *cls)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->get_property = lp_event_get_property;
  gobject_class->set_property = lp_event_set_property;
  gobject_class->finalize = lp_event_finalize;

  g_object_class_install_property
    (gobject_class, PROP_SOURCE, g_param_spec_object
     ("source", "source", "object that posted the event",
      G_TYPE_OBJECT,
      (GParamFlags) (G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));
}
