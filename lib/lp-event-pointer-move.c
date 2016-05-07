/* lp-event-pointer-move.c -- Pointer move event.
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

/* Pointer move event.  */
struct _lp_EventPointerMove
{
  lp_Event parent;              /* parent object */
  struct
  {
    gdouble x;                  /* x coordinate */
    gdouble y;                  /* y coordinate */
  } prop;
};

/* Pointer move event properties.  */
enum
{
  PROP_0,
  PROP_X,
  PROP_Y,
  PROP_LAST
};

/* Property defaults.  */
#define DEFAULT_X  0.           /* origin */
#define DEFAULT_Y  0.           /* origin */

/* Define the lp_EventPointerMove type.  */
GX_DEFINE_TYPE (lp_EventPointerMove, lp_event_pointer_move, LP_TYPE_EVENT)


/* methods */

static void
lp_event_pointer_move_init (lp_EventPointerMove *event)
{
  event->prop.x = DEFAULT_X;
  event->prop.y = DEFAULT_Y;
}

static void
lp_event_pointer_move_get_property (GObject *object, guint prop_id,
                                    GValue *value, GParamSpec *pspec)
{
  lp_EventPointerMove *event;

  event = LP_EVENT_POINTER_MOVE (object);
  switch (prop_id)
    {
    case PROP_X:
      g_value_set_double (value, event->prop.x);
      break;
    case PROP_Y:
      g_value_set_double (value, event->prop.y);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_pointer_move_set_property (GObject *object, guint prop_id,
                                    const GValue *value, GParamSpec *pspec)
{
  lp_EventPointerMove *event;

  event = LP_EVENT_POINTER_MOVE (object);
  switch (prop_id)
    {
    case PROP_X:
      event->prop.x = g_value_get_double (value);
      break;
    case PROP_Y:
      event->prop.y = g_value_get_double (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_pointer_move_constructed (GObject *object)
{
  lp_Event *event;
  GObject *source;
  lp_EventMask mask;

  event = LP_EVENT (object);
  g_object_get (event, "source", &source, "mask", &mask, NULL);
  g_assert (LP_IS_SCENE (source));
  g_assert (mask == LP_EVENT_MASK_POINTER_MOVE);
}

static void
lp_event_pointer_move_finalize (GObject *object)
{
  G_OBJECT_CLASS (lp_event_pointer_move_parent_class)->finalize (object);
}

static gchar *
lp_event_pointer_move_to_string (lp_Event *event)
{
  lp_EventPointerMove *move;

  move = LP_EVENT_POINTER_MOVE (event);
  return _lp_event_to_string (event, "\
  x: %g\n\
  y: %g\n\
",                            move->prop.x,
                              move->prop.y);
}

static void
lp_event_pointer_move_class_init (lp_EventPointerMoveClass *cls)
{
  GObjectClass *gobject_class;
  lp_EventClass *lp_event_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->get_property = lp_event_pointer_move_get_property;
  gobject_class->set_property = lp_event_pointer_move_set_property;
  gobject_class->constructed = lp_event_pointer_move_constructed;
  gobject_class->finalize = lp_event_pointer_move_finalize;

  lp_event_class = LP_EVENT_CLASS (cls);
  lp_event_class->to_string = lp_event_pointer_move_to_string;

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
}


/* internal */

/* Creates a new pointer move event.  */

lp_EventPointerMove *
_lp_event_pointer_move_new (lp_Scene *source, gdouble x, gdouble y)
{
  return LP_EVENT_POINTER_MOVE
    (g_object_new (LP_TYPE_EVENT_POINTER_MOVE,
                   "source", source,
                   "mask", LP_EVENT_MASK_POINTER_MOVE,
                   "x", x,
                   "y", y, NULL));
}
