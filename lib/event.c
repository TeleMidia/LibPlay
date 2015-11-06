/* event.c -- Event object.
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
#include "macros.h"

/*-
 * Initializes EVENT as a start event.
 */
void
lp_event_init_start (lp_event_t *event)
{
  event->type = LP_EVENT_START;
}

/*-
 * Initializes EVENT as a stop event.
 */
void
lp_event_init_stop (lp_event_t *event)
{
  event->type = LP_EVENT_STOP;
}

/*-
 * Returns true if events E1 and E2 are equal, i.e., if they have the same
 * type and carry identical data.  Otherwise, returns false.
 */
ATTR_PURE int
lp_event_equals (const lp_event_t *e1, const lp_event_t *e2)
{
  return e1->type == e2->type;
}
