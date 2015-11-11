/* play.h -- Simple multimedia library.
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

#ifndef PLAY_H
#define PLAY_H

#include <playconf.h>

LP_BEGIN_DECLS

/* status */

typedef enum _lp_status_t
{
  LP_STATUS_SUCCESS = 0,
  LP_STATUS_NULL_POINTER,
  LP_STATUS_READ_ERROR,
  LP_STATUS_WRITE_ERROR,
  LP_STATUS_FILE_NOT_FOUND,
  LP_STATUS_NEGATIVE_COUNT,
  LP_STATUS_INVALID_PARENT,
  LP_STATUS_LAST_STATUS
} lp_status_t;

LP_API const char *
lp_status_to_string (lp_status_t);

/* event */

typedef enum _lp_event_type_t
{
  LP_EVENT_START = 0,
  LP_EVENT_STOP
} lp_event_type_t;

typedef struct _lp_event_t
{
  lp_event_type_t type;
} lp_event_t;

LP_API void
lp_event_init_start (lp_event_t *);

LP_API void
lp_event_init_stop (lp_event_t *);

LP_API int
lp_event_equals (const lp_event_t *, const lp_event_t *);

/* media */

typedef struct _lp_media_t lp_media_t;
typedef int (*lp_event_func_t) (lp_media_t *, lp_event_t *);

LP_API lp_media_t *
lp_media_create (const char *uri);

LP_API lp_media_t *
lp_media_create_for_parent (lp_media_t *, const char *);

LP_API void
lp_media_destroy (lp_media_t *);

LP_API lp_status_t
lp_media_status (lp_media_t *);

LP_API lp_media_t *
lp_media_reference (lp_media_t *);

LP_API unsigned int
lp_media_get_reference_count (const lp_media_t *);

LP_END_DECLS

#endif /* PLAY_H */
