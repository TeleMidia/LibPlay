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
  lp_EventMask mask;            /* event mask */
  GObject *source;              /* source object */
};

/* Event properties.  */
enum
{
  PROP_0,
  PROP_MASK,
  PROP_SOURCE,
  PROP_LAST
};

/* Property defaults.  */
#define DEFAULT_MASK    LP_EVENT_MASK_NONE /* not initialized */
#define DEFAULT_SOURCE  NULL               /* not initialized */

/* Define the lp_Event type.  */
GX_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (lp_Event, lp_event, G_TYPE_OBJECT)


/* internal */

gchar *
_lp_event_to_string (lp_Event *event, const char *fmt, ...)
{
  va_list args;
  gchar *suffix = NULL;
  gchar *str;
  gint n;

  const char *type;
  gpointer addr;

  if (LP_IS_EVENT_TICK (event))
    {
      type = "lp_EventTick";
      addr = LP_EVENT_TICK (event);
    }
  else if (LP_IS_EVENT_KEY (event))
    {
      type = "lp_EventKey";
      addr = LP_EVENT_KEY (event);
    }
  else if (LP_IS_EVENT_POINTER_CLICK (event))
    {
      type = "lp_EventPointerClick";
      addr = LP_EVENT_POINTER_CLICK (event);
    }
  else if (LP_IS_EVENT_POINTER_MOVE (event))
    {
      type = "lp_EventPointerMove";
      addr = LP_EVENT_POINTER_MOVE (event);
    }
  else if (LP_IS_EVENT_ERROR (event))
    {
      type = "lp_EventError";
      addr = LP_EVENT_ERROR (event);
    }
  else if (LP_IS_EVENT_START (event))
    {
      type = "lp_EventStart";
      addr = LP_EVENT_START (event);
    }
  else if (LP_IS_EVENT_STOP (event))
    {
      type = "lp_EventStop";
      addr = LP_EVENT_STOP (event);
    }
  else
    {
      g_assert_not_reached ();
    }

  va_start (args, fmt);
  n = g_vasprintf (&suffix, fmt, args);
  g_assert_nonnull (suffix);
  g_assert (n > 0);
  va_end (args);

  str = g_strdup_printf ("\
%s at %p\n\
  source: %s at %p\n\
  mask: 0x%x\n\
%s\
",                       type,
                         addr,
                         LP_IS_SCENE (event->priv->source) ? "lp_Scene"
                         : LP_IS_MEDIA (event->priv->source) ? "lp_Media"
                         : "unknown",
                         event->priv->source,
                         event->priv->mask,
                         suffix);
  g_assert_nonnull (str);
  g_free (suffix);

  return str;
}


/* methods */

static void
lp_event_init (lp_Event *event)
{
  event->priv = (lp_EventPrivate *) lp_event_get_instance_private (event);
  event->priv->mask = DEFAULT_MASK;
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
      case PROP_MASK:
        g_value_set_int (value, event->priv->mask);
        break;
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
    case PROP_MASK:
      {
        gint mask;
        g_assert (event->priv->mask == DEFAULT_MASK);
        mask =  g_value_get_int (value);
        event->priv->mask = (lp_EventMask) mask;
        switch (event->priv->mask)
          {
          case LP_EVENT_MASK_TICK:
          case LP_EVENT_MASK_KEY:
          case LP_EVENT_MASK_POINTER_CLICK:
          case LP_EVENT_MASK_POINTER_MOVE:
          case LP_EVENT_MASK_ERROR:
          case LP_EVENT_MASK_START:
          case LP_EVENT_MASK_STOP:
            break;
          default:
            g_assert_not_reached ();
          }
        break;
      }
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
lp_event_constructed (GObject *object)
{
  lp_Event *event;
  GObject *source;
  lp_EventMask mask;

  event = LP_EVENT (object);
  g_object_get (event, "source", &source, "mask", &mask, NULL);
  g_assert (G_IS_OBJECT (source));
  g_assert (mask > LP_EVENT_MASK_NONE && mask <= LP_EVENT_MASK_STOP);
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
  gobject_class->constructed = lp_event_constructed;
  gobject_class->finalize = lp_event_finalize;

  g_object_class_install_property
    (gobject_class, PROP_MASK, g_param_spec_int
      ("mask", "mask", "event mask",
       0, G_MAXINT, DEFAULT_MASK,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_SOURCE, g_param_spec_object
     ("source", "source", "object that posted the event",
      G_TYPE_OBJECT,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));
}


/* public */

/**
 * lp_event_to_string:
 * @event: an #lp_Event
 *
 * Gets a string representation of @event.
 *
 * Returns: (transfer full): a string representing the event
 */
gchar *
lp_event_to_string (lp_Event *event)
{
  lp_EventClass *cls;

  cls = LP_EVENT_GET_CLASS (event);
  g_assert (cls->to_string != NULL);

  return cls->to_string (event);
}

/**
 * lp_event_get_source:
 * @event: an #lp_Event
 *
 * Gets the source object of @event.
 *
 * Returns: (transfer none): the event source
 */
GObject *
lp_event_get_source (lp_Event *event)
{
  GObject *source;

  g_object_get (event, "source", &source, NULL);
  g_assert_nonnull (source);

  return source;
}

/**
 * lp_event_get_mask:
 * @evnet: an #lp_Event
 *
 * Gets the simplest mask that matches @event.
 *
 * Returns: the event mask
 */
lp_EventMask
lp_event_get_mask (lp_Event *event)
{
  lp_EventMask mask;

  g_object_get (event, "mask", &mask, NULL);
  g_assert (mask > LP_EVENT_MASK_NONE && mask <= LP_EVENT_MASK_STOP);

  return mask;
}
