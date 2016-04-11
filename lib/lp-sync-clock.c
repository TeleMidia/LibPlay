/* lp-sync-clock.c -- Clock object.
   Copyright (C) 2015 PUC-Rio/Laboratorio TeleMidia

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

#include "play.h"
#include "play-internal.h"

#include <gstsynchronousclock.h>

struct _lp_sync_clock_t
{
  GstClock *internal_clock;    /* synchronous clock */
  int ref_count;               /* reference counter */
  GRecMutex mutex;             /* sync access to media */
};

/* Forward declarations:  */
/* *INDENT-OFF* */
static void __lp_sync_clock_free (lp_sync_clock_t *);
/* *INDENT-ON* */

/*-
 * lp_sync_clock_create:
 *
 * Creates a new instance of lp_sync_clock_t.
 * Return value: new #lp_sync_clock_t with reference count of 1.
 * The caller owns the returned object and should call lp_sync_clock_destroy()
 * when finished with it.
 */
lp_sync_clock_t *
lp_sync_clock_create ()
{
  lp_sync_clock_t *clock;
  clock =  g_new0 (lp_sync_clock_t, 1);
  clock->internal_clock = gst_synchronous_clock_new ();
  g_rec_mutex_init (&clock->mutex);
  clock->ref_count = 1;

  return clock;
}

/*-
 * lp_sync_clock_destroy:
 * @clock: a #lp_sync_clock_t
 *
 * Decreases the reference count of @clock by 1.
 * If the result is 0, then @clock and all associated resources are freed.
 */
void
lp_sync_clock_destroy (lp_sync_clock_t *clock)
{
  if (clock == NULL)
    return;

  if (unlikely (g_atomic_int_get (&clock->ref_count) < 0))
    return;

  if (!g_atomic_int_dec_and_test (&clock->ref_count))
    return;

  _lp_assert (g_atomic_int_get (&clock->ref_count) == 0);
  __lp_sync_clock_free (clock);
}

/*-
 * lp_sync_clock_get_time:
 * @clock: a #lp_sync_clock_t
 *
 * Returns the current clock of @clock em nanoseconds.
 *
 * Return value: current time.
 */
uint64_t
lp_sync_clock_get_time (lp_sync_clock_t *clock)
{
  uint64_t time;
  if (clock == NULL)
    return 0;

  time = gst_clock_get_time (clock->internal_clock); 

  return time;
}

/*-
 * lp_sync_clock_set_tick:
 * @clock: a #lp_sync_clock_t
 * @tick: the new tick value
 *
 * Sets the internal tick of @clock to #tick. This value is used within the
 * function #lp_sync_clock_tick_for ().
 *
 * Return value: #TRUE if the tick was properly set or #FALSE otherwise.
 */

lp_bool_t
lp_sync_clock_set_tick (lp_sync_clock_t *clock, uint64_t tick)
{
  if (clock == NULL || tick < 1)
    return FALSE;
  g_object_set (G_OBJECT (clock->internal_clock), "tick", tick, NULL);

  return TRUE;
}

/*-
 * lp_sync_clock_get_tick:
 * @clock: a #lp_sync_clock_t
 *
 * Gets the internal tick of @clock. .
 *
 * Return value: The current tick of @clock.
 */

uint64_t
lp_sync_clock_get_tick (lp_sync_clock_t *clock)
{
  uint64_t tick;

  g_return_val_if_fail (clock != NULL, 0);
  g_return_val_if_fail (GST_IS_SYNCHRONOUSCLOCK(clock->internal_clock), 0);

  g_object_get (G_OBJECT (clock->internal_clock), "tick", &tick, NULL);

  return tick;
}

/*-
 * lp_sync_clock_advance_time:
 * @clock: a #lp_sync_clock_t
 * @time: amount of time
 *
 * Deterministically advances the current time of @clock.
 *
 * Return value: #TRUE if the operation was successfully performed 
 * or #FALSE otherwise.
 */
lp_bool_t
lp_sync_clock_advance_time (lp_sync_clock_t *clock, uint64_t time)
{
  if (clock == NULL)
    return FALSE;

  return gst_synchronous_clock_advance_time (clock->internal_clock, time);
}

/*-
 * lp_sync_clock_tick_for:
 * @clock: a #lp_sync_clock_t
 * @time: amount of time
 *
 * Deterministically advances the time of @clock by #time. The internal clock
 * is incremented step-by-step using the value of the property 'tick'. The 
 * function blocks by at least #time nanoseconds.
 *
 */
void
lp_sync_clock_tick_for (lp_sync_clock_t *clock, uint64_t time)
{
  _lp_assert (clock != NULL);
  gst_synchronous_clock_tick_for (clock->internal_clock, time, NULL);
}

/*-
 * lp_sync_clock_refence:
 * @clock: a #lp_sync_clock_t
 *
 * Increases the reference count of @clock by 1. This prevents the
 * #lp_sync_clock_t from being destroyed until a matching call to
 * lp_sync_clock_destroy () is made.
 *
 * Return value: the referenced #lp_sync_clockPt
 *
 */
lp_sync_clock_t *
lp_sync_clock_reference (lp_sync_clock_t *clock)
{
  _lp_assert (clock != NULL);
  g_atomic_int_inc (&clock->ref_count);
  return clock;
}

/*-
 * lp_sync_clock_get_refence_count:
 * @clock: a #lp_sync_clock_t
 *
 * Returns the current reference count of @clock
 *
 * Return value: the current reference count of @clock
 *
 */
unsigned int
lp_sync_clock_get_reference_count (const lp_sync_clock_t *clock)
{
  unsigned int ref_count;
  _lp_assert (clock != NULL);
  
  ref_count = g_atomic_int_get (&clock->ref_count);
  _lp_assert (ref_count > 0);

  return ref_count;
}

/*-
 * lp_media_set_sync_clock:
 * @media: a #lp_media_t
 * @clock: a #lp_sync_clock_t
 *
 * Sets the clock of @media to @clock
 *
 * Return value: #TRUE if the @clock was properly set or #FALSE otherwise.
 *
 */
lp_bool_t
lp_media_set_sync_clock (lp_media_t *media, lp_sync_clock_t *clock)
{
  if (media == NULL || clock == NULL)
    return FALSE;

  _lp_media_gst_set_sync_clock (_lp_media_get_backend(media), clock);
  return TRUE;
}

void *
_lp_sync_clock_get_internal_clock (lp_sync_clock_t *clock)
{
  if (clock == NULL)
    return NULL;
  
  return clock->internal_clock;
}

static void
__lp_sync_clock_free (lp_sync_clock_t *clock)
{
  _lp_assert (clock != NULL);
  g_rec_mutex_clear (&clock->mutex);
  g_object_unref (clock->internal_clock);
  g_free (clock);
}


