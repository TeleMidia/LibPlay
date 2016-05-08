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
    gchar *key;                 /* key name */
    gboolean press;             /* true if key was pressed  */
  } prop;
};

/* Key event properties.  */
enum
{
  PROP_0,
  PROP_KEY,
  PROP_PRESS,
  PROP_LAST
};

/* Property defaults.  */
#define DEFAULT_KEY    NULL     /* not initialized */
#define DEFAULT_PRESS  TRUE     /* press */

/* Define the lp_EventKey type.  */
GX_DEFINE_TYPE (lp_EventKey, lp_event_key, LP_TYPE_EVENT)


/* methods */

static void
lp_event_key_init (lp_EventKey *event)
{
  event->prop.key = DEFAULT_KEY;
  event->prop.press = DEFAULT_PRESS;
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
    case PROP_PRESS:
      g_value_set_boolean (value, event->prop.press);
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
      g_assert (event->prop.key == DEFAULT_KEY);
      event->prop.key = g_value_dup_string (value);
      break;
    case PROP_PRESS:
      event->prop.press = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_key_constructed (GObject *object)
{
  lp_Event *event;
  GObject *source;
  lp_EventMask mask;

  event = LP_EVENT (object);
  g_object_get (event, "source", &source, "mask", &mask, NULL);
  g_assert (LP_IS_SCENE (source));
  g_assert (mask == LP_EVENT_MASK_KEY);

  G_OBJECT_CLASS (lp_event_key_parent_class)->constructed (object);
}

static void
lp_event_key_finalize (GObject *object)
{
  lp_EventKey *event;

  event = LP_EVENT_KEY (object);
  g_assert_nonnull (event->prop.key);
  g_free (event->prop.key);
  G_OBJECT_CLASS (lp_event_key_parent_class)->finalize (object);
}

static gchar *
lp_event_key_to_string (lp_Event *event)
{
  lp_EventKey *key;

  key = LP_EVENT_KEY (event);
  return _lp_event_to_string (event, "\
  key: %s\n\
  press: %s\n\
",                            key->prop.key,
                              strbool (key->prop.press));
}

static void
lp_event_key_class_init (lp_EventKeyClass *cls)
{
  GObjectClass *gobject_class;
  lp_EventClass *lp_event_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->get_property = lp_event_key_get_property;
  gobject_class->set_property = lp_event_key_set_property;
  gobject_class->constructed = lp_event_key_constructed;
  gobject_class->finalize = lp_event_key_finalize;

  lp_event_class = LP_EVENT_CLASS (cls);
  lp_event_class->to_string = lp_event_key_to_string;

  g_object_class_install_property
    (gobject_class, PROP_KEY, g_param_spec_string
     ("key", "key", "key name",
      DEFAULT_KEY,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_PRESS, g_param_spec_boolean
     ("press", "press", "true if key was pressed",
      DEFAULT_PRESS,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));
}


/* internal */

/* Creates a new key event.  */

lp_EventKey *
_lp_event_key_new (lp_Scene *source, const gchar *key,
                   gboolean press)
{
  return LP_EVENT_KEY (g_object_new (LP_TYPE_EVENT_KEY,
                                     "source", source,
                                     "mask", LP_EVENT_MASK_KEY,
                                     "key", key,
                                     "press", press, NULL));
}
