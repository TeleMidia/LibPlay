/* lp-event-error.c -- Error event.
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

/* Error event.  */
struct _lp_EventError
{
  lp_Event parent;              /* parent object */
  struct
  {
    GError *error;              /* error */
  } prop;
};

/* Error event properties.  */
enum
{
  PROP_0,
  PROP_ERROR,
  PROP_LAST
};

/* Property defaults.  */
#define DEFAULT_ERROR  NULL     /* not initialized */

/* Define the lp_EventError type.  */
GX_DEFINE_TYPE (lp_EventError, lp_event_error, LP_TYPE_EVENT)


/* methods */

static void
lp_event_error_init (lp_EventError *event)
{
  event->prop.error = DEFAULT_ERROR;
}

static void
lp_event_error_get_property (GObject *object, guint prop_id,
                             GValue *value, GParamSpec *pspec)
{
  lp_EventError *event;

  event = LP_EVENT_ERROR (object);
  switch (prop_id)
    {
      case PROP_ERROR:
        g_value_set_boxed (value, event->prop.error);
        break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_error_set_property (GObject *object, guint prop_id,
                             const GValue *value, GParamSpec *pspec)
{
  lp_EventError *event;

  event = LP_EVENT_ERROR (object);
  switch (prop_id)
    {
    case PROP_ERROR:            /* take ownership (transfer full) */
      g_assert (event->prop.error == DEFAULT_ERROR);
      event->prop.error = (GError *) g_value_dup_boxed (value);
      g_assert_nonnull (event->prop.error);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_error_constructed (GObject *object)
{
  lp_Event *event;
  GObject *source;

  event = LP_EVENT (object);
  g_object_get (event, "source", &source, NULL);
  g_assert (LP_IS_MEDIA (source));
}

static void
lp_event_error_finalize (GObject *object)
{
  lp_EventError *event;

  event = LP_EVENT_ERROR (object);

  g_assert_nonnull (event->prop.error);
  g_error_free (event->prop.error);

  _lp_debug ("finalizing error event %p", event);
  G_OBJECT_CLASS (lp_event_error_parent_class)->finalize (object);
}

static void
lp_event_error_class_init (lp_EventErrorClass *cls)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->get_property = lp_event_error_get_property;
  gobject_class->set_property = lp_event_error_set_property;
  gobject_class->constructed = lp_event_error_constructed;
  gobject_class->finalize = lp_event_error_finalize;

  g_object_class_install_property
    (gobject_class, PROP_ERROR, g_param_spec_boxed
     ("error", "error", "error object",
      G_TYPE_ERROR,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));
}


/* public */

/**
 * lp_event_error_new:
 * @source: (transfer none): the source #lp_Media
 * @error: (transfer full): the associated #GError
 *
 * Creates a new error event.
 *
 * Returns: (transfer full): a new #lp_EventError
 */
lp_EventError *
lp_event_error_new (lp_Media *source, GError *error)
{
  return LP_EVENT_ERROR (g_object_new (LP_TYPE_EVENT_ERROR,
                                       "source", source,
                                       "error", error, NULL));
}

/**
 * lp_event_error_get_error:
 * @event: an #lp_EventError
 *
 * Gets the the event #GError.
 *
 * Returns: (transfer none): the event #GError
 */
GError *
lp_event_error_get_error (lp_EventError *event)
{
  GError *error = NULL;

  g_object_get (event, "error", &error, NULL);
  g_assert_nonnull (error);

  return error;
}
