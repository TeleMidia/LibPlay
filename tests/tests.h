/* tests.h -- Common declarations for tests.
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

#ifndef TESTS_H
#define TESTS_H

#include <config.h>
#include <assert.h>

#include "macros.h"
#include "gx-macros.h"
#include "gstx-macros.h"

GX_INCLUDE_PROLOGUE
#include "play.h"
#include "play-internal.h"
GX_INCLUDE_EPILOGUE

PRAGMA_DIAG_IGNORE (-Wfloat-equal)

/* Sleeps for @n seconds.  */
#define SLEEP(n) g_usleep ((n) * 1000000)

/* Waits for @n events that matches @mask in @scene.
   Returns the last matched event.  */

static ATTR_UNUSED lp_Event *
await_filtered (lp_Scene *scene, guint n, guint mask)
{
  lp_Event *event;

  if (unlikely (n == 0))
    return NULL;

  for (;;)
    {
      event = lp_scene_receive (scene, TRUE);
      g_assert_nonnull (event);

      if (lp_event_get_mask (event) & mask && --n == 0)
        break;

      g_object_unref (event);
    }
  return event;
}

/* Waits for @n ticks in @scene.  */

static ATTR_UNUSED void
await_ticks (lp_Scene *scene, guint n)
{
  lp_Event *event;

  if (n == 0)
    return;

  event = await_filtered (scene, n, LP_EVENT_MASK_TICK
                          | LP_EVENT_MASK_ERROR);
  g_assert_nonnull (event);
  g_assert (LP_IS_EVENT_TICK (event));
  g_object_unref (event);
}

#endif /* TESTS_H */
