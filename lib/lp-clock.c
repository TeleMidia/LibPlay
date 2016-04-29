/* lp-media.c -- Media object.
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
#include "play.h"

/* Clock properties. */
enum
{
  PROP_SYNC = 1,
  PROP_LAST
};

/* Default values for clock properties.  */
#define DEFAULT_SYNC FALSE  /* nonsynchronous */

struct _lp_Clock
{
  GstSystemClock parent;                /* parent object */
  gboolean sync;                        /* sync mode */
  GMutex mutex;                         /* concurrent access */
  GstClockTime curtime;                 /* deterministic time */
  GstClockTime reftime;                 /* reference time */
  GstClock *internal_clock;             /* internal clock */
};

#define lp_clock_lock(c) g_mutex_lock(&c->mutex)
#define lp_clock_unlock(c) g_mutex_unlock(&c->mutex)


/* Define the lp_Clock type.  */
GX_DEFINE_TYPE (lp_Clock, lp_clock, GST_TYPE_SYSTEM_CLOCK)

static void
lp_clock_init (lp_Clock *self)
{
  g_mutex_init (&self->mutex);
  self->sync = DEFAULT_SYNC;
  self->internal_clock = gst_system_clock_obtain ();
  self->curtime = self->reftime = 0;
}

static GstClockTime 
lp_clock_get_internal_time (GstClock *clock)
{
  lp_Clock *myclock = LP_CLOCK (clock);
  GstClockTime curtime;

  lp_clock_lock (myclock);
  if (myclock->sync == FALSE)
  {
    myclock->curtime += 
      (gst_clock_get_time (myclock->internal_clock) - myclock->reftime) 
      - myclock->curtime;
  }

  curtime = myclock->curtime;
  lp_clock_unlock (myclock);
 
  return curtime;
}

static void
lp_clock_finalize (GObject *object)
{
  lp_Clock *self = LP_CLOCK (object);
  g_mutex_clear (&self->mutex);
  g_object_unref (self->internal_clock);

  G_OBJECT_CLASS (lp_clock_parent_class)->finalize(object);
}

static void
lp_clock_set_property (GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec)
{
  lp_Clock *clock = LP_CLOCK (object);

  switch (prop_id)
  {
    case PROP_SYNC:
    {
      gboolean newval = g_value_get_boolean(value);
      if (clock->sync != newval)
      {
        GstClockTime curtime = gst_clock_get_time (clock->internal_clock);

        lp_clock_lock (clock);
        if (newval == TRUE)
          clock->curtime = curtime;
        else
          clock->reftime = curtime;

        clock->sync = newval;
        lp_clock_unlock (clock);
      }
      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
lp_clock_get_property (GObject * object, guint prop_id,
    GValue *value, GParamSpec *pspec)
{
  lp_Clock *clock = LP_CLOCK (object);

  switch (prop_id)
  {
    case PROP_SYNC:
    {
      g_value_set_boolean (value, g_atomic_int_get(&clock->sync));
      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}
static void
lp_clock_class_init (lp_ClockClass *klass)
{
  GObjectClass *gobject_class;
  GstClockClass *clock_class;

  gobject_class = (GObjectClass *) klass;
  clock_class = (GstClockClass *) klass;

  gobject_class->finalize = lp_clock_finalize;
  gobject_class->set_property = lp_clock_set_property;
  gobject_class->get_property = lp_clock_get_property;

  clock_class->get_internal_time = lp_clock_get_internal_time;
  
  g_object_class_install_property (gobject_class, PROP_SYNC,
      g_param_spec_boolean ("sync", "Sync", "use sync clock ", DEFAULT_SYNC, 
        G_PARAM_READWRITE));
}

gboolean
_lp_clock_advance_time (lp_Clock *clock, GstClockTime value)
{
  g_return_val_if_fail (clock->sync, FALSE);

  lp_clock_lock (clock);
  clock->curtime += value;
  lp_clock_unlock (clock);

  return TRUE;
}
