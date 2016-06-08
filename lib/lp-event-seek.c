/* lp-event-seek.c -- Seek event.
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

/* Seek event.  */
struct _lp_EventSeek
{
  lp_Event parent;              /* parent object */
  struct
  {
    gboolean relative;          /* true if seek was relative */
    gint64 offset;              /* offset time */
  } prop;
};

/* Seek event properties.  */
enum
{
  PROP_0,
  PROP_RELATIVE,
  PROP_OFFSET,
  PROP_LAST
};

/* Property defaults.  */
#define DEFAULT_RELATIVE  FALSE /* absolute */
#define DEFAULT_OFFSET    0     /* no offset */

/* Define the lp_EventSeek type.  */
GX_DEFINE_TYPE (lp_EventSeek, lp_event_seek, LP_TYPE_EVENT)


/* methods */

static void
lp_event_seek_init (lp_EventSeek *event)
{
  event->prop.relative = DEFAULT_RELATIVE;
  event->prop.offset = DEFAULT_OFFSET;
}

static void
lp_event_seek_get_property (GObject *object, guint prop_id,
                            GValue *value, GParamSpec *pspec)
{
  lp_EventSeek *event;

  event = LP_EVENT_SEEK (object);
  switch (prop_id)
    {
    case PROP_RELATIVE:
      g_value_set_boolean (value, event->prop.relative);
      break;
    case PROP_OFFSET:
      g_value_set_int64 (value, event->prop.offset);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_seek_set_property (GObject *object, guint prop_id,
                             const GValue *value, GParamSpec *pspec)
{
  lp_EventSeek *event;

  event = LP_EVENT_SEEK (object);
  switch (prop_id)
    {
    case PROP_RELATIVE:
      event->prop.relative = g_value_get_boolean (value);
      break;
    case PROP_OFFSET:
      event->prop.offset = g_value_get_int64 (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_seek_constructed (GObject *object)
{
  lp_Event *event;
  GObject *source;
  lp_EventMask mask;

  event = LP_EVENT (object);
  g_object_get (event, "source", &source, "mask", &mask, NULL);
  g_assert (LP_IS_MEDIA (source));
  g_assert (mask == LP_EVENT_MASK_SEEK);

  G_OBJECT_CLASS (lp_event_seek_parent_class)->constructed (object);
}

static void
lp_event_seek_finalize (GObject *object)
{
  G_OBJECT_CLASS (lp_event_seek_parent_class)->finalize (object);
}

static gchar *
lp_event_seek_to_string (lp_Event *event)
{
  lp_EventSeek *seek;

  seek = LP_EVENT_SEEK (event);
  return _lp_event_to_string (event, "\
  relative: %s\n\
  offset: %" G_GINT64_FORMAT "\n\
",                            strbool (seek->prop.relative),
                              seek->prop.offset);
}

static void
lp_event_seek_class_init (lp_EventSeekClass *cls)
{
  GObjectClass *gobject_class;
  lp_EventClass *lp_event_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->get_property = lp_event_seek_get_property;
  gobject_class->set_property = lp_event_seek_set_property;
  gobject_class->constructed = lp_event_seek_constructed;
  gobject_class->finalize = lp_event_seek_finalize;

  lp_event_class = LP_EVENT_CLASS (cls);
  lp_event_class->to_string = lp_event_seek_to_string;

  g_object_class_install_property
    (gobject_class, PROP_RELATIVE, g_param_spec_boolean
     ("relative", "relative", "true if seek was relative",
      DEFAULT_RELATIVE,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_OFFSET, g_param_spec_int64
     ("offset", "offset", "offset time (in nanoseconds)",
      G_MININT64, G_MAXINT64, DEFAULT_OFFSET,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));
}


/* internal */

/* Creates a new seek event.  */

lp_EventSeek *
_lp_event_seek_new (lp_Media *source, gboolean relative, gint64 offset)
{
  return LP_EVENT_SEEK (g_object_new (LP_TYPE_EVENT_SEEK,
                                      "source", source,
                                      "mask", LP_EVENT_MASK_SEEK,
                                      "relative", relative,
                                      "offset", offset, NULL));
}
