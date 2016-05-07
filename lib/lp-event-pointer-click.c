/* lp-event-pointer-click.c -- Pointer click event.
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

/* Pointer click event.  */
struct _lp_EventPointerClick
{
  lp_Event parent;              /* parent object */
  struct
  {
    double x;                   /* x coordinate */
    double y;                   /* y coordinate */
    int button;                 /* button number */
    gboolean press;             /* true if button was pressed */
  } prop;
};

/* Pointer click event properties.  */
enum
{
  PROP_0,
  PROP_X,
  PROP_Y,
  PROP_BUTTON,
  PROP_PRESS,
  PROP_LAST
};

/* Property defaults.  */
#define DEFAULT_X       0.      /* origin */
#define DEFAULT_Y       0.      /* origin */
#define DEFAULT_BUTTON  0       /* left */
#define DEFAULT_PRESS   TRUE    /* press */

/* Define the lp_EventPointerClick type.  */
GX_DEFINE_TYPE (lp_EventPointerClick, lp_event_pointer_click, LP_TYPE_EVENT)


/* methods */

static void
lp_event_pointer_click_init (lp_EventPointerClick *event)
{
  event->prop.x = DEFAULT_X;
  event->prop.y = DEFAULT_Y;
  event->prop.button = DEFAULT_BUTTON;
  event->prop.press = DEFAULT_PRESS;
}

static void
lp_event_pointer_click_get_property (GObject *object, guint prop_id,
                                     GValue *value, GParamSpec *pspec)
{
  lp_EventPointerClick *event;

  event = LP_EVENT_POINTER_CLICK (object);
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
lp_event_pointer_click_set_property (GObject *object, guint prop_id,
                                     const GValue *value, GParamSpec *pspec)
{
  lp_EventPointerClick *event;

  event = LP_EVENT_POINTER_CLICK (object);
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
lp_event_pointer_click_constructed (GObject *object)
{
  lp_Event *event;
  GObject *source;
  lp_EventMask mask;

  event = LP_EVENT (object);
  g_object_get (event, "source", &source, "mask", &mask, NULL);
  g_assert (LP_IS_SCENE (source));
  g_assert (mask == LP_EVENT_MASK_POINTER_CLICK);
}

static void
lp_event_pointer_click_finalize (GObject *object)
{
  G_OBJECT_CLASS (lp_event_pointer_click_parent_class)->finalize (object);
}

static gchar *
lp_event_pointer_click_to_string (lp_Event *event)
{
  return _lp_event_to_string (event, "\
  x: %g\n\
  y: %g\n\
  button: %d\n\
  press: %s\n\
",                            LP_EVENT_POINTER_CLICK (event)->prop.x,
                              LP_EVENT_POINTER_CLICK (event)->prop.y,
                              LP_EVENT_POINTER_CLICK (event)->prop.button,
                              LP_EVENT_POINTER_CLICK (event)->prop.press
                              ? "true" : "false");
}

static void
lp_event_pointer_click_class_init (lp_EventPointerClickClass *cls)
{
  GObjectClass *gobject_class;
  lp_EventClass *lp_event_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->get_property = lp_event_pointer_click_get_property;
  gobject_class->set_property = lp_event_pointer_click_set_property;
  gobject_class->constructed = lp_event_pointer_click_constructed;
  gobject_class->finalize = lp_event_pointer_click_finalize;

  lp_event_class = LP_EVENT_CLASS (cls);
  lp_event_class->to_string = lp_event_pointer_click_to_string;

  g_object_class_install_property
    (gobject_class, PROP_X, g_param_spec_double
     ("x", "x", "x coordinate",
      -G_MAXDOUBLE, G_MAXDOUBLE, DEFAULT_X,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_Y, g_param_spec_double
     ("y", "y", "y coordinate",
      -G_MAXDOUBLE, G_MAXDOUBLE, DEFAULT_Y,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_BUTTON, g_param_spec_int
     ("button", "button", "clicked button",
      0, G_MAXINT, DEFAULT_BUTTON,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_PRESS, g_param_spec_boolean
     ("press", "press", "true if button was pressed",
      DEFAULT_PRESS,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));
}


/* internal */

/* Creates a new pointer click event.  */

lp_EventPointerClick *
_lp_event_pointer_click_new (lp_Scene *source, double x, double y,
                             int button, gboolean press)
{
  return LP_EVENT_POINTER_CLICK
    (g_object_new (LP_TYPE_EVENT_POINTER_CLICK,
                   "source", source,
                   "mask", LP_EVENT_MASK_POINTER_CLICK,
                   "x", x,
                   "y", y,
                   "button", button,
                   "press", press, NULL));
}
