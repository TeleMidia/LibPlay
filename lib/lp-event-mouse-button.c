/* lp-event-mouse-button.c -- Mouse_Button event.
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

/* MouseButton event.  */
struct _lp_EventMouseButton
{
  lp_Event parent;              /* parent object */
  struct
  {
    double x;                     /* x position */
    double y;                     /* y position */
    int button;                   /* button number */
    gboolean press;               /* TRUE if it is a press event */
  } prop;
};

/* MouseButton event properties.  */
enum
{
  PROP_0,
  PROP_X,
  PROP_Y,
  PROP_BUTTON,
  PROP_PRESS,
  PROP_LAST
};

/* Define the lp_EventMouseButton type.  */
GX_DEFINE_TYPE (lp_EventMouseButton, lp_event_mouse_button, LP_TYPE_EVENT)


/* methods */
static void
lp_event_mouse_button_init (arg_unused (lp_EventMouseButton *evt))
{
}

static void
lp_event_mouse_button_get_property (GObject *object, guint prop_id,
                                    GValue *value, GParamSpec *pspec)
{
  lp_EventMouseButton *event;

  event = LP_EVENT_MOUSE_BUTTON (object);
  switch (prop_id)
    {
      case PROP_X:
        g_value_set_double (value, event->prop.x);
        break;
      case PROP_Y:
        g_value_set_double (value, event->prop.y);
        break;
      case PROP_BUTTON:
        g_value_set_int (value, event->prop.button);
        break;
      case PROP_PRESS:
        g_value_set_boolean (value, event->prop.press);
        break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_mouse_button_set_property (GObject *object, guint prop_id,
                            const GValue *value, GParamSpec *pspec)
{
  lp_EventMouseButton *event;

  event = LP_EVENT_MOUSE_BUTTON (object);
  switch (prop_id)
  {
    case PROP_X:
      event->prop.x = g_value_get_double (value);
      break;
    case PROP_Y:
      event->prop.y = g_value_get_double (value);
      break;
    case PROP_BUTTON:
      event->prop.button = g_value_get_int (value);
      break;
    case PROP_PRESS:
      event->prop.press = g_value_get_boolean (value); 
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
lp_event_mouse_button_constructed (GObject *object)
{
  lp_Event *event;
  GObject *source;

  event = LP_EVENT (object);
  g_object_get (event, "source", &source, NULL);
  g_assert (LP_IS_SCENE (source));
}

static void
lp_event_mouse_button_finalize (GObject *object)
{
  lp_EventMouseButton *event;

  event = LP_EVENT_MOUSE_BUTTON (object);

  _lp_debug ("finalizing mouse event %p", event);
  G_OBJECT_CLASS (lp_event_mouse_button_parent_class)->finalize (object);
}

static void
lp_event_mouse_button_class_init (lp_EventMouseButtonClass *cls)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->get_property = lp_event_mouse_button_get_property;
  gobject_class->set_property = lp_event_mouse_button_set_property;
  gobject_class->constructed = lp_event_mouse_button_constructed;
  gobject_class->finalize = lp_event_mouse_button_finalize;

  g_object_class_install_property
    (gobject_class, PROP_X, g_param_spec_double
     ("x", "x", "x position",
      0., G_MAXDOUBLE, 0.,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_Y, g_param_spec_double
     ("y", "y", "y position",
      0., G_MAXDOUBLE, 0.,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_BUTTON, g_param_spec_int
     ("button", "button", "mouse button",
      0, G_MAXINT, 0,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_PRESS, g_param_spec_boolean
     ("press", "press", "is mouse 'press' button event?",
      TRUE, (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));
}


/* public */

/**
 * lp_event_mouse_button_new:
 * @source: (transfer none): the source #lp_Scene
 * @x: x position
 * @y: y position
 * @press: is press event
 *
 * Creates a new mouse event.
 *
 * Returns: (transfer full): a new #lp_EventMouseButton
 */
lp_EventMouseButton *
lp_event_mouse_button_new (lp_Scene *source, double x, double y,
                           int button, gboolean press)
{
  return LP_EVENT_MOUSE_BUTTON (g_object_new (LP_TYPE_EVENT_MOUSE_BUTTON,
                                       "source", source,
                                       "x", x,
                                       "y", y,
                                       "button", button,
                                       "press", press, NULL));
}
