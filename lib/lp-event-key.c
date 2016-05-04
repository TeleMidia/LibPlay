/* lp-event-key.c -- Key event.
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

/* Key event.  */
struct _lp_EventKey
{
  lp_Event parent;              /* parent object */
  struct
  {
    char *key;                  /* key */
    lp_EventKeyType type;       /* press or release */
  } prop;
};

/* Key event properties.  */
enum
{
  PROP_0,
  PROP_KEY,
  PROP_TYPE,
  PROP_LAST
};

/* Define the lp_EventKey type.  */
GX_DEFINE_TYPE (lp_EventKey, lp_event_key, LP_TYPE_EVENT)


/* methods */
static void
lp_event_key_init (arg_unused (lp_EventKey *event))
{
}

static void
lp_event_key_get_property (GObject *object, guint prop_id,
                           GValue *value, GParamSpec *pspec)
{
  lp_EventKey *event;

  event = LP_EVENT_KEY (object);
  switch (prop_id)
    {
      case PROP_KEY:
        g_value_set_string (value, event->prop.key);
        break;
      case PROP_TYPE:
        g_value_set_int (value, event->prop.type);
        break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_key_set_property (GObject *object, guint prop_id,
                            const GValue *value, GParamSpec *pspec)
{
  lp_EventKey *event;

  event = LP_EVENT_KEY (object);
  switch (prop_id)
  {
    case PROP_KEY:
      g_free (event->prop.key);
      event->prop.key = g_strdup(g_value_get_string (value));
      break;
    case PROP_TYPE:
      {
        gint type = g_value_get_int (value);
        event->prop.type = (lp_EventKeyType) type;
        break;
      }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
lp_event_key_constructed (GObject *object)
{
  lp_Event *event;
  GObject *source;

  event = LP_EVENT (object);
  g_object_get (event, "source", &source, NULL);
  g_assert (LP_IS_SCENE (source));
}

static void
lp_event_key_finalize (GObject *object)
{
  lp_EventKey *event;

  event = LP_EVENT_KEY (object);
  g_free (event->prop.key);

  _lp_debug ("finalizing key event %p", event);
  G_OBJECT_CLASS (lp_event_key_parent_class)->finalize (object);
}

static void
lp_event_key_class_init (lp_EventKeyClass *cls)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->get_property = lp_event_key_get_property;
  gobject_class->set_property = lp_event_key_set_property;
  gobject_class->constructed = lp_event_key_constructed;
  gobject_class->finalize = lp_event_key_finalize;

  g_object_class_install_property
    (gobject_class, PROP_KEY, g_param_spec_string
     ("key", "key", "key that triggered the event",
      "",
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_TYPE, g_param_spec_int
     ("type", "type", "key event type",
      0, G_MAXINT, 0,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));
}


/* public */

/**
 * lp_event_key_new:
 * @source: (transfer none): the source #lp_Scene
 * @key: the key
 * @type: key event type
 *
 * Creates a new key event.
 *
 * Returns: (transfer full): a new #lp_EventKey
 */
lp_EventKey *
lp_event_key_new (lp_Scene *source, const char *key,
                  lp_EventKeyType type)
{
  return LP_EVENT_KEY (g_object_new (LP_TYPE_EVENT_KEY,
                                     "source", source,
                                     "key", key,
                                     "type", type, NULL));
}
