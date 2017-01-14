/* lp-clock.c -- Clock object.
   Copyright (C) 2015-2017 PUC-Rio/Laboratorio TeleMidia

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

/* Clock object.  */
struct _lp_Clock
{
  GstSystemClock parent;        /* parent object */
  GMutex mutex;                 /* sync access to clock object */
  gboolean lockstep;            /* true if clock advances in lock-step */
  GstClock *sysclock;           /* system clock */
  GstClockTime time;            /* clock time */
  GstClockTime unlock_time;     /* clock time when lock-step was disabled */
  GstClockTime unlock_systime;  /* systime when lock-step was disabled */
  GstClockTime init_systime;    /* systime when the clock is instantiated */
};

/* Clock properties. */
enum
{
  PROP_LOCKSTEP = 1,
  PROP_LAST
};

/* Property defaults.  */
#define DEFAULT_LOCKSTEP  FALSE  /* clock advances in real-time */

/* Define the lp_Clock type.  */
GX_DEFINE_TYPE (lp_Clock, lp_clock, GST_TYPE_SYSTEM_CLOCK)


#define clock_lock(clock)         g_mutex_lock (&((clock)->mutex))
#define clock_unlock(clock)       g_mutex_unlock (&((clock)->mutex))
#define clock_get_systime(clock)  gst_clock_get_time ((clock)->sysclock)


/* methods */

static void
lp_clock_init (lp_Clock *clock)
{
  g_mutex_init (&clock->mutex);
  clock->lockstep = DEFAULT_LOCKSTEP;
  clock->sysclock = gst_system_clock_obtain ();
  g_assert_nonnull (clock->sysclock);
  clock->time = 0;
  clock->unlock_time = 0;
  clock->unlock_systime = 0;
  clock->init_systime = clock_get_systime (clock);
}

static void
lp_clock_get_property (GObject *object, guint prop_id,
                       GValue *value, GParamSpec *pspec)
{
  lp_Clock *clock;

  clock = LP_CLOCK (object);
  switch (prop_id)
    {
    case PROP_LOCKSTEP:
      {
        clock_lock (clock);
        g_value_set_boolean (value, g_atomic_int_get (&clock->lockstep));
        clock_unlock (clock);
        break;
      }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_clock_set_property (GObject *object, guint prop_id,
                       const GValue *value, GParamSpec *pspec)
{
  lp_Clock *clock;

  clock = LP_CLOCK (object);
  switch (prop_id)
    {
    case PROP_LOCKSTEP:
      {
        gboolean lockstep;

        lockstep = g_value_get_boolean (value);
        clock_lock (clock);

        if (unlikely (lockstep == clock->lockstep))
          goto done;            /* nothing to do */

        clock->lockstep = lockstep;
        if (!lockstep)
          {
            clock->unlock_time = clock->time;
            clock->unlock_systime = gst_clock_get_time (clock->sysclock);
          }

      done:
        clock_unlock (clock);
        break;
      }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_clock_finalize (GObject *object)
{
  lp_Clock *clock;

  clock = LP_CLOCK (object);
  g_mutex_clear (&clock->mutex);
  g_object_unref (clock->sysclock);
  G_OBJECT_CLASS (lp_clock_parent_class)->finalize (object);
}

static GstClockTime
lp_clock_get_internal_time (GstClock *gst_clock)
{
  lp_Clock *clock;
  GstClockTime result;

  clock = LP_CLOCK (gst_clock);
  clock_lock (clock);

  if (!clock->lockstep)
    {
      GstClockTime now = clock_get_systime (clock);
      clock->time = (now - clock->unlock_systime) + clock->unlock_time -
        clock->init_systime;
    }
  result = clock->time;

  clock_unlock (clock);
  return result;
}

static void
lp_clock_class_init (lp_ClockClass *cls)
{
  GObjectClass *gobject_class;
  GstClockClass *gstclock_class;

  gobject_class = (GObjectClass *) cls;
  gobject_class->get_property = lp_clock_get_property;
  gobject_class->set_property = lp_clock_set_property;
  gobject_class->finalize = lp_clock_finalize;

  gstclock_class = (GstClockClass *) cls;
  gstclock_class->get_internal_time = lp_clock_get_internal_time;

  g_object_class_install_property
    (gobject_class, PROP_LOCKSTEP, g_param_spec_boolean
     ("lockstep", "lock-step mode", "enable lock-step mode",
      DEFAULT_LOCKSTEP, G_PARAM_READWRITE));
}


/* internal */

/* Advances @clock time by @time nanoseconds.
   Does nothing if clock is not operating in lock-step mode.  */

gboolean
_lp_clock_advance (lp_Clock *clock, GstClockTime time)
{
  gboolean status;

  clock_lock (clock);

  if (likely (clock->lockstep))
    {
      clock->time += time;
      status = TRUE;
    }
  else
    {
      status = FALSE;
    }

  clock_unlock (clock);
  return status;
}
